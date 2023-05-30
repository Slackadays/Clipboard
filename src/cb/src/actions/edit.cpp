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

void edit() {
    if (!path.holdsRawData()) {
        if (path.holdsData())
            error_exit("%s", formatMessage("[error]❌ You can currently only edit text content. 💡 [help]Try copying some text instead.[blank]\n"));
        else
            error_exit("%s", formatMessage("[error]❌ You can't edit an empty clipboard. 💡 [help]Try copying some text instead.[blank]\n"));
        return;
    }

    auto preferredEditor = []() -> std::optional<std::string> {
        if (auto editor = std::getenv("CLIPBOARD_EDITOR"); editor != nullptr) return editor;
        if (auto editor = std::getenv("EDITOR"); editor != nullptr) return editor;
        if (auto editor = std::getenv("VISUAL"); editor != nullptr) return editor;
        return std::nullopt;
    };

    auto editor = preferredEditor();

    if (!editor) error_exit("%s", formatMessage("[error]❌ CB couldn't find a suitable editor to use. 💡 [help]Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));

    // now run this editor with the text file as the argument
    std::string command = editor.value() + " " + path.data.raw.string();

    stopIndicator();

    int res = system(command.data());

    if (res != 0) error_exit("%s", formatMessage("[error]❌ CB couldn't open the editor. 💡 [help]Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));
}

} // namespace PerformAction