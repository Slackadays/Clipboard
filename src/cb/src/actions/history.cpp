/*  The Clipboard Project - Cut, copy, and paste anything, anywhere, all from the terminal.
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
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace PerformAction {

void history() {
    stopIndicator();
    auto available = thisTerminalSize();
    fprintf(stderr, "%s", formatMessage("[info]┍━┫ ").data());
    Message clipboard_history_message = "[info]Entry history for clipboard [bold][help]%s[blank]";
    fprintf(stderr, clipboard_history_message().data(), clipboard_name.data());
    fprintf(stderr, "%s", formatMessage("[info] ┣").data());
    int columns = available.columns - ((clipboard_history_message.rawLength() - 2) + clipboard_name.length() + 7);
    for (int i = 0; i < columns; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatMessage("┑[blank]").data());

    std::vector<std::string> dates;
    for (auto entry = 0; entry < path.entryIndex.size(); entry++) {
        path.setEntry(entry);
        std::string agoMessage;
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
        struct stat info;
        stat(fs::path(path.data).string().data(), &info);
        auto timeSince = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(info.st_ctime);
#else
        auto timeSince = std::chrono::system_clock::now() - decltype(fs::last_write_time(path.data))::clock::to_time_t(fs::last_write_time(path.data));
#endif
        // format time like 1y 2d 3h 4m 5s
        auto years = std::chrono::duration_cast<std::chrono::years>(timeSince);
        auto days = std::chrono::duration_cast<std::chrono::days>(timeSince - years);
        auto hours = std::chrono::duration_cast<std::chrono::hours>(timeSince - days);
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timeSince - days - hours);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeSince - days - hours - minutes);
        if (years.count() > 0) agoMessage += std::to_string(years.count()) + "y ";
        if (days.count() > 0) agoMessage += std::to_string(days.count()) + "d ";
        if (hours.count() > 0) agoMessage += std::to_string(hours.count()) + "h ";
        if (minutes.count() > 0) agoMessage += std::to_string(minutes.count()) + "m ";
        agoMessage += std::to_string(seconds.count()) + "s";
        dates.emplace_back(agoMessage);
    }

    auto numberLength = [](const unsigned long& number) {
        if (number < 10) return 1;
        if (number < 100) return 2;
        if (number < 1000) return 3;
        if (number < 10000) return 4;
        if (number < 100000) return 5;
        if (number < 1000000) return 6;
        if (number < 10000000) return 7;
        if (number < 100000000) return 8;
        if (number < 1000000000) return 9;
        return 10; // because 4 billion is the max for unsigned long, we know we'll have 10 or fewer digits
    };

    auto longestDateLength = (*std::max_element(dates.begin(), dates.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); })).size();

    auto longestEntryLength = numberLength(*std::max_element(path.entryIndex.begin(), path.entryIndex.end(), [](const auto& a, const auto& b) { return a < b; }));

    for (long entry = path.entryIndex.size() - 1; entry >= 0; entry--) {
        path.setEntry(entry);
        int widthRemaining = available.columns - (numberLength(entry) + longestEntryLength + longestDateLength + 7);

        fprintf(stderr,
                formatMessage("\n[info]\033[%ldG│\r│ [bold]%*s%lu[blank][info]│ [bold]%*s[blank][info]│ ").data(),
                available.columns,
                longestEntryLength - numberLength(entry),
                "",
                entry,
                longestDateLength,
                dates.at(entry).data());

        if (path.holdsRawData()) {
            std::string content(fileContents(path.data.raw));
            std::erase(content, '\n');
            fprintf(stderr, formatMessage("[help]%s[blank]").data(), content.substr(0, widthRemaining).data());
            continue;
        }

        for (bool first = true; const auto& entry : fs::directory_iterator(path.data)) {
            auto filename = entry.path().filename().string();
            int entryWidth = filename.length();

            if (widthRemaining <= 0) break;

            if (!first) {
                if (entryWidth <= widthRemaining - 2) {
                    fprintf(stderr, "%s", formatMessage("[help], [blank]").data());
                    widthRemaining -= 2;
                }
            }

            if (entryWidth <= widthRemaining) {
                std::string stylizedEntry;
                if (entry.is_directory())
                    stylizedEntry = "\033[4m" + filename + "\033[24m";
                else
                    stylizedEntry = "\033[1m" + filename + "\033[22m";
                fprintf(stderr, formatMessage("[help]%s[blank]").data(), stylizedEntry.data());
                widthRemaining -= entryWidth;
                first = false;
            }
        }
    }

    fprintf(stderr, "%s", formatMessage("[info]\n┕━┫ ").data());
    Message status_legend_message = "Text, \033[1mFiles\033[22m, \033[4mDirectories\033[24m";
    auto cols = available.columns - (status_legend_message.rawLength() + 7);
    std::string bar2 = " ┣";
    for (int i = 0; i < cols; i++)
        bar2 += "━";
    fprintf(stderr, "%s", (status_legend_message() + bar2).data());
    fprintf(stderr, "%s", formatMessage("┙[blank]\n").data());
}

void historyJSON() {
    printf("{\n");
    for (unsigned long entry = 0; entry < path.entryIndex.size(); entry++) {
        path.setEntry(entry);
        printf("    \"%lu\": {\n", entry);
        printf("        \"date\": %zu,\n", fs::last_write_time(path.data).time_since_epoch().count());
        printf("        \"content\": ");
        if (path.holdsRawData()) {
            printf("\"%s\"", escape(fileContents(path.data.raw)).data());
        } else if (path.holdsData()) {
            printf("[\n");
            std::vector<fs::path> itemsInPath(fs::directory_iterator(path.data), fs::directory_iterator());
            for (const auto& entry : itemsInPath) {
                printf("            {\n");
                printf("                \"name\": \"%s\",\n", entry.filename().string().data());
                printf("                \"isDirectory\": %s\n", fs::is_directory(entry) ? "true" : "false");
                printf("            }%s\n", entry == itemsInPath.back() ? "" : ",");
            }
            printf("\n        ]");
        } else {
            printf("null");
        }
        printf("\n    }%s\n", entry == path.entryIndex.size() - 1 ? "" : ",");
    }
    printf("}\n");
}

} // namespace PerformAction