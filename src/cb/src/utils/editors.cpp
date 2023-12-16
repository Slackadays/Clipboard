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

std::optional<std::string> findUsableEditor() {
    auto preferredEditor = []() -> std::optional<std::string> {
        if (!copying.items.empty()) return copying.items.at(0).string();
        if (auto editor = getenv("CLIPBOARD_EDITOR"); editor != nullptr) return editor;
        if (auto editor = getenv("EDITOR"); editor != nullptr) return editor;
        if (auto editor = getenv("VISUAL"); editor != nullptr) return editor;
        return std::nullopt;
    };

    auto fallbackEditor = []() -> std::optional<std::string> {
        constexpr std::array fallbacks {"nano", "vim", "nvim", "micro", "code", "gedit", "vi", "notepad.exe", "notepad++.exe", "wordpad.exe", "word.exe"};

        std::string pathContent(getenv("PATH"));
        std::vector<fs::path> paths;

        // split paths by : or ; (: for posix, ; for windows)
        auto strings = regexSplit(pathContent, std::regex("[:;]"));
        std::transform(strings.begin(), strings.end(), std::back_inserter(paths), [](const std::string& path) { return fs::path(path); });

        for (const auto& path : paths)
            for (const auto& fallback : fallbacks)
                if (fs::exists(path / fallback)) return fallback;

        return std::nullopt;
    };

    auto editor = preferredEditor();

    if (!editor) editor = fallbackEditor();

    return editor;
}