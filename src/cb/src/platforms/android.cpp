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
#include <iostream>
#include <memory>
#include <string>

#include "../clipboard.hpp"

const bool GUIClipboardSupportsCut = false;

ClipboardContent getGUIClipboard(const std::string& requested_mime) {
    return ClipboardContent();
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    if (clipboard.type() == ClipboardContentType::Text || clipboard.type() == ClipboardContentType::Binary) {

    } else if (clipboard.type() == ClipboardContentType::Paths) {
    }
}

bool playAsyncSoundEffect(const std::valarray<short>& samples) {
    return false;
}