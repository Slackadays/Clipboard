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
#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cwchar>
#include <deque>
#include <filesystem>
#include <functional>
#include <mutex>
#include <regex>
#include <string_view>
#include <thread>
#include <valarray>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <shlobj.h>
#include <windows.h>
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <clipboard/fork.hpp>
#include <clipboard/gui.hpp>

namespace fs = std::filesystem;

#if !defined(GIT_COMMIT_HASH)
#define GIT_COMMIT_HASH "not available"
#endif

#if !defined(CLIPBOARD_VERSION)
#define CLIPBOARD_VERSION "(version not available)"
#endif

extern Forker forker;

extern std::string maximumHistorySize;

struct GlobalFilepaths {
    fs::path temporary;
    fs::path persistent;
    fs::path main;
    fs::path home;
};
extern GlobalFilepaths global_path;

struct Constants {
    std::string_view clipboard_version = CLIPBOARD_VERSION;
    std::string_view clipboard_commit = GIT_COMMIT_HASH;
    std::string_view data_file_name = "rawdata.clipboard";
    std::string_view default_clipboard_name = "0";
    unsigned long default_clipboard_entry = 0;
    std::string_view temporary_directory_name = "Clipboard";
    std::string_view persistent_directory_name = ".clipboard";
    std::string_view original_files_name = "originals";
    std::string_view notes_name = "notes";
    std::string_view mime_name = "mime";
    std::string_view lock_name = "lock";
    std::string_view data_directory = "data";
    std::string_view metadata_directory = "metadata";
    std::string_view import_export_directory = "Exported_Clipboards";
    std::string_view ignore_regex_name = "ignore";
};
constexpr Constants constants;

enum class CopyPolicy { ReplaceAll, ReplaceOnce, SkipOnce, SkipAll, Unknown };

struct Copying {
    bool use_safe_copy = true;
    CopyPolicy policy = CopyPolicy::Unknown;
    fs::copy_options opts = fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks;
    std::vector<fs::path> items;
    std::vector<std::pair<std::string, std::error_code>> failedItems;
    std::string buffer;
    std::string mime;
};
extern Copying copying;

std::vector<std::string> regexSplit(const std::string& content, const std::regex& regex);

bool isPersistent(const auto& clipboard) {
    static auto temp = getenv("CLIPBOARD_CUSTOMPERSIST");
    if (temp != nullptr) {
        std::string custom_persist(temp);
        auto patterns = regexSplit(custom_persist, std::regex(" "));
        for (const auto& pattern : patterns)
            if (std::regex_match(clipboard, std::regex(pattern))) return true;
    }
    return clipboard.find_first_of("_") != std::string::npos;
}

static auto thisPID() {
#if defined(_WIN32) || defined(_WIN64)
    return GetCurrentProcessId();
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    return getpid();
#endif
}

static size_t directoryOverhead(const fs::path& directory) {
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__FreeBSD__)
    struct stat info;
    if (stat(directory.string().data(), &info) != 0) return 0;
    return info.st_size;
#else
    return 0;
#endif
}

static size_t totalDirectorySize(const fs::path& directory) {
    size_t size = directoryOverhead(directory);
    for (const auto& entry : fs::recursive_directory_iterator(directory))
        try {
            size += entry.is_directory() ? directoryOverhead(entry) : entry.file_size();
        } catch (const fs::filesystem_error& e) {
            if (e.code() != std::errc::no_such_file_or_directory) throw e;
        }
    return size;
}

std::string fileContents(const fs::path& path);
std::vector<std::string> fileLines(const fs::path& path);

bool stopIndicator(bool change_condition_variable = true);

void deduplicate(auto& items) {
    std::sort(items.begin(), items.end());
    items.erase(std::unique(items.begin(), items.end()), items.end());
}

size_t writeToFile(const fs::path& path, const std::string& content, bool append = false);

extern std::vector<std::string> arguments;

extern std::string clipboard_invocation;

extern std::string clipboard_name;

extern unsigned long clipboard_entry;

extern std::string clipboard_suffix;

extern std::string locale;

extern bool output_silent;
extern bool progress_silent;
extern bool confirmation_silent;
extern bool no_color;
extern bool all_option;

extern std::string preferred_mime;
extern std::vector<std::string> available_mimes;

enum class ClipboardState : int { Setup, Action, Error };
enum class IndicatorState : int { Done, Active, Cancel };

extern std::condition_variable cv;
extern std::mutex m;
extern std::atomic<ClipboardState> clipboard_state;
extern std::atomic<IndicatorState> progress_state;
static std::thread indicator;

