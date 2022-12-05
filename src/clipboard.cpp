#include <vector>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <utility>
#include <string_view>
#include <locale>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <array>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

fs::path filepath;

enum class Action { Cut, Copy, Paste, PipeIn, PipeOut };
Action action;

std::vector<fs::path> items;

unsigned long files_success = 0;
unsigned long directories_success = 0;
unsigned long long bytes_success = 0;

constexpr std::string_view clipboard_version = "0.1.2";

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

std::unordered_map<Action, std::string_view> actions = {
    {Action::Cut, "cut"},
    {Action::Copy, "copy"},
    {Action::Paste, "paste"},
    {Action::PipeIn, "pipe in"},
    {Action::PipeOut, "pipe out"},
};

std::unordered_map<Action, std::string_view> doing_action = {
    {Action::Cut, "Cutting"},
    {Action::Copy, "Copying"},
    {Action::Paste, "Pasting"},
    {Action::PipeIn, "Piping in"},
    {Action::PipeOut, "Piping out"}
};

std::unordered_map<Action, std::string_view> did_action = {
    {Action::Cut, "Cut"},
    {Action::Copy, "Copied"},
    {Action::Paste, "Pasted"},
    {Action::PipeIn, "Piped in"},
    {Action::PipeOut, "Piped out"}
};

std::string_view help_message = "{blue}▏This is Clipboard %s, the cut, copy, and paste system for the command line.{blank}\n"
                                "{blue}{bold}▏How To Use{blank}\n"
                                "{orange}▏clipboard cut (item) [items]{blank}\n"
                                "{orange}▏clipboard copy (item) [items]{blank}\n"
                                "{orange}▏clipboard paste{blank}\n"
                                "{blue}▏You can substitute \"cb\" for \"clipboard\" to save time.{blank}\n"
                                "{blue}{bold}▏Examples{blank}\n"
                                "{orange}▏clipboard copy dogfood.conf{blank}\n"
                                "{orange}▏cb cut Nuclear_Launch_Codes.txt contactsfolder{blank}\n"
                                "{orange}▏cb paste{blank}\n"
                                "{blue}▏You can show this help screen anytime with {bold}clipboard -h{blank}{blue}, {bold}clipboard --help{blank}{blue}, or{bold} clipboard help{blank}{blue}.\n"
                                "{blue}▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.{blank}\n"
                                "{blue}▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.{blank}\n";
std::string_view no_valid_action_message = "{red}╳ You did not specify a valid action, or you forgot to include one. {pink}Try using or adding {bold}cut, copy, or paste{blank}{pink} instead, like {bold}clipboard copy.{blank}\n";
std::string_view no_action_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}%s, %s, or %s{blank}{pink} to the end, like {bold}clipboard %s{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
std::string_view choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
std::string_view fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
std::string_view paste_success_message = "{green}√ Pasted successfully{blank}\n";
std::string_view paste_fail_message = "{red}╳ Failed to paste{blank}\n";
std::string_view clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items.{blank}\n";
std::string_view and_more_message = "{red}▏ ...and %i more.{blank}\n";
std::string_view fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                                       "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
std::string_view working_message = "{yellow}• %s...{blank}\r";
std::string_view pipe_success_message = "{green}√ %s %i bytes{blank}\n";
std::string_view one_item_success_message = "{green}√ %s %s{blank}\n";
std::string_view multiple_files_success_message = "{green}√ %s %i files{blank}\n";
std::string_view multiple_directories_success_message = "{green}√ %s %i directories{blank}\n";
std::string_view multiple_files_directories_success_message = "{green}√ %s %i files and %i directories{blank}\n";
std::string_view internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug.{blank}\n";

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

void setupVariables(const int argc, char *argv[]) {
    filepath = fs::temp_directory_path() / "Clipboard";

    for (int i = 2; i < argc; i++) {
        items.emplace_back(argv[i]);
    }

    if (getenv("NO_COLOR") != nullptr) {
        for (auto& key : colors) {
            key.second = "";
        }
    }

    if (std::locale("").name().substr(0, 2) == "es") {
        setLanguageES();
    } else if (std::locale("").name().substr(0, 2) == "pt") {
        setLanguagePT();
    }
}

void checkFlags(const int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || (argc >= 2 && !strcmp(argv[1], "help"))) {
            printf(replaceColors(help_message).data(), clipboard_version.data());
            exit(0);
        }
    }
}

