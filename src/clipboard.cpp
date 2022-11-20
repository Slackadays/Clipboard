#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

fs::path filepath;

std::string action;
std::vector<std::string> flags;
std::vector<std::string> items;

unsigned int files_success = 0;
unsigned int directories_success = 0;
unsigned int files_failed = 0;
unsigned int directories_failed = 0;

void displayHelpMessage() {
    std::cout << "\033[38:5:51m▏This is Clipboard 0.1.0, the copy and paste system for the command line.\033[0m" << std::endl;
    std::cout << "\033[38:5:51m\033[1m▏Usage:\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏cut [options] (item) [items]\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏copy [options] (item) [items]\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏paste [options]\033[0m" << std::endl;
    std::cout << "\033[38:5:51m\033[1m▏Examples:\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏cut nuclearlaunchcodes.txt Contacts_Folder\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏copy dogfood.conf\033[0m" << std::endl;
    std::cout << "\033[38:5:208m▏paste\033[0m" << std::endl;
    std::cout << "\033[38:5:51m▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3\033[0m" << std::endl;
    std::cout << "\033[38:5:51m▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.\033[0m" << std::endl;
}

void setupVariables(int argc, char *argv[]) {
    filepath = fs::temp_directory_path() / fs::path("Clipboard/files");

    for (int i = 1; i < argc; i++) { //start at 1 to skip the program name
        flags.push_back(argv[i]);
    }

    for (const auto& f : flags) {
        if (f.at(0) != '-') {
            items.push_back(f);
        } else {
            if (f == "-h" || f == "--help") {
                displayHelpMessage();
                exit(0);
            }
        }
    }
    
    if (getenv("ACTION") != NULL) {
        action = getenv("ACTION");
    }
}

void checkForNoItems() {
    if ((action != "paste") && items.size() < 1) {
        if (action == "copy") {
            std::cout << "\033[38:5:196m╳ You need to choose something to copy.\033[38:5:219m Try adding the items you want to copy to the end, like \033[1mcopy contacts.txt myprogram.cpp\033[0m" << std::endl;
        } else if (action == "cut") {
            std::cout << "\033[38:5:196m╳ You need to choose something to cut.\033[38:5:219m Try adding the items you want to cut to the end, like \033[1mcut contacts.txt myprogram.cpp\033[0m" << std::endl;
        } else {
            std::cout << "\033[38:5:196m╳ Invalid action. \033[38:5:219mIf you were trying to cut, copy, or paste, then this is a bug. If you were wanting to find more info about Clipboard, try \033[1mclipboard -h\033[0m" << std::endl;
        }
        exit(1);
    }
}

void setupTempDirectory() {
    if (fs::is_directory(filepath)) {
        if (action != "paste") {
            for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
                fs::remove_all(entry.path());
            }
        }
    } else {
        fs::create_directories(filepath);
    }
}

void performAction() {
    if (action == "copy") {
        for (const auto& f : items) {
            fs::copy(f, filepath / f, fs::copy_options::recursive | fs::copy_options::copy_symlinks);
        }
    } else if (action == "cut") {
        for (const auto& f : items) {
            fs::rename(f, filepath / f);
        }  
    } else if (action == "paste") {
        for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
            fs::copy(entry.path(), fs::current_path(), fs::copy_options::recursive | fs::copy_options::copy_symlinks);
        }
    }
}

void countSuccessesAndFailures() {
    if (action != "paste") {
        for (const auto& f : items) {
            if (action == "copy") {
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
            } else if (action == "cut") {
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
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
            if (fs::exists(fs::current_path() / entry.path().filename())) {
                if (fs::is_directory(entry.path())) {
                    directories_success++;
                } else {
                    files_success++;
                }
            } else {
                if (fs::is_directory(entry.path())) {
                    directories_failed++;
                } else {
                    files_failed++;
                }
            }
        }
    }
}

void showSuccesses() {
    if ((files_success == 1) != (directories_success == 1)) {
        if (action == "copy") {
            std::cout << "\033[38:5:40m√ Copied "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == "cut") {
            std::cout << "\033[38:5:40m√ Cut "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == "paste") {
            std::cout << "\033[38:5:40m√ Pasted "<< items.at(0) << "\033[0m" << std::endl;
        }
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            if (action == "copy") {
                std::cout << "\033[38:5:40m√ Copied " << files_success << " files\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:40m√ Cut " << files_success << " files\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:40m√ Pasted " << files_success << " files\033[0m" << std::endl;
            }
        } else if ((files_success == 0) && (directories_success > 1)) {
            if (action == "copy") {
                std::cout << "\033[38:5:40m√ Copied " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:40m√ Cut " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:40m√ Pasted " << directories_success << " directories\033[0m" << std::endl;
            }
        } else if ((files_success > 1) && (directories_success > 1)) {
            if (action == "copy") {
                std::cout << "\033[38:5:40m√ Copied " << files_success << " files and " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:40m√ Cut " << files_success << " files and " << directories_success << " directories\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:40m√ Pasted " << files_success << " files and " << directories_success << " directories\033[0m" << std::endl;
            }
        }
    }
}

void showFailures() {
    if ((files_failed == 1) != (directories_failed == 1)) {
        if (action == "copy") {
            std::cout << "\033[38:5:196m╳\033[91m Failed to copy "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == "cut") {
            std::cout << "\033[38:5:196m╳\033[91m Failed to cut "<< items.at(0) << "\033[0m" << std::endl;
        } else if (action == "paste") {
            std::cout << "\033[38:5:196m╳\033[91m Failed to paste "<< items.at(0) << "\033[0m" << std::endl;
        }
    } else {
        if ((files_failed > 1) && (directories_failed == 0)) {
            if (action == "copy") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to copy " << files_failed << " files\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to cut " << files_failed << " files\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to paste " << files_failed << " files\033[0m" << std::endl;
            }
        } else if ((files_failed == 0) && (directories_failed > 1)) {
            if (action == "copy") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to copy " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to cut " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to paste " << directories_failed << " directories\033[0m" << std::endl;
            }
        } else if ((files_failed > 1) && (directories_failed > 1)) {
            if (action == "copy") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to copy " << files_failed << " files and " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == "cut") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to cut " << files_failed << " files and " << directories_failed << " directories\033[0m" << std::endl;
            } else if (action == "paste") {
                std::cout << "\033[38:5:196m╳\033[91m Failed to paste " << files_failed << " files and " << directories_failed << " directories\033[0m" << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupVariables(argc, argv);

        checkForNoItems();

        setupTempDirectory();

        performAction();

        countSuccessesAndFailures();

        showSuccesses();

        showFailures();
    } catch(const std::exception& e) {
        std::cout << "\033[38:5:196m╳ Internal error: " << e.what() << std::endl << "This is probably a bug.\033[0m" << std::endl;
        exit(1);
    }
    return 0;
}