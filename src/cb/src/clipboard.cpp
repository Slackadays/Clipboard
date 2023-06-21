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
#include "clipboard.hpp"

Clipboard::Clipboard(const std::string& clipboard_name, const unsigned long& clipboard_entry) {
    this_name = clipboard_name;
    this_entry = clipboard_entry;

    is_persistent = isPersistent(this_name) || getenv("CLIPBOARD_ALWAYS_PERSIST");

    root = (is_persistent ? global_path.persistent : global_path.temporary) / this_name;

    entryIndex = generatedEntryIndex();

    try {
        data = root / constants.data_directory / std::to_string(entryIndex.at(this_entry));
    } catch (...) {
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        fprintf(stderr,
                formatMessage("[error]❌ The history entry you chose (\"[bold]%lu[blank][error]\") doesn't exist. 💡 [help]Try choosing a different or newer one instead.\n[blank]").data(),
                this_entry);
        exit(EXIT_FAILURE);
    }

    data.raw = data / constants.data_file_name;

    metadata = root / constants.metadata_directory;
    metadata.notes = metadata / constants.notes_name;
    metadata.originals = metadata / constants.original_files_name;
    metadata.lock = metadata / constants.lock_name;
    metadata.ignore = metadata / constants.ignore_regex_name;

    fs::create_directories(data);
    fs::create_directories(metadata);
}

std::deque<unsigned long> Clipboard::generatedEntryIndex() {
    // auto then = std::chrono::system_clock::now();
    std::deque<unsigned long> pathNames;
    fs::path entriesDir = root / constants.data_directory;
    fs::create_directories(entriesDir);
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    auto dirptr = opendir(entriesDir.string().data());
    char* endptr = nullptr;
    errno = 0;
    for (auto* dir = readdir(dirptr); dir != nullptr; dir = readdir(dirptr), errno = 0)
        if (auto num = strtoul(dir->d_name, &endptr, 10); errno == 0 && endptr != dir->d_name) [[likely]]
            pathNames.emplace_back(num);
        else [[unlikely]]
            continue;
#else
    for (const auto& entry : fs::directory_iterator(entriesDir))
        try {
            pathNames.emplace_back(std::stoul(entry.path().filename().string()));
        } catch (...) {}
#endif
    if (pathNames.empty()) pathNames.emplace_back(0);
    std::sort(pathNames.begin(), pathNames.end(), std::greater<>());
    // auto now = std::chrono::system_clock::now();
    // std::cout << "Took " << std::chrono::duration_cast<std::chrono::microseconds>(now - then).count() << "us to index " << pathNames.size() << " entries" << std::endl;
    return pathNames;
}

bool Clipboard::holdsRawDataInCurrentEntry() const {
    std::error_code ec;
    bool empty = fs::is_empty(data.raw, ec);
    if (ec) return false; // errors out if the file doesn't exist, return false to save on a syscall
    return !empty;
}

bool Clipboard::holdsDataInCurrentEntry() {
    if (fs::is_empty(data)) return false;
    if (holdsRawDataInCurrentEntry()) return true;
    for (const auto& entry : fs::directory_iterator(data))
        if (!fs::is_empty(entry)) return true;
    return false;
}

bool Clipboard::holdsIgnoreRegexes() {
    return fs::exists(metadata.ignore) && !fs::is_empty(metadata.ignore);
}

std::vector<std::regex> Clipboard::ignoreRegexes() {
    std::vector<std::regex> regexes;
    if (!holdsIgnoreRegexes()) return regexes;
    for (const auto& line : fileLines(metadata.ignore))
        regexes.emplace_back(line);
    return regexes;
}

void Clipboard::applyIgnoreRegexes() {
    if (!holdsIgnoreRegexes()) return;
    auto regexes = ignoreRegexes();
    if (holdsRawDataInCurrentEntry())
        for (const auto& regex : regexes)
            writeToFile(data.raw, std::regex_replace(fileContents(data.raw), regex, ""));
    else
        for (const auto& regex : regexes)
            for (const auto& entry : fs::directory_iterator(data))
                if (std::regex_match(entry.path().filename().string(), regex)) fs::remove_all(entry.path());
}

bool Clipboard::isUnused() {
    if (holdsDataInCurrentEntry()) return false;
    if (fs::exists(metadata.notes) && !fs::is_empty(metadata.notes)) return false;
    if (fs::exists(metadata.originals) && !fs::is_empty(metadata.originals)) return false;
    return true;
}

