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

#include "os.hpp"

extern "C" {
    bool holdsText();
    char* getText();

    bool holdsFiles();
    char** getFiles();

    void writeText(const char* text);
    void writeFiles(const char** files);
}

ClipboardContent getGUIClipboard() {
    bool thisClipboardHoldsText = holdsText();
    bool thisClipboardHoldsFiles = holdsFiles();
    if (thisClipboardHoldsFiles) {
        std::vector<fs::path> fileVector;
        char** files = getFiles();
        for (int i = 0; files[i] != nullptr; i++) {
            fileVector.push_back(files[i]);
        }
        delete[] files;
        ClipboardPaths paths(fileVector);
        return ClipboardContent(paths);
    }
    if (thisClipboardHoldsText) {
        std::string text(getText());
        return ClipboardContent(text);
    }
    return ClipboardContent();
}

void writeToGUIClipboard(ClipboardContent const& clipboard) {
    if (clipboard.type() == ClipboardContentType::Text) {
        writeText(clipboard.text().c_str());
    } else if (clipboard.type() == ClipboardContentType::Paths) {
        std::vector<fs::path> paths(clipboard.paths().paths());
        std::unique_ptr<const char*[]> files = std::make_unique<const char*[]>(paths.size() + 1);
        for (int i = 0; i < paths.size(); i++) {
            files[i] = paths[i].c_str();
        }
        files[paths.size()] = nullptr;
        writeFiles(files.get());
    }
}