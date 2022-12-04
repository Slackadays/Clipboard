#include <vector>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <utility>
#include <string_view>
#include <locale>
#include <iostream>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#define FMT_HEADER_ONLY
#include "fmt/format.h"

namespace fs = std::filesystem;

fs::path filepath;

enum class Action { Cut, Copy, Paste, PipeIn, PipeOut };
Action action;

std::vector<fs::path> items;

unsigned long files_success = 0;
unsigned long directories_success = 0;
unsigned long long bytes_success = 0;

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

std::string_view copied_action = "Copied";
std::string_view cut_past_action = "Cut";
std::string_view pasted_action = "Pasted";
std::string_view pipedin_action = "Piped in";
std::string_view pipedout_action = "Piped out";

std::string_view help_message = "{blue}▏This is Clipboard {version}, the {cut}, {copy}, and {paste} system for the command line.{blank}\n"
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
std::string_view no_action_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}{cut}, {copy}, or {paste}{blank}{pink} to the end, like {bold}clipboard {copy}{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
std::string_view choose_action_items_message = "{red}╳ You need to choose something to {action}.{pink} Try adding the items you want to {action} to the end, like {bold}clipboard {action} contacts.txt myprogram.cpp{blank}\n";
std::string_view fix_redirection_action_message = "{red}╳ You can't use the {bold}{badaction}{blank}{red} action with redirection here. {pink}Try removing {bold}{badaction}{blank}{pink} or use {bold}{goodaction}{blank}{pink} instead, like {bold}clipboard {goodaction}{blank}{pink}.\n";
std::string_view copying_message = "{yellow}• Copying...{blank}\r";
std::string_view cutting_message = "{yellow}• Cutting...{blank}\r";
std::string_view pasting_message = "{yellow}• Pasting...{blank}\r";
std::string_view pipingin_message = "{yellow}• Piping in...{blank}\r";
std::string_view pipingout_message = "{yellow}• Piping out...{blank}\r";
std::string_view paste_success_message = "{green}√ Pasted successfully{blank}\n";
std::string_view paste_fail_message = "{red}╳ Failed to paste{blank}\n";
std::string_view clipboard_failed_message = "{red}╳ Clipboard couldn't {action} these items.{blank}\n";
std::string_view and_more_message = "{red}▏ ...and {num} more.{blank}\n";
std::string_view fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                                       "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
std::string_view pipe_success_message = "{green}√ {actioned} {num} bytes{blank}\n";
std::string_view one_item_success_message = "{green}√ {actioned} {item}{blank}\n";
std::string_view multiple_files_success_message = "{green}√ {actioned} {num} files{blank}\n";
std::string_view multiple_directories_success_message = "{green}√ {actioned} {num} directories{blank}\n";
std::string_view multiple_files_directories_success_message = "{green}√ {actioned} {numfiles} files and {numdirs} directories{blank}\n";
std::string_view internal_error_message = "{red}╳ Internal error: {error}\n▏ This is probably a bug.{blank}\n";

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
        red = "";
        green = "";
        yellow = "";
        blue = "";
        pink = "";
        orange = "";
        bold = "";
        blank = "";
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
            fmt::vprint(help_message, fmt::make_format_args(fmt::arg("blue", blue), fmt::arg("version", clipboard_version), fmt::arg("blank", blank), fmt::arg("bold", bold), fmt::arg("orange", orange), fmt::arg("cut", cut_action), fmt::arg("copy", copy_action), fmt::arg("paste", paste_action)));
            exit(0);
        }
    }
    if (argc >= 2) {
        if (!strcmp(argv[1], "help")) {
            fmt::vprint(help_message, fmt::make_format_args(fmt::arg("blue", blue), fmt::arg("version", clipboard_version), fmt::arg("blank", blank), fmt::arg("bold", bold), fmt::arg("orange", orange)));
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
                fmt::vprint(stderr, fix_redirection_action_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("bold", bold), fmt::arg("badaction", cut_action), fmt::arg("goodaction", copy_action), fmt::arg("blank", blank), fmt::arg("pink", pink)));
                exit(1);
            }
        } else if (!strncmp(argv[1], copy_action.data(), 2)) {
            if (isatty(fileno(stdin))) {
                action = Action::Copy;
            } else if (!isatty(fileno(stdin))) {
                action = Action::PipeIn;
            } else {
                fmt::vprint(stderr, fix_redirection_action_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("bold", bold), fmt::arg("badaction", copy_action), fmt::arg("goodaction", paste_action), fmt::arg("blank", blank), fmt::arg("pink", pink)));
                exit(1);
            }
        } else if (!strncmp(argv[1], paste_action.data(), 1)) {
            if (isatty(fileno(stdin)) && isatty(fileno(stdout))) {
                action = Action::Paste;
            } else if (!isatty(fileno(stdout))) {
                action = Action::PipeOut;
            } else {
                fmt::vprint(stderr, fix_redirection_action_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("bold", bold), fmt::arg("badaction", paste_action), fmt::arg("goodaction", copy_action), fmt::arg("blank", blank), fmt::arg("pink", pink)));
                exit(1);
            }
        } else {
            fmt::vprint(no_valid_action_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("pink", pink), fmt::arg("bold", bold), fmt::arg("blank", blank)));
            exit(1);
        }
    } else if (!isatty(fileno(stdin))) {
        action = Action::PipeIn;
    } else if (!isatty(fileno(stdout))) {
        action = Action::PipeOut;
    } else {
        fmt::vprint(no_action_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("pink", pink), fmt::arg("bold", bold), fmt::arg("blank", blank), fmt::arg("cut", cut_action), fmt::arg("copy", copy_action), fmt::arg("paste", paste_action)));
        exit(1);
    }
    if (action == Action::PipeIn || action == Action::PipeOut) {
        if (argc >= 3) {
            fmt::vprint(stderr, "{red}╳ You can't specify items when you use redirection. {pink}Try removing the items that come after {bold}clipboard [action].\n", fmt::make_format_args(fmt::arg("red", red), fmt::arg("pink", pink), fmt::arg("bold", bold)));
            exit(1);
        }
    }
}

