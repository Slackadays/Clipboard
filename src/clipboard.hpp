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

constexpr std::string_view clipboard_version = "0.2.0";
constexpr std::string_view pipe_file = "clipboard.txt";
constexpr std::string_view default_clipboard_name = "0";

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