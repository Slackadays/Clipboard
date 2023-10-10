/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
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
#include "../clipboard.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <climits>
#include <clipboard/fork.hpp>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

Forker forker {};

GlobalFilepaths global_path;
Clipboard path;
Copying copying;

bool output_silent = false;
bool progress_silent = false;
bool confirmation_silent = false;
bool no_color = false;
bool all_option = false;

std::string maximumHistorySize;

std::string preferred_mime;
std::vector<std::string> available_mimes;

std::vector<std::string> arguments;

std::string clipboard_invocation;

std::string clipboard_suffix;

std::string clipboard_name = "0";

unsigned long clipboard_entry = 0;

std::string locale;

Action action;

IOType io_type;

Successes successes;

IsTTY is_tty;

std::condition_variable cv;
std::mutex m;
std::atomic<ClipboardState> clipboard_state;
std::atomic<IndicatorState> progress_state;

std::array<std::pair<std::string_view, std::string_view>, 10> colors = {
        {{"[error]", "\033[38;5;196m"},    // red
         {"[success]", "\033[38;5;40m"},   // green
         {"[progress]", "\033[38;5;214m"}, // yellow
         {"[info]", "\033[38;5;45m"},      // blue
         {"[help]", "\033[38;5;207m"},     // pink
         {"[bold]", "\033[1m"},
         {"[nobold]", "\033[22m"},
         {"[inverse]", "\033[7m"},
         {"[noinverse]", "\033[27m"},
         {"[blank]", "\033[0m"}}};

#if defined(_WIN64) || defined(_WIN32)
UINT old_code_page;
#endif

bool envVarIsTrue(const std::string_view& name) {
    auto temp = getenv(name.data());
    if (temp == nullptr) return false;
    std::string result(temp);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    if (result == "1" || result == "true" || result == "yes" || result == "y" || result == "on" || result == "enabled") return true;
    return false;
}

std::vector<std::string> regexSplit(const std::string& content, const std::regex& regex) {
    std::sregex_token_iterator begin(content.begin(), content.end(), regex, -1), end; // -1: return the things that are not matched
    return std::vector<std::string>(begin, end);
}

std::string pipedInContent(bool count) {
    std::string content;
#if !defined(_WIN32) && !defined(_WIN64)
    int len = -1;
    int stdinFd = fileno(stdin);
    constexpr int bufferSize = 65536;
    std::array<char, bufferSize> buffer;
    while (len != 0) {
        len = read(stdinFd, buffer.data(), bufferSize);
        content.append(buffer.data(), len);
        if (count) successes.bytes += len;
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
        if (count) successes.bytes += dwRead;
    }
#endif
    return content;
}

unsigned int suitableThreadAmount() {
    auto maxThreads = std::thread::hardware_concurrency();
    if (maxThreads < 4) return 1;
    return static_cast<float>(maxThreads) * 0.9;
}

unsigned long numberLength(const unsigned long& number) {
    if (number < 10) return 1;
    if (number < 100) return 2;
    if (number < 1000) return 3;
    if (number < 10000) return 4;
    if (number < 100000) return 5;
    if (number < 1000000) return 6;
    if (number < 10000000) return 7;
    if (number < 100000000) return 8;
    if (number < 1000000000) return 9;
    return 10; // because 4 billion is the max for unsigned long, we know we'll have 10 or fewer digits
}

void ignoreItemsPreemptively(std::vector<fs::path>& items) {
    if (!path.holdsIgnoreRegexes() || copying.items.empty() || action == Action::Ignore || io_type == IOType::Pipe) return;
    auto regexes = path.ignoreRegexes();
    for (const auto& regex : regexes)
        for (const auto& item : items)
            if (std::regex_match(item.string(), regex)) items.erase(std::find(items.begin(), items.end(), item));
}

bool userIsARobot() {
    return !is_tty.err || !is_tty.in || !is_tty.out || envVarIsTrue("CI");
}

bool isAWriteAction() {
    using enum Action;
    return action_is_one_of(Cut, Copy, Add, Clear, Remove, Swap, Load, Import, Edit);
}

bool isAClearingAction() {
    using enum Action;
    return action_is_one_of(Copy, Cut, Clear);
}

bool needsANewEntry() {
    using enum Action;
    return (action == Copy || action == Cut || (action == Clear && !all_option)) && clipboard_entry == constants.default_clipboard_entry;
}

