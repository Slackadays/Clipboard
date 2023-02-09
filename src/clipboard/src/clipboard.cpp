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
#include <vector>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <utility>
#include <string_view>
#include <locale>
#include <fstream>
#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <csignal>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <system_error>
#include "clipboard.hpp"
#include <clipboard/fork.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#include <shlobj.h>
#define isatty _isatty
#define fileno _fileno
#include "windows.hpp"
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace fs = std::filesystem;

Forker forker {};

Filepath path;
Copying copying;

bool output_silent = false;
bool no_color = false;

std::string clipboard_name = "0";

Action action;

IOType io_type;

#if defined(_WIN64) || defined (_WIN32)
UINT old_code_page;
#endif

bool stopIndicator(bool change_condition_variable = true) {
    ProgressState expect = ProgressState::Active;
    if (!change_condition_variable) {
        return progress_state.exchange(ProgressState::Cancel) == expect;
    }
    if (!progress_state.compare_exchange_strong(expect, ProgressState::Done)) {
        return false;
    }
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
    buffer << std::ifstream(path).rdbuf();
    return buffer.str();
}

void writeToFile(const fs::path& path, const std::string& content, bool append = false) {
    std::ofstream file(path, append ? std::ios::app : std::ios::trunc);
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

[[nodiscard]] CopyPolicy userDecision(const std::string& item) {
    using enum CopyPolicy;
    if (userIsARobot()) {
        return ReplaceAll;
    }
    fprintf(stderr, item_already_exists_message().data(), item.data());
    std::string decision;
    while (true) {
        std::getline(std::cin, decision);
        fprintf(stderr, "%s", replaceColors("[blank]").data());
        if (decision == "y" || decision == "yes") {
            return ReplaceOnce;
        } else if (decision == "a" || decision == "all") {
            return ReplaceAll;
        } else if (decision == "n" || decision == "no") {
            return SkipOnce;
        } else if (decision == "s" || decision == "skip") {
            return SkipAll;
        } else {
            fprintf(stderr, "%s", bad_response_message().data());
        }
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
            if (action == Action::Cut) {
                writeToFile(path.original_files, fs::absolute(f).string() + "\n", true);
            }
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
        if (copying.items.size() == 1 && !fs::exists(copying.items.at(0))) {
            copying.buffer = copying.items.at(0).string();
            writeToFile(path.data, copying.buffer);
            printf(replaceColors("[green]✓ Copied text \"[bold]%s[blank][green]\"[blank]\n").data(), copying.items.at(0).string().data());
            return;
        }
        for (const auto& f : copying.items) {
            copyItem(f);
        }
    }

    void paste() {
        for (const auto& f : fs::directory_iterator(path.main)) {
            auto pasteItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
                if (fs::exists(fs::current_path() / f.path().filename()) && fs::equivalent(f, fs::current_path() / f.path().filename())) {
                    if (fs::is_directory(f)) {
                        successes.directories++;
                    } else {
                        successes.files++;
                    }
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
        std::string line;
        while (std::getline(std::cin, line)) {
            successes.bytes += line.size() + 1;
            copying.buffer.append(line + "\n");
            if (copying.buffer.size() >= 32000000) {
                writeToFile(path.data, copying.buffer, true);
                copying.buffer.clear();
            }
        }
        writeToFile(path.data, copying.buffer, true);
    }

    void pipeOut() {
        for (const auto& entry : fs::recursive_directory_iterator(path.main)) {
            std::string content(fileContents(entry.path()));
            printf("%s", content.data());
            fflush(stdout);
            successes.bytes += content.size();
        }
    }

    void clear() {
        if (fs::is_empty(path.main)) {
            printf("%s", clear_success_message().data());
        } else {
            printf("%s", clear_fail_message().data());
        }
    }

    void show() {
        stopIndicator();
        if (fs::is_directory(path.main) && !fs::is_empty(path.main)) {
            TerminalSize termSpaceRemaining(getTerminalSize());
            if (fs::is_regular_file(path.data)) {
                std::string content(fileContents(path.data));
                content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
                printf(clipboard_text_contents_message().data(), std::min(static_cast<size_t>(250), content.size()), clipboard_name.data());
                printf(replaceColors("[bold][blue]%s\n[blank]").data(), content.substr(0, 250).data());
                if (content.size() > 250) {
                    printf(and_more_items_message().data(), content.size() - 250);
                }
                return;
            }
            size_t total_items = 0;
            for (auto dummy : fs::directory_iterator(path.main)) {
                total_items++;
            }
            size_t rowsAvailable = termSpaceRemaining.accountRowsFor(clipboard_item_many_contents_message().length());
            rowsAvailable -= 3;
            printf(total_items > rowsAvailable ? clipboard_item_too_many_contents_message().data() : clipboard_item_many_contents_message().data(), std::min(rowsAvailable, total_items), clipboard_name.data());
            auto it = fs::directory_iterator(path.main);
            for (size_t i = 0; i < std::min(rowsAvailable, total_items); i++) {

                printf(replaceColors("[blue]▏ [bold][pink]%s[blank]\n").data(), it->path().filename().string().data());

                if (i == rowsAvailable - 1 && total_items > rowsAvailable) {
                    printf(and_more_items_message().data(), total_items - rowsAvailable);
                }

                it++;

            }
        } else {
            printf(no_clipboard_contents_message().data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
        }
    }

    void edit() {
        
    }

    void add() {
        if (fs::is_regular_file(path.data)) {
            copying.buffer = fileContents(path.data);
            if (!is_tty.in) {
                std::string line;
                while (std::getline(std::cin, line)) {
                    copying.buffer.append(line);
                    successes.bytes += line.size();
                }
                writeToFile(path.data, copying.buffer, true);
            } else if (copying.items.size() == 1 && !fs::exists(copying.items.at(0))) {
                copying.buffer.append(copying.items.at(0).string());
                writeToFile(path.data, copying.buffer, true);
                successes.bytes += copying.items.at(0).string().size();
            } else {
                fprintf(stderr, "%s", replaceColors("[red]╳ You can't add files to text. [blank][pink]Try copying text first, or add a file instead.[blank]\n").data());
            }
        } else if (!fs::is_empty(path.main)) {
            for (const auto& f : copying.items) {
                copyItem(f);
            }
        }
    }

    void remove() {

    }
}

void clearTempDirectory(bool force_clear = false) {
    using enum Action;
    if (force_clear || action == Cut || action == Copy || action == Clear) {
        fs::remove(path.original_files);
        for (const auto& entry : fs::directory_iterator(path.main)) {
            fs::remove_all(entry.path());
        }
    }
}

void convertFromGUIClipboard(const std::string& text) {
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

    if (allOutsideFilepath) {
        clearTempDirectory(true);
    }

    for (auto&& path : clipboard.paths()) {
        if (!fs::exists(path)) {
            continue;
        }

        auto target = ::path.main / path.filename();
        if (fs::exists(target) && fs::equivalent(path, target)) {
            continue;
        }

        try {
            fs::copy(path, target, copying.opts | fs::copy_options::create_hard_links);
        } catch (const fs::filesystem_error& e) {
            try {
                fs::copy(path, target, copying.opts);
            } catch (const fs::filesystem_error& e) {} // Give up
        }
    }

    if (clipboard.action() == ClipboardPathsAction::Cut) {
        std::ofstream originalFiles { path.original_files };
        for (auto&& path : clipboard.paths()) {
            originalFiles << path.string() << std::endl;
        }
    }
}

[[nodiscard]] ClipboardContent thisClipboard() {
    if (fs::exists(path.original_files)) {
        std::ifstream originalFiles { path.original_files };
        std::vector<fs::path> files;

        std::string line;
        while (!originalFiles.eof()) {
            std::getline(originalFiles, line);
            if (!line.empty()) {
                files.emplace_back(line);
            }
        }

        return { std::move(files), ClipboardPathsAction::Cut };
    }

    if (!copying.buffer.empty()) {
        return { copying.buffer };
    }

    if (!copying.items.empty()) {
        std::vector<fs::path> paths;

        for (const auto& entry : fs::directory_iterator(path.main)) { //count all items which were actually successfully actioned on
            paths.push_back(entry.path());
        }

        return ClipboardContent(ClipboardPaths(std::move(paths)));
    }

    return {};
}

void setupHandlers() {
    atexit([]{
    #if defined(_WIN64) || defined (_WIN32)
    SetConsoleOutputCP(old_code_page);
    #endif
    });

    signal(SIGINT, [](int dummy) {
        if (!stopIndicator(false)) {
            // Indicator thread is not currently running. TODO: Write an unbuffered newline, and maybe a cancelation message, directly to standard error. Note: There is no standard C++ interface for this, so this requires an OS call.
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
        if (std::locale("").name().substr(0, 2) == "es") {
            setLanguageES();
        } else if (std::locale("").name().substr(0, 2) == "pt") {
            setLanguagePT();
        } else if (std::locale("").name().substr(0, 2) == "tr") {
            setLanguageTR();
        }
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
        if (clipboard_name.empty()) {
            clipboard_name = constants.default_clipboard_name;
        } else {
            arguments.at(0) = arguments.at(0).substr(0, arguments.at(0).length() - (clipboard_name.length() + copying.is_persistent));
        }
    }
}

void setupVariables(int& argc, char *argv[]) {
    is_tty.in = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdin));
    is_tty.out = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdout));
    is_tty.err = getenv("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stderr));

    #if defined(_WIN64) || defined (_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
        no_color = true;
	}
    old_code_page = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows
    #endif
    path.home = getenv("USERPROFILE") ? getenv("USERPROFILE") : getenv("HOME");

    no_color = getenv("NO_COLOR") && !getenv("FORCE_COLOR");

    output_silent = getenv("CLIPBOARD_SILENT") ? true : false;

    arguments.assign(argv + 1, argv + argc);
}

void syncWithGUIClipboard(bool force = false) {
    if ((!isAWriteAction() && clipboard_name == constants.default_clipboard_name && !getenv("CLIPBOARD_NOGUI")) || force) {
        using enum ClipboardContentType;
        auto content = getGUIClipboard();
        if (content.type() == Text) {
            convertFromGUIClipboard(content.text());
        } else if (content.type() == Paths) {
            convertFromGUIClipboard(content.paths());
        }
    }
}

void showClipboardStatus() {
    syncWithGUIClipboard(true);
    std::vector<std::pair<fs::path, bool>> clipboards_with_contents;
    auto iterateClipboards = [&](const fs::path& path, bool persistent) { //use zip ranges here when gcc 13 comes out
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_directory(entry.path() / "data") && !fs::is_empty(entry.path() / "data")) {
                clipboards_with_contents.push_back({entry.path(), persistent});
            }
        }
    };
    iterateClipboards(path.temporary.parent_path().parent_path(), false);
    iterateClipboards(path.persistent.parent_path().parent_path(), true);
    std::sort(clipboards_with_contents.begin(), clipboards_with_contents.end());
    if (clipboards_with_contents.empty()) {
        printf("%s", no_clipboard_contents_message().data());
    } else {
        TerminalSize termSizeAvailable(getTerminalSize());

        termSizeAvailable.accountRowsFor(clipboard_action_prompt().size(), check_clipboard_status_message().size());
        if (clipboards_with_contents.size() > termSizeAvailable.rows) {
            termSizeAvailable.accountRowsFor(and_more_items_message().size());
        }

        printf("%s", check_clipboard_status_message().data());

        for (size_t clipboard = 0; clipboard < std::min(clipboards_with_contents.size(), termSizeAvailable.rows); clipboard++) {

            int widthRemaining = termSizeAvailable.columns - (clipboards_with_contents.at(clipboard).first.filename().string().length() + 4 + std::string_view(clipboards_with_contents.at(clipboard).second ? " (p)" : "").length());
            printf(replaceColors("[bold][blue]▏ %s%s: [blank]").data(), clipboards_with_contents.at(clipboard).first.filename().string().data(), clipboards_with_contents.at(clipboard).second ? " (p)" : "");

            if (fs::is_regular_file(clipboards_with_contents.at(clipboard).first / "data" / constants.data_file_name)) {
                std::string content(fileContents(clipboards_with_contents.at(clipboard).first / "data" / constants.data_file_name));
                content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
                printf(replaceColors("[pink]%s[blank]\n").data(), content.substr(0, widthRemaining).data());
                continue;
            }

            for (bool first = true; const auto& entry : fs::directory_iterator(clipboards_with_contents.at(clipboard).first / "data")) {
                int entryWidth = entry.path().filename().string().length();

                if (widthRemaining <= 0) {
                    break;
                }

                if (!first) {
                    if (entryWidth <= widthRemaining - 2) {
                        printf("%s", replaceColors("[pink], [blank]").data());
                        widthRemaining -= 2;
                    }
                }

                if (entryWidth <= widthRemaining) {
                    printf(replaceColors("[pink]%s[blank]").data(), entry.path().filename().string().data());
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
    printf("%s", clipboard_action_prompt().data());
}

template<typename T>
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
    if constexpr (std::is_same_v<T, std::string>) {
        return std::string();
    } else {
        return false;
    }
}

Action getAction() {
    using enum Action;
    using enum IOType;
    if (arguments.size() >= 1) {
        for (const auto& entry : {Cut, Copy, Add, Remove}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "-")) {
                if (!is_tty.in) { io_type = Pipe; }
                return entry;
            }
        }
        for (const auto& entry : {Paste, Show, Clear, Edit}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "-")) {
                if (!is_tty.out) { io_type = Pipe; }
                return entry;
            }
        }
        printf(no_valid_action_message().data(), arguments.at(0).data());
        exit(EXIT_FAILURE);
    } else if (!is_tty.in) {
        io_type = Pipe;
        return Copy;
    } else if (!is_tty.out) {
        io_type = Pipe;
        return Paste;
    } else {
        showClipboardStatus();
        exit(EXIT_SUCCESS);
    }
}

