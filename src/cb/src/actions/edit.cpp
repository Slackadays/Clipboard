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
    if (!path.holdsRawDataInCurrentEntry()) {
        if (path.holdsDataInCurrentEntry())
            error_exit("%s", formatMessage("[error]❌ You can currently only edit text content. 💡 [help]Try copying some text instead.[blank]\n"));
        else
            error_exit("%s", formatMessage("[error]❌ You can't edit an empty clipboard. 💡 [help]Try copying some text instead.[blank]\n"));
    }

    auto preferredEditor = []() -> std::optional<std::string> {
        if (!copying.items.empty()) return copying.items.at(0).string();
        if (auto editor = getenv("CLIPBOARD_EDITOR"); editor != nullptr) return editor;
        if (auto editor = getenv("EDITOR"); editor != nullptr) return editor;
        if (auto editor = getenv("VISUAL"); editor != nullptr) return editor;
        return std::nullopt;
    };

    auto fallbackEditor = []() -> std::optional<std::string> {
        constexpr std::array fallbacks {"nano", "vim", "nvim", "micro", "gedit", "vi", "notepad.exe", "notepad++.exe", "wordpad.exe", "word.exe"};

        std::string pathContent(getenv("PATH"));
        std::vector<fs::path> paths;

        // split paths by : or ; (: for posix, ; for windows)
        std::regex regex("[:;]");
        auto strings = regexSplit(pathContent, regex);
        std::transform(strings.begin(), strings.end(), std::back_inserter(paths), [](const std::string& path) { return fs::path(path); });

        for (const auto& path : paths)
            for (const auto& fallback : fallbacks)
                if (fs::exists(path / fallback)) return fallback;

        return std::nullopt;
    };

    auto editor = preferredEditor();

    if (!editor) editor = fallbackEditor();

    if (!editor) error_exit("%s", formatMessage("[error]❌ CB couldn't find a suitable editor to use. 💡 [help]Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));

    // now run this editor with the text file as the argument
    auto command = editor.value() + " " + path.data.raw.string();

    stopIndicator();

    int res = system(command.data());

    if (res != 0) error_exit("%s", formatMessage("[error]❌ CB couldn't open the editor. 💡 [help]Try setting the CLIPBOARD_EDITOR environment variable.[blank]\n"));
}

} // namespace PerformAction