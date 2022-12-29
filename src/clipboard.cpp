/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
    Copyright (C) 2022 Jackson Huff and other contributors on GitHub.com
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

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define NOMINMAX
#include <Windows.h>
#include <Shlobj.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#if defined(X11_AVAILABLE)
#include <X11/Xlib.h>
#endif

#if defined(WAYLAND_AVAILABLE)
#include <wayland-client-core.h>
#endif

namespace fs = std::filesystem;

bool use_perma_clip = false;
bool use_safe_copy = true;
fs::path main_filepath;
fs::path temporary_filepath;
fs::path persistent_filepath;
fs::path original_files_path;
fs::path home_directory;
fs::copy_options opts = fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks;
std::vector<fs::path> items;
std::vector<std::pair<std::string, std::error_code>> failedItems;
std::string clipboard_name = "0";

std::condition_variable cv;
std::mutex m;
std::jthread indicator; //If this fails to compile, then you need C++20!

unsigned int output_length = 0;
unsigned long files_success = 0;
unsigned long directories_success = 0;
unsigned long long bytes_success = 0;

bool stdin_is_tty = true;
bool stdout_is_tty = true;
bool stderr_is_tty = true;

constexpr std::string_view clipboard_version = "0.1.4";
constexpr std::string_view pipe_file = "clipboard.txt";

std::array<std::pair<std::string_view, std::string_view>, 8> colors = {{
    {"{red}", "\033[38;5;196m"},
    {"{green}", "\033[38;5;40m"},
    {"{yellow}", "\033[38;5;214m"},
    {"{blue}", "\033[38;5;51m"},
    {"{orange}", "\033[38;5;208m"},
    {"{pink}", "\033[38;5;219m"},
    {"{bold}", "\033[1m"},
    {"{blank}", "\033[0m"}
}};

enum class Action : unsigned int { Cut, Copy, Paste, PipeIn, PipeOut, Clear, Show };
Action action;

template <typename T, size_t N>
class ActionArray : public std::array<T, N> {
public:
    T& operator[](Action index) {
        return std::array<T, N>::operator[](static_cast<unsigned int>(index)); //switch to std::to_underlying when available
    }
};

ActionArray<std::string_view, 7> actions = {{
    "cut",
    "copy",
    "paste",
    "pipe in",
    "pipe out",
    "clear",
    "show"
}};

ActionArray<std::string_view, 7> action_shortcuts = {{
    "ct",
    "cp",
    "p",
    "pin",
    "pout",
    "clr",
    "sh"
}};

ActionArray<std::string_view, 7> doing_action = {{
    "Cutting",
    "Copying",
    "Pasting",
    "Piping in",
    "Piping out",
    "Clearing"
}};

ActionArray<std::string_view, 7> did_action = {{
    "Cut",
    "Copied",
    "Pasted",
    "Piped in",
    "Piped out",
    "Cleared"
}};