void setFlags() {
    if (flagIsPresent<bool>("--fast-copy") || flagIsPresent<bool>("-fc")) {
        copying.use_safe_copy = false;
    }
    if (flagIsPresent<bool>("--ee")) {
        printf("%s", replaceColors("[bold][blue]https://youtu.be/Lg_Pn45gyMs\n[blank]").data());
        exit(EXIT_SUCCESS);
    }
    if (auto flag = flagIsPresent<std::string>("-c"); flag != "") {
        clipboard_name = flag;
    }
    if (auto flag = flagIsPresent<std::string>("--clipboard"); flag != "") {
        clipboard_name = flag;
    }
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
    auto tryThisInstead = [&](const Action& tryThisAction) {
        fprintf(stderr, fix_redirection_action_message().data(), actions[action].data(), actions[action].data(), actions[tryThisAction].data(), actions[tryThisAction].data());
        exit(EXIT_FAILURE);
    };
    if (action == Action::Paste && !is_tty.in) { tryThisInstead(Action::Copy); }
    if (io_type == IOType::Pipe && arguments.size() >= 2) {
        fprintf(stderr, "%s", redirection_no_items_message().data());
        exit(EXIT_FAILURE);
    }
}

void setFilepaths() {
    path.temporary = (getenv("CLIPBOARD_TMPDIR") ? getenv("CLIPBOARD_TMPDIR") : getenv("TMPDIR") ? getenv("TMPDIR") : fs::temp_directory_path()) / constants.temporary_directory_name / clipboard_name / "data";

    path.persistent = (getenv("CLIPBOARD_PERSISTDIR") ? getenv("CLIPBOARD_PERSISTDIR") : (getenv("XDG_CACHE_HOME") ? getenv("XDG_CACHE_HOME") : path.home)) / constants.persistent_directory_name / clipboard_name / "data";
    
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
    if (!is_tty.err) { return; }
    std::unique_lock<std::mutex> lock(m);
    int output_length = 0;
    const std::array<std::string_view, 10> spinner_steps{"━       ", "━━      ", " ━━     ", "  ━━    ", "   ━━   ", "    ━━  ", "     ━━ ", "      ━━", "       ━", "        "};
    static unsigned int percent_done = 0;
    if ((action == Action::Cut || action == Action::Copy) && io_type == IOType::File) {
        static size_t items_size = copying.items.size();
        for (int i = 0; progress_state == ProgressState::Active; i == 9 ? i = 0 : i++) {
            percent_done = ((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size + 1;
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return progress_state != ProgressState::Active; });
        }
    } else if (io_type == IOType::Pipe) {
        for (int i = 0; progress_state == ProgressState::Active; i == 9 ? i = 0 : i++) {
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), static_cast<int>(successes.bytes), "B", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return progress_state != ProgressState::Active; });
        }
    } else if (action == Action::Paste) {
        static size_t items_size = 0;
        if (items_size == 0) {
            for (auto dummy : fs::directory_iterator(path.main)) {
                items_size++;
            }
            if (items_size == 0) {
                items_size = 1;
            }
        }
        for (int i = 0; progress_state == ProgressState::Active; i == 9 ? i = 0 : i++) {
            percent_done = ((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size;
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return progress_state != ProgressState::Active; });
        }
    } else {
        while (progress_state == ProgressState::Active) {
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), 0, "%", "");
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return progress_state != ProgressState::Active; });
        }
    }
    fprintf(stderr, "\r%*s\r", output_length, "");
    fflush(stderr);
    if (progress_state == ProgressState::Cancel) {
        fprintf(stderr, cancelled_message().data(), actions[action].data());
        if (action == Action::Copy || action == Action::Cut) {
            fprintf(stderr, "\n");
        } else {
            fflush(stderr);
        }
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
    for (const auto& i : copying.items) {
        try {
            if (fs::is_directory(i)) {
                for (const auto& entry : fs::recursive_directory_iterator(i)) {
                    total_item_size += entry.is_regular_file() ? entry.file_size() : 16;
                }
            } else {
                total_item_size += fs::is_regular_file(i) ? fs::file_size(i) : 16;
            }
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(i.string(), e.code());
        }
    }
    return total_item_size;
}

