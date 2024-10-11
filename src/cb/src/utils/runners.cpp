/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2024 Jackson Huff and other contributors on GitHub.com
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

std::optional<std::string> findUsableScriptRunner() {
    auto preferredScriptRunner = []() -> std::optional<std::string> {
        if (auto runner = getenv("CLIPBOARD_SCRIPT_RUNNER"); runner != nullptr) return runner;
        return std::nullopt;
    };

    auto fallbackScriptRunner = []() -> std::optional<std::string> {
#if defined(_WIN32) || defined(_WIN64)
        constexpr std::array fallbacks {"cmd.exe", "powershell.exe", "wsl.exe", "bash.exe", "sh.exe", "zsh.exe", "fish.exe", "pwsh.exe"};
#else
        constexpr std::array fallbacks {"bash", "sh", "zsh", "ksh", "csh", "tcsh", "dash", "fish"};
#endif
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

    auto runner = preferredScriptRunner();

    if (!runner) runner = fallbackScriptRunner();

    return runner;
}