std::string_view help_message = "{blue}▏This is Clipboard %s, the cut, copy, and paste system for the command line.{blank}\n"
                                "{blue}{bold}▏How To Use{blank}\n"
                                "{orange}▏clipboard cut (item) [items]{blank} {pink}(This cuts an item or items.){blank}\n"
                                "{orange}▏clipboard copy (item) [items]{blank} {pink}(This copies an item or items.){blank}\n"
                                "{orange}▏clipboard paste{blank} {pink}(This pastes a clipboard.){blank}\n"
                                "{orange}▏clipboard show{blank} {pink}(This shows what's in a clipboard.){blank}\n"
                                "{orange}▏clipboard clear{blank} {pink}(This clears a clipboard's contents.){blank}\n"
                                "{blue}▏You can substitute \"cb\" for \"clipboard\" and use various shorthands for the actions to save time.{blank}\n"
                                "{blue}▏You can also choose which clipboard you want to use by adding a number to the end, or {bold}-{blank}{blue} to use a permanent clipboard.{blank}\n"
                                "{blue}{bold}▏Examples{blank}\n"
                                "{orange}▏cb ct Nuclear_Launch_Codes.txt contactsfolder{blank} {pink}(This cuts the following items into the default clipboard, 0.){blank}\n"
                                "{orange}▏clipboard cp1 dogfood.conf{blank} {pink}(This copies the following items into clipboard 1.){blank}\n"
                                "{orange}▏cb p1{blank} {pink}(This pastes clipboard 1.){blank}\n"
                                "{orange}▏cb sh4{blank} {pink}(This shows the contents of clipboard 4.){blank}\n"
                                "{orange}▏cb clr{blank} {pink}(This clears the contents of the default clipboard.){blank}\n"
                                "{blue}{bold}▏Configuration{blank}\n"
                                "{orange}▏CI: {pink}Set to make Clipboard overwrite existing items without a user prompt when pasting.{blank}\n"
                                "{orange}▏FORCE_COLOR: {pink}Set to make Clipboard always show color regardless of what you set NO_COLOR to.{blank}\n"
                                "{orange}▏TMPDIR: {pink}Set to the directory that Clipboard will use to hold the items you cut or copy.{blank}\n"
                                "{orange}▏NO_COLOR: {pink}Set to make Clipboard not show color.{blank}\n"
                                "{blue}{bold}▏Other Info{blank}\n"
                                "{orange}▏%s: {pink}The directory Clipboard uses to store temporary data.{blank}\n"
                                "{orange}▏%s: {pink}The directory Clipboard uses to store persistent data.{blank}\n"
                                "{blue}▏You can show this help screen anytime with {bold}clipboard -h{blank}{blue}, {bold}clipboard --help{blank}{blue}, or{bold} clipboard help{blank}{blue}.\n"
                                "{blue}▏You can also get more help in our Discord server at {bold}https://discord.gg/J6asnc3pEG{blank}\n"
                                "{blue}▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.{blank}\n"
                                "{blue}▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.{blank}\n";
std::string_view check_clipboard_status_message = "{blue}• There are items in these clipboards: ";
std::string_view clipboard_contents_message = "{blue}• Here are the first {bold}%i{blank}{blue} items in clipboard {bold}%s{blank}{blue}: {blank}\n";
std::string_view no_clipboard_contents_message = "{blue}• There is currently nothing in the clipboard.{blank}\n";
std::string_view clipboard_action_prompt = "{pink}Add {bold}%s, %s, {blank}{pink}or{bold} %s{blank}{pink} to the end, like {bold}clipboard %s{blank}{pink} to get started, or if you need help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
std::string_view no_valid_action_message = "{red}╳ You did not specify a valid action ({bold}\"%s\"{blank}{red}), or you forgot to include one. {pink}Try using or adding {bold}cut, copy, {blank}{pink}or {bold}paste{blank}{pink} instead, like {bold}clipboard copy.{blank}\n";
std::string_view choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
std::string_view fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
std::string_view redirection_no_items_message = "{red}╳ You can't specify items when you use redirection. {pink}Try removing the items that come after {bold}clipboard [action].\n";
std::string_view paste_success_message = "{green}✓ Pasted successfully{blank}\n";
std::string_view clear_success_message = "{green}✓ Cleared the clipboard{blank}\n";
std::string_view clear_fail_message = "{red}╳ Failed to clear the clipboard{blank}\n";
std::string_view clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items:{blank}\n";
std::string_view and_more_fails_message = "{red}▏ ...and {bold}%i{blank}{red} more.{blank}\n";
std::string_view and_more_items_message = "{blue}▏ ...and {bold}%i{blank}{blue} more.{blank}\n";
std::string_view fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                                       "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
std::string_view not_enough_storage_message = "{red}╳ There won't be enough storage available to paste all your items (%gkB to paste, %gkB available).{blank}{pink} Try double-checking what items you've selected or delete some files to free up space.{blank}\n";
std::string_view item_already_exists_message = "{yellow}• The item {bold}%s{blank}{yellow} already exists here. Would you like to replace it? {pink}Add {bold}all {blank}{pink}or {bold}a{blank}{pink} to use this decision for all items. {bold}[(y)es/(n)o)] ";
std::string_view bad_response_message = "{red}╳ Sorry, that wasn't a valid choice. Try again: {blank}{pink}{bold}[(y)es/(n)o)] ";
std::string_view working_message = "{yellow}• %s... %i%s %s{blank}\r";
std::string_view cancelled_message = "{green}✓ Cancelled %s{blank}\n";
std::string_view pipe_success_message = "{green}✓ %s %i bytes{blank}\n";
std::string_view one_item_success_message = "{green}✓ %s %s{blank}\n";
std::string_view multiple_files_success_message = "{green}✓ %s %i files{blank}\n";
std::string_view multiple_directories_success_message = "{green}✓ %s %i directories{blank}\n";
std::string_view multiple_files_directories_success_message = "{green}✓ %s %i files and %i directories{blank}\n";
std::string_view internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug, or you might be lacking permissions on this system.{blank}\n";

