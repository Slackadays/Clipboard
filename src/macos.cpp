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

#include "gui.hpp"

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
        char** files = getFiles();
        std::vector<fs::path> fileVector;
        for (int i = 0; files[i] != nullptr; i++) {
            fileVector.push_back(files[i]);
        }
        ClipboardPaths paths(fileVector);
        delete[] files;
        return ClipboardContent(paths);
    }
    if (thisClipboardHoldsText) {
        std::string text(getText());
        return ClipboardContent(text);
    }
    return ClipboardContent();
}

void writeToGUIClipboard(ClipboardContent& clipboard) {
    if (clipboard.type() == ClipboardContentType::Text) {
        writeText(clipboard.text().c_str());
    } else if (clipboard.type() == ClipboardContentType::Paths) {
        std::vector<fs::path> paths(clipboard.paths().paths());
        const char** files = new const char*[paths.size() + 1];
        for (int i = 0; i < paths.size(); i++) {
            files[i] = paths[i].c_str();
        }
        files[paths.size()] = nullptr;
        writeFiles(files);
        delete[] files;
    }
}