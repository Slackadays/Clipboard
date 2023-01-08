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
#pragma once
#include <vector>
#include <filesystem>
#include <string_view>
#include <array>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <clipboard/gui.hpp>

namespace fs = std::filesystem;

struct Filepath {
    fs::path main;
    fs::path temporary;
    fs::path persistent;
    fs::path original_files;
    fs::path home;
};
extern Filepath filepath;

struct Copying {
    bool is_persistent = false;
    bool use_safe_copy = true;
    fs::copy_options opts = fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks;
    std::vector<fs::path> items;
    std::vector<std::pair<std::string, std::error_code>> failedItems;
    std::string buffer;
};
extern Copying copying;

extern std::string clipboard_name;

enum class SpinnerState : int { Done, Active, Cancel };

extern std::condition_variable cv;
extern std::mutex m;
extern std::atomic<SpinnerState> spinner_state;
extern std::thread indicator;

struct Successes {
    std::atomic<unsigned long> files;
    std::atomic<unsigned long> directories;
    std::atomic<unsigned long long> bytes;
};
extern Successes successes;

struct IsTTY {
    bool std_in = true;
    bool std_out = true;
    bool std_err = true;
};
extern IsTTY is_tty;

struct Constants {
    std::string_view clipboard_version = "0.2.1";
    std::string_view pipe_file = "clipboard.rawdata";
    std::string_view default_clipboard_name = "0";
    std::string_view temporary_directory_name = "Clipboard";
    std::string_view persistent_directory_name = ".clipboard";
    std::string_view original_files_extension = ".files";
};
constexpr Constants constants;

enum class Action : unsigned int { Cut, Copy, Paste, PipeIn, PipeOut, Clear, Show };
extern Action action;

template <typename T, size_t N>
class ActionArray : public std::array<T, N> {
public:
    T& operator[](Action index) {
        return std::array<T, N>::operator[](static_cast<unsigned int>(index)); //switch to std::to_underlying when available
    }
};

extern ActionArray<std::string_view, 7> actions;
extern ActionArray<std::string_view, 7> action_shortcuts;
extern ActionArray<std::string_view, 7> doing_action;
extern ActionArray<std::string_view, 7> did_action;

extern std::string_view help_message;
extern std::string_view check_clipboard_status_message;
extern std::string_view clipboard_item_contents_message;
extern std::string_view clipboard_text_contents_message;
extern std::string_view no_clipboard_contents_message;
extern std::string_view clipboard_action_prompt;
extern std::string_view no_valid_action_message;
extern std::string_view choose_action_items_message;
extern std::string_view fix_redirection_action_message;
extern std::string_view redirection_no_items_message;
extern std::string_view paste_success_message;
extern std::string_view clear_success_message;
extern std::string_view clear_fail_message;
extern std::string_view clipboard_failed_message;
extern std::string_view and_more_fails_message;
extern std::string_view and_more_items_message;
extern std::string_view fix_problem_message;
extern std::string_view not_enough_storage_message;
extern std::string_view item_already_exists_message;
extern std::string_view bad_response_message;
extern std::string_view working_message;
extern std::string_view cancelled_message;
extern std::string_view pipe_success_message;
extern std::string_view one_item_success_message;
extern std::string_view multiple_files_success_message;
extern std::string_view multiple_directories_success_message;
extern std::string_view multiple_files_directories_success_message;
extern std::string_view internal_error_message;

void setLanguagePT();
void setLanguageTR();
void setLanguageES();
std::string replaceColors(const std::string_view& str);
void setupSignals();
void setLocale();
void showHelpMessage(int& argc, char *argv[]);
void setupItems(int& argc, char *argv[]);
void setClipboardName(int& argc, char *argv[]);
void setupVariables(int& argc, char *argv[]);
void createTempDirectory();
void syncWithGUIClipboard(std::string const& text);
void syncWithGUIClipboard(ClipboardPaths const& clipboard);
void syncWithGUIClipboard();
void showClipboardStatus();
void showClipboardContents();
void setupAction(int& argc, char *argv[]);
void checkForNoItems();
bool stopIndicator(bool change_condition_variable);
void startIndicator();
void setupIndicator();
void deduplicateItems();
unsigned long long calculateTotalItemSize();
void checkItemSize();
void clearTempDirectory(bool force_clear);
void copyFiles();
void removeOldFiles();
bool userIsARobot();
int getUserDecision(const std::string& item);
void pasteFiles();
void pipeIn();
void pipeOut();
void clearClipboard();
void performAction();
void updateGUIClipboard();
void showFailures();
void showSuccesses();