#include "langs.hpp"

std::string replaceColors(const std::string_view& str) {
    std::string temp(str); //a string to do scratch work on
    for (const auto& key : colors) { //iterate over all the possible colors to replace
        for (int i = 0; (i = temp.find(key.first, i)) != std::string::npos; i += key.second.length()) { //
            temp.replace(i, key.first.length(), key.second);
        }
    }
    return temp;
}

void setupSignals() {
    signal(SIGINT, [](int dummy) {
        indicator.request_stop();
        fprintf(stderr, "\r%*s\r", output_length, "");
        fprintf(stderr, replaceColors(cancelled_message).data(), actions[action].data());
        fflush(stderr);
        exit(1);
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

void showHelpMessage(int& argc, char *argv[]) {
    for (int i = 1; i < argc && strcmp(argv[i], "--"); i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || (argc >= 2 && !strcmp(argv[1], "help"))) {
            printf(replaceColors(help_message).data(), clipboard_version.data(), temporary_filepath.parent_path().string().data(), persistent_filepath.parent_path().string().data());
            exit(0);
        }
    }
}

void setupItems(int& argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        items.push_back(argv[i]);
    }
}

void setClipboardName(int& argc, char *argv[]) {
    if (argc >= 2) {
        clipboard_name = argv[1];
        if (clipboard_name.find_first_of("_:;|") != std::string::npos) {
            clipboard_name = clipboard_name.substr(clipboard_name.find_first_of("_:;|") + 1);
            use_perma_clip = true;
        } else {
            clipboard_name = clipboard_name.substr(clipboard_name.find_last_not_of("0123456789") + 1);
        }
        if (clipboard_name.empty()) {
            clipboard_name = "0";
        } else {
            argv[1][strlen(argv[1]) - (clipboard_name.length() + use_perma_clip)] = '\0';
        }
    }

    if (argc >= 3) {
        if (!strcmp(argv[2], "-c") && argc >= 4) {
            clipboard_name = argv[3];
            for (int i = 2; i < argc - 2; i++) {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        } else if (!strncmp(argv[2], "--clipboard=", 12)) {
            clipboard_name = argv[2] + 12;
            for (int i = 2; i < argc - 1; i++) {
                argv[i] = argv[i + 1];
            }
            argc -= 1;
        }
    }

    if (getenv("TMPDIR") != nullptr) {
        temporary_filepath = fs::path(getenv("TMPDIR")) / "Clipboard" / clipboard_name;
    } else {
        temporary_filepath = fs::temp_directory_path() / "Clipboard" / clipboard_name;
    }

    persistent_filepath = home_directory / ".clipboard" / clipboard_name;

    if (use_perma_clip) {
        main_filepath = persistent_filepath;
    } else {
        main_filepath = temporary_filepath;
    }

    original_files_path = main_filepath.parent_path() / (std::string(clipboard_name) + ".files");
}

void setupVariables(int& argc, char *argv[]) {
    #if defined(_WIN64) || defined (_WIN32)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //Windows terminal color compatibility
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	if (!SetConsoleMode(hOut, (dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT))) {
        for (auto& key : colors) {
            key.second = "";
        }
	}
	SetConsoleOutputCP(CP_UTF8); //fix broken accents on Windows
    #endif

    stdin_is_tty = isatty(fileno(stdin));
    stdout_is_tty = isatty(fileno(stdout));
    stderr_is_tty = isatty(fileno(stderr));

    #if defined(_WIN64) || defined (_WIN32)
    home_directory = getenv("USERPROFILE");
    #else
    home_directory = getenv("HOME");
    #endif

    if (getenv("NO_COLOR") != nullptr && getenv("FORCE_COLOR") == nullptr) {
        for (auto& key : colors) {
            key.second = "";
        }
    }
}

void createTempDirectory() {
    if (!fs::is_directory(main_filepath)) {
        fs::create_directories(main_filepath);
    }
    if (!fs::is_directory(home_directory / ".clipboard")) {
        fs::create_directories(home_directory / ".clipboard");
    }
}

void syncWithGUIClipboard() {
    //check if the clipboard number is the default ("0")
    //if it is, check if the system clipboard is newer than main_filepath (check the last write time)
    //if it's newer, write the contents of the system clipboard to main_filepath
    //if it's older, do nothing

    #if defined(X11_AVAILABLE)
    Display* dpy;
    Window root;
    int screen;
    Atom selection;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        return;
    }
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    selection = XInternAtom(dpy, "CLIPBOARD", False);

    #endif

    #if defined(WAYLAND_AVAILABLE)

    #endif

    #if defined(_WIN32) || defined(_WIN64)
    
    #elif defined(__APPLE__)

    #endif
}

void showClipboardStatus() {
    std::vector<std::pair<std::string, bool>> clipboards_with_contents;
    for (const auto& entry : fs::directory_iterator(main_filepath.parent_path())) {
        if (fs::is_directory(entry) && !fs::is_empty(entry)) {
            clipboards_with_contents.push_back({entry.path().filename().string(), false});
        }
    }
    for (const auto& entry : fs::directory_iterator(home_directory / ".clipboard")) {
        if (fs::is_directory(entry) && !fs::is_empty(entry)) {
            clipboards_with_contents.push_back({entry.path().filename().string(), true});
        }
    }
    std::sort(clipboards_with_contents.begin(), clipboards_with_contents.end());
    if (clipboards_with_contents.empty()) {
        printf("%s", replaceColors(no_clipboard_contents_message).data());
        printf(replaceColors(clipboard_action_prompt).data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
    } else {
        printf("%s", replaceColors(check_clipboard_status_message).data());
        for (int clipboard = 0; clipboard < clipboards_with_contents.size(); clipboard++) {
            printf(replaceColors("{bold}%s{blank}{blue}").data(), (clipboards_with_contents.at(clipboard).first + (clipboards_with_contents.at(clipboard).second ? " (p)" : "")).data());
            if (clipboard != clipboards_with_contents.size() - 1) {
                printf(", ");
            }
        }
        printf("\n");
        printf(replaceColors(clipboard_action_prompt).data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
    }
}

void showClipboardContents() {
    if (fs::is_directory(main_filepath) && !fs::is_empty(main_filepath)) {
        unsigned int total_items = 0;
        for (const auto& entry : fs::directory_iterator(main_filepath)) {
            total_items++;
        }
        printf(replaceColors(clipboard_contents_message).data(), std::min(static_cast<unsigned int>(20), total_items), clipboard_name.data());
        auto it = fs::directory_iterator(main_filepath);
        for (int i = 0; i < std::min(static_cast<unsigned int>(20), total_items); i++) {
            printf(replaceColors("{blue}▏ {bold}%s{blank}\n").data(), it->path().filename().string().data());
            if (i == 19 && total_items > 20) {
                printf(replaceColors(and_more_items_message).data(), total_items - 20);
            }
            it++;
        }
    } else {
        printf(replaceColors(no_clipboard_contents_message).data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
    }
}

void setupAction(int& argc, char *argv[]) {
    auto flagIsPresent = [&](const std::string_view& flag){
        for (int i = 1; i < argc && strcmp(argv[i], "--"); i++) {
            if (!strcmp(argv[i], flag.data())) {
                for (int j = i; j < argc - 1; j++) {
                    argv[j] = argv[j + 1];
                }
                argc--;
                return true;
            }
        }
        return false;
    };
    if (argc >= 2) {
        if (flagIsPresent(actions[Action::Cut]) || flagIsPresent(action_shortcuts[Action::Cut]) || flagIsPresent("--" + std::string(actions[Action::Cut])) || flagIsPresent("-" + std::string(action_shortcuts[Action::Cut]))) { //replace with join_with_view when C++23 becomes available
            action = Action::Cut;
            if (!stdin_is_tty || !stdout_is_tty) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Copy].data(), actions[Action::Copy].data());
                exit(1);
            }
        } else if (flagIsPresent(actions[Action::Copy]) || flagIsPresent(action_shortcuts[Action::Copy]) || flagIsPresent("--" + std::string(actions[Action::Copy])) || flagIsPresent("-" + std::string(action_shortcuts[Action::Copy]))) {
            action = Action::Copy;
            if (!stdin_is_tty) {
                action = Action::PipeIn;
            } else if (!stdout_is_tty) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Paste].data(), actions[Action::Paste].data());
                exit(1);
            }
        } else if (flagIsPresent(actions[Action::Paste]) || flagIsPresent(action_shortcuts[Action::Paste]) || flagIsPresent("--" + std::string(actions[Action::Paste])) || flagIsPresent("-" + std::string(action_shortcuts[Action::Paste]))) {
            action = Action::Paste;
            if (!stdout_is_tty) {
                action = Action::PipeOut;
            } else if (!stdin_is_tty) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Copy].data(), actions[Action::Copy].data());
                exit(1);
            }
        } else if (flagIsPresent(actions[Action::Show]) || flagIsPresent(action_shortcuts[Action::Show]) || flagIsPresent("--" + std::string(actions[Action::Show])) || flagIsPresent("-" + std::string(action_shortcuts[Action::Show]))) {
            action = Action::Show;
        } else if (flagIsPresent(actions[Action::Clear]) || flagIsPresent(action_shortcuts[Action::Clear]) || flagIsPresent("--" + std::string(actions[Action::Clear])) || flagIsPresent("-" + std::string(action_shortcuts[Action::Clear]))) {
            action = Action::Clear;
            if (!stdin_is_tty) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Cut].data(), actions[Action::Cut].data());
                exit(1);
            } else if (!stdout_is_tty) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Paste].data(), actions[Action::Paste].data());
                exit(1);
            }
        } else if (flagIsPresent("ee")) {
            printf("%s", replaceColors("{bold}{blue}https://youtu.be/Lg_Pn45gyMs\n{blank}").data());
            exit(0);
        } else {
            printf(replaceColors(no_valid_action_message).data(), argv[1]);
            exit(1);
        }
        if (flagIsPresent("--fast-copy") || flagIsPresent("-fc")) {
            use_safe_copy = false;
        }
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "--")) {
                for (int j = i; j < argc; j++) {
                    argv[j] = argv[j + 1];
                }
                argc--;
                break;
            }
        }
    } else if (!stdin_is_tty) {
        action = Action::PipeIn;
    } else if (!stdout_is_tty) {
        action = Action::PipeOut;
    } else {
        showClipboardStatus();
        exit(0);
    }
    if (action == Action::PipeIn || action == Action::PipeOut) {
        if (argc >= 3) {
            fprintf(stderr, "%s", replaceColors(redirection_no_items_message).data());
            exit(1);
        }
    }
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && items.size() < 1) {
        printf(replaceColors(choose_action_items_message).data(), actions[action].data(), actions[action].data(), actions[action].data());
        exit(1);
    }
    if (action == Action::Paste && fs::is_empty(main_filepath)) {
        showClipboardStatus();
        exit(0);
    }
}

