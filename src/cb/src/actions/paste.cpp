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

void paste() {
    std::vector<std::regex> regexes;
    if (!copying.items.empty()) {
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(regexes), [](const auto& item) { return std::regex(item.string()); });
    }
    if (!is_tty.in) {
        auto splitted = regexSplit(pipedInContent(false), std::regex("[\\n]"));
        std::transform(splitted.begin(), splitted.end(), std::back_inserter(regexes), [](const auto& item) { return std::regex(item); });
    }

    for (const auto& entry : fs::directory_iterator(path.data)) {
        auto target = [&] {
            if (path.holdsRawDataInCurrentEntry())
                return (fs::current_path() / ("clipboard" + clipboard_name + "-" + std::to_string(clipboard_entry)))
                        .replace_extension(inferFileExtension(fileContents(path.data.raw).value()).value_or(".txt"));
            else
                return fs::current_path() / entry.path().filename();
        }();
        auto pasteItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
            if (!(fs::exists(target) && fs::equivalent(entry, target))) {
                fs::copy(entry, target, use_regular_copy || entry.is_directory() ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
            }
            incrementSuccessesForItem(entry);
        };
        if (!regexes.empty() && !std::any_of(regexes.begin(), regexes.end(), [&](const auto& regex) {
                return std::regex_match(entry.path().filename().string(), regex) || std::regex_match(entry.path().string(), regex);
            }))
            continue;
        try {
            if (fs::exists(target)) {
                using enum CopyPolicy;
                switch (copying.policy) {
                case SkipAll:
                    break;
                case ReplaceAll:
                    pasteItem();
                    break;
                default:
                    stopIndicator();
                    copying.policy = userDecision(target.filename().string());
                    startIndicator();
                    if (copying.policy == ReplaceOnce || copying.policy == ReplaceAll) {
                        pasteItem();
                    }
                    break;
                }
            } else {
                pasteItem();
            }
        } catch (const fs::filesystem_error& e) {
            if (!copying.use_safe_copy) {
                try {
                    pasteItem(true);
                } catch (const fs::filesystem_error& e) {
                    copying.failedItems.emplace_back(entry.path().filename().string(), e.code());
                }
            } else {
                copying.failedItems.emplace_back(entry.path().filename().string(), e.code());
            }
        }
    }
    removeOldFiles();
}

} // namespace PerformAction