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
#include "../clipboard.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <format>
#include <io.h>
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace PerformAction {

void info() {
    stopIndicator();
    fprintf(stderr, "%s", formatMessage("[info]┏━━[inverse] ").data());
    fprintf(stderr, clipboard_name_message().data(), clipboard_name.data());
    fprintf(stderr, "%s", formatMessage(" [noinverse][info]━").data());
    int columns = thisTerminalSize().columns - ((clipboard_name_message.columnLength() - 2) + clipboard_name.length() + 7);
    for (int i = 0; i < columns; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatMessage("┓[blank]\n").data());

    // creation date
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__)
    struct stat info;
    stat(path.string().data(), &info);
    std::string time(std::ctime(&info.st_ctime));
    std::erase(time, '\n');
    fprintf(stderr, formatMessage("[info]%s┃ Created [help]%s[blank]\n").data(), generatedEndbar().data(), time.data());
#elif defined(__WIN32__) || defined(__WIN64__)
    fprintf(stderr, formatMessage("[info]┃ Created [help]n/a[blank]\n").data());
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__)
    time_t latest = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        struct stat info;
        stat(entry.path().string().data(), &info);
        if (info.st_ctime > latest) latest = info.st_ctime;
    }
    time = std::ctime(&latest);
    std::erase(time, '\n');
    fprintf(stderr, formatMessage("[info]%s┃ Last changed [help]%s[blank]\n").data(), generatedEndbar().data(), time.data());
#elif defined(__WIN32__) || defined(__WIN64__)
    fprintf(stderr, formatMessage("[info]┃ Last changed [help]%s[blank]\n").data(), std::format("{}", fs::last_write_time(path)).data());
#endif

    fprintf(stderr, formatMessage("[info]%s┃ Stored in [help]%s[blank]\n").data(), generatedEndbar().data(), path.string().data());

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__FreeBSD__)
    struct passwd* pw = getpwuid(info.st_uid);
    fprintf(stderr, formatMessage("[info]%s┃ Owned by [help]%s[blank]\n").data(), generatedEndbar().data(), pw->pw_name);
#elif defined(__WIN32__) || defined(__WIN64__)
    fprintf(stderr, formatMessage("[info]┃ Owned by [help]n/a[blank]\n").data());
#endif

    fprintf(stderr, formatMessage("[info]%s┃ Persistent? [help]%s[blank]\n").data(), generatedEndbar().data(), path.is_persistent ? "Yes" : "No");
    fprintf(stderr, formatMessage("[info]%s┃ Total entries: [help]%zu[blank]\n").data(), generatedEndbar().data(), path.totalEntries());
    fprintf(stderr, formatMessage("[info]%s┃ Total clipboard size: [help]%s[blank]\n").data(), generatedEndbar().data(), formatBytes(totalDirectorySize(path)).data());
    fprintf(stderr, formatMessage("[info]%s┃ Total space remaining: [help]%s[blank]\n").data(), generatedEndbar().data(), formatBytes(fs::space(path).available).data());

    if (path.holdsRawDataInCurrentEntry()) {
        fprintf(stderr, formatMessage("[info]%s┃ Content size: [help]%s[blank]\n").data(), generatedEndbar().data(), formatBytes(fs::file_size(path.data.raw)).data());
        fprintf(stderr, formatMessage("[info]%s┃ Content type: [help]%s[blank]\n").data(), generatedEndbar().data(), inferMIMEType(fileContents(path.data.raw)).value_or("(Unknown)").data());
    } else {
        size_t files = 0;
        size_t directories = 0;
        fprintf(stderr, formatMessage("[info]%s┃ Content size: [help]%s[blank]\n").data(), generatedEndbar().data(), formatBytes(totalDirectorySize(path.data)).data());
        for (const auto& entry : fs::directory_iterator(path.data))
            entry.is_directory() ? directories++ : files++;
        fprintf(stderr, formatMessage("[info]%s┃ Files: [help]%zu[blank]\n").data(), generatedEndbar().data(), files);
        fprintf(stderr, formatMessage("[info]%s┃ Directories: [help]%zu[blank]\n").data(), generatedEndbar().data(), directories);
    }

    if (!available_mimes.empty()) {
        fprintf(stderr, formatMessage("[info]%s┃ Available types from GUI: [help]").data(), generatedEndbar().data());
        for (const auto& mime : available_mimes) {
            fprintf(stderr, "%s", mime.data());
            if (mime != available_mimes.back()) fprintf(stderr, ", ");
        }
        fprintf(stderr, "%s", formatMessage("[blank]\n").data());
    }
    fprintf(stderr, formatMessage("[info]%s┃ Content cut? [help]%s[blank]\n").data(), generatedEndbar().data(), fs::exists(path.metadata.originals) ? "Yes" : "No");

    fprintf(stderr, formatMessage("[info]%s┃ Locked by another process? [help]%s[blank]\n").data(), generatedEndbar().data(), path.isLocked() ? "Yes" : "No");

    if (path.isLocked()) {
        fprintf(stderr, formatMessage("[info]%s┃ Locked by process with pid [help]%s[blank]\n").data(), generatedEndbar().data(), fileContents(path.metadata.lock).data());
    }

    if (fs::exists(path.metadata.notes))
        fprintf(stderr, formatMessage("[info]%s┃ Note: [help]%s[blank]\n").data(), generatedEndbar().data(), fileContents(path.metadata.notes).data());
    else
        fprintf(stderr, formatMessage("[info]%s┃ There is no note for this clipboard.[blank]\n").data(), generatedEndbar().data());

    if (path.holdsIgnoreRegexes()) {
        fprintf(stderr, formatMessage("[info]%s┃ Ignore regexes: [help]").data(), generatedEndbar().data());
        auto regexes = fileLines(path.metadata.ignore);
        for (const auto& regex : regexes) {
            fprintf(stderr, "%s", regex.data());
            if (regex != regexes.back()) fprintf(stderr, ", ");
        }
        fprintf(stderr, "%s", formatMessage("[blank]\n").data());
    } else
        fprintf(stderr, formatMessage("[info]%s┃ There are no ignore regexes for this clipboard.[blank]\n").data(), generatedEndbar().data());

    fprintf(stderr, "%s", formatMessage("[info]┗").data());
    int cols = thisTerminalSize().columns;
    for (int i = 0; i < cols - 2; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatMessage("┛[blank]\n").data());
}