void setupIndicator(std::stop_token st) {
    std::unique_lock<std::mutex> lock(m);
    const std::array<std::string_view, 10> spinner_steps{"━       ", "━━      ", " ━━     ", "  ━━    ", "   ━━   ", "    ━━  ", "     ━━ ", "      ━━", "       ━", "        "};
    static unsigned int percent_done = 0;
    if ((action == Action::Cut || action == Action::Copy) && stderr_is_tty) {
        static unsigned long items_size = items.size();
        for (int i = 0; !st.stop_requested(); i == 9 ? i = 0 : i++) {
            percent_done = ((files_success + directories_success + failedItems.size()) * 100) / items_size;
            output_length = fprintf(stderr, replaceColors(working_message).data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return st.stop_requested(); });
        }
    } else if ((action == Action::PipeIn || action == Action::PipeOut) && stderr_is_tty) {
        for (int i = 0; !st.stop_requested(); i == 9 ? i = 0 : i++) {
            output_length = fprintf(stderr, replaceColors(working_message).data(), doing_action[action].data(), bytes_success, "B", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return st.stop_requested(); });
        }
    } else if (action == Action::Paste && stderr_is_tty) {
        static unsigned long items_size = 0;
        if (items_size == 0) {
            for (const auto& f : fs::directory_iterator(main_filepath)) {
                items_size++;
            }
            if (items_size == 0) {
                items_size = 1;
            }
        }
        for (int i = 0; !st.stop_requested(); i == 9 ? i = 0 : i++) {
            percent_done = ((files_success + directories_success + failedItems.size()) * 100) / items_size;
            output_length = fprintf(stderr, replaceColors(working_message).data(), doing_action[action].data(), percent_done, "%", spinner_steps.at(i).data());
            fflush(stderr);
            cv.wait_for(lock, std::chrono::milliseconds(50), [&]{ return st.stop_requested(); });
        }
    } else if (stderr_is_tty) {
        output_length = fprintf(stderr, replaceColors(working_message).data(), doing_action[action].data(), 0, "%", "");
        fflush(stderr);
    }
}