void error_exit(const std::string& message, const auto&... args) {
    clipboard_state = ClipboardState::Error;
    stopIndicator();
    fprintf(stderr, message.data(), args.data()...);
    exit(EXIT_FAILURE);
}

struct Successes {
    std::atomic<unsigned long> files;
    std::atomic<unsigned long> directories;
    std::atomic<unsigned long long> bytes;
    std::atomic<unsigned long> clipboards;
};
extern Successes successes;

struct IsTTY {
    bool in = true;
    bool out = true;
    bool err = true;
};
extern IsTTY is_tty;

enum class Action : unsigned int { Cut, Copy, Paste, Clear, Show, Edit, Add, Remove, Note, Swap, Status, Info, Load, Import, Export, History, Ignore, Search };

extern Action action;

enum class IOType : unsigned int { File, Pipe, Text };

extern IOType io_type;

template <typename T, size_t N>
class EnumArray : public std::array<T, N> {
public:
    std::optional<std::array<T, N>> internal_original;

    EnumArray(const auto&... args) : std::array<T, N> {args...} { // inherit constructors
        if (!internal_original) internal_original = std::array<T, N> {args...};
    }

    T& operator[](const Action& index) { return std::array<T, N>::operator[](static_cast<unsigned int>(index)); } // switch to std::to_underlying when available
    T& original(const Action& index) { return internal_original.value()[static_cast<unsigned int>(index)]; }
};

extern EnumArray<std::string_view, 18> actions;
extern EnumArray<std::string_view, 18> action_shortcuts;
extern EnumArray<std::string_view, 18> doing_action;
extern EnumArray<std::string_view, 18> did_action;
extern EnumArray<std::string_view, 18> action_descriptions;

extern std::array<std::pair<std::string_view, std::string_view>, 10> colors;

bool action_is_one_of(auto... options) {
    return ((action == options) || ...);
}

class TerminalSize {
public:
    size_t rows = 0;
    size_t columns = 0;
    TerminalSize(const unsigned int& rows, const unsigned int& columns) : rows {rows}, columns {columns} {}
};

std::string JSONescape(const std::string_view& input);
std::string formatColors(const std::string_view& str, bool colorful = !no_color);

class Clipboard {
    fs::path root;
    std::string this_name;
    unsigned long this_entry;

public:
    std::deque<unsigned long> entryIndex;
    bool is_persistent = false;

    class DataDirectory {
        fs::path root;

    public:
        fs::path raw;

        operator fs::path() { return root; }
        operator fs::path() const { return root; }
        auto operator=(const auto& other) { return root = other; }
        auto operator/(const auto& other) { return root / other; }
    } data;

    class MetadataDirectory {
        fs::path root;

    public:
        fs::path notes;
        fs::path originals;
        fs::path lock;
        fs::path ignore;
        operator fs::path() { return root; }
        operator fs::path() const { return root; }
        auto operator=(const auto& other) { return root = other; }
        auto operator/(const auto& other) { return root / other; }
    } metadata;

    std::deque<unsigned long> generatedEntryIndex();

    Clipboard() = default;
    Clipboard(const std::string& clipboard_name, const unsigned long& clipboard_entry = constants.default_clipboard_entry);
    operator fs::path() { return root; }
    operator fs::path() const { return root; }
    auto operator=(const auto& other) { return root = other; }
    auto operator/(const auto& other) { return root / other; }
    std::string string() { return root.string(); }
    bool holdsRawDataInCurrentEntry() const;
    bool holdsDataInCurrentEntry();
    bool holdsIgnoreRegexes();
    std::vector<std::regex> ignoreRegexes();
    void applyIgnoreRegexes();
    bool isUnused();
    bool isLocked() { return fs::exists(metadata.lock); }
    void getLock();
    void releaseLock() { fs::remove(metadata.lock); }
    std::string name() const { return this_name; }
    unsigned long entry() { return this_entry; }
    size_t totalEntries() { return entryIndex.size(); }
    void makeNewEntry();
    void setEntry(const unsigned long& entry);
    fs::path entryPathFor(const unsigned long& entry);
    bool holdsData();
    void trimHistoryEntries();
};
extern Clipboard path;

void incrementSuccessesForItem(const auto& item) {
    fs::is_directory(item) ? successes.directories++ : successes.files++;
}

class Message {
private:
    std::string_view internal_message;

public:
    Message(const auto& message) : internal_message(std::move(message)) {}
    std::string operator()() const { return std::move(formatColors(internal_message)); }
    operator std::string_view() const { return internal_message; }
};

std::string formatNumbers(const auto& num) {
    static std::stringstream ss;
    ss.str(std::string());
    ss << std::fixed << std::setprecision(2) << num;
    return ss.str();
}

