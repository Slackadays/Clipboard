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

#include "macos.hpp"
extern "C" {
    #include "macos.h"
}

ClipboardContent getGUIClipboard() {
    if (thisClipboardHoldsText()) {
        std::string text(getClipboardText());
        std::cout << "Text: " << text << std::endl;
    }
    if (thisClipboardHoldsFiles()) {
        char** files = getClipboardFiles();
        std::vector<fs::path> fileVector;
        for (int i = 0; files[i] != NULL; i++) {
            fileVector.push_back(files[i]);
        }
        ClipboardPaths paths(ClipboardPathsAction::Copy, fileVector);
        return ClipboardContent(paths);
    }
    return ClipboardContent();
}