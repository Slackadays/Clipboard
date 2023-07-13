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
            int clipboards_cleared = 0;
            if (decision.substr(0, 1) != "y" && decision.substr(0, 1) != "Y") return;
            startIndicator();
            for (const auto& entry : fs::directory_iterator(global_path.temporary)) {
                bool predicate = Clipboard(entry.path().filename().string()).holdsDataInCurrentEntry();
                fs::remove_all(entry);
                if (predicate) clipboards_cleared++;
            }
            for (const auto& entry : fs::directory_iterator(global_path.persistent)) {
                bool predicate = Clipboard(entry.path().filename().string()).holdsDataInCurrentEntry();
                fs::remove_all(entry);
                if (predicate) clipboards_cleared++;
            }
            stopIndicator();
            fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Cleared %d clipboard%s[blank]\n").data(), clipboards_cleared, clipboards_cleared == 1 ? "" : "s");
        }
    } else {
        fs::remove(path.metadata.originals);
        fs::remove(path.metadata.notes);
        fs::remove(path.metadata.ignore);
        stopIndicator();
        if (!output_silent && !confirmation_silent) fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Cleared clipboard[blank]\n").data());
    }
}

} // namespace PerformAction