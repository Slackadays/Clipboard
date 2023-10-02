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

namespace PerformAction {

void show() {
    std::vector<std::regex> regexes;
    if (!copying.items.empty()) {
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(regexes), [](const auto& item) { return std::regex(item.string()); });
    }

    stopIndicator();

    auto available = thisTerminalSize();

    if (path.holdsRawDataInCurrentEntry()) {
        std::string content(fileContents(path.data.raw).value());
        content = makeControlCharactersVisible(content, available.columns);
        fprintf(stderr, clipboard_text_contents_message().data(), std::min(static_cast<size_t>(250), content.size()), clipboard_name.data());
        fprintf(stderr, formatColors("[bold][info]%s\n[blank]").data(), content.substr(0, 250).data());
        if (content.size() > 250) {
            fprintf(stderr, and_more_items_message().data(), content.size() - 250);
        }
        return;
    }

    fprintf(stderr, "%s", formatColors("[info]┏━━").data());
    fprintf(stderr, clipboard_item_many_contents_message().data(), clipboard_name.data());
    fprintf(stderr, "%s", formatColors("[info]━").data());
    auto usedSpace = (columnLength(clipboard_item_many_contents_message) - 2) + clipboard_name.length() + 5;
    if (usedSpace > available.columns) available.columns = usedSpace;
    int columns = available.columns - usedSpace;
    for (int i = 0; i < columns; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatColors("┓[blank]").data());

    for (const auto& entry : fs::directory_iterator(path.data)) {
        if (!regexes.empty() && !std::any_of(regexes.begin(), regexes.end(), [&](const auto& regex) { return std::regex_match(entry.path().filename().string(), regex); })) continue;
        std::string stylizedEntry;
        if (entry.is_directory())
            stylizedEntry = "\033[4m" + entry.path().filename().string() + "\033[24m";
        else
            stylizedEntry = "\033[1m" + entry.path().filename().string() + "\033[22m";
        fprintf(stderr, formatColors("\n[info]\033[%zuG┃\r┃ [help]%s[blank]").data(), available.columns, stylizedEntry.data());
    }

    fprintf(stderr, "%s", formatColors("[info]\n┗━▌").data());
    Message status_legend_message = "[help]\033[1mFiles\033[22m, \033[4mDirectories\033[24m[info]";
    usedSpace = columnLength(status_legend_message) + 5;
    if (usedSpace > available.columns) available.columns = usedSpace;
    auto cols = available.columns - usedSpace;
    std::string bar2 = "▐" + repeatString("━", cols);
    fprintf(stderr, "%s", (status_legend_message() + bar2).data());
    fprintf(stderr, "%s", formatColors("┛[blank]\n").data());
}

void showFilepaths() {
    std::vector<std::regex> regexes;
    if (!copying.items.empty()) {
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(regexes), [](const auto& item) { return std::regex(item.string()); });
    }

    std::vector<fs::path> paths(fs::directory_iterator(path.data), fs::directory_iterator {});
    if (!regexes.empty())
        paths.erase(
                std::remove_if(
                        paths.begin(),
                        paths.end(),
                        [&](const auto& entry) { return !std::any_of(regexes.begin(), regexes.end(), [&](const auto& regex) { return std::regex_match(entry.filename().string(), regex); }); }
                ),
                paths.end()
        );

    for (const auto& entry : paths) {
        fprintf(stderr, "\"%s\"", entry.string().data());
        incrementSuccessesForItem(entry);
        if (entry != paths.back()) printf(" ");
    }
}

} // namespace PerformAction