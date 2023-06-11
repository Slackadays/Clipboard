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

namespace PerformAction {

std::vector<Clipboard> clipboardsWithContent() {
    std::vector<Clipboard> clipboards;
    for (const auto& entry : fs::directory_iterator(global_path.temporary))
        if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsDataInCurrentEntry()) clipboards.emplace_back(cb);
    for (const auto& entry : fs::directory_iterator(global_path.persistent))
        if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsDataInCurrentEntry()) clipboards.emplace_back(cb);
    std::sort(clipboards.begin(), clipboards.end(), [](const auto& a, const auto& b) { return a.name() < b.name(); });
    return clipboards;
}

void status() {
    syncWithExternalClipboards(true);
    auto clipboards_with_contents = clipboardsWithContent();
    stopIndicator();
    if (clipboards_with_contents.empty()) {
        printf("%s", no_clipboard_contents_message().data());
        printf(clipboard_action_prompt().data(), clipboard_invocation.data(), clipboard_invocation.data());
        return;
    }
    auto longestClipboardLength =
            (*std::max_element(clipboards_with_contents.begin(), clipboards_with_contents.end(), [](const auto& a, const auto& b) { return a.name().size() < b.name().size(); })).name().size();
    auto available = thisTerminalSize();

    stopIndicator();

    fprintf(stderr, "%s", formatMessage("[info]┍━┫ ").data());
    fprintf(stderr, "%s", check_clipboard_status_message().data());
    fprintf(stderr, "%s", formatMessage("[info] ┣").data());
    int columns = available.columns - (check_clipboard_status_message.rawLength() + 7);
    std::string bar;
    for (int i = 0; i < columns; i++)
        bar += "━";
    fprintf(stderr, "%s", bar.data());
    fprintf(stderr, "%s", formatMessage("┑[blank]\n").data());

    for (auto& clipboard : clipboards_with_contents) {
        clipboard.getLock();

        int widthRemaining = available.columns - (clipboard.name().length() + 5 + longestClipboardLength);
        fprintf(stderr,
                formatMessage("[info]\033[%ldG│\r│ [bold]%*s%s[blank][info]│ [blank]").data(),
                available.columns,
                longestClipboardLength - clipboard.name().length(),
                "",
                clipboard.name().data());

        if (clipboard.holdsRawDataInCurrentEntry()) {
            std::string content(fileContents(clipboard.data.raw));
            if (auto type = inferMIMEType(content); type.has_value())
                content = "\033[7m\033[1m" + std::string(type.value()) + ", " + formatBytes(content.length()) + "\033[22m\033[27m";
            else
                std::erase(content, '\n');
            fprintf(stderr, formatMessage("[help]%s[blank]\n").data(), content.substr(0, widthRemaining).data());
            continue;
        }

        for (bool first = true; const auto& entry : fs::directory_iterator(clipboard.data)) {
            int entryWidth = entry.path().filename().string().length();

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
                    stylizedEntry = "\033[4m" + entry.path().filename().string() + "\033[24m";
                else
                    stylizedEntry = "\033[1m" + entry.path().filename().string() + "\033[22m";
                fprintf(stderr, formatMessage("[help]%s[blank]").data(), stylizedEntry.data());
                widthRemaining -= entryWidth;
                first = false;
            }
        }
        clipboard.releaseLock();
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "%s", formatMessage("[info]┕━┫ ").data());
    Message status_legend_message = "Text, \033[1mFiles\033[22m, \033[4mDirectories\033[24m, \033[7m\033[1mData\033[22m\033[27m";
    int cols = available.columns - (status_legend_message.rawLength() + 7);
    std::string bar2 = " ┣";
    for (int i = 0; i < cols; i++)
        bar2 += "━";
    fprintf(stderr, "%s", (status_legend_message() + bar2).data());
    fprintf(stderr, "%s", formatMessage("┙[blank]\n").data());
}

void statusJSON() {
    printf("{\n");

    auto clipboards_with_contents = clipboardsWithContent();

    for (const auto& clipboard : clipboards_with_contents) {

        printf("    \"%s\": ", clipboard.name().data());

        if (clipboard.holdsRawDataInCurrentEntry()) {
            std::string content(fileContents(clipboard.data.raw));
            if (auto type = inferMIMEType(content); type.has_value()) {
                printf("{\n");
                printf("        \"dataType\": \"%s\",\n", type.value().data());
                printf("        \"dataSize\": %zu,\n", content.length());
                printf("        \"path\": \"%s\"\n", clipboard.data.raw.string().data());
                printf("    }");
            } else {
                printf("\"%s\"", JSONescape(content).data());
            }
        } else {
            printf("[");
            for (bool first = true; const auto& entry : fs::directory_iterator(clipboard.data)) {
                if (!first) printf(", ");
                printf("\n");
                printf("        {\n");
                printf("            \"filename\": \"%s\",\n", entry.path().filename().string().data());
                printf("            \"path\": \"%s\",\n", entry.path().string().data());
                printf("            \"isDirectory\": %s\n", entry.is_directory() ? "true" : "false");
                printf("        }");
                first = false;
            }
            printf("\n    ]");
        }
        if (clipboard.name() != clipboards_with_contents.back().name()) printf(",\n");
    }
    printf("\n}\n");
}

} // namespace PerformAction
