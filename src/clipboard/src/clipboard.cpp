/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
#include "clipboard.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <clipboard/fork.hpp>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <sstream>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <shlobj.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#include "windows.hpp"
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

Forker forker {};

Filepath path;
Copying copying;

bool output_silent = false;
bool no_color = false;

std::vector<std::string> arguments;

std::string clipboard_name = "0";

Action action;

IOType io_type;

Successes successes;

IsTTY is_tty;

std::array<std::pair<std::string_view, std::string_view>, 7> colors = {
        {{"[error]", "\033[38;5;196m"},    // red
         {"[success]", "\033[38;5;40m"},   // green
         {"[progress]", "\033[38;5;214m"}, // yellow
         {"[info]", "\033[38;5;51m"},      // blue
         {"[help]", "\033[38;5;219m"},     // pink
         {"[bold]", "\033[1m"},
         {"[blank]", "\033[0m"}}};

#if defined(_WIN64) || defined(_WIN32)
UINT old_code_page;
#endif

bool stopIndicator(bool change_condition_variable = true) {
    ProgressState expect = ProgressState::Active;

    if (!change_condition_variable) return progress_state.exchange(ProgressState::Cancel) == expect;

    if (!progress_state.compare_exchange_strong(expect, ProgressState::Done)) return false;

    cv.notify_one();
    indicator.join();
    return true;
}

TerminalSize getTerminalSize() {
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return TerminalSize(csbi.srWindow.Bottom - csbi.srWindow.Top + 1, csbi.srWindow.Right - csbi.srWindow.Left + 1);
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    ioctl(STDERR_FILENO, TIOCGWINSZ, &w);
    return TerminalSize(w.ws_row, w.ws_col);
#endif
    return TerminalSize(80, 24);
}

std::string fileContents(const fs::path& path) {
    std::stringstream buffer;
    buffer << std::ifstream(path, std::ios::binary).rdbuf();
    return buffer.str();
}