void deduplicateItems() {
    std::sort(items.begin(), items.end());
    items.erase(std::unique(items.begin(), items.end()), items.end());
}

unsigned long long calculateTotalItemSize() {
    unsigned long long total_item_size = 0;
    for (const auto& i : items) {
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
            failedItems.emplace_back(i.string(), e.code());
        }
    }   
    return total_item_size;
}

void checkItemSize() {
    const unsigned long long space_available = fs::space(main_filepath).available;
    unsigned long long total_item_size = 0;
    if (action == Action::Cut || action == Action::Copy) {
        total_item_size = calculateTotalItemSize();
        if (total_item_size > (space_available / 2)) {
            printf(replaceColors(not_enough_storage_message).data(), total_item_size / 1024.0, space_available / 1024.0);
            indicator.request_stop();
            cv.notify_one();
            exit(1);
        }
    }
}

void clearTempDirectory() {
    if (action != Action::Paste) {
        fs::remove(original_files_path);
    }
    if (action == Action::Copy || action == Action::Cut || action == Action::PipeIn || action == Action::Clear) {
        for (const auto& entry : fs::directory_iterator(main_filepath)) {
            fs::remove_all(entry.path());
        }
    }
}

void copyFiles() {
    std::ofstream originalFiles;
    if (action == Action::Cut) {
        originalFiles.open(original_files_path);
    }
    for (const auto& f : items) {
        auto copyItem = [&](const bool use_regular_copy = use_safe_copy) {
            if (fs::is_directory(f)) {
                if (f.filename() == "") {
                    fs::create_directories(main_filepath / f.parent_path().filename());
                    fs::copy(f, main_filepath / f.parent_path().filename(), opts);
                } else {
                    fs::create_directories(main_filepath / f.filename());
                    fs::copy(f, main_filepath / f.filename(), opts);
                }
                directories_success++;
            } else {
                fs::copy(f, main_filepath / f.filename(), use_regular_copy ? opts : opts | fs::copy_options::create_hard_links);
                files_success++;
            }
            if (action == Action::Cut) {
                originalFiles << fs::absolute(f).string() << std::endl;
            }
        };
        try {
            copyItem();
        } catch (const fs::filesystem_error& e) {
            if (!use_safe_copy) {
                try {
                    copyItem(true);
                } catch (const fs::filesystem_error& e) {
                    failedItems.emplace_back(f.string(), e.code());
                }
            } else {
                failedItems.emplace_back(f.string(), e.code());
            }
        }
    }
}