std::string formatBytes(const auto& bytes) {
    if (bytes < (1024.0 * 10.0)) return formatNumbers(bytes) + "B";
    if (bytes < (1024.0 * 1024.0 * 10.0)) return formatNumbers(bytes / 1024.0) + "kB";
    if (bytes < (1024.0 * 1024.0 * 1024.0 * 10.0)) return formatNumbers(bytes / (1024.0 * 1024.0)) + "MB";
    return formatNumbers(bytes / (1024.0 * 1024.0 * 1024.0)) + "GB";
}

unsigned int suitableThreadAmount();
bool envVarIsTrue(const std::string_view& name);
size_t columnLength(const std::string_view& message);
std::string generatedEndbar();
std::string repeatString(const std::string_view& character, const size_t& length);
unsigned long levenshteinDistance(const std::string_view& one, const std::string_view& two);
void setLanguagePT();
void setLanguageTR();
void setLanguageES();
void setupHandlers();
void setupTerminal();
bool isAClearingAction();
void setClipboardAttributes();
void setFlags();
void setFilepaths();
void makeTerminalRaw();
void makeTerminalNormal();
unsigned long numberLength(const unsigned long& number);
Action getAction();
IOType getIOType();
void verifyAction();
bool needsANewEntry();
void checkItemSize(unsigned long long total_item_size);
bool isAWriteAction();
std::string getMIMEType();
void ignoreItemsPreemptively(std::vector<fs::path>& items);
void setLocale();
void showHelpMessage(int& argc, char* argv[]);
void setupItems(int& argc, char* argv[]);
void setupVariables(int& argc, char* argv[]);
void createTempDirectory();
void syncWithExternalClipboards(bool force = false);
void syncWithExternalClipboards(const std::string& text);
void syncWithExternalClipboards(const ClipboardPaths& clipboard);
void showClipboardContents();
void setupAction(int& argc, char* argv[]);
void checkForNoItems();
void startIndicator();
void setupIndicator();
void deduplicateItems();
unsigned long long totalItemSize();
void checkItemSize();
TerminalSize thisTerminalSize();
void clearData(bool force_clear);
void copyFiles();
void removeOldFiles();
bool userIsARobot();
void pasteFiles();
void clearClipboard();
void performAction();
void updateExternalClipboards(bool force = false);
std::string pipedInContent(bool count = true);
void showFailures();
void showSuccesses();
[[nodiscard]] CopyPolicy userDecision(const std::string& item);
void setTheme(const std::string_view& theme);

extern Message help_message;
extern Message check_clipboard_status_message;
extern Message clipboard_item_one_contents_message;
extern Message clipboard_item_many_contents_message;
extern Message clipboard_text_contents_message;
extern Message no_clipboard_contents_message;
extern Message clipboard_action_prompt;
extern Message no_valid_action_message;
extern Message no_valid_action_with_candidate_message;
extern Message choose_action_items_message;
extern Message fix_redirection_action_message;
extern Message redirection_no_items_message;
extern Message paste_success_message;
extern Message clear_success_message;
extern Message clear_fail_message;
extern Message clipboard_failed_one_message;
extern Message clipboard_failed_many_message;
extern Message and_more_fails_message;
extern Message and_more_items_message;
extern Message fix_problem_message;
extern Message not_enough_storage_message;
extern Message item_already_exists_message;
extern Message bad_response_message;
extern Message working_message;
extern Message cancelled_message;
extern Message cancelled_with_progress_message;
extern Message byte_success_message;
extern Message one_item_success_message;
extern Message many_files_success_message;
extern Message many_directories_success_message;
extern Message one_file_one_directory_success_message;
extern Message one_file_many_directories_success_message;
extern Message many_files_one_directory_success_message;
extern Message many_files_many_directories_success_message;
extern Message one_clipboard_success_message;
extern Message many_clipboards_success_message;
extern Message clipboard_name_message;
extern Message internal_error_message;

extern ClipboardContent getGUIClipboard(const std::string& requested_mime);
extern void writeToGUIClipboard(const ClipboardContent& clipboard);
extern const bool GUIClipboardSupportsCut;
extern bool playAsyncSoundEffect(const std::valarray<short>& samples);

namespace PerformAction {
void copyItem(const fs::path& f);
void copy();
void copyText();
void paste();
void pipeIn();
void pipeOut();
void clear();
void show();
void edit();
void addData();
void addText();
void removeFiles();
void removeRegex();
void noteText();
void notePipe();
void swap();
void addFiles();
void status();
void info();
void load();
void showFilepaths();
void swap();
void importClipboards();
void exportClipboards();
void infoJSON();
void ignoreRegex();
void statusJSON();
void history();
void historyJSON();
void search();
void searchJSON();
} // namespace PerformAction