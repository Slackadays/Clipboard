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
#include "clipboard.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
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

static Action action;

bool stopIndicator(bool change_condition_variable = true) {
    SpinnerState expect = SpinnerState::Active;
    if (!change_condition_variable) {
        return spinner_state.exchange(SpinnerState::Cancel) == expect;
    }
    if (!spinner_state.compare_exchange_strong(expect, SpinnerState::Done)) {
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

namespace PerformAction {
    void copy() {
        if (copying.items.size() == 1 && !fs::exists(copying.items.at(0))) {
            std::ofstream file(filepath.main / constants.pipe_file);
            file << copying.items.at(0).string();
            copying.buffer = copying.items.at(0).string();
            printf(replaceColors("{green}✓ Copied text \"{bold}%s{blank}{green}\"{blank}\n").data(), copying.items.at(0).string().data());
            return;
        }
        std::ofstream originalFiles;
        if (action == Action::Cut) {
            originalFiles.open(filepath.original_files);
        }
        for (const auto& f : copying.items) {
            auto copyItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
                if (fs::is_directory(f)) {
                    if (f.filename() == "") {
                        fs::create_directories(filepath.main / f.parent_path().filename());
                        fs::copy(f, filepath.main / f.parent_path().filename(), copying.opts);
                    } else {
                        fs::create_directories(filepath.main / f.filename());
                        fs::copy(f, filepath.main / f.filename(), copying.opts);
                    }
                    successes.directories++;
                } else {
                    fs::copy(f, filepath.main / f.filename(), use_regular_copy ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
                    successes.files++;
                }
                if (action == Action::Cut) {
                    originalFiles << fs::absolute(f).string() << std::endl;
                }
            };
            try {
                copyItem();
            } catch (const fs::filesystem_error& e) {
                if (!copying.use_safe_copy) {
                    try {
                        copyItem(true);
                    } catch (const fs::filesystem_error& e) {
                        copying.failedItems.emplace_back(f.string(), e.code());
                    }
                } else {
                    copying.failedItems.emplace_back(f.string(), e.code());
                }
            }
        }
    }

    void paste() {
        int user_decision = 0;
        for (const auto& f : fs::directory_iterator(filepath.main)) {
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
                    switch (user_decision) {
                        case -2:
                            break;
                        case -1:
                        case 0:
                        case 1:
                            stopIndicator();
                            user_decision = getUserDecision(f.path().filename().string());
                            startIndicator();
                            break;
                        case 2:
                            pasteItem();
                            break;
                    }
                    switch (user_decision) {
                        case -1:
                            break;
                        case 1:
                            pasteItem();
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
        std::ofstream file(filepath.main / constants.pipe_file);
        std::string line;
        while (std::getline(std::cin, line)) {
            successes.bytes += line.size() + 1;
            copying.buffer.append(line + "\n");
            if (copying.buffer.size() >= 32000000) {
                file << copying.buffer;
                copying.buffer.clear();
            }
        }
        file << copying.buffer;
    }

    void pipeOut() {
        std::string line;
        for (const auto& entry : fs::recursive_directory_iterator(filepath.main)) {
            std::ifstream file(entry.path());
            while (std::getline(file, line)) {
                printf("%s\n", line.data());
                successes.bytes += line.size() + 1;
            }
            file.close();
        }
    }

    void clear() {
        if (fs::is_empty(filepath.main)) {
            printf("%s", clear_success_message().data());
        } else {
            printf("%s", clear_fail_message().data());
        }
    }

    void show() {
        stopIndicator();
        if (fs::is_directory(filepath.main) && !fs::is_empty(filepath.main)) {
            TerminalSize termSpaceRemaining(getTerminalSize());
            if (fs::is_regular_file(filepath.main / constants.pipe_file)) {
                std::string content;
                std::ifstream input(filepath.main / constants.pipe_file);
                std::stringstream buffer;
                buffer << input.rdbuf();
                content = buffer.str();
                content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
                printf(clipboard_text_contents_message().data(), std::min(static_cast<size_t>(250), content.size()), clipboard_name.data());
                printf(replaceColors("{bold}{blue}%s\n{blank}").data(), content.substr(0, 250).data());
                if (content.size() > 250) {
                    printf(and_more_items_message().data(), content.size() - 250);
                }
                return;
            }
            size_t total_items = 0;
            for (auto dummy : fs::directory_iterator(filepath.main)) {
                total_items++;
            }
            size_t rowsAvailable = termSpaceRemaining.accountRowsFor(clipboard_item_many_contents_message().length());
            rowsAvailable -= 3;
            printf(total_items > rowsAvailable ? clipboard_item_too_many_contents_message().data() : clipboard_item_many_contents_message().data(), std::min(rowsAvailable, total_items), clipboard_name.data());
            auto it = fs::directory_iterator(filepath.main);
            for (size_t i = 0; i < std::min(rowsAvailable, total_items); i++) {

                printf(replaceColors("{blue}▏ {bold}{pink}%s{blank}\n").data(), it->path().filename().string().data());

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
}

void clearTempDirectory(bool force_clear = false) {
    if (force_clear || (action != Action::Paste && action != Action::PipeOut && action != Action::Show)) {
        fs::remove(filepath.original_files);
        for (const auto& entry : fs::directory_iterator(filepath.main)) {
            fs::remove_all(entry.path());
        }
    }
}

void convertFromGUIClipboard(const std::string& text) {
    clearTempDirectory(true);
    std::ofstream output(filepath.main / constants.pipe_file);
    output << text;
}

void convertFromGUIClipboard(const ClipboardPaths& clipboard) {
    // Only clear the temp directory if all files in the clipboard are outside the temp directory
    // This avoids the situation where we delete the very files we're trying to copy
    auto allOutsideFilepath = std::all_of(clipboard.paths().begin(), clipboard.paths().end(), [](auto& path) {
        auto relative = fs::relative(path, filepath.main);
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

        auto target = filepath.main / path.filename();
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
        std::ofstream originalFiles { filepath.original_files };
        for (auto&& path : clipboard.paths()) {
            originalFiles << path.string() << std::endl;
        }
    }
}

ClipboardContent thisClipboard() {
    if (fs::exists(filepath.original_files)) {
        std::ifstream originalFiles { filepath.original_files };
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

        for (const auto& entry : fs::directory_iterator(filepath.main)) { //count all items which were actually successfully actioned on
            paths.push_back(entry.path());
        }

        return ClipboardContent(ClipboardPaths(std::move(paths)));
    }

    return {};
}

void setupHandlers() {
    signal(SIGINT, [](int dummy) {
        if (!stopIndicator(false)) {
            // Indicator thread is not currently running. TODO: Write an unbuffered newline, and maybe a cancelation message, directly to standard error. Note: There is no standard C++ interface for this, so this requires an OS call.
            _exit(EXIT_FAILURE);
        }
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
    is_tty.in = isatty(fileno(stdin));
    is_tty.out = isatty(fileno(stdout));
    is_tty.err = isatty(fileno(stderr));

    if (getenv("IS_ACTUALLY_A_TTY")) { //add test compatibility where isatty returns false, but there is actually a tty
        is_tty.in = true;
        is_tty.out = true;
        is_tty.err = true;
    }

    #if defined(_WIN64) || defined (_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
        use_colors = false;
	}
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows

    #endif
    filepath.home = getenv("USERPROFILE") ? getenv("USERPROFILE") : getenv("HOME");

    if (getenv("NO_COLOR") && !getenv("FORCE_COLOR")) {
        use_colors = false;
    }

    for (int i = 1; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }
}

void createTempDirectory() {
    fs::create_directories(filepath.temporary);
    fs::create_directories(filepath.persistent);
}

void syncWithGUIClipboard() {
    if (clipboard_name == constants.default_clipboard_name && !getenv("CLIPBOARD_NOGUI")) {
        ClipboardContent guiClipboard = getGUIClipboard();
        if (guiClipboard.type() == ClipboardContentType::Text) {
            convertFromGUIClipboard(guiClipboard.text());
        } else if (guiClipboard.type() == ClipboardContentType::Paths) {
            convertFromGUIClipboard(guiClipboard.paths());
        }
    }
}

void showClipboardStatus() {
    std::vector<std::pair<fs::path, bool>> clipboards_with_contents;
    auto iterateClipboards = [&](const fs::path& path, bool persistent) { //use zip ranges here when gcc 13 comes out
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_directory(entry) && !fs::is_empty(entry)) {
                clipboards_with_contents.push_back({entry.path(), persistent});
            }
        }
    };
    iterateClipboards(filepath.temporary.parent_path(), false);
    iterateClipboards(filepath.persistent.parent_path(), true);
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
            printf(replaceColors("{bold}{blue}▏ %s%s: {blank}").data(), clipboards_with_contents.at(clipboard).first.filename().string().data(), clipboards_with_contents.at(clipboard).second ? " (p)" : "");

            if (fs::is_regular_file(clipboards_with_contents.at(clipboard).first / constants.pipe_file)) {
                std::string content;
                std::ifstream input(clipboards_with_contents.at(clipboard).first / constants.pipe_file);
                std::stringstream buffer;
                buffer << input.rdbuf();
                content = buffer.str();
                content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
                printf(replaceColors("{pink}%s{blank}\n").data(), content.substr(0, widthRemaining).data());
                continue;
            }

            for (bool first = true; const auto& entry : fs::directory_iterator(clipboards_with_contents.at(clipboard).first)) {
                int entryWidth = entry.path().filename().string().length();

                if (widthRemaining <= 0) {
                    break;
                }

                if (!first) {
                    if (entryWidth <= widthRemaining - 2) {
                        printf("%s", replaceColors("{pink}, {blank}").data());
                        widthRemaining -= 2;
                    }
                }

                if (entryWidth <= widthRemaining) {
                    printf(replaceColors("{pink}%s{blank}").data(), entry.path().filename().string().data());
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
auto flagIsPresent(const std::string_view& flag, const std::string_view& shortcut = "") {
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

void setAction() {
    if (arguments.size() >= 1) {
        for (const auto& entry : {Action::Cut, Action::Copy, Action::Paste, Action::Show, Action::Clear, Action::Edit}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "-")) {
                action = entry;
                if (action == Action::Copy && !is_tty.in) { action = Action::PipeIn; }
                if (action == Action::Paste && !is_tty.out) { action = Action::PipeOut; }
                return;
            }
        }
        printf(no_valid_action_message().data(), arguments.at(0).data());
        exit(EXIT_FAILURE);
    } else if (!is_tty.in) {
        action = Action::PipeIn;
    } else if (!is_tty.out) {
        action = Action::PipeOut;
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
        printf("%s", replaceColors("{bold}{blue}https://youtu.be/Lg_Pn45gyMs\n{blank}").data());
        exit(EXIT_SUCCESS);
    }
    if (auto flag = flagIsPresent<std::string>("-c"); flag != "") {
        clipboard_name = flag;
    }
    if (auto flag = flagIsPresent<std::string>("--clipboard"); flag != "") {
        clipboard_name = flag;
    }
    if (flagIsPresent<bool>("-h") || flagIsPresent<bool>("help", "--")) {
        printf(help_message().data(), constants.clipboard_version.data());
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
    if (action == Action::Cut && (!is_tty.in || !is_tty.in)) { tryThisInstead(Action::Copy); }
    if (action == Action::Copy && !is_tty.out) { tryThisInstead(Action::Paste); }
    if (action == Action::Paste && !is_tty.in) { tryThisInstead(Action::Copy); }
    if ((action == Action::PipeIn || action == Action::PipeOut) && arguments.size() >= 2) {
        fprintf(stderr, "%s", redirection_no_items_message().data());
        exit(EXIT_FAILURE);
    }
}

void setFilepaths() {
    filepath.temporary = (getenv("CLIPBOARD_TMPDIR") ? getenv("CLIPBOARD_TMPDIR") : getenv("TMPDIR") ? getenv("TMPDIR") : fs::temp_directory_path()) / constants.temporary_directory_name / clipboard_name;

    filepath.persistent = (getenv("CLIPBOARD_PERSISTDIR") ? getenv("CLIPBOARD_PERSISTDIR") : (getenv("XDG_CACHE_HOME") ? getenv("XDG_CACHE_HOME") : filepath.home)) / constants.persistent_directory_name / clipboard_name;
    
    filepath.main = (copying.is_persistent || getenv("CLIPBOARD_ALWAYS_PERSIST")) ? filepath.persistent : filepath.temporary;

    filepath.original_files = filepath.main.parent_path() / (clipboard_name + std::string(constants.original_files_extension));
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && copying.items.size() < 1) {
        printf(choose_action_items_message().data(), actions[action].data(), actions[action].data(), actions[action].data());
        exit(EXIT_FAILURE);
    }
    if (action == Action::Paste && fs::is_empty(filepath.main)) {
        showClipboardStatus();
        exit(EXIT_SUCCESS);
    }
}

void setupIndicator() {
    std::unique_lock<std::mutex> lock(m);
    int output_length = 0;
    const std::array<std::string_view, 10> spinner_steps{"━       ", "━━      ", " ━━     ", "  ━━    ", "   ━━   ", "    ━━  ", "     ━━ ", "      ━━", "       ━", "        "};
    static unsigned int percent_done = 0;
    if ((action == Action::Cut || action == Action::Copy) && is_tty.err) {
        static size_t items_size = copying.items.size();
        for (int i = 0; spinner_state == SpinnerState::Active; i == 9 ? i = 0 : i++) {
            percent_done = ((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size;
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return spinner_state != SpinnerState::Active; });
        }
    } else if ((action == Action::PipeIn || action == Action::PipeOut) && is_tty.err) {
        for (int i = 0; spinner_state == SpinnerState::Active; i == 9 ? i = 0 : i++) {
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), static_cast<int>(successes.bytes), "B", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return spinner_state != SpinnerState::Active; });
        }
    } else if (action == Action::Paste && is_tty.err) {
        static size_t items_size = 0;
        if (items_size == 0) {
            for (auto dummy : fs::directory_iterator(filepath.main)) {
                items_size++;
            }
            if (items_size == 0) {
                items_size = 1;
            }
        }
        for (int i = 0; spinner_state == SpinnerState::Active; i == 9 ? i = 0 : i++) {
            percent_done = ((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size;
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return spinner_state != SpinnerState::Active; });
        }
    } else if (is_tty.err) {
        while (spinner_state == SpinnerState::Active) {
            output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), 0, "%", "");
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return spinner_state != SpinnerState::Active; });
        }
    }
    if (is_tty.err) {
        fprintf(stderr, "\r%*s\r", output_length, "");
    }
    if (spinner_state == SpinnerState::Cancel) {
        fprintf(stderr, cancelled_message().data(), actions[action].data());
        fflush(stderr);
        _exit(EXIT_FAILURE);
    }
    fflush(stderr);
}

void startIndicator() { // If cancelled, leave cancelled
    SpinnerState expect = SpinnerState::Done;
    spinner_state.compare_exchange_strong(expect, SpinnerState::Active);
    indicator = std::thread(setupIndicator);
}

void deduplicateItems() {
    std::sort(copying.items.begin(), copying.items.end());
    copying.items.erase(std::unique(copying.items.begin(), copying.items.end()), copying.items.end());
}

unsigned long long totalItemSize() {
    unsigned long long total_item_size = 0;
    for (const auto& i : copying.items) {
        try {
            if (fs::is_directory(i)) {
                for (const auto& entry : fs::recursive_directory_iterator(i)) {
                    if (fs::is_regular_file(entry)) {
                        total_item_size += fs::file_size(entry);
                    } else {
                        total_item_size += 16;
                    }
                }
            } else if (fs::is_regular_file(i)) {
                total_item_size += fs::file_size(i);
            } else {
                total_item_size += 16;
            }
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(i.string(), e.code());
        }
    }
    return total_item_size;
}

void checkItemSize(unsigned long long total_item_size) {
    if (action == Action::Cut || action == Action::Copy) {
        const unsigned long long space_available = fs::space(filepath.main).available;
        if (total_item_size > (space_available / 2)) {
            stopIndicator();
            fprintf(stderr, not_enough_storage_message().data(), total_item_size / 1024.0, space_available / 1024.0);
            exit(EXIT_FAILURE);
        }
    }
}

void removeOldFiles() {
    if (fs::is_regular_file(filepath.original_files)) {
        std::ifstream files(filepath.original_files);
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
            fs::remove(filepath.original_files);
        }
        action = Action::Cut;
    }
}

bool userIsARobot() {
    return !is_tty.err || !is_tty.in || !is_tty.out || getenv("CI");
}

int getUserDecision(const std::string& item) {
    if (userIsARobot()) {
        return 2;
    }
    fprintf(stderr, item_already_exists_message().data(), item.data());
    std::string decision;
    while (true) {
        std::getline(std::cin, decision);
        fprintf(stderr, "%s", replaceColors("{blank}").data());
        if (decision == "y" || decision == "yes") {
            return 1;
        } else if (decision == "ya" || decision == "yesall") {
            return 2;
        } else if (decision == "n" || decision == "no") {
            return -1;
        } else if (decision == "na" || decision == "noall") {
            return -2;
        } else {
            fprintf(stderr, "%s", bad_response_message().data());
        }
    }
}

void performAction() {
    switch (action) {
        case Action::Copy:
        case Action::Cut:
            PerformAction::copy();
            break;
        case Action::Paste:
            PerformAction::paste();
            break;
        case Action::PipeIn:
            PerformAction::pipeIn();
            break;
        case Action::PipeOut:
            PerformAction::pipeOut();
            break;
        case Action::Clear:
            PerformAction::clear();
            break;
        case Action::Show:
            PerformAction::show();
            break;
        case Action::Edit:
            PerformAction::edit();
            break;
    }
}

bool isAWriteAction() {
    return action == Action::Cut || action == Action::Copy || action == Action::PipeIn || action == Action::Clear;
}

void updateGUIClipboard() {
    if (isAWriteAction() && !getenv("CLIPBOARD_NOGUI")) { //only update GUI clipboard on write operations
        writeToGUIClipboard(thisClipboard());
    }
}

void deduplicateFailures() {
    std::sort(copying.failedItems.begin(), copying.failedItems.end());
    copying.failedItems.erase(std::unique(copying.failedItems.begin(), copying.failedItems.end()), copying.failedItems.end());
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
            printf(replaceColors("{red}▏ {bold}%s{blank}{red}: %s{blank}\n").data(), copying.failedItems.at(i).first.data(), copying.failedItems.at(i).second.message().data());
            if (i == available.rows - 1 && copying.failedItems.size() > available.rows) {
                printf(and_more_fails_message().data(), int(copying.failedItems.size() - available.rows));
            }
        }
        printf("%s", fix_problem_message().data());
    }
}

