#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

fs::path filepath;

enum class Action {
    Cut,
    Copy,
    Paste
};
Action action;

std::vector<std::string> items;

unsigned int files_success = 0;
unsigned int directories_success = 0;
unsigned int files_failed = 0;
unsigned int directories_failed = 0;

void displayHelpMessage() {
    std::cout << "\033[38;5;51m▏This is Clipboard 0.1.0, the copy and paste system for the command line.\033[0m" << std::endl;
    std::cout << "\033[38;5;51m\033[1m▏How To Use\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏clipboard cut [options] (item) [items]\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏clipboard copy [options] (item) [items]\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏clipboard paste [options]\033[0m" << std::endl;
    std::cout << "\033[38;5;51m▏You can substitute \"cb\" for \"clipboard\" to save time.\033[0m" << std::endl;
    std::cout << "\033[38;5;51m\033[1m▏Examples\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏cb cut nuclearlaunchcodes.txt Contacts_Folder\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏clipboard copy dogfood.conf\033[0m" << std::endl;
    std::cout << "\033[38;5;208m▏cb paste\033[0m" << std::endl;
    std::cout << "\033[38;5;51m▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.\033[0m" << std::endl;
    std::cout << "\033[38;5;51m▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.\033[0m" << std::endl;
}

void setupVariables(int argc, char *argv[]) {
    filepath = fs::temp_directory_path() / "Clipboard/files";

    for (int i = 0; i < argc; i++) {
        items.push_back(argv[i]);
    }
}

void checkFlags() {
    for (const auto& i : items) {
        if (i == "-h" || i == "--help") {
            displayHelpMessage();
            exit(0);
        }
    }
}

void setupAction() {
    if (items.at(0) == "clipboard" || items.at(0) == "cb" || items.at(0) == "./clipboard" || items.at(0) == "./cb") {
        if (items.size() > 1) {
            if (items.at(1) == "cut") {
                action = Action::Cut;
            } else if (items.at(1) == "copy") {
                action = Action::Copy;
            } else if (items.at(1) == "paste") {
                action = Action::Paste;
            } else {
                std::cout << "\033[38;5;196m╳ You did not specify a valid action, or you forgot to include one. \033[38;5;219mTry using or adding \033[1mcut, copy, or paste\033[0m\033[38;5;219m instead, like \033[1mclipboard copy\033[0m." << std::endl;
                exit(1);
            }
            items.erase(items.begin());
            items.erase(items.begin());
        } else {
            std::cout << "\033[38;5;196m╳ You did not specify an action. \033[38;5;219mTry adding \033[1mcut, copy, or paste\033[0m\033[38;5;219m to the end, like \033[1mclipboard copy\033[0m\033[38;5;219m. If you need more help, try \033[1mclipboard -h\033[0m\033[38;5;219m to show the help screen.\033[0m" << std::endl;
            exit(1);
        }
    } else {
        throw std::runtime_error("Could not determine desired action");
    }
}

void checkForNoItems() {
    if ((action != Action::Paste) && items.size() < 1) {
        if (action == Action::Copy) {
            std::cout << "\033[38;5;196m╳ You need to choose something to copy.\033[38;5;219m Try adding the items you want to copy to the end, like \033[1mcopy contacts.txt myprogram.cpp\033[0m" << std::endl;
        } else if (action == Action::Cut) {
            std::cout << "\033[38;5;196m╳ You need to choose something to cut.\033[38;5;219m Try adding the items you want to cut to the end, like \033[1mcut contacts.txt myprogram.cpp\033[0m" << std::endl;
        }
        exit(1);
    }
}

void checkForItemExistence() {
    std::vector<std::string> missingItems;
    for (const auto& i : items) {
        if (!fs::exists(i)) {
            missingItems.push_back(i);
        }
    }
    if (missingItems.size() > 0) {
        std::cout << "\033[38;5;196m╳ The following items do not exist:\033[0m" << std::endl;
        for (const auto& i : missingItems) {
            std::cout << "\033[38;5;196m▏ " << i << "\033[0m" << std::endl;
        }
        std::cout << "\033[38;5;219m▏ Try double-checking the spelling of the files or what directory you're in.\033[0m" << std::endl;
        exit(1);
    }
}