void removeOldFiles() {
    if (fs::is_regular_file(original_files_path)) {
        std::ifstream files(original_files_path);
        std::string line;
        while (std::getline(files, line)) {
            try {
                fs::remove_all(line);
            } catch (const fs::filesystem_error& e) {
                failedItems.emplace_back(line, e.code());
            }
        }
        files.close();
        if (failedItems.size() == 0) {
            fs::remove(original_files_path);
        }
        action = Action::Cut;
    }
}

bool userIsARobot() {
    if (!stderr_is_tty || !stdin_is_tty || !stdout_is_tty) {
        return true;
    }
    if (getenv("CI") != nullptr) {
        return true;
    }  
    return false;
}

int getUserDecision(const std::string& item) {
    if (userIsARobot()) {
        return 2;
    }
    fprintf(stderr, replaceColors(item_already_exists_message).data(), item.data());
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
            fprintf(stderr, "%s", replaceColors(bad_response_message).data());
        }
    }
}

void pasteFiles() {
    int user_decision = 0;
    for (const auto& f : fs::directory_iterator(main_filepath)) {
        auto pasteItem = [&](const bool use_regular_copy = use_safe_copy) {
            if (fs::equivalent(f, fs::current_path() / f.path().filename())) {
                if (fs::is_directory(f)) {
                    directories_success++;
                } else {
                    files_success++;
                }
                return;
            }
            if (fs::is_directory(f)) {
                fs::copy(f, fs::current_path() / f.path().filename(), opts);
                directories_success++;
            } else {
                fs::copy(f, fs::current_path() / f.path().filename(), use_regular_copy ? opts : opts | fs::copy_options::create_hard_links);
                files_success++;
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
                        indicator.request_stop();
                        user_decision = getUserDecision(f.path().filename().string());
                        indicator = std::jthread(setupIndicator);
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
            if (!use_safe_copy) {
                try {
                    pasteItem(true);
                } catch (const fs::filesystem_error& e) {
                    failedItems.emplace_back(f.path().filename().string(), e.code());
                }
            } else {
                failedItems.emplace_back(f.path().filename().string(), e.code());
            }
        }
    }
    removeOldFiles();
}

void pipeIn() {
    std::ofstream file(main_filepath / pipe_file);
    std::string buffer;
    std::string line;
    for (int i = 0; std::getline(std::cin, line); i == 19 ? i = 0 : i++) {
        buffer += line + "\n";
        bytes_success += line.size() + 1;
        if (i == 19) {
            file << buffer;
            buffer = "";
        }
    }
    if (buffer != "") {
        file << buffer;
    }
    file.close();
}

void pipeOut() {
    std::string line;
    for (const auto& entry : fs::recursive_directory_iterator(main_filepath)) {
        std::ifstream file(entry.path());
        while (std::getline(file, line)) {
            printf("%s\n", line.data());
            bytes_success += line.size() + 1;
        }
        file.close();
    }
}

void clearClipboard() {
    if (fs::is_empty(main_filepath)) {
        printf("%s", replaceColors(clear_success_message).data());
    } else {
        printf("%s", replaceColors(clear_fail_message).data());
    }
}

void performAction() {
    switch (action) {
        case Action::Copy:
        case Action::Cut:
            copyFiles();
            break;
        case Action::Paste:
            pasteFiles();
            break;
        case Action::PipeIn:
            pipeIn();
            break;
        case Action::PipeOut:
            pipeOut();
            break;
        case Action::Clear:
            clearClipboard();
            break;
        case Action::Show:
            showClipboardContents();
            break;
    }
}

#if defined(_WIN32) || defined(_WIN64)
void onWindowsError(const std::string_view function) {
    auto errorCode = GetLastError();

    char* errorMessage;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        errorCode,
        0,
        static_cast<LPTSTR>(static_cast<void*>(&errorMessage)),
        0,
        nullptr
    );
    std::cerr << function << ": " << errorMessage << std::endl;
    exit(EXIT_FAILURE);
}