void Clipboard::getLock() {
    if (isLocked()) {
        auto pid = std::stoi(fileContents(metadata.lock));
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        if (getpgrp() == getpgid(pid)) return; // if we're in the same process group, we're probably in a self-referencing pipe like cb | cb
#elif defined(_WIN32) || defined(_WIN64)
        if (GetCurrentProcessId() == pid) return;
#endif
        while (true) {
#if defined(_WIN32) || defined(_WIN64)
            if (WaitForSingleObject(OpenProcess(SYNCHRONIZE, FALSE, pid), 0) == WAIT_OBJECT_0) break;
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
            if (kill(pid, 0) == -1) break;
#endif
            if (!isLocked()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
    writeToFile(metadata.lock, std::to_string(thisPID()));
}

void Clipboard::makeNewEntry() {
    entryIndex.emplace_front(entryIndex.front() + 1);

    data = root / constants.data_directory / std::to_string(entryIndex.at(this_entry));
    data.raw = data / constants.data_file_name;

    fs::create_directories(data);
}

void Clipboard::setEntry(const unsigned long& entry) {
    this_entry = entry;
    data = root / constants.data_directory / std::to_string(entryIndex.at(this_entry));
    data.raw = data / constants.data_file_name;
}

fs::path Clipboard::entryPathFor(const unsigned long& entry) {
    try {
        return root / constants.data_directory / std::to_string(entryIndex.at(entry));
    } catch (...) {
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        fprintf(stderr, formatMessage("[error]❌ The history entry you chose (\"[bold]%lu[blank][error]\") doesn't exist. 💡 [help]Try choosing a different or newer one instead.\n[blank]").data(), entry
        );
        exit(EXIT_FAILURE);
    }
}

bool Clipboard::holdsData() {
    for (const auto& entry : entryIndex)
        if (!fs::is_empty(entryPathFor(entry))) return true;
    return false;
}

void Clipboard::trimHistoryEntries() {
    if (maximumHistorySize.empty()) return;
    auto settings = regexSplit(maximumHistorySize, std::regex("\\s+"));
    unsigned long long maximumBytes = 0;
    unsigned long maximumSeconds = 0;
    unsigned long maximumEntries = 0;
    for (const auto& setting : settings) {
        try {
            std::string lastTwoChars(setting.end() - 2, setting.end());
            std::transform(lastTwoChars.begin(), lastTwoChars.end(), lastTwoChars.begin(), ::tolower);
            if (lastTwoChars == "tb")
                maximumBytes = std::stold(setting) * 1024.0 * 1024.0 * 1024.0 * 1024.0;
            else if (lastTwoChars == "gb")
                maximumBytes = std::stold(setting) * 1024.0 * 1024.0 * 1024.0;
            else if (lastTwoChars == "mb")
                maximumBytes = std::stold(setting) * 1024.0 * 1024.0;
            else if (lastTwoChars == "kb")
                maximumBytes = std::stold(setting) * 1024.0;
            else if (lastTwoChars.at(1) == 'b')
                maximumBytes = std::stoull(setting);
            else if (lastTwoChars.at(1) == 'y')
                maximumSeconds = std::stold(setting) * 60.0 * 60.0 * 24.0 * 365.0;
            else if (lastTwoChars.at(1) == 'm')
                maximumSeconds = std::stold(setting) * 60.0 * 60.0 * 24.0 * 30.0;
            else if (lastTwoChars.at(1) == 'w')
                maximumSeconds = std::stold(setting) * 60.0 * 60.0 * 24.0 * 7.0;
            else if (lastTwoChars.at(1) == 'd')
                maximumSeconds = std::stold(setting) * 60.0 * 60.0 * 24.0;
            else if (lastTwoChars.at(1) == 'h')
                maximumSeconds = std::stold(setting) * 60.0 * 60.0;
            else if (lastTwoChars.at(1) == 's')
                maximumSeconds = std::stoul(setting);
            else
                maximumEntries = std::stoul(setting);
        } catch (...) {}
    }

    // std::cout << "maximumBytes = " << maximumBytes << std::endl;
    // std::cout << "maximumSeconds = " << maximumSeconds << std::endl;
    // std::cout << "maximumEntries = " << maximumEntries << std::endl;

    if (maximumBytes > 0) {
        size_t startingClipboardSize = totalDirectorySize(root);

        while (startingClipboardSize > maximumBytes) {
            auto oldestPath = entryPathFor(entryIndex.size() - 1);
            size_t oldestEntrySize = totalDirectorySize(oldestPath);
            fs::remove_all(oldestPath);
            entryIndex.pop_back();
            startingClipboardSize -= oldestEntrySize;
        }
    }

    if (maximumSeconds > 0) {
        auto now = std::chrono::system_clock::now();
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        struct stat info;
        auto lastModified = [&](const fs::path path) {
            if (stat(path.string().data(), &info) != 0) return now;
            return std::chrono::system_clock::from_time_t(info.st_mtime);
        };

        while (lastModified(entryPathFor(entryIndex.size() - 1)) < now - std::chrono::seconds(maximumSeconds)) {
            fs::remove_all(entryPathFor(entryIndex.size() - 1));
            entryIndex.pop_back();
        }
#endif
    }

    if (maximumEntries > 0) {
        if (entryIndex.size() <= maximumEntries || maximumEntries == 0) return;
        while (entryIndex.size() > maximumEntries) {
            fs::remove_all(entryPathFor(entryIndex.size() - 1));
            entryIndex.pop_back();
        }
    }
}