void checkItemSize(unsigned long long total_item_size) {
    if (action == Action::Cut || action == Action::Copy) {
        const unsigned long long space_available = fs::space(path.main).available;
        if (total_item_size > (space_available / 2)) {
            stopIndicator();
            fprintf(stderr, not_enough_storage_message().data(), total_item_size / 1024.0, space_available / 1024.0);
            exit(EXIT_FAILURE);
        }
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
        if (copying.failedItems.empty()) {
            fs::remove(path.original_files);
        }
        action = Action::Cut;
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
                //add();
                break;
            case Remove:
                remove();
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
        }
    }
}

void updateGUIClipboard() {
    if (isAWriteAction() && clipboard_name == constants.default_clipboard_name && !getenv("CLIPBOARD_NOGUI")) { //only update GUI clipboard on write operations
        writeToGUIClipboard(thisClipboard());
    }
}

void showFailures() {
    if (copying.failedItems.size() > 0) {
        TerminalSize available(getTerminalSize());
        available.accountRowsFor(clipboard_failed_many_message().length());
        if (copying.failedItems.size() > available.rows) {
            available.accountRowsFor(and_more_fails_message().length());
        }
        available.rows -= 3;
        printf(copying.failedItems.size() > 1 ? clipboard_failed_many_message().data() : clipboard_failed_one_message().data(), actions[action].data());
        for (size_t i = 0; i < std::min(available.rows, copying.failedItems.size()); i++) {
            printf(replaceColors("[red]▏ [bold]%s[blank][red]: %s[blank]\n").data(), copying.failedItems.at(i).first.data(), copying.failedItems.at(i).second.message().data());
            if (i == available.rows - 1 && copying.failedItems.size() > available.rows) {
                printf(and_more_fails_message().data(), int(copying.failedItems.size() - available.rows));
            }
        }
        printf("%s", fix_problem_message().data());
    }
}