void setupTempDirectory() {
    if (fs::is_directory(filepath)) {
        if (action != Action::Paste) {
            for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
                fs::remove_all(entry.path());
            }
        }
    } else {
        fs::create_directories(filepath);
    }
}

void performAction() {
    if (action == Action::Copy) {
        for (const auto& f : items) {
            fs::copy(f, filepath / f, fs::copy_options::recursive | fs::copy_options::copy_symlinks);
        }
    } else if (action == Action::Cut) {
        for (const auto& f : items) {
            fs::rename(f, filepath / f);
        }  
    } else if (action == Action::Paste) {
        fs::copy(filepath, fs::current_path(), fs::copy_options::recursive | fs::copy_options::copy_symlinks | fs::copy_options::overwrite_existing);
        std::cout << "\033[38;5;40m√ Pasted\033[0m" << std::endl;
    }
}

void countSuccessesAndFailures() {
    for (const auto& f : items) {
        if (action == Action::Copy) {
            if (fs::exists(filepath / f)) {
                if (fs::is_directory(f)) {
                    directories_success++;
                } else {
                    files_success++;
                }
            } else {
                if (fs::is_directory(f)) {
                    directories_failed++;
                } else {
                    files_failed++;
                }
            }
        } else if (action == Action::Cut) {
            if (fs::exists(filepath / f)) {
                if (fs::is_directory(filepath / f)) {
                directories_success++;
                } else {
                   files_success++;
                }
            } else {
                if (fs::is_directory(filepath / f)) {
                    directories_failed++;
                } else {
                    files_failed++;
                }
            }
        }
    }
}

void showSuccesses() {
    if ((files_success >= 1) != (directories_success >= 1)) {
        if (action == Action::Copy) {
            std::cout << "\033[38;5;40m√ Copied "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == Action::Cut) {
            std::cout << "\033[38;5;40m√ Cut "<< items.at(0) << "\033[0m" << std::endl;
        }
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;40m√ Copied " << files_success << " files\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;40m√ Cut " << files_success << " files\033[0m" << std::endl;
            }
        } else if ((files_success == 0) && (directories_success > 1)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;40m√ Copied " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;40m√ Cut " << directories_success << " directories\033[0m" << std::endl;
            }
        } else if ((files_success >= 1) && (directories_success >= 1)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;40m√ Copied " << files_success << " files and " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;40m√ Cut " << files_success << " files and " << directories_success << " directories\033[0m" << std::endl;
            }
        }
    }
}

void showFailures() {
    if ((files_failed >= 1) != (directories_failed >= 1)) {
        if (action == Action::Copy) {
            std::cout << "\033[38;5;196m╳\033[91m Failed to copy "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == Action::Cut) {
            std::cout << "\033[38;5;196m╳\033[91m Failed to cut "<< items.at(0) << "\033[0m" << std::endl;
        }
    } else {
        if ((files_failed > 1) && (directories_failed == 0)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to copy " << files_failed << " files\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to cut " << files_failed << " files\033[0m" << std::endl;
            }
        } else if ((files_failed == 0) && (directories_failed > 1)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to copy " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to cut " << directories_failed << " directories\033[0m" << std::endl;
            }
        } else if ((files_failed >= 1) && (directories_failed >= 1)) {
            if (action == Action::Copy) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to copy " << files_failed << " files and " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == Action::Cut) {
                std::cout << "\033[38;5;196m╳\033[91m Failed to cut " << files_failed << " files and " << directories_failed << " directories\033[0m" << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupVariables(argc, argv);

        checkFlags();

        setupAction();

        checkForNoItems();

        checkForItemExistence();

        setupTempDirectory();

        performAction();

        countSuccessesAndFailures();

        showSuccesses();

        showFailures();
    } catch(const std::exception& e) {
        std::cout << "\033[38;5;196m╳ Internal error: " << e.what() << std::endl << "▏ This is probably a bug.\033[0m" << std::endl;
        exit(1);
    }
    return 0;
}