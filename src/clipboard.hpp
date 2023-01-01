#pragma once
#include <vector>
#include <filesystem>
#include <string_view>
#include <array>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace fs = std::filesystem;

extern bool use_perma_clip;
extern bool use_safe_copy;
extern fs::path main_filepath;
extern fs::path temporary_filepath;
extern fs::path persistent_filepath;
extern fs::path original_files_path;
extern fs::path home_directory;
extern fs::copy_options opts;
extern std::vector<fs::path> items;
extern std::vector<std::pair<std::string, std::error_code>> failedItems;
extern std::string clipboard_name;

extern std::condition_variable cv;
extern std::mutex m;
extern std::jthread indicator; //If this fails to compile, then you need C++20!

extern unsigned int output_length;
extern unsigned long files_success;
extern unsigned long directories_success;
extern unsigned long long bytes_success;

extern bool stdin_is_tty;
extern bool stdout_is_tty;
extern bool stderr_is_tty;

constexpr std::string_view clipboard_version = "0.2.0";
constexpr std::string_view pipe_file = "clipboard.txt";
constexpr std::string_view default_clipboard_name = "0";

extern std::array<std::pair<std::string_view, std::string_view>, 8> colors;

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
extern std::string_view clipboard_contents_message;
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
void forceClearTempDirectory();
void setupSignals();
void setLocale();
void showHelpMessage(int& argc, char *argv[]);
void setupItems(int& argc, char *argv[]);
void setClipboardName(int& argc, char *argv[]);
void setupVariables(int& argc, char *argv[]);
void createTempDirectory();
void syncWithGUIClipboard();
void showClipboardStatus();
void showClipboardContents();
void setupAction(int& argc, char *argv[]);
void checkForNoItems();
void setupIndicator(std::stop_token st);
void deduplicateItems();
unsigned long long calculateTotalItemSize();
void checkItemSize();
void clearTempDirectory();
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