[[nodiscard]] CopyPolicy userDecision(const std::string& item) {
    using enum CopyPolicy;

    if (userIsARobot()) return ReplaceAll;

    fprintf(stderr, item_already_exists_message().data(), item.data());
    std::string decision;
    while (true) {
        std::getline(std::cin, decision);
        fprintf(stderr, "%s", formatColors("[blank]").data());

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

void setupHandlers() {
    atexit([] {
        path.releaseLock();
        stopIndicator(true);
#if defined(_WIN64) || defined(_WIN32)
        SetConsoleOutputCP(old_code_page);
        if (isAWriteAction()) {
            FlushFileBuffers(CreateFileA(global_path.temporary.string().data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
            FlushFileBuffers(CreateFileA(global_path.persistent.string().data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
        }
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__unix__)
        if (isAWriteAction()) {
            fsync(open(global_path.temporary.string().data(), O_RDONLY));
            fsync(open(global_path.persistent.string().data(), O_RDONLY));
        }
#endif
    });

    signal(SIGINT, [](int) {
        fprintf(stderr, "%s", formatColors("[blank]").data());
        if (!stopIndicator(false)) {
            // Indicator thread is not currently running. TODO: Write an unbuffered newline, and maybe a cancelation
            // message, directly to standard error. Note: There is no standard C++ interface for this, so this requires
            // an OS call.
            path.releaseLock();
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
        // indicator.detach();
    });

    forker.atNonFork([]() {
        // If the process didn't fork, we need to stop the indicator thread to ensure it won't
        // keep running in the background while we perform the required work
        stopIndicator();
    });
}

void setLocale() {
    try {
        locale = getenv("CLIPBOARD_LOCALE") ? getenv("CLIPBOARD_LOCALE") : std::locale("").name();
        std::locale::global(std::locale(locale));
    } catch (...) {}
    if (locale.substr(0, 2) == "es")
        setLanguageES();
    else if (locale.substr(0, 2) == "pt")
        setLanguagePT();
    else if (locale.substr(0, 2) == "tr")
        setLanguageTR();
    else if (locale.substr(0, 2) == "fr")
        setLanguageFR();
    else if (locale.substr(0, 2) == "de")
        setLanguageDE();
    return;
}

void setClipboardAttributes() {
    if (arguments.empty()) return;
    std::string temp = arguments.at(0);
    if (temp.find_first_of("_0123456789") == std::string::npos) return;
    clipboard_name = temp.substr(temp.find_first_of("_0123456789"));
    clipboard_suffix = clipboard_name;
    try {
        if (temp.find_last_of("-") != std::string::npos) {
            clipboard_entry = std::stoul(temp.substr(temp.find_last_of("-") + 1));
            clipboard_name = clipboard_name.substr(0, clipboard_name.find_last_of("-"));
        }
    } catch (...) {}
    arguments.at(0) = arguments.at(0).substr(0, arguments.at(0).find_first_of("_0123456789"));
}

void verifyClipboardName() {
#if defined(_WIN32) || defined(_WIN64)
    constexpr std::array forbiddenFilenameCharacters {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};
#elif defined(__APPLE__)
    constexpr std::array forbiddenFilenameCharacters {'/', ':'};
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__unix__)
    constexpr std::array forbiddenFilenameCharacters {'/'};
#else
    constexpr std::array forbiddenFilenameCharacters {};
#endif

#if defined(_WIN32) || defined(_WIN64)
    for (const auto& forbiddenFilename :
         {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"}) {
        if (clipboard_name == forbiddenFilename) {
            error_exit(
                    formatColors("[error][inverse] ✘ [noinverse] The clipboard name you chose (\"[bold]%s[blank][error]\") won't work on this system possibly due to special characters. [help]⬤ Try "
                                 "choosing a different one instead.\n[blank]"),
                    clipboard_name
            );
        }
    }
#endif

    if (std::find_first_of(clipboard_name.begin(), clipboard_name.end(), forbiddenFilenameCharacters.begin(), forbiddenFilenameCharacters.end()) != clipboard_name.end()) {
        error_exit(
                formatColors("[error][inverse] ✘ [noinverse] The clipboard name you chose (\"[bold]%s[blank][error]\") won't work on this system possibly due to special characters. [help]⬤ Try "
                             "choosing a different one instead.\n[blank]"),
                clipboard_name
        );
    }
}

void setupVariables(int& argc, char* argv[]) {
    is_tty.in = envVarIsTrue("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdin));
    is_tty.out = envVarIsTrue("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stdout));
    is_tty.err = envVarIsTrue("CLIPBOARD_FORCETTY") ? true : isatty(fileno(stderr));

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
    global_path.home = getenv("USERPROFILE") ? getenv("USERPROFILE") : getenv("HOME");

    bool CLICOLOR = getenv("CLICOLOR") && !strcmp(getenv("CLICOLOR"), "0");
    bool NO_COLOR = getenv("NO_COLOR");
    bool CLICOLOR_FORCE = getenv("CLICOLOR_FORCE") && strcmp(getenv("CLICOLOR_FORCE"), "0");
    bool FORCE_COLOR = getenv("FORCE_COLOR");

    no_color = (NO_COLOR || CLICOLOR) && !FORCE_COLOR && !CLICOLOR_FORCE;

    output_silent = envVarIsTrue("CLIPBOARD_SILENT");

    progress_silent = envVarIsTrue("CLIPBOARD_NOPROGRESS");

    if (auto setting = getenv("CLIPBOARD_THEME"); setting != nullptr) setTheme(setting);

    if (auto size = getenv("CLIPBOARD_HISTORY"); size != nullptr) maximumHistorySize = size;

    if (argc == 0) return;

    arguments.assign(argv + 1, argv + argc);

    clipboard_invocation = argv[0];
}

template <typename T>
[[nodiscard]] auto flagIsPresent(const std::string_view& flag, const std::string_view& shortcut = "") {
    for (const auto& arg : arguments) {
        if (arg == "--") break;
        if (arg == flag || arg == (std::string(shortcut).append(flag))) {
            if constexpr (std::is_same_v<T, std::string>) {
                auto potentialArg = arguments.erase(std::find(arguments.begin(), arguments.end(), arg));
                if (potentialArg == arguments.end()) return std::string();
                std::string temp(*potentialArg);
                arguments.erase(std::find(arguments.begin(), arguments.end(), temp));
                return temp;
            } else {
                arguments.erase(std::find(arguments.begin(), arguments.end(), arg));
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
        for (const auto& entry : {Cut, Copy, Paste, Clear, Show, Edit, Add, Remove, Note, Swap, Status, Info, Load, Import, Export, History, Ignore, Search, Undo, Redo}) {
            if (flagIsPresent<bool>(actions[entry], "--") || flagIsPresent<bool>(action_shortcuts[entry], "--") || flagIsPresent<bool>(actions.original(entry), "--")
                || flagIsPresent<bool>(action_shortcuts.original(entry), "--")) {
                return entry;
            }
        }
        auto possible_action = arguments.at(0);
        auto lowest_distance_action =
                *(std::min_element(actions.begin(), actions.end(), [&](const auto& a, const auto& b) { return levenshteinDistance(possible_action, a) < levenshteinDistance(possible_action, b); }));
        auto lowest_distance_for_action = levenshteinDistance(possible_action, lowest_distance_action);
        auto lowest_distance_shortcut = *(std::min_element(action_shortcuts.begin(), action_shortcuts.end(), [&](const auto& a, const auto& b) {
            return levenshteinDistance(possible_action, a) < levenshteinDistance(possible_action, b);
        }));
        auto lowest_distance_for_shortcut = levenshteinDistance(possible_action, lowest_distance_shortcut);
        auto lowest_distance = std::min(lowest_distance_for_action, lowest_distance_for_shortcut);
        auto lowest_distance_candidate = lowest_distance_for_shortcut < lowest_distance_for_action ? lowest_distance_shortcut : lowest_distance_action;
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        if (lowest_distance <= 2)
            printf(no_valid_action_with_candidate_message().data(), arguments.at(0).data(), clipboard_invocation.data(), lowest_distance_candidate.data(), clipboard_suffix.data());
        else
            printf(no_valid_action_message().data(), arguments.at(0).data(), clipboard_invocation.data());
        exit(EXIT_FAILURE);
    } else if (!is_tty.in) {
        return Copy;
    } else if (!is_tty.out) {
        return Paste;
    }
    return Status;
}

IOType getIOType() {
    using enum Action;
    using enum IOType;
    if (action_is_one_of(Cut, Copy, Add)) {
        if (copying.items.size() >= 1 && std::all_of(copying.items.begin(), copying.items.end(), [](const auto& item) { return !fs::exists(item); })) return Text;
        if (!is_tty.in && copying.items.empty()) return Pipe;
    } else if (action_is_one_of(Paste, Show, Clear, Edit, Status, Info, History, Search)) {
        if (!is_tty.out) return Pipe;
        return Text;
    } else if (action_is_one_of(Remove, Note, Ignore, Swap, Load, Import, Export)) {
        if (!is_tty.in && copying.items.empty()) return Pipe;
        return Text;
    }
    return File;
}

void setFlags() {
    if (flagIsPresent<bool>("--all") || flagIsPresent<bool>("-a")) all_option = true;
    if (flagIsPresent<bool>("--fast-copy") || flagIsPresent<bool>("-fc")) copying.use_safe_copy = false;
    if (auto flag = flagIsPresent<std::string>("--mime"); flag != "") preferred_mime = flag;
    if (auto flag = flagIsPresent<std::string>("-m"); flag != "") preferred_mime = flag;
    if (flagIsPresent<bool>("--no-progress") || flagIsPresent<bool>("-np")) progress_silent = true;
    if (flagIsPresent<bool>("--no-confirmation") || flagIsPresent<bool>("-nc")) confirmation_silent = true;
    if (flagIsPresent<bool>("--bachata")) {
        printf("%s", formatColors("[info]Here's some nice bachata music from Aventura! [help]https://www.youtube.com/watch?v=RxIM2bMBhCo\n[blank]").data());
        printf("%s", formatColors("[info]How about some in English? [help]https://www.youtube.com/watch?v=jnD8Av4Dl4o\n[blank]").data());
        printf("%s", formatColors("[info]Here's one from Romeo, the head of Aventura: [help]https://www.youtube.com/watch?v=yjdHGmRKz08\n[blank]").data());
        printf("%s", formatColors("[info]This one isn't bachata but it is from Aventura: [help]https://youtu.be/Lg_Pn45gyMs\n[blank]").data());
        printf("%s", formatColors("[info]How about this from Antony Santos, AKA El Mayimbe or El Bachatú?: [help]https://www.youtube.com/watch?v=gDYhGBy6304\n[blank]").data());
        exit(EXIT_SUCCESS);
    }
    if (auto flag = flagIsPresent<std::string>("-c"); flag != "") clipboard_name = flag;
    if (auto flag = flagIsPresent<std::string>("--clipboard"); flag != "") clipboard_name = flag;
    if (auto flag = flagIsPresent<std::string>("-e"); flag != "") try {
            clipboard_entry = std::stoul(flag);
        } catch (...) {}
    if (auto flag = flagIsPresent<std::string>("--entry"); flag != "") try {
            clipboard_entry = std::stoul(flag);
        } catch (...) {}
    if (flagIsPresent<bool>("-h") || flagIsPresent<bool>("help", "--")) {
        auto longestAction = columnLength(*(std::max_element(actions.begin(), actions.end(), [](const auto& a, const auto& b) { return columnLength(a) < columnLength(b); })));
        auto longestActionShortcut = columnLength(*std::max_element(action_shortcuts.begin(), action_shortcuts.end(), [](const auto& a, const auto& b) { return columnLength(a) < columnLength(b); }));
        std::string actionsList;
        for (int i = 0; i < actions.size(); i++) {
            actionsList.append("[progress]┃ ")
                    .append(actions.at(i))
                    .append(", ")
                    .append(repeatString(" ", longestAction - columnLength(actions.at(i))))
                    .append(action_shortcuts[static_cast<Action>(i)])
                    .append(repeatString(" ", longestActionShortcut - columnLength(action_shortcuts[static_cast<Action>(i)])))
                    .append("│ [help]")
                    .append(action_descriptions[static_cast<Action>(i)])
                    .append("[blank]\n");
        }
        printf(help_message().data(), constants.clipboard_version.data(), constants.clipboard_commit.data(), formatColors(actionsList).data());
        exit(EXIT_SUCCESS);
    }
}

void verifyAction() {
    if (io_type == IOType::Pipe && arguments.size() >= 2 && action != Action::Show) {
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        fprintf(stderr, redirection_no_items_message().data(), clipboard_invocation.data());
        exit(EXIT_FAILURE);
    }
}

void setFilepaths() {
    global_path.temporary = (getenv("CLIPBOARD_TMPDIR")  ? getenv("CLIPBOARD_TMPDIR")
                             : getenv("XDG_RUNTIME_DIR") ? getenv("XDG_RUNTIME_DIR")
                                                         : fs::temp_directory_path())
                            / constants.temporary_directory_name;

    global_path.persistent =
            (getenv("CLIPBOARD_PERSISTDIR") ? getenv("CLIPBOARD_PERSISTDIR") : (getenv("XDG_STATE_HOME") ? getenv("XDG_STATE_HOME") : global_path.home)) / constants.persistent_directory_name;

    path = Clipboard(clipboard_name, clipboard_entry);
}

void fixMissingItems() {
    using enum Action;
    if (action_is_one_of(Cut, Copy, Add) && io_type == IOType::File) {
        for (auto& item : copying.items) {
            if (fs::exists(item)) continue;
            std::vector<std::string> candidates;
            for (const auto& entry : fs::directory_iterator(item.parent_path().empty() ? fs::current_path() : item.parent_path()))
                candidates.emplace_back(entry.path().filename().string());
            auto closestCandidate = *(std::min_element(candidates.begin(), candidates.end(), [&](const auto& a, const auto& b) {
                return levenshteinDistance(a, item.filename().string()) < levenshteinDistance(b, item.filename().string());
            }));
            auto closestScore = levenshteinDistance(closestCandidate, item.filename().string());
            if (closestScore >= 3) continue;
            stopIndicator();
            fprintf(stderr,
                    formatColors(
                            "[progress]⬤ CB couldn't find the item [bold]%s[nobold], but did find a similar one named [bold]%s[nobold]. Would you like to use that one instead? [bold][y(es)/n(o)] "
                    )
                            .data(),
                    item.filename().string().data(),
                    closestCandidate.data());
            std::string decision;
            std::getline(std::cin, decision);
            fprintf(stderr, "%s", formatColors("[blank]").data());
            startIndicator();
            if (decision == "y" || decision == "yes") {
                auto index = std::distance(copying.items.begin(), std::find(copying.items.begin(), copying.items.end(), item));
                copying.items.at(index) = item.parent_path().empty() ? fs::path(closestCandidate) : item.parent_path() / closestCandidate;
            }
        }
    }
}

void checkForNoItems() {
    using enum Action;
    if (action_is_one_of(Cut, Copy, Add, Remove) && io_type != IOType::Pipe && copying.items.size() < 1) {
        error_exit(choose_action_items_message(), actions[action], actions[action], clipboard_invocation, actions[action]);
    }
    if (((action_is_one_of(Paste, Show) || (action == Clear && !all_option))) && (!fs::exists(path.data) || fs::is_empty(path.data))) {
        PerformAction::status();
        exit(EXIT_SUCCESS);
    }
}

unsigned long long totalItemSize() {
    unsigned long long total_item_size = 0;
    using enum Action;
    if (action_is_one_of(Cut, Copy, Add) && io_type == IOType::File) {
        for (const auto& item : copying.items) {
            try {
                if (fs::is_directory(item))
                    total_item_size += totalDirectorySize(item);
                else
                    total_item_size += fs::is_directory(item) ? directoryOverhead(item) : fs::file_size(item);
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(item.string(), e.code());
            }
        }
    } else if (action == Action::Paste && io_type == IOType::File)
        total_item_size += totalDirectorySize(path.data);
    return total_item_size;
}

void checkItemSize(unsigned long long total_item_size) {
    unsigned long long space_available = 0;
    using enum Action;
    if (action_is_one_of(Cut, Copy, Add) && io_type == IOType::File)
        space_available = fs::space(path).available;
    else if (action == Action::Paste && io_type == IOType::File)
        space_available = fs::space(fs::current_path()).available;
    if (total_item_size > space_available) {
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        fprintf(stderr, not_enough_storage_message().data(), actions[action].data(), total_item_size / (1024.0 * 1024.0), space_available / (1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }
}

void removeOldFiles() {
    if (!fs::is_regular_file(path.metadata.originals)) return;
    std::ifstream files(path.metadata.originals);
    std::string line;
    while (std::getline(files, line)) {
        try {
            fs::remove_all(line);
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(line, e.code());
        }
    }
    files.close();
    if (copying.failedItems.empty()) fs::remove(path.metadata.originals);
}

void performAction() {
    using enum IOType;
    using enum Action;
    using namespace PerformAction;
    auto complainAboutMissingAction = [&](const std::string_view& io_type_name) {
        error_exit(formatColors("[error][inverse] ✘ [noinverse] Error! CB is trying to do an action that doesn't exist yet: action name %s, IO type %s[blank]\n"), actions[action], io_type_name);
    };
    if (io_type == File) {
        if (action == Copy || action == Cut)
            copy();
        else if (action == Add)
            addFiles();
        else
            complainAboutMissingAction("file");
    } else if (io_type == Pipe) {
        if (action == Copy || action == Cut)
            pipeIn();
        else if (action == Paste)
            pipeOut();
        else if (action == Add)
            addData();
        else if (action == Note)
            notePipe();
        else if (action == Show)
            showFilepaths();
        else if (action == Info)
            infoJSON();
        else if (action == Remove)
            removeRegex();
        else if (action == Ignore)
            ignoreRegex();
        else if (action == Status)
            statusJSON();
        else if (action == Load)
            load();
        else if (action == History)
            historyJSON();
        else if (action == Search)
            searchJSON();
        else
            complainAboutMissingAction("pipe");
    } else if (io_type == Text) {
        if (action == Copy || action == Cut)
            copyText();
        else if (action == Add)
            addData();
        else if (action == Remove)
            removeRegex();
        else if (action == Note)
            noteText();
        else if (action == Info)
            info();
        else if (action == Ignore)
            ignoreRegex();
        else if (action == Import)
            importClipboards();
        else if (action == Export)
            exportClipboards();
        else if (action == Status)
            status();
        else if (action == Load)
            load();
        else if (action == Swap)
            swap();
        else if (action == Edit)
            edit();
        else if (action == Paste)
            paste();
        else if (action == Clear)
            clear();
        else if (action == Show)
            show();
        else if (action == History)
            history();
        else if (action == Search)
            search();
        else
            complainAboutMissingAction("text");
    }
}

std::string getMIMEType() {
    if (io_type == IOType::File) {
        return "text/uri-list";
    } else if (io_type == IOType::Pipe || io_type == IOType::Text) {
        return std::string(inferMIMEType(copying.buffer).value_or("text/plain"));
    }
    return "text/plain";
}

void showFailures() {
    if (copying.failedItems.size() <= 0) return;
    TerminalSize available(thisTerminalSize());
    available.rows -= columnLength(clipboard_failed_many_message) / available.columns;

    if (copying.failedItems.size() > available.rows) available.rows -= columnLength(and_more_fails_message) / available.columns;

    available.rows -= 3;
    printf(copying.failedItems.size() > 1 ? clipboard_failed_many_message().data() : clipboard_failed_one_message().data(), actions[action].data());
    for (size_t i = 0; i < std::min(available.rows, copying.failedItems.size()); i++) {
        printf(formatColors("[error]┃ [bold]%s[blank][error]: %s[blank]\n").data(), copying.failedItems.at(i).first.data(), copying.failedItems.at(i).second.message().data());
        if (i == available.rows - 1 && copying.failedItems.size() > available.rows) printf(and_more_fails_message().data(), int(copying.failedItems.size() - available.rows));
    }
    printf("%s", fix_problem_message().data());
}

void showSuccesses() {
    if (output_silent || confirmation_silent || !is_tty.err) return;
    if (successes.bytes > 0 && is_tty.err) {
        fprintf(stderr, byte_success_message().data(), did_action[action].data(), formatBytes(successes.bytes.load()).data());
    } else if ((successes.files == 1 && successes.directories == 0) || (successes.files == 0 && successes.directories == 1)) {
        fprintf(stderr, one_item_success_message().data(), did_action[action].data());
    } else if (successes.clipboards == 1) {
        fprintf(stderr, one_clipboard_success_message().data(), did_action[action].data(), successes.clipboards.load());
    } else if (successes.clipboards > 1) {
        fprintf(stderr, many_clipboards_success_message().data(), did_action[action].data(), successes.clipboards.load());
    } else {
        if ((successes.files > 1) && (successes.directories == 0))
            fprintf(stderr, many_files_success_message().data(), did_action[action].data(), successes.files.load());
        else if ((successes.files == 0) && (successes.directories > 1))
            fprintf(stderr, many_directories_success_message().data(), did_action[action].data(), successes.directories.load());
        else if ((successes.files == 1) && (successes.directories == 1))
            fprintf(stderr, one_file_one_directory_success_message().data(), did_action[action].data());
        else if ((successes.files > 1) && (successes.directories == 1))
            fprintf(stderr, many_files_one_directory_success_message().data(), did_action[action].data(), successes.files.load());
        else if ((successes.files == 1) && (successes.directories > 1))
            fprintf(stderr, one_file_many_directories_success_message().data(), did_action[action].data(), successes.directories.load());
        else if ((successes.files > 1) && (successes.directories > 1))
            fprintf(stderr, many_files_many_directories_success_message().data(), did_action[action].data(), successes.files.load(), successes.directories.load());
    }
}
