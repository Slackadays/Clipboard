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
#include <fstream>

namespace PerformAction {

void edit() {
    if (!path.holdsRawDataInCurrentEntry()) {
        if (path.holdsDataInCurrentEntry())
            error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] You can currently only edit text content. [help]⬤ Try copying some text instead.[blank]\n"));
        else
            std::ofstream temp(path.data.raw);
    }

    auto editor = findUsableEditor();

    if (!editor) error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] CB couldn't find a suitable editor to use. [help]⬤ Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));

    // now run this editor with the text file as the argument
    auto command = editor.value() + " " + path.data.raw.string();

    stopIndicator();

    int res = system(command.data());

    if (res != 0) error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] CB couldn't open the editor. [help]⬤ Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));
}

} // namespace PerformAction