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
#include <Clipboard.h>
#include <iostream>
#include <memory>
#include <string>

#include "../clipboard.hpp"

const bool GUIClipboardSupportsCut = false;

ClipboardContent getGUIClipboard(const std::string& requested_mime) {
    std::unique_ptr<BClipboard> gui_clipboard = std::make_unique<BClipboard>("system");
    if (!gui_clipboard->Lock()) return {};
    BMessage* content = gui_clipboard->Data();
    gui_clipboard->Unlock();
    const char* temp;
    ssize_t tempLength = 0;
    if (!content->FindData("text/plain", B_MIME_TYPE, (const void**)&temp, &tempLength)) {
        std::string CBcontent(temp, tempLength);
        return ClipboardContent(CBcontent);
    }
    return {};
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    std::unique_ptr<BClipboard> gui_clipboard = std::make_unique<BClipboard>("system");
    if (!gui_clipboard->Lock()) return;
    gui_clipboard->Clear();
    BMessage* content = (BMessage*)NULL;
    if (content = gui_clipboard->Data()) {
        if (clipboard.type() == ClipboardContentType::Text || clipboard.type() == ClipboardContentType::Binary) {
            content->AddData("text/plain", B_MIME_TYPE, clipboard.text().data(), clipboard.text().length());
        } else if (clipboard.type() == ClipboardContentType::Paths) {
        }
        gui_clipboard->Commit();
    }
    gui_clipboard->Unlock();
}

bool playAsyncSoundEffect(const std::valarray<short>& samples) {
    return false;
}