void showSuccesses() {
    if ((action == Action::PipeIn || action == Action::PipeOut) && is_tty.err) {
        fprintf(stderr, pipe_success_message().data(), did_action[action].data(), static_cast<int>(successes.bytes));
        return;
    }
    if ((successes.files == 1 && successes.directories == 0) || (successes.files == 0 && successes.directories == 1)) {
        printf(one_item_success_message().data(), did_action[action].data(), action == Action::Paste ? (*(fs::directory_iterator(filepath.main))).path().filename().string().data() : copying.items.at(0).string().data());
    } else {
        if ((successes.files > 1) && (successes.directories == 0)) {
            printf(many_files_success_message().data(), did_action[action].data(), static_cast<int>(successes.files));
        } else if ((successes.files == 0) && (successes.directories > 1)) {
            printf(many_directories_success_message().data(), did_action[action].data(), static_cast<int>(successes.directories));
        } else if ((successes.files == 1) && (successes.directories == 1)) {
            printf(one_file_one_directory_success_message().data(), did_action[action].data(), static_cast<int>(successes.files), static_cast<int>(successes.directories));
        } else if ((successes.files > 1) && (successes.directories == 1)) {
            printf(many_files_one_directory_success_message().data(), did_action[action].data(), static_cast<int>(successes.files), static_cast<int>(successes.directories));
        } else if ((successes.files == 1) && (successes.directories > 1)) {
            printf(one_file_many_directories_success_message().data(), did_action[action].data(), static_cast<int>(successes.files), static_cast<int>(successes.directories));
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

        createTempDirectory();

        syncWithGUIClipboard();

        setAction();

        verifyAction();

        copying.items.assign(arguments.begin(), arguments.end());

        checkForNoItems();

        startIndicator();

        deduplicateItems();

        checkItemSize(totalItemSize());

        clearTempDirectory();

        performAction();

        updateGUIClipboard();

        stopIndicator();

        deduplicateFailures();

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