void setWindowsClipboardDataPipe() {
    std::ifstream file(main_filepath / pipe_file);
    std::vector<char> utf8Data(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    auto utf16Len = MultiByteToWideChar(
        CP_UTF8,
        0,
        &utf8Data[0],
        utf8Data.size(),
        nullptr,
        0
    );
    if (utf16Len <= 0) {
        onWindowsError("MultiByteToWideChar");
    }

    auto bufferLen = utf16Len + 1;

    auto bufferSize = bufferLen * sizeof(wchar_t);

    HGLOBAL clipboardHandle = GlobalAlloc(GMEM_MOVEABLE, bufferSize);
    if (clipboardHandle == nullptr) {
        onWindowsError("GlobalAlloc");
    }

    auto clipboardPointer = static_cast<wchar_t*>(GlobalLock(clipboardHandle));
    if (clipboardPointer == nullptr) {
        onWindowsError("GlobalLock");
    }

    ZeroMemory(clipboardPointer, bufferSize);
    auto bytesWritten = MultiByteToWideChar(
        CP_UTF8,
        0,
        &utf8Data[0],
        utf8Data.size(),
        clipboardPointer,
        bufferLen
    );
    if (bytesWritten <= 0) {
        onWindowsError("MultiByteToWideChar");
    }

    if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
        onWindowsError("GlobalUnlock");
    }
    if (SetClipboardData(CF_UNICODETEXT, clipboardHandle) == nullptr) {
        onWindowsError("SetClipboardData");
    }
}