std::string pipedInContent() {
    std::string content;
#if !defined(_WIN32) && !defined(_WIN64)
    int len = -1;
    int stdinFd = fileno(stdin);
    constexpr int bufferSize = 65536;
    std::array<char, bufferSize> buffer;
    while (len != 0) {
        len = read(stdinFd, buffer.data(), bufferSize);
        content.append(buffer.data(), len);
        successes.bytes += len;
    }
#elif defined(_WIN32) || defined(_WIN64)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwRead;
    CHAR chBuf[1024];
    BOOL bSuccess = FALSE;

    while (true) {
        bSuccess = ReadFile(hStdin, chBuf, 1024, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;
        content.append(chBuf, dwRead);
        successes.bytes += dwRead;
    }
#endif
    return content;
}

void writeToFile(const fs::path& path, const std::string& content, bool append = false) {
    std::ofstream file(path, append ? std::ios::app : std::ios::trunc | std::ios::binary);
    file << content;
}

void deduplicate(auto& items) {
    std::sort(items.begin(), items.end());
    items.erase(std::unique(items.begin(), items.end()), items.end());
}

bool userIsARobot() {
    return !is_tty.err || !is_tty.in || !is_tty.out || getenv("CI");
}

bool isAWriteAction() {
    using enum Action;
    return action != Paste && action != Show;
}

std::string formatBytes(const auto& bytes) {
    if (bytes < 1024) return std::to_string(bytes) + "B";
    if (bytes < (1024 * 1024)) return std::to_string(bytes / 1024.0) + "kB";
    if (bytes < (1024 * 1024 * 1024)) return std::to_string(bytes / (1024.0 * 1024.0)) + "MB";
    return std::to_string(bytes / (1024.0 * 1024.0 * 1024.0)) + "GB";
}

[[nodiscard]] CopyPolicy userDecision(const std::string& item) {
    using enum CopyPolicy;

    if (userIsARobot()) return ReplaceAll;

    fprintf(stderr, item_already_exists_message().data(), item.data());
    std::string decision;
    while (true) {
        std::getline(std::cin, decision);
        fprintf(stderr, "%s", replaceColors("[blank]").data());

        if (decision == "y" || decision == "yes")
            return ReplaceOnce;
        else if (decision == "a" || decision == "all")
            return ReplaceAll;
        else if (decision == "n" || decision == "no")
            return SkipOnce;
        else if (decision == "s" || decision == "skip")
            return SkipAll;
        else
            fprintf(stderr, "%s", bad_response_message().data());
    }
}

namespace PerformAction {
void copyItem(const fs::path& f) {
    auto actuallyCopyItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
        if (fs::is_directory(f)) {
            auto target = f.filename().empty() ? f.parent_path().filename() : f.filename();
            fs::create_directories(path.main / target);
            fs::copy(f, path.main / target, copying.opts);
            successes.directories++;
        } else {
            fs::copy(f, path.main / f.filename(), use_regular_copy ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
            successes.files++;
        }
        if (action == Action::Cut) writeToFile(path.original_files, fs::absolute(f).string() + "\n", true);
    };
    try {
        actuallyCopyItem();
    } catch (const fs::filesystem_error& e) {
        if (!copying.use_safe_copy && e.code() == std::errc::cross_device_link) {
            try {
                actuallyCopyItem(true);
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(f.string(), e.code());
            }
        } else {
            copying.failedItems.emplace_back(f.string(), e.code());
        }
    }
}

void copy() {
    for (const auto& f : copying.items)
        copyItem(f);
}

void copyText() {
    copying.buffer = copying.items.at(0).string();
    writeToFile(path.data, copying.buffer);

    if (!output_silent) {
        printf(replaceColors("[success]✅ %s text \"[bold]%s[blank][success]\"[blank]\n").data(), did_action[action].data(), copying.buffer.data());
    }

    if (action == Action::Cut) writeToFile(path.original_files, path.data.string());
    successes.bytes = 0; // temporarily disable the bytes success message
}

void paste() {
    for (const auto& f : fs::directory_iterator(path.main)) {
        auto pasteItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
            if (fs::exists(fs::current_path() / f.path().filename()) && fs::equivalent(f, fs::current_path() / f.path().filename())) {
                if (fs::is_directory(f))
                    successes.directories++;
                else
                    successes.files++;
                return;
            }
            if (fs::is_directory(f)) {
                fs::copy(f, fs::current_path() / f.path().filename(), copying.opts);
                successes.directories++;
            } else {
                fs::copy(f, fs::current_path() / f.path().filename(), use_regular_copy ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
                successes.files++;
            }
        };
        try {
            if (fs::exists(fs::current_path() / f.path().filename())) {
                switch (copying.policy) {
                case CopyPolicy::SkipAll:
                    break;
                case CopyPolicy::ReplaceAll:
                    pasteItem();
                    break;
                default:
                    stopIndicator();
                    copying.policy = userDecision(f.path().filename().string());
                    startIndicator();
                    if (copying.policy == CopyPolicy::ReplaceOnce || copying.policy == CopyPolicy::ReplaceAll) {
                        pasteItem();
                    }
                    break;
                }
            } else {
                pasteItem();
            }
        } catch (const fs::filesystem_error& e) {
            if (!copying.use_safe_copy) {
                try {
                    pasteItem(true);
                } catch (const fs::filesystem_error& e) {
                    copying.failedItems.emplace_back(f.path().filename().string(), e.code());
                }
            } else {
                copying.failedItems.emplace_back(f.path().filename().string(), e.code());
            }
        }
    }
    removeOldFiles();
}

void pipeIn() {
    copying.buffer = pipedInContent();
    writeToFile(path.data, copying.buffer);
    if (action == Action::Cut) writeToFile(path.original_files, path.data.string());
}

void pipeOut() {
    for (const auto& entry : fs::recursive_directory_iterator(path.main)) {
        std::string content(fileContents(entry.path()));
#if !defined(_WIN32) && !defined(_WIN64)
        int len = write(fileno(stdout), content.data(), content.size());
        if (len < 0) throw std::runtime_error("write() failed");
#elif defined(_WIN32) || defined(_WIN64)
        fwrite(content.data(), sizeof(char), content.size(), stdout);
#endif
        fflush(stdout);
        successes.bytes += content.size();
    }
    removeOldFiles();
}

void clear() {
    if (fs::is_empty(path.main)) printf("%s", no_clipboard_contents_message().data());
    clearTempDirectory(true);
}

void show() {
    stopIndicator();
    if (fs::is_directory(path.main) && !fs::is_empty(path.main)) {
        TerminalSize termSpaceRemaining(getTerminalSize());
        if (fs::is_regular_file(path.data)) {
            std::string content(fileContents(path.data));
            content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
            printf(clipboard_text_contents_message().data(), std::min(static_cast<size_t>(250), content.size()), clipboard_name.data());
            printf(replaceColors("[bold][info]%s\n[blank]").data(), content.substr(0, 250).data());
            if (content.size() > 250) {
                printf(and_more_items_message().data(), content.size() - 250);
            }
            return;
        }
        size_t total_items = 0;

        for (auto dummy : fs::directory_iterator(path.main))
            total_items++;

        size_t rowsAvailable = termSpaceRemaining.accountRowsFor(clipboard_item_many_contents_message().length());
        rowsAvailable -= 3;
        printf(total_items > rowsAvailable ? clipboard_item_too_many_contents_message().data() : clipboard_item_many_contents_message().data(),
               std::min(rowsAvailable, total_items),
               clipboard_name.data());
        auto it = fs::directory_iterator(path.main);
        for (size_t i = 0; i < std::min(rowsAvailable, total_items); i++) {

            printf(replaceColors("[info]▏ [bold][help]%s[blank]\n").data(), it->path().filename().string().data());

            if (i == rowsAvailable - 1 && total_items > rowsAvailable) printf(and_more_items_message().data(), total_items - rowsAvailable);

            it++;
        }
    } else {
        printf(no_clipboard_contents_message().data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
    }
}

void edit() {}

void addFiles() {
    if (fs::is_regular_file(path.data)) {
        fprintf(stderr,
                "%s",
                replaceColors("[error]❌ You can't add items to text. [blank][help]Try copying text first, or add "
                              "text instead.[blank]\n")
                        .data());
        return;
    }
    for (const auto& f : copying.items)
        copyItem(f);
}

void addData() {
    if (fs::is_regular_file(path.data)) {
        std::string content(pipedInContent());
        successes.bytes += content.size();
        writeToFile(path.data, content, true);
    } else if (!fs::is_empty(path.main)) {
        fprintf(stderr,
                "%s",
                replaceColors("[error]❌ You can't add text to items. [blank][help]Try copying text first, or add a "
                              "file instead.[blank]\n")
                        .data());
    } else {
        pipeIn();
    }
}

void addText() {
    writeToFile(path.data, copying.items.at(0).string(), true);
    successes.bytes += copying.items.at(0).string().size();
}

void remove() {}

void note() {}
} // namespace PerformAction

void clearTempDirectory(bool force_clear = false) {
    using enum Action;
    if (force_clear || action == Cut || action == Copy) {
        fs::remove(path.original_files);
        if (action == Clear && fs::is_regular_file(path.data)) {
            successes.bytes += fs::file_size(path.data);
            fs::remove(path.data);
        }
        for (const auto& entry : fs::directory_iterator(path.main)) {
            fs::remove_all(entry.path());
            if (action == Clear) {
                if (entry.is_directory())
                    successes.directories++;
                else
                    successes.files++;
            }
        }
    }
}

void convertFromGUIClipboard(const std::string& text) {
    if (fs::is_regular_file(path.data) && fileContents(path.data) == text) return;
    clearTempDirectory(true);
    writeToFile(path.data, text);
}

void convertFromGUIClipboard(const ClipboardPaths& clipboard) {
    // Only clear the temp directory if all files in the clipboard are outside the temp directory
    // This avoids the situation where we delete the very files we're trying to copy
    auto allOutsideFilepath = std::all_of(clipboard.paths().begin(), clipboard.paths().end(), [](auto& path) {
        auto relative = fs::relative(path, ::path.main);
        auto firstElement = *(relative.begin());
        return firstElement == fs::path("..");
    });

    if (allOutsideFilepath) clearTempDirectory(true);

    for (auto&& path : clipboard.paths()) {
        if (!fs::exists(path)) continue;

        auto target = ::path.main / path.filename();

        if (fs::exists(target) && fs::equivalent(path, target)) continue;

        try {
            fs::copy(path, target, copying.opts | fs::copy_options::create_hard_links);
        } catch (const fs::filesystem_error& e) {
            try {
                fs::copy(path, target, copying.opts);
            } catch (const fs::filesystem_error& e) {} // Give up
        }
    }

    if (clipboard.action() == ClipboardPathsAction::Cut) {
        std::ofstream originalFiles {path.original_files};
        for (auto&& path : clipboard.paths())
            originalFiles << path.string() << std::endl;
    }
}

[[nodiscard]] ClipboardContent thisClipboard() {
    if (fs::exists(path.original_files) && GUIClipboardSupportsCut) {
        std::ifstream originalFiles {path.original_files};
        std::vector<fs::path> files;

        std::string line;
        while (!originalFiles.eof()) {
            std::getline(originalFiles, line);
            if (!line.empty()) files.emplace_back(line);
        }

        return {std::move(files), ClipboardPathsAction::Cut};
    }

    if (!copying.buffer.empty()) return {copying.buffer};

    if (!copying.items.empty()) {
        std::vector<fs::path> paths;

        for (const auto& entry : fs::directory_iterator(path.main)) // count all items which were actually successfully actioned on
            paths.push_back(entry.path());

        return ClipboardContent(ClipboardPaths(std::move(paths)));
    }

    return {};
}

void setupHandlers() {
    atexit([] {
#if defined(_WIN64) || defined(_WIN32)
        SetConsoleOutputCP(old_code_page);
#endif
    });

    signal(SIGINT, [](int dummy) {
        if (!stopIndicator(false)) {
            // Indicator thread is not currently running. TODO: Write an unbuffered newline, and maybe a cancelation
            // message, directly to standard error. Note: There is no standard C++ interface for this, so this requires
            // an OS call.
            _exit(EXIT_FAILURE);
        } else {
            indicator.join();
            exit(EXIT_FAILURE);
        }
    });

    forker.atFork([]() {
        // As the indicator thread still exists in memory in the forked process,
        // the main process exiting creates an exception because it has not been joined in the X11 process.
        // So we need to remove it from our forked memory
        indicator.detach();
    });

    forker.atNonFork([]() {
        // If the process didn't fork, we need to stop the indicator thread to ensure it won't
        // keep running in the background while we perform the required work
        stopIndicator();
    });
}

void setLocale() {
    try {
        if (std::locale("").name().substr(0, 2) == "es")
            setLanguageES();
        else if (std::locale("").name().substr(0, 2) == "pt")
            setLanguagePT();
        else if (std::locale("").name().substr(0, 2) == "tr")
            setLanguageTR();
    } catch (...) {}
}

void setClipboardName() {
    if (arguments.size() >= 1) {
        clipboard_name = arguments.at(0);
        if (clipboard_name.find_first_of("_") != std::string::npos) {
            clipboard_name = clipboard_name.substr(clipboard_name.find_first_of("_") + 1);
            copying.is_persistent = true;
        } else {
            clipboard_name = clipboard_name.substr(clipboard_name.find_last_not_of("0123456789") + 1);
        }
        if (clipboard_name.empty())
            clipboard_name = constants.default_clipboard_name;
        else
            arguments.at(0) = arguments.at(0).substr(0, arguments.at(0).length() - (clipboard_name.length() + copying.is_persistent));
    }
}

void setupVariables(int& argc, char* argv[]) {
    is_tty.in = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdin));
    is_tty.out = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdout));
    is_tty.err = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stderr));

