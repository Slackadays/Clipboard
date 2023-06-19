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
#include "windows.hpp"
#include "../clipboard.hpp"
#include <Shlobj.h>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iostream>
#include <optional>
#include <vector>

const bool GUIClipboardSupportsCut = false;

ClipboardContent getGUIClipboard(const std::string& requested_mime) {
    if (OpenClipboard(nullptr) == 0) {
        onWindowsError("OpenClipboard");
    }

    auto hasFiles = IsClipboardFormatAvailable(CF_HDROP) != 0;
    auto hasText = IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    auto hasAny = hasFiles || hasText;

    ClipboardContent clipboard;
    if (hasAny) {
        HANDLE clipboardHandle;
        if (hasFiles) {
            clipboardHandle = GetClipboardData(CF_HDROP);
        } else {
            clipboardHandle = GetClipboardData(CF_UNICODETEXT);
        }

        if (clipboardHandle == nullptr) {
            onWindowsError("GetClipboardData");
        }

        auto clipboardPointer = GlobalLock(clipboardHandle);
        if (clipboardPointer == nullptr) {
            onWindowsError("GlobalLock");
        }

        if (hasFiles) {
            auto files = getWindowsClipboardDataFiles(clipboardPointer);
            clipboard = {std::move(files)};
        } else {
            clipboard = {getWindowsClipboardDataPipe(clipboardPointer)};
        }

        if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
            onWindowsError("GlobalUnlock");
        }
    }

    if (CloseClipboard() == 0) {
        onWindowsError("CloseClipboard");
    }

    return clipboard;
}

void onWindowsError(const std::string_view function) {
    auto errorCode = GetLastError();

    char* errorMessage;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, static_cast<LPTSTR>(static_cast<void*>(&errorMessage)), 0, nullptr);
    std::cerr << function << ": " << errorMessage << std::endl;
    exit(EXIT_FAILURE);
}

std::vector<fs::path> getWindowsClipboardDataFiles(void* clipboardPointer) {
    auto dropfiles = static_cast<DROPFILES*>(clipboardPointer);
    auto offset = dropfiles->pFiles;

    auto filesList = static_cast<void*>(static_cast<char*>(clipboardPointer) + offset);
    std::vector<fs::path> paths;

    if (dropfiles->fWide == TRUE) {
        decodeWindowsDropfilesPaths<wchar_t>(filesList, paths);
    } else {
        decodeWindowsDropfilesPaths<char>(filesList, paths);
    }

    return std::move(paths);
}

std::string getWindowsClipboardDataPipe(void* clipboardPointer) {
    auto utf16 = static_cast<wchar_t*>(clipboardPointer);

    auto length = wcslen(utf16);

    auto utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16, length, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) {
        onWindowsError("WideCharToMultiByte");
    }

    std::vector<char> utf8Buffer(utf8Len);
    auto bytesWritten = WideCharToMultiByte(CP_UTF8, 0, utf16, length, &utf8Buffer[0], utf8Len, nullptr, nullptr);
    if (bytesWritten <= 0) {
        onWindowsError("WideCharToMultiByte");
    }

    return {utf8Buffer.begin(), utf8Buffer.end()};
}

void setWindowsClipboardDataPipe() {
    std::string utf8Data(fileContents(path.data.raw));

    auto utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Data.data(), utf8Data.size(), nullptr, 0);
    if (utf16Len <= 0) {
        onWindowsError("MultiByteToWideChar");
    }

    auto bufferLen = utf16Len + 1;

    auto bufferSize = bufferLen * sizeof(wchar_t);

    HGLOBAL clipboardHandle = GlobalAlloc(GMEM_MOVEABLE, bufferSize);
    if (clipboardHandle == nullptr) {
        onWindowsError("GlobalAlloc");
    }

    auto clipboardPointer = static_cast<wchar_t*>(GlobalLock(clipboardHandle));
    if (clipboardPointer == nullptr) {
        onWindowsError("GlobalLock");
    }

    ZeroMemory(clipboardPointer, bufferSize);
    auto bytesWritten = MultiByteToWideChar(CP_UTF8, 0, utf8Data.data(), utf8Data.size(), clipboardPointer, bufferLen);
    if (bytesWritten <= 0) {
        onWindowsError("MultiByteToWideChar");
    }

    auto PNG_FORMAT = RegisterClipboardFormatA("PNG");

    UINT clipboardFormat;

    if (inferMIMEType(utf8Data).value_or("text/plain") == "image/png") {
        clipboardFormat = PNG_FORMAT;
    } else {
        clipboardFormat = CF_UNICODETEXT;
    }

    if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
        onWindowsError("GlobalUnlock");
    }
    if (SetClipboardData(clipboardFormat, clipboardHandle) == nullptr) {
        onWindowsError("SetClipboardData");
    }
}

void setWindowsClipboardDataFiles() {

    std::vector<wchar_t> data;
    for (const auto& entry : fs::directory_iterator(path.data)) {
        for (const auto& c : entry.path().wstring()) {
            data.push_back(c);
        }

        data.push_back(0);
    }

    data.push_back(0);

    size_t clipboardSize = sizeof(DROPFILES) + data.size() * sizeof(wchar_t);
    HGLOBAL clipboardHandle = GlobalAlloc(GMEM_MOVEABLE, clipboardSize);
    if (clipboardHandle == nullptr) {
        onWindowsError("GlobalAlloc");
    }

    auto clipboardPointer = GlobalLock(clipboardHandle);
    if (clipboardPointer == nullptr) {
        onWindowsError("GlobalLock");
    }

    ZeroMemory(clipboardPointer, clipboardSize);

    auto dropfiles = static_cast<DROPFILES*>(clipboardPointer);
    dropfiles->pFiles = sizeof(DROPFILES);
    dropfiles->fWide = TRUE;

    auto fileList = static_cast<void*>(static_cast<char*>(clipboardPointer) + sizeof(DROPFILES));
    std::memcpy(fileList, &data[0], data.size() * sizeof(wchar_t));

    if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
        onWindowsError("GlobalUnlock");
    }

    if (SetClipboardData(CF_HDROP, clipboardHandle) == nullptr) {
        onWindowsError("SetClipboardData");
    }
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    if (OpenClipboard(nullptr) == 0) {
        onWindowsError("OpenClipboard");
    }
    if (EmptyClipboard() == 0) {
        onWindowsError("EmptyClipboard");
    }

    if (clipboard.type() == ClipboardContentType::Text || clipboard.type() == ClipboardContentType::Binary) {
        setWindowsClipboardDataPipe();
    } else if (clipboard.type() == ClipboardContentType::Paths) {
        setWindowsClipboardDataFiles();
    }

    if (CloseClipboard() == 0) {
        onWindowsError("CloseClipboard");
    }
}

bool playAsyncSoundEffect(const std::valarray<short>& samples) {
    return false;
}