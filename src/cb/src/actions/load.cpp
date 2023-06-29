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

void load() {
    if (!path.holdsDataInCurrentEntry()) {
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] The clipboard you're trying to load from is empty. [help]⬤ Try choosing a different source instead.[blank]\n"));
    }

    std::vector<std::string> destinations;
    if (!copying.items.empty())
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(destinations), [](const auto& item) { return item.string(); });
    else
        destinations.emplace_back(constants.default_clipboard_name);

    if (std::find(destinations.begin(), destinations.end(), clipboard_name) != destinations.end())
        error_exit(
                "%s",
                formatColors("[error][inverse] ✘ [noinverse] You can't load a clipboard into itself. [help]⬤ Try choosing a different source instead, or choose different destinations.[blank]\n")
        );

    for (const auto& destination_number : destinations) {
        Clipboard destination(destination_number);
        try {
            for (const auto& entry : fs::directory_iterator(path.data)) {
                auto target = destination.data / entry.path().filename();
                auto loadItem = [&](bool use_regular_copy = copying.use_safe_copy) {
                    if (entry.is_directory())
                        fs::copy(entry.path(), target, copying.opts);
                    else
                        fs::copy(entry.path(), target, use_regular_copy ? copying.opts : (copying.opts | fs::copy_options::create_hard_links));
                };
                try {
                    loadItem();
                } catch (const fs::filesystem_error& e) {
                    if (!copying.use_safe_copy && e.code() == std::errc::cross_device_link) loadItem(true);
                }
            }

            destination.applyIgnoreRegexes();

            successes.clipboards++;
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(destination_number, e.code());
        }
    }

    stopIndicator();

    if (std::find(destinations.begin(), destinations.end(), constants.default_clipboard_name) != destinations.end()) updateExternalClipboards(true);
}

} // namespace PerformAction