void setWindowsClipboardDataFiles() {

    std::vector<wchar_t> data;
    for (const auto& entry : fs::directory_iterator(main_filepath)) {
        for (const auto& c : entry.path().wstring()) {
            data.push_back(c);
        }

        data.push_back(0);
    }

    data.push_back(0);

    size_t clipboardSize = sizeof(DROPFILES) + data.size() * sizeof(wchar_t);
    HGLOBAL clipboardHandle = GlobalAlloc(GMEM_MOVEABLE, clipboardSize);
    if (clipboardHandle == nullptr) {
        onWindowsError("GlobalAlloc");
    }

    auto clipboardPointer = GlobalLock(clipboardHandle);
    if (clipboardPointer == nullptr) {
        onWindowsError("GlobalLock");
    }

    ZeroMemory(clipboardPointer, clipboardSize);

    auto dropfiles = static_cast<DROPFILES*>(clipboardPointer);
    dropfiles->pFiles = sizeof(DROPFILES);
    dropfiles->fWide = TRUE;

    auto fileList = static_cast<void*>(static_cast<char*>(clipboardPointer) + sizeof(DROPFILES));
    std::memcpy(fileList, &data[0], data.size() * sizeof(wchar_t));

    if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
        onWindowsError("GlobalUnlock");
    }

    if (SetClipboardData(CF_HDROP, clipboardHandle) == nullptr) {
        onWindowsError("SetClipboardData");
    }
}
#endif

void updateGUIClipboard() {
#if defined(_WIN32) || defined(_WIN64)
    if (OpenClipboard(nullptr) == 0) {
        onWindowsError("OpenClipboard");
    }
    if (EmptyClipboard() == 0) {
        onWindowsError("EmptyClipboard");
    }

    if (fs::is_regular_file(main_filepath / pipe_file)) {
        setWindowsClipboardDataPipe();

    } else {
        setWindowsClipboardDataFiles();
    }

    if (CloseClipboard() == 0) {
        onWindowsError("CloseClipboard");
    }
#endif
}


void showFailures() {
    if (failedItems.size() > 0) {
        printf(replaceColors(clipboard_failed_message).data(), actions[action].data());
        for (int i = 0; i < std::min(5, static_cast<int>(failedItems.size())); i++) {
            printf(replaceColors("{red}▏ {bold}%s{blank}{red}: %s{blank}\n").data(), failedItems.at(i).first.data(), failedItems.at(i).second.message().data());
            if (i == 4 && failedItems.size() > 5) {
                printf(replaceColors(and_more_fails_message).data(), int(failedItems.size() - 5));
            }
        }
        printf("%s", replaceColors(fix_problem_message).data());
    }
}

void showSuccesses() {
    if (action == Action::PipeIn || action == Action::PipeOut && stderr_is_tty) {
        fprintf(stderr, replaceColors(pipe_success_message).data(), did_action[action].data(), bytes_success);
        return;
    }
    if ((files_success == 1 && directories_success == 0) || (files_success == 0 && directories_success == 1)) {
        if (action == Action::Paste) {
            printf("%s", replaceColors(paste_success_message).data());
        } else {
            printf(replaceColors(one_item_success_message).data(), did_action[action].data(), items.at(0).string().data());
        }
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            printf(replaceColors(multiple_files_success_message).data(), did_action[action].data(), files_success);
        } else if ((files_success == 0) && (directories_success > 1)) {
            printf(replaceColors(multiple_directories_success_message).data(), did_action[action].data(), directories_success);
        } else if ((files_success >= 1) && (directories_success >= 1)) {
            printf(replaceColors(multiple_files_directories_success_message).data(), did_action[action].data(), files_success, directories_success);
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupSignals();

        setupVariables(argc, argv);

        setLocale();

        setClipboardName(argc, argv);

        showHelpMessage(argc, argv);

        createTempDirectory();

        syncWithGUIClipboard();

        setupAction(argc, argv);

        setupItems(argc, argv);

        checkForNoItems();

        indicator = std::jthread(setupIndicator);

        deduplicateItems();

        checkItemSize();

        clearTempDirectory();

        performAction();

        if (action == Action::Cut || action == Action::Copy || action == Action::PipeIn || action == Action::Clear) { //only update GUI clipboard on write operations
            updateGUIClipboard();
        }

        indicator.request_stop();
        cv.notify_one();

        if (stderr_is_tty) {
            fprintf(stderr, "\r%*s\r", output_length, "");
            fflush(stderr);
        }

        showFailures();

        showSuccesses();
    } catch (const std::exception& e) {
        fprintf(stderr, replaceColors(internal_error_message).data(), e.what());
        indicator.request_stop();
        cv.notify_one();
        exit(1);
    }
    return 0;
}