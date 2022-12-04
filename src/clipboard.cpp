#include <vector>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <utility>
#include <string_view>
#include <locale>
#include <iostream>
#include <fstream>
#include <string>

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

bool colors = true;

std::string_view clipboard_version = "0.1.2";

std::string_view red = "\033[38;5;196m";
std::string_view green = "\033[38;5;40m";
std::string_view yellow = "\033[38;5;214m";
std::string_view orange = "\033[38;5;208m";
std::string_view blue = "\033[38;5;51m";
std::string_view pink = "\033[38;5;219m";
std::string_view bold = "\033[1m";
std::string_view blank = "\033[0m";

std::string_view copy_action = "copy";
std::string_view cut_action = "cut";
std::string_view paste_action = "paste";
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
std::string_view no_action_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}cut, copy, or paste{blank}{pink} to the end, like {bold}clipboard copy{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
std::string_view choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
std::string_view fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
std::string_view copying_message = "{yellow}• Copying...{blank}\r";
std::string_view cutting_message = "{yellow}• Cutting...{blank}\r";
std::string_view pasting_message = "{yellow}• Pasting...{blank}\r";
std::string_view pipingin_message = "{yellow}• Piping in...{blank}\r";
std::string_view pipingout_message = "{yellow}• Piping out...{blank}\r";
std::string_view paste_success_message = "{green}√ Pasted successfully{blank}\n";
std::string_view paste_fail_message = "{red}╳ Failed to paste{blank}\n";
std::string_view clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items.{blank}\n";
std::string_view and_more_message = "{red}▏ ...and %d more.{blank}\n";
std::string_view fix_problem_message = "{pink}▏ See if you have the needed permissions, or{blank}\n"
                                       "{pink}▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
std::string_view pipein_success_message = "{green}√ Piped in %i bytes{blank}\n";
std::string_view pipeout_success_message = "{green}√ Piped out %i bytes{blank}\n";
std::string_view copied_one_item_message = "{green}√ Copied %s{blank}\n";
std::string_view cut_one_item_message = "{green}√ Cut %s{blank}\n";
std::string_view copied_multiple_files_message = "{green}√ Copied %i files{blank}\n";
std::string_view cut_multiple_files_message = "{green}√ Cut %i files{blank}\n";
std::string_view copied_multiple_directories_message = "{green}√ Copied %i directories{blank}\n";
std::string_view cut_multiple_directories_message = "{green}√ Cut %i directories{blank}\n";
std::string_view copied_multiple_files_directories_message = "{green}√ Copied %i files and %i directories{blank}\n";
std::string_view cut_multiple_files_directories_message = "{green}√ Cut %i files and %i directories{blank}\n";
std::string_view internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug.{blank}\n";

std::string_view trimColors(const std::string_view& str) {
    std::string result{str};
    if (colors) {
        return str;
    } else {
        for (int i = 0; (i = str.find("{red}", i)) != std::string::npos;) {
            result.replace(i, 5, "");
        }
        for (int i = 0; (i = str.find("{green}", i)) != std::string::npos;) {
            result.replace(i, 7, "");
        }
        for (int i = 0; (i = str.find("{yellow}", i)) != std::string::npos;) {
            result.replace(i, 8, "");
        }
        for (int i = 0; (i = str.find("{blue}", i)) != std::string::npos;) {
            result.replace(i, 6, "");
        }
        for (int i = 0; (i = str.find("{pink}", i)) != std::string::npos;) {
            result.replace(i, 6, "");
        }
        for (int i = 0; (i = str.find("{orange}", i)) != std::string::npos;) {
            result.replace(i, 8, "");
        }
        for (int i = 0; (i = str.find("{bold}", i)) != std::string::npos;) {
            result.replace(i, 6, "");
        }
        for (int i = 0; (i = str.find("{blank}", i)) != std::string::npos;) {
            result.replace(i, 7, "");
        }
        return std::string_view{result};
    }
}

void setLanguageES() {

}

void setLanguagePT() {

}

void setupVariables(const int argc, char *argv[]) {
    filepath = fs::temp_directory_path() / "Clipboard";

    for (int i = 2; i < argc; i++) {
        items.emplace_back(argv[i]);
    }

    if (getenv("NO_COLOR") != nullptr) {
        colors = false;
    }

    if (std::locale("").name().substr(0, 3) == "es_") {
        setLanguageES();
    } else if (std::locale("").name().substr(0, 3) == "pt_") {
        setLanguagePT();
    }
}

void checkFlags(const int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf(trimColors(help_message).data(), clipboard_version.data());
            exit(0);
        }
    }
    if (argc >= 2) {
        if (!strcmp(argv[1], "help")) {
            printf(help_message.data(), clipboard_version.data());
            exit(0);
        }
    }
}

