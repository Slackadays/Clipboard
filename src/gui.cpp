/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
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
#include "clipboard.hpp"
#include "gui.hpp"

#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

void readDataFromGUIClipboard(const std::string& text) {
    forceClearTempDirectory();
    std::ofstream output(filepath.main / constants.pipe_file);
    output << text;
}

void readDataFromGUIClipboard(const ClipboardPaths& clipboard) {
    // Only clear the temp directory if all files in the clipboard are outside the temp directory
    // This avoids the situation where we delete the very files we're trying to copy
    auto allOutsideFilepath = std::all_of(clipboard.paths().begin(), clipboard.paths().end(), [](auto& path) {
        auto relative = fs::relative(path, filepath.main);
        auto firstElement = *(relative.begin());
        return firstElement == fs::path("..");
    });

    if (allOutsideFilepath) {
        forceClearTempDirectory();
    }

    for (auto&& path : clipboard.paths()) {
        if (!fs::exists(path)) {
            continue;
        }

        auto target = filepath.main / path.filename();
        if (fs::exists(target) && fs::equivalent(path, target)) {
            continue;
        }

        try {
            fs::copy(path, target, copying.opts | fs::copy_options::create_hard_links);
        } catch (const fs::filesystem_error& e) {
            try {
                fs::copy(path, target, copying.opts);
            } catch (const fs::filesystem_error& e) {
                // Give up
            }
        }
    }

    if (clipboard.action() == ClipboardPathsAction::Cut) {
        std::ofstream originalFiles { filepath.original_files };
        for (auto&& path : clipboard.paths()) {
            originalFiles << path.string() << std::endl;
        }
    }
}

ClipboardContent getThisClipboard() {
    if (fs::is_regular_file(filepath.main / constants.pipe_file)) {
        std::string content;
        std::ifstream file(filepath.main / constants.pipe_file);
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        return ClipboardContent(content);
    } else {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(filepath.main)) {
            files.push_back(entry.path());
        }
        ClipboardPaths paths(ClipboardPathsAction::Copy, files);
        return ClipboardContent(paths);
    }
}