#if defined(_WIN64) || defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // Windows terminal color compatibility
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
        no_color = true;
    }
    old_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8); // fix broken accents on Windows
#endif
    path.home = getenv("USERPROFILE") ? getenv("USERPROFILE") : getenv("HOME");

    no_color = getenv("NO_COLOR") && !getenv("FORCE_COLOR");

    output_silent = getenv("CLIPBOARD_SILENT") ? true : false;

    // if (auto setting = getenv("CLIPBOARD_THEME"); setting != nullptr) setTheme(std::string(setting));

    arguments.assign(argv + 1, argv + argc);
}

void syncWithGUIClipboard(bool force = false) {
    if ((!isAWriteAction() && clipboard_name == constants.default_clipboard_name && !getenv("CLIPBOARD_NOGUI")) || force) {
        using enum ClipboardContentType;
        auto content = getGUIClipboard();
        if (content.type() == Text)
            convertFromGUIClipboard(content.text());
        else if (content.type() == Paths)
            convertFromGUIClipboard(content.paths());
    }
}

void showClipboardStatus() {
    syncWithGUIClipboard(true);
    std::vector<std::pair<fs::path, bool>> clipboards_with_contents;
    auto iterateClipboards = [&](const fs::path& path, bool persistent) { // use zip ranges here when gcc 13 comes out
        for (const auto& entry : fs::directory_iterator(path))
            if (fs::is_directory(entry.path() / "data") && !fs::is_empty(entry.path() / "data")) clipboards_with_contents.push_back({entry.path(), persistent});
    };
    iterateClipboards(path.temporary.parent_path().parent_path(), false);
    iterateClipboards(path.persistent.parent_path().parent_path(), true);
    std::sort(clipboards_with_contents.begin(), clipboards_with_contents.end());
    if (clipboards_with_contents.empty()) {
        printf("%s", no_clipboard_contents_message().data());
        printf("%s", clipboard_action_prompt().data());
    } else {
        TerminalSize termSizeAvailable(getTerminalSize());

        termSizeAvailable.accountRowsFor(check_clipboard_status_message().size());
        if (clipboards_with_contents.size() > termSizeAvailable.rows) {
            termSizeAvailable.accountRowsFor(and_more_items_message().size());
        }

        printf("%s", check_clipboard_status_message().data());

        for (size_t clipboard = 0; clipboard < std::min(clipboards_with_contents.size(), termSizeAvailable.rows); clipboard++) {

            int widthRemaining = termSizeAvailable.columns
                                 - (clipboards_with_contents.at(clipboard).first.filename().string().length() + 4
                                    + std::string_view(clipboards_with_contents.at(clipboard).second ? " (p)" : "").length());
            printf(replaceColors("[bold][info]▏ %s%s: [blank]").data(),
                   clipboards_with_contents.at(clipboard).first.filename().string().data(),
                   clipboards_with_contents.at(clipboard).second ? " (p)" : "");

            if (fs::is_regular_file(clipboards_with_contents.at(clipboard).first / "data" / constants.data_file_name)) {
                std::string content(fileContents(clipboards_with_contents.at(clipboard).first / "data" / constants.data_file_name));
                content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
                printf(replaceColors("[help]%s[blank]\n").data(), content.substr(0, widthRemaining).data());
                continue;
            }

            for (bool first = true; const auto& entry : fs::directory_iterator(clipboards_with_contents.at(clipboard).first / "data")) {
                int entryWidth = entry.path().filename().string().length();

                if (widthRemaining <= 0) break;

                if (!first) {
                    if (entryWidth <= widthRemaining - 2) {
                        printf("%s", replaceColors("[help], [blank]").data());
                        widthRemaining -= 2;
                    }
                }

                if (entryWidth <= widthRemaining) {
                    printf(replaceColors("[help]%s[blank]").data(), entry.path().filename().string().data());
                    widthRemaining -= entryWidth;
                    first = false;
                }
            }
            printf("\n");
        }
        if (clipboards_with_contents.size() > termSizeAvailable.rows) {
            printf(and_more_items_message().data(), clipboards_with_contents.size() - termSizeAvailable.rows);
        }
    }
}

template <typename T>
[[nodiscard]] auto flagIsPresent(const std::string_view& flag, const std::string_view& shortcut = "") {
    for (const auto& entry : arguments) {
        if (entry == flag || entry == (std::string(shortcut).append(flag))) {
            if constexpr (std::is_same_v<T, std::string>) {
                std::string temp(*arguments.erase(std::find(arguments.begin(), arguments.end(), entry)));
                arguments.erase(std::find(arguments.begin(), arguments.end(), temp));
                return temp;
            } else {
                arguments.erase(std::find(arguments.begin(), arguments.end(), entry));
                return true;
            }
        }
    }

    if constexpr (std::is_same_v<T, std::string>)
        return std::string();
    else
        return false;
}

Action getAction() {
    using enum Action;
    if (arguments.size() >= 1) {
        for (const auto& entry : {Cut, Copy, Add, Remove}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "-")) {
                return entry;
            }
        }
        for (const auto& entry : {Paste, Show, Clear, Edit}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "-")) {
                return entry;
            }
        }
        printf(no_valid_action_message().data(), arguments.at(0).data());
        exit(EXIT_FAILURE);
    } else if (!is_tty.in) {
        return Copy;
    } else if (!is_tty.out) {
        return Paste;
    } else {
        showClipboardStatus();
        exit(EXIT_SUCCESS);
    }
}

IOType getIOType() {
    using enum Action;
    using enum IOType;
    if (action == Cut || action == Copy || action == Add || action == Remove) {
        if (copying.items.size() == 1 && !fs::exists(copying.items.at(0))) return Text;
        if (!is_tty.in) return Pipe;
    } else if (action == Paste || action == Show || action == Clear || action == Edit) {
        if (!is_tty.out) return Pipe;
    }
    return File;
}

void setFlags() {
    if (flagIsPresent<bool>("--fast-copy") || flagIsPresent<bool>("-fc")) copying.use_safe_copy = false;
    if (flagIsPresent<bool>("--ee")) {
        printf("%s", replaceColors("[bold][info]https://youtu.be/Lg_Pn45gyMs\n[blank]").data());
        exit(EXIT_SUCCESS);
    }
    if (auto flag = flagIsPresent<std::string>("-c"); flag != "") clipboard_name = flag;
    if (auto flag = flagIsPresent<std::string>("--clipboard"); flag != "") clipboard_name = flag;
    if (flagIsPresent<bool>("-h") || flagIsPresent<bool>("help", "--")) {
        printf(help_message().data(), constants.clipboard_version.data(), constants.clipboard_commit.data());
        exit(EXIT_SUCCESS);
    }
    for (const auto& entry : arguments) {
        if (entry == "--") {
            arguments.erase(std::find(arguments.begin(), arguments.end(), entry));
            break;
        }
    }
}

void verifyAction() {
    if (io_type == IOType::Pipe && arguments.size() >= 2) {
        fprintf(stderr, "%s", redirection_no_items_message().data());
        exit(EXIT_FAILURE);
    }
}

void setFilepaths() {
    path.temporary = (getenv("CLIPBOARD_TMPDIR") ? getenv("CLIPBOARD_TMPDIR")
                      : getenv("TMPDIR")         ? getenv("TMPDIR")
                                                 : fs::temp_directory_path())
                     / constants.temporary_directory_name / clipboard_name / "data";

    path.persistent = (getenv("CLIPBOARD_PERSISTDIR") ? getenv("CLIPBOARD_PERSISTDIR") : (getenv("XDG_CACHE_HOME") ? getenv("XDG_CACHE_HOME") : path.home))
                      / constants.persistent_directory_name / clipboard_name / "data";

    path.main = (copying.is_persistent || getenv("CLIPBOARD_ALWAYS_PERSIST")) ? path.persistent : path.temporary;

    path.original_files = path.main.parent_path() / constants.original_files_name;

    path.data = path.main / constants.data_file_name;
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && io_type == IOType::File && copying.items.size() < 1) {
        printf(choose_action_items_message().data(), actions[action].data(), actions[action].data(), actions[action].data());
        exit(EXIT_FAILURE);
    }
    if (action == Action::Paste && fs::is_empty(path.main)) {
        showClipboardStatus();
        exit(EXIT_SUCCESS);
    }
}

void setupIndicator() {
    if (!is_tty.err || output_silent) return;

    fprintf(stderr, "\033]0;%s - Clipboard\007", doing_action[action].data()); // set the terminal title
    fprintf(stderr, "\033[?25l");                                              // hide the cursor
    fflush(stderr);

    std::unique_lock<std::mutex> lock(m);
    int output_length = 0;
    const std::array<std::string_view, 22> spinner_steps {"╸         ", "━         ", "╺╸        ", " ━        ", " ╺╸       ", "  ━       ", "  ╺╸      ", "   ━      ",
                                                          "   ╺╸     ", "    ━     ", "    ╺╸    ", "     ━    ", "     ╺╸   ", "      ━   ", "      ╺╸  ", "       ━  ",
                                                          "       ╺╸ ", "        ━ ", "        ╺╸", "         ━", "         ╺", "          "};
    auto itemsToProcess = [&] {
        size_t items = 1;
        for (auto dummy : fs::directory_iterator(path.main))
            items++;
        return items;
    };
    static size_t items_size = (action == Action::Cut || action == Action::Copy) ? copying.items.size() : itemsToProcess();
    if (items_size == 0) items_size++;
    auto percent_done = [&] {
        return std::to_string(((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size) + "%";
    };
    for (int i = 0; progress_state == ProgressState::Active; i == 21 ? i = 0 : i++) {
        auto display_progress = [&](const auto& formattedNum) {
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), formattedNum, spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(25), [&] { return progress_state != ProgressState::Active; });
        };

        if (io_type == IOType::File)
            display_progress(percent_done().data());
        else if (io_type == IOType::Pipe)
            display_progress(formatBytes(successes.bytes.load(std::memory_order_relaxed)).data());
    }
    fprintf(stderr, "\r%*s\r", output_length, "");
    fprintf(stderr, "\033[?25h"); // restore the cursor
    fflush(stderr);
    if (progress_state == ProgressState::Cancel) {
        fprintf(stderr, cancelled_message().data(), actions[action].data());
        fflush(stderr);
        _exit(EXIT_FAILURE);
    }
    fflush(stderr);
}

void startIndicator() { // If cancelled, leave cancelled
    ProgressState expect = ProgressState::Done;
    progress_state.compare_exchange_strong(expect, ProgressState::Active);
    indicator = std::thread(setupIndicator);
}

unsigned long long totalItemSize() {
    unsigned long long total_item_size = 0;
    if ((action == Action::Cut || action == Action::Copy || action == Action::Add) && io_type == IOType::File) {
        for (const auto& i : copying.items) {
            try {
                if (fs::is_directory(i))
                    for (const auto& entry : fs::recursive_directory_iterator(i))
                        total_item_size += entry.is_regular_file() ? entry.file_size() : 16;
                else
                    total_item_size += fs::is_regular_file(i) ? fs::file_size(i) : 16;
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(i.string(), e.code());
            }
        }
    } else if (action == Action::Paste && io_type == IOType::File) {
        for (const auto& entry : fs::recursive_directory_iterator(path.main))
            total_item_size += entry.is_regular_file() ? entry.file_size() : 16;
    }
    return total_item_size;
}

void checkItemSize(unsigned long long total_item_size) {
    unsigned long long space_available = 0;
    if ((action == Action::Cut || action == Action::Copy || action == Action::Add) && io_type == IOType::File)
        space_available = fs::space(path.main).available;
    else if (action == Action::Paste && io_type == IOType::File)
        space_available = fs::space(fs::current_path()).available;
    if (total_item_size > space_available) {
        stopIndicator();
        fprintf(stderr, not_enough_storage_message().data(), actions[action].data(), total_item_size / (1024.0 * 1024.0), space_available / (1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }
}

void removeOldFiles() {
    if (fs::is_regular_file(path.original_files)) {
        std::ifstream files(path.original_files);
        std::string line;
        while (std::getline(files, line)) {
            try {
                fs::remove_all(line);
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(line, e.code());
            }
        }
        files.close();
        if (copying.failedItems.empty()) fs::remove(path.original_files);
    }
}

void performAction() {
    using enum IOType;
    using enum Action;
    using namespace PerformAction;
    if (io_type == File) {
        switch (action) {
        case Copy:
        case Cut:
            copy();
            break;
        case Paste:
            paste();
            break;
        case Clear:
            clear();
            break;
        case Show:
            show();
            break;
        case Edit:
            edit();
            break;
        case Add:
            addFiles();
            break;
        case Remove:
            remove();
            break;
        default:
            break;
        }
    } else if (io_type == Pipe) {
        switch (action) {
        case Copy:
        case Cut:
            pipeIn();
            break;
        case Paste:
            pipeOut();
            break;
        case Add:
            addData();
            break;
        case Remove:
            break;
        default:
            break;
        }
    } else if (io_type == Text) {
        switch (action) {
        case Copy:
        case Cut:
            copyText();
            break;
        case Add:
            addText();
            break;
        case Remove:
            break;
        default:
            break;
        }
    }
}

void updateGUIClipboard() {
    if (isAWriteAction() && clipboard_name == constants.default_clipboard_name && !getenv("CLIPBOARD_NOGUI")) { // only update GUI clipboard on write operations
        writeToGUIClipboard(thisClipboard());
    }
}

void showFailures() {
    if (copying.failedItems.size() > 0) {
        TerminalSize available(getTerminalSize());
        available.accountRowsFor(clipboard_failed_many_message().length());

        if (copying.failedItems.size() > available.rows) available.accountRowsFor(and_more_fails_message().length());

        available.rows -= 3;
        printf(copying.failedItems.size() > 1 ? clipboard_failed_many_message().data() : clipboard_failed_one_message().data(), actions[action].data());
        for (size_t i = 0; i < std::min(available.rows, copying.failedItems.size()); i++) {

            printf(replaceColors("[error]▏ [bold]%s[blank][error]: %s[blank]\n").data(), copying.failedItems.at(i).first.data(), copying.failedItems.at(i).second.message().data());

            if (i == available.rows - 1 && copying.failedItems.size() > available.rows) printf(and_more_fails_message().data(), int(copying.failedItems.size() - available.rows));
        }
        printf("%s", fix_problem_message().data());
    }
}

void showSuccesses() {
    if (output_silent) return;
    if (successes.bytes > 0 && is_tty.err) {
        fprintf(stderr, byte_success_message().data(), did_action[action].data(), formatBytes(successes.bytes.load()).data());
    } else if ((successes.files == 1 && successes.directories == 0) || (successes.files == 0 && successes.directories == 1)) {
        printf(one_item_success_message().data(), did_action[action].data());
    } else {
        if ((successes.files > 1) && (successes.directories == 0))
            printf(many_files_success_message().data(), did_action[action].data(), successes.files.load());
        else if ((successes.files == 0) && (successes.directories > 1))
            printf(many_directories_success_message().data(), did_action[action].data(), successes.directories.load());
        else if ((successes.files == 1) && (successes.directories == 1))
            printf(one_file_one_directory_success_message().data(), did_action[action].data());
        else if ((successes.files > 1) && (successes.directories == 1))
            printf(many_files_one_directory_success_message().data(), did_action[action].data(), successes.files.load());
        else if ((successes.files == 1) && (successes.directories > 1))
            printf(one_file_many_directories_success_message().data(), did_action[action].data(), successes.directories.load());
        else if ((successes.files > 1) && (successes.directories > 1))
            printf(many_files_many_directories_success_message().data(), did_action[action].data(), successes.files.load(), successes.directories.load());
    }
}

int main(int argc, char* argv[]) {
    try {
        setupHandlers();

        setupVariables(argc, argv);

        setLocale();

        setClipboardName();

        setFlags();

        setFilepaths();

        fs::create_directories(path.temporary), fs::create_directories(path.persistent);

        action = getAction();

        syncWithGUIClipboard();

        copying.items.assign(arguments.begin(), arguments.end());

        io_type = getIOType();

        verifyAction();

        checkForNoItems();

        startIndicator();

        deduplicate(copying.items);

        checkItemSize(totalItemSize());

        clearTempDirectory();

        performAction();

        updateGUIClipboard();

        stopIndicator();

        deduplicate(copying.failedItems);

        showFailures();

        showSuccesses();
    } catch (const std::exception& e) {
        if (stopIndicator()) fprintf(stderr, internal_error_message().data(), e.what());
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
