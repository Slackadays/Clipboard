/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    SPDX-License-Identifier: GPL-3.0-or-later
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

void clear() {
    if (all_option) {
        if (!userIsARobot()) {
            stopIndicator();
            fprintf(stderr,
                    formatColors("[progress]⬤ Are you sure you want to clear all clipboards?[blank] [help]This will remove everything in locations [bold]%s[nobold] and "
                                 "[bold]%s[nobold]. [bold][y(es)/n(o)] ")
                            .data(),
                    global_path.temporary.string().data(),
                    global_path.persistent.string().data());
            std::string decision;
            std::getline(std::cin, decision);
            fprintf(stderr, "%s", formatColors("[blank]").data());
            unsigned long clipboards_cleared = 0;
            if (decision.substr(0, 1) != "y" && decision.substr(0, 1) != "Y") return;
            startIndicator();
            for (const auto& entry : fs::directory_iterator(global_path.temporary)) {
                fs::remove_all(entry);
                clipboards_cleared++;
            }
            for (const auto& entry : fs::directory_iterator(global_path.persistent)) {
                fs::remove_all(entry);
                clipboards_cleared++;
            }
            stopIndicator();
            fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Cleared %d clipboard%s[blank]\n").data(), clipboards_cleared, clipboards_cleared == 1 ? "" : "s");
        }
    } else {
        if (copying.items.size() >= 1) {
            std::vector<unsigned long> entries_to_clear = {};
            for (const auto& item : copying.items) {
                // First, check if it fits the format abc-xyz
                // If it does, treat that as a range
                if (item.string().find('-') != std::string::npos) {
                    std::vector<std::string> range = regexSplit(item.string(), std::regex("-"));
                    if (range.size() == 2) {
                        try {
                            unsigned long start = std::stoul(range.at(0));
                            unsigned long end = std::stoul(range.at(1));
                            if (start > end) std::swap(start, end);
                            for (unsigned long i = start; i <= end; i++)
                                entries_to_clear.push_back(i);
                        } catch (const std::invalid_argument& e) {}
                    }
                    continue;
                }

                try {
                    unsigned long num = std::stoul(item.string());
                    entries_to_clear.push_back(num);
                } catch (const std::invalid_argument& e) {}
            }

            if (entries_to_clear.empty()) {
                stopIndicator();
                fprintf(stderr,
                        "%s",
                        formatColors("[error][inverse] ✘ [noinverse] CB couldn't find any valid entries to clear. [help]⬤ Make sure you enter only numbers, or a range like 5-9 or 0-30.[blank]\n")
                                .data());
                return;
            }

            // Now clear these entries
            for (const auto& entry : entries_to_clear) {
                for (const auto& item : fs::directory_iterator(path.entryPathFor(entry)))
                    fs::remove_all(item);
            }

        } else {
            fs::remove(path.metadata.originals);
            fs::remove(path.metadata.notes);
            fs::remove(path.metadata.ignore);
        }
        stopIndicator();
        if (!output_silent && !confirmation_silent) fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Cleared clipboard[blank]\n").data());
    }
}

} // namespace PerformAction