void setupAction(const int argc, char *argv[]) {
    if (argc >= 2) {
        if (!strncmp(argv[1], actions[Action::Cut].data(), 2)) {
            action = Action::Cut;
            if (!isatty(fileno(stdin)) || !isatty(fileno(stdout))) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Copy].data(), actions[Action::Copy].data());
                exit(1);
            }
        } else if (!strncmp(argv[1], actions[Action::Copy].data(), 2)) {
            action = Action::Copy;
            if (!isatty(fileno(stdin))) {
                action = Action::PipeIn;
            } else if (!isatty(fileno(stdout))) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Paste].data(), actions[Action::Paste].data());
                exit(1);
            }
        } else if (!strncmp(argv[1], actions[Action::Paste].data(), 1)) {
            action = Action::Paste;
            if (!isatty(fileno(stdout))) {
                action = Action::PipeOut;
            } else if (!isatty(fileno(stdin)))) {
                fprintf(stderr, replaceColors(fix_redirection_action_message).data(), actions[action].data(), actions[action].data(), actions[Action::Copy].data(), actions[Action::Copy].data());
                exit(1);
            }
        } else {
            printf("%s", replaceColors(no_valid_action_message).data());
            exit(1);
        }
    } else if (!isatty(fileno(stdin))) {
        action = Action::PipeIn;
    } else if (!isatty(fileno(stdout))) {
        action = Action::PipeOut;
    } else {
        printf(replaceColors(no_action_message).data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
        exit(1);
    }
    if (action == Action::PipeIn || action == Action::PipeOut) {
        if (argc >= 3) {
            fprintf(stderr, "%s", replaceColors("{red}╳ You can't specify items when you use redirection. {pink}Try removing the items that come after {bold}clipboard [action].\n").data());
            exit(1);
        }
    }
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && items.size() < 1) {
        printf(replaceColors(choose_action_items_message).data(), actions[action].data(), actions[action].data(), actions[action].data());
        exit(1);
    }
}

void setupTempDirectory() {
    if (fs::is_directory(filepath)) {
        if (action != Action::Paste && action != Action::PipeOut) {
            for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
                fs::remove_all(entry.path());
            }
        }
    } else {
        fs::create_directories(filepath);
    }
}

void performAction() {
    std::vector<std::pair<fs::path, fs::filesystem_error>> failedItems;
    fs::copy_options opts = fs::copy_options::recursive | fs::copy_options::copy_symlinks | fs::copy_options::overwrite_existing;
    if (action == Action::Copy) {
        for (const auto& f : items) {
            try {
                if (fs::is_directory(f)) {
                    if (f.filename() == "") {
                        fs::create_directories(filepath / f.parent_path().filename());
                        fs::copy(f, filepath / f.parent_path().filename(), opts);
                    } else {
                        fs::create_directories(filepath / f.filename());
                        fs::copy(f, filepath / f.filename(), opts);
                    }
                    directories_success++;
                } else {
                    fs::copy(f, filepath / f.filename(), opts);
                    files_success++;
                }
            } catch (const fs::filesystem_error& e) {
                failedItems.emplace_back(f, e);
            }
        }
    } else if (action == Action::Cut) {
        for (const auto& f : items) {
            try {
                if (fs::is_directory(f)) {
                    fs::create_directories(filepath / f.parent_path().filename());
                    fs::rename(f, filepath / f.parent_path().filename());
                    directories_success++;
                } else {
                    fs::rename(f, filepath / f.filename());
                    files_success++;
                }
            } catch (const fs::filesystem_error& e) {
                failedItems.emplace_back(f, e);
            }
        }  
    } else if (action == Action::Paste) {
        try {
            fs::copy(filepath, fs::current_path(), opts);
            printf("%s", replaceColors(paste_success_message).data());
        } catch (const fs::filesystem_error& e) {
            printf("%s", replaceColors(paste_fail_message).data());
        }
    } else if (action == Action::PipeIn) {
        std::ofstream file(filepath / "clipboard.txt");
        std::string line;
        while (std::getline(std::cin, line)) {
            file << line << std::endl;
            bytes_success += line.size();
        }
        file.close();
    } else if (action == Action::PipeOut) {
        std::ifstream file(filepath / "clipboard.txt");
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::flush;
            bytes_success += line.size();
        }
        file.close();
    }
    if (failedItems.size() > 0) {
        printf(replaceColors(clipboard_failed_message).data(), actions[action].data());
        for (int i = 0; i < std::min(5, int(failedItems.size())); i++) {
            printf(replaceColors("{red}▏ %s: %s{blank}\n").data(), failedItems.at(i).first.string().data(), failedItems.at(i).second.code().message().data());
            if (i == 4 && failedItems.size() > 5) {
                printf(replaceColors(and_more_message).data(), int(failedItems.size() - 5));
            }
        }
        printf("%s", replaceColors(fix_problem_message).data());
    }
    for (const auto& f : failedItems) {
        items.erase(std::remove(items.begin(), items.end(), f.first), items.end());
    }
}

void showSuccesses() {
    if (action == Action::PipeIn || action == Action::PipeOut) {
        fprintf(stderr, replaceColors(pipe_success_message).data(), did_action[action].data(), bytes_success);
        return;
    }
    if ((files_success == 1 && directories_success == 0) || (files_success == 0 && directories_success == 1)) {
        printf(replaceColors(one_item_success_message).data(), did_action[action].data(), items.at(0).string().data());
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
        setupVariables(argc, argv);

        checkFlags(argc, argv);

        setupAction(argc, argv);

        checkForNoItems();

        printf(replaceColors(working_message).data(), doing_action[action].data()); //action indicator
        fflush(stdout);

        setupTempDirectory();

        performAction();

        showSuccesses();
    } catch (const std::exception& e) {
        fprintf(stderr, replaceColors(internal_error_message).data(), e.what());
        exit(1);
    }
    return 0;
}