void checkForNoItems() {
    if ((action == Action::Cut || action == Action::Copy) && items.size() < 1) {
        if (action == Action::Copy) {
            fmt::vprint(choose_action_items_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("action", copy_action), fmt::arg("pink", pink), fmt::arg("bold", bold), fmt::arg("blank", blank)));
        } else if (action == Action::Cut) {
            fmt::vprint(choose_action_items_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("action", cut_action), fmt::arg("pink", pink), fmt::arg("bold", bold), fmt::arg("blank", blank)));
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
        fmt::vprint(copying_message, fmt::make_format_args(fmt::arg("yellow", yellow), fmt::arg("blank", blank)));
    } else if (action == Action::Cut) {
        fmt::vprint(cutting_message, fmt::make_format_args(fmt::arg("yellow", yellow), fmt::arg("blank", blank)));
    } else if (action == Action::Paste) {
        fmt::vprint(pasting_message, fmt::make_format_args(fmt::arg("yellow", yellow), fmt::arg("blank", blank)));
    } else if (action == Action::PipeIn) {
        fmt::vprint(stderr, pipingin_message, fmt::make_format_args(fmt::arg("yellow", yellow), fmt::arg("blank", blank)));
    } else if (action == Action::PipeOut) {
        fmt::vprint(stderr, pipingout_message, fmt::make_format_args(fmt::arg("yellow", yellow), fmt::arg("blank", blank)));
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
            fmt::vprint(paste_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("blank", blank)));
        } catch (const fs::filesystem_error& e) {
            fmt::vprint(paste_fail_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("blank", blank)));
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
        fmt::vprint(clipboard_failed_message, fmt::make_format_args(fmt::arg("action", action == Action::Copy ? copy_action : cut_action), fmt::arg("red", red), fmt::arg("blank", blank)));
        for (int i = 0; i < std::min(5, int(failedItems.size())); i++) {
            fmt::vprint("{red}▏ {item}: {error}{blank}", fmt::make_format_args(fmt::arg("red", red), fmt::arg("error", failedItems.at(i).first.string()), fmt::arg("error", failedItems.at(i).second.code().message()), fmt::arg("blank", blank)));
            if (i == 4 && failedItems.size() > 5) {
                fmt::vprint(and_more_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("num", int(failedItems.size() - 5)), fmt::arg("blank", blank)));
            }
        }
        fmt::vprint(fix_problem_message, fmt::make_format_args(fmt::arg("pink", pink), fmt::arg("blank", blank)));
    }
    for (const auto& f : failedItems) {
        items.erase(std::remove(items.begin(), items.end(), f.first), items.end());
    }
}

void showSuccesses() {
    if (bytes_success > 0) {
        fmt::vprint(stderr, pipe_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("num", bytes_success), fmt::arg("blank", blank), fmt::arg("actioned", action == Action::PipeIn ? pipedin_action : pipedout_action)));
        return;
    }
    if ((files_success == 1 && directories_success == 0) || (files_success == 0 && directories_success == 1)) {
        fmt::vprint(one_item_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("item", items.at(0).string()), fmt::arg("blank", blank), fmt::arg("actioned", action == Action::Copy ? copied_action : cut_past_action)));
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            fmt::vprint(multiple_files_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("num", files_success), fmt::arg("blank", blank), fmt::arg("actioned", action == Action::Copy ? copied_action : cut_past_action)));
        } else if ((files_success == 0) && (directories_success > 1)) {
            fmt::vprint(multiple_directories_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("num", directories_success), fmt::arg("blank", blank), fmt::arg("actioned", action == Action::Copy ? copied_action : cut_past_action)));
        } else if ((files_success >= 1) && (directories_success >= 1)) {
            fmt::vprint(multiple_files_directories_success_message, fmt::make_format_args(fmt::arg("green", green), fmt::arg("numfiles", files_success), fmt::arg("numdirs", directories_success), fmt::arg("blank", blank), fmt::arg("actioned", action == Action::Copy ? copied_action : cut_past_action)));
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
        fmt::vprint(stderr, internal_error_message, fmt::make_format_args(fmt::arg("red", red), fmt::arg("error", e.what()), fmt::arg("blank", blank)));
        exit(1);
    }
    return 0;
}