void infoJSON() {
    printf("{\n");

    printf("    \"name\": \"%s\",\n", clipboard_name.data());

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__)
    struct stat info;
    stat(path.string().data(), &info);
    std::string time(std::ctime(&info.st_ctime));
    std::erase(time, '\n');
    printf("    \"created\": \"%s\",\n", time.data());
#elif defined(__WIN32__) || defined(__WIN64__)
    printf("    \"created\": \"n/a\",\n");
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    time_t latest = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        struct stat info;
        stat(entry.path().string().data(), &info);
        if (info.st_ctime > latest) latest = info.st_ctime;
    }
    time = std::ctime(&latest);
    std::erase(time, '\n');
    printf("    \"lastChanged\": \"%s\",\n", time.data());
#elif defined(__WIN32__) || defined(__WIN64__)
    printf("    \"lastChanged\": \"%s\",\n", std::format("{}", fs::last_write_time(path)).data());
#endif

    printf("    \"path\": \"%s\",\n", path.string().data());

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__FreeBSD__)
    struct passwd* pw = getpwuid(getuid());
    printf("    \"owner\": \"%s\",\n", pw->pw_name);
#elif defined(__WIN32__) || defined(__WIN64__)
    printf("    \"owner\": \"n/a\",\n");
#endif

    printf("    \"isPersistent\": %s,\n", path.is_persistent ? "true" : "false");
    printf("    \"totalEntries\": %zu,\n", path.totalEntries());
    printf("    \"totalBytesUsed\": %zu,\n", totalDirectorySize(path));
    printf("    \"totalBytesRemaining\": %zu,\n", fs::space(path).available);

    if (path.holdsRawDataInCurrentEntry()) {
        printf("    \"contentBytes\": %zu,\n", fs::file_size(path.data.raw));
        printf("    \"contentType\": \"%s\",\n", inferMIMEType(fileContents(path.data.raw)).value_or("(Unknown)").data());
    } else {
        size_t files = 0;
        size_t directories = 0;
        for (const auto& entry : fs::directory_iterator(path.data))
            entry.is_directory() ? directories++ : files++;
        printf("    \"contentBytes\": %zu,\n", totalDirectorySize(path.data));
        printf("    \"files\": %zu,\n", files);
        printf("    \"directories\": %zu,\n", directories);
    }

    if (!available_mimes.empty()) {
        printf("    \"availableTypes\": [");
        for (const auto& mime : available_mimes)
            printf("\"%s\"%s", mime.data(), mime != available_mimes.back() ? ", " : "");
        printf("],\n");
    }

    printf("    \"contentCut\": %s,\n", fs::exists(path.metadata.originals) ? "true" : "false");

    printf("    \"locked\": %s,\n", path.isLocked() ? "true" : "false");
    if (path.isLocked()) printf("    \"lockedBy\": \"%s\",\n", fileContents(path.metadata.lock).data());

    if (fs::exists(path.metadata.notes))
        printf("    \"note\": \"%s\"\n", std::regex_replace(fileContents(path.metadata.notes), std::regex("\""), "\\\"").data());
    else
        printf("    \"note\": \"\"\n");

    if (path.holdsIgnoreRegexes()) {
        printf("    \"ignoreRegexes\": [");
        auto regexes = fileLines(path.metadata.ignore);
        for (const auto& regex : regexes)
            printf("\"%s\"%s", std::regex_replace(regex, std::regex("\""), "\\\"").data(), regex != regexes.back() ? ", " : "");
        printf("]\n");
    } else {
        printf("    \"ignoreRegexes\": []\n");
    }

    printf("}\n");
}

} // namespace PerformAction