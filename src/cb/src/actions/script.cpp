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
void script() {
    if (io_type == IOType::File) {
        if (copying.items.size() > 1) {
            error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] You can only set one script file to run. [help]⬤ Try providing a single script file instead.[blank]\n"));
            return;
        }
        if (copying.items.empty()) {
            stopIndicator();
            if (fs::is_regular_file(path.metadata.script)) {
                fprintf(stderr, formatColors("[info]┃ The current script is: [help]%s[blank]\n").data(), fileContents(path.metadata.script).value().data());
            } else {

                fprintf(stderr, "%s", formatColors("[error][inverse] ✘ [noinverse] No script is currently set. [help]⬤ Try providing a script file to set one.[blank]\n").data());
            }
            return;
        }
        if (copying.items.at(0).string() == "") {
            fs::remove(path.metadata.script);
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Removed script[blank]\n").data());
        } else {
            fs::remove(path.metadata.script);
            fs::copy(copying.items.at(0), path.metadata.script);
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved script \"%s\"[blank]\n").data(), fileContents(path.metadata.script).value().data());
        }
    } else if (io_type == IOType::Text) {
    }
}

} // namespace PerformAction