void setupAction(const int argc, char *argv[]) {
    if (argc >= 2) {
        if (!strncmp(argv[1], cut_action.data(), 2)) {
            if (isatty(fileno(stdin)) && isatty(fileno(stdout))) {
                action = Action::Cut;
            } else {
                fprintf(stderr, fix_redirection_action_message.data(), cut_action.data(), cut_action.data(), copy_action.data(), copy_action.data());
                exit(1);
            }
        } else if (!strncmp(argv[1], copy_action.data(), 2)) {
            if (isatty(fileno(stdin))) {
                action = Action::Copy;
            } else if (!isatty(fileno(stdin))) {
                action = Action::PipeIn;
            } else {
                fprintf(stderr, fix_redirection_action_message.data(), copy_action.data(), copy_action.data(), paste_action.data(), paste_action.data());
                exit(1);
            }
        } else if (!strncmp(argv[1], paste_action.data(), 1)) {
            if (isatty(fileno(stdin)) && isatty(fileno(stdout))) {
                action = Action::Paste;
            } else if (!isatty(fileno(stdout))) {
                action = Action::PipeOut;
            } else {
                fprintf(stderr, fix_redirection_action_message.data(), paste_action.data(), paste_action.data(), copy_action.data(), copy_action.data());
                exit(1);
            }
        } else {
            printf("%s", no_valid_action_message.data());
            exit(1);
        }
    } else if (!isatty(fileno(stdin))) {
        action = Action::PipeIn;
    } else if (!isatty(fileno(stdout))) {
        action = Action::PipeOut;
    } else {
        printf("%s", no_action_message.data());
        exit(1);
    }
    if (action == Action::PipeIn || action == Action::PipeOut) {
        if (argc >= 3) {
            fprintf(stderr, "{red}╳ You can't specify items when you use redirection. {pink}Try removing the items that come after {bold}clipboard [action].\n");
            exit(1);
        }
    }
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && items.size() < 1) {
        if (action == Action::Copy) {
            printf(choose_action_items_message.data(), copy_action.data(), copy_action.data(), copy_action.data());
        } else if (action == Action::Cut) {
            printf(choose_action_items_message.data(), cut_action.data(), cut_action.data(), cut_action.data());
        }
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

void setupIndicator() {
    if (action == Action::Copy) {
        printf("%s", copying_message.data());
    } else if (action == Action::Cut) {
        printf("%s", cutting_message.data());
    } else if (action == Action::Paste) {
        printf("%s", pasting_message.data());
    } else if (action == Action::PipeIn) {
        fprintf(stderr, "%s", pipingin_message.data());
    } else if (action == Action::PipeOut) {
        fprintf(stderr, "%s", pipingout_message.data());
    }
    fflush(stdout);
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
            printf("%s", paste_success_message.data());
        } catch (const fs::filesystem_error& e) {
            printf("%s", paste_fail_message.data());
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
        printf(clipboard_failed_message.data(), action == Action::Copy ? copy_action.data() : cut_action.data());
        for (int i = 0; i < std::min(5, int(failedItems.size())); i++) {
            printf("{red}▏ %s: %s{blank}\n", failedItems.at(i).first.string().data(), failedItems.at(i).second.code().message().data());
            if (i == 4 && failedItems.size() > 5) {
                printf(and_more_message.data(), int(failedItems.size()) - 5);
            }
        }
        printf("%s", fix_problem_message.data());
    }
    for (const auto& f : failedItems) {
        items.erase(std::remove(items.begin(), items.end(), f.first), items.end());
    }
}

void showSuccesses() {
    if (bytes_success > 0) {
        if (action == Action::PipeIn) {
            fprintf(stderr, pipein_success_message.data(), bytes_success);
        } else if (action == Action::PipeOut) {
            fprintf(stderr, pipeout_success_message.data(), bytes_success);
        }
        return;
    }
    if ((files_success == 1 && directories_success == 0) || (files_success == 0 && directories_success == 1)) {
        if (action == Action::Copy) {
            printf(copied_one_item_message.data(), items.at(0).string().data());
        } else if (action == Action::Cut) {
            printf(cut_one_item_message.data(), items.at(0).string().data());
        }
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            if (action == Action::Copy) {
                printf(copied_multiple_files_message.data(), files_success);
            } else if (action == Action::Cut) {
                printf(cut_multiple_files_message.data(), files_success);
            }
        } else if ((files_success == 0) && (directories_success > 1)) {
            if (action == Action::Copy) {
                printf(copied_multiple_directories_message.data(), directories_success);
            } else if (action == Action::Cut) {
                printf(cut_multiple_directories_message.data(), directories_success);
            }
        } else if ((files_success >= 1) && (directories_success >= 1)) {
            if (action == Action::Copy) {
                printf(copied_multiple_files_directories_message.data(), files_success, directories_success);
            } else if (action == Action::Cut) {
                printf(cut_multiple_files_directories_message.data(), files_success, directories_success);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupVariables(argc, argv);

        checkFlags(argc, argv);

        setupAction(argc, argv);

        checkForNoItems();

        setupIndicator();

        setupTempDirectory();

        performAction();

        showSuccesses();
    } catch (const std::exception& e) {
        fprintf(stderr, internal_error_message.data(), e.what());
        exit(1);
    }
    return 0;
}