/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
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
#include <filesystem>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

template <typename char_t>
void decodeWindowsDropfilesPaths(void* filesList, std::vector<fs::path>& paths) {

    auto data = static_cast<char_t*>(filesList);
    std::vector<char_t> currentPath;

    while (true) {
        auto c = *data++;
        currentPath.push_back(c);

        if (c == 0) {
            if (currentPath.size() == 1) {
                break;
            }

            paths.emplace_back(&currentPath[0]);
            currentPath.clear();
        }
    }
}

void onWindowsError(const std::string_view function);
std::vector<fs::path> getWindowsClipboardDataFiles(void* clipboardPointer);
std::string getWindowsClipboardDataPipe(void* clipboardPointer);
void setWindowsClipboardDataPipe();
void setWindowsClipboardDataFiles();
ClipboardContent getGUIClipboard(const std::string& requested_mime);
void writeToGUIClipboard(const ClipboardContent& clipboard);
