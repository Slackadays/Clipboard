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
#include <iostream>
#include <memory>

#include "clipboard.hpp"

extern "C" {
char const* textContent();
char** fileContent();

void writeText(char const* text);
void writeFiles(char const** files);
}

ClipboardContent getGUIClipboard() {
    if (char** files = fileContent(); files != nullptr) {
        std::vector<fs::path> fileVector;
        for (int i = 0; files[i] != nullptr; i++) {
            fileVector.push_back(files[i]);
        }
        delete[] files;
        ClipboardPaths paths(fileVector);
        return ClipboardContent(paths);
    } else if (std::string text(textContent()); text != "") {
        return ClipboardContent(text);
    }
    return ClipboardContent();
}

void writeToGUIClipboard(ClipboardContent const& clipboard) {
    if (clipboard.type() == ClipboardContentType::Text) {
        writeText(clipboard.text().c_str());
    } else if (clipboard.type() == ClipboardContentType::Paths) {
        std::vector<fs::path> paths(clipboard.paths().paths());
        std::unique_ptr<char const*[]> files = std::make_unique<char const*[]>(paths.size() + 1);
        for (int i = 0; i < paths.size(); i++) {
            files[i] = paths[i].c_str();
        }
        files[paths.size()] = nullptr;
        writeFiles(files.get());
    }
}