void showSuccesses() {
    if (successes.bytes > 0 && is_tty.err) {
        fprintf(stderr, byte_success_message().data(), did_action[action].data(), static_cast<int>(successes.bytes));
        return;
    }
    if ((successes.files == 1 && successes.directories == 0) || (successes.files == 0 && successes.directories == 1)) {
        printf(one_item_success_message().data(), did_action[action].data(), action == Action::Paste ? (*(fs::directory_iterator(path.main))).path().filename().string().data() : copying.items.at(0).string().data());
    } else {
        if ((successes.files > 1) && (successes.directories == 0)) {
            printf(many_files_success_message().data(), did_action[action].data(), static_cast<int>(successes.files));
        } else if ((successes.files == 0) && (successes.directories > 1)) {
            printf(many_directories_success_message().data(), did_action[action].data(), static_cast<int>(successes.directories));
        } else if ((successes.files == 1) && (successes.directories == 1)) {
            printf(one_file_one_directory_success_message().data(), did_action[action].data());
        } else if ((successes.files > 1) && (successes.directories == 1)) {
            printf(many_files_one_directory_success_message().data(), did_action[action].data(), static_cast<int>(successes.files));
        } else if ((successes.files == 1) && (successes.directories > 1)) {
            printf(one_file_many_directories_success_message().data(), did_action[action].data(), static_cast<int>(successes.directories));
        } else if ((successes.files > 1) && (successes.directories > 1)) {
            printf(many_files_many_directories_success_message().data(), did_action[action].data(), static_cast<int>(successes.files), static_cast<int>(successes.directories));
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupHandlers();

        setupVariables(argc, argv);

        setLocale();

        setClipboardName();

        setFlags();

        setFilepaths();

        fs::create_directories(path.temporary), fs::create_directories(path.persistent);

        syncWithGUIClipboard();

        action = getAction();

        verifyAction();

        copying.items.assign(arguments.begin(), arguments.end());

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
        if (stopIndicator()) {
            fprintf(stderr, internal_error_message().data(), e.what());
        }
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}