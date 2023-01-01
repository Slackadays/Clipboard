#include <io.h>
#include <Windows.h>
#include <Shlobj.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "clipboard.hpp"
#include "windows.hpp"



void onWindowsError(const std::string_view function) {
    auto errorCode = GetLastError();

    char* errorMessage;
    FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            errorCode,
            0,
            static_cast<LPTSTR>(static_cast<void*>(&errorMessage)),
            0,
            nullptr
    );
    std::cerr << function << ": " << errorMessage << std::endl;
    exit(EXIT_FAILURE);
}

void getWindowsClipboardDataFiles(void* clipboardPointer) {
    auto dropfiles = static_cast<DROPFILES*>(clipboardPointer);
    auto offset = dropfiles->pFiles;

    auto filesList = static_cast<void*>(static_cast<char*>(clipboardPointer) + offset);
    std::vector<fs::path> paths;

    if (dropfiles->fWide == TRUE) {
        decodeWindowsDropfilesPaths<wchar_t>(filesList, paths);
    } else {
        decodeWindowsDropfilesPaths<char>(filesList, paths);
    }

    // Only clear the temp directory if all files in the clipboard are outside the temp directory
    // This avoids the situation where we delete the very files we're trying to copy
    auto allOutsideFilepath = std::all_of(paths.begin(), paths.end(), [](auto& path) {
        auto relative = fs::relative(path, main_filepath);
        auto firstElement = *(relative.begin());
        return firstElement == fs::path("..");
    });

    if (allOutsideFilepath) {
        forceClearTempDirectory();
    }

    for (const auto& path : paths) {
        if (!fs::exists(path)) {
            continue;
        }

        auto target = main_filepath / path.filename();
        if (fs::exists(target) && fs::equivalent(path, target)) {
            continue;
        }

        try {
            fs::copy(path, target, opts | fs::copy_options::create_hard_links);
        } catch (const fs::filesystem_error& e) {
            try {
                fs::copy(path, target, opts);
            } catch (const fs::filesystem_error& e) {
                // Give up
            }
        }
    }
}

void getWindowsClipboardDataPipe(void* clipboardPointer) {
    forceClearTempDirectory();

    auto utf16 = static_cast<wchar_t*>(clipboardPointer);

    auto utf8Len = WideCharToMultiByte(
        CP_UTF8,
        0,
        utf16,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );
    if (utf8Len <= 0) {
        onWindowsError("WideCharToMultiByte");
    }

    std::vector<char> utf8Buffer(utf8Len);
    auto bytesWritten = WideCharToMultiByte(
        CP_UTF8,
        0,
        utf16,
        -1,
        &utf8Buffer[0],
        utf8Len,
        nullptr,
        nullptr
    );
    if (bytesWritten <= 0) {
        onWindowsError("WideCharToMultiByte");
    }

    std::string utf8(&utf8Buffer[0]);

    std::ofstream output(main_filepath / pipe_file);
    output << utf8;
}

oid setWindowsClipboardDataPipe() {
    std::ifstream file(main_filepath / pipe_file);
    std::vector<char> utf8Data(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    auto utf16Len = MultiByteToWideChar(
        CP_UTF8,
        0,
        &utf8Data[0],
        utf8Data.size(),
        nullptr,
        0
    );
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
    auto bytesWritten = MultiByteToWideChar(
        CP_UTF8,
        0,
        &utf8Data[0],
        utf8Data.size(),
        clipboardPointer,
        bufferLen
    );
    if (bytesWritten <= 0) {
        onWindowsError("MultiByteToWideChar");
    }

    if (GlobalUnlock(clipboardHandle) == 0 && GetLastError() != NO_ERROR) {
        onWindowsError("GlobalUnlock");
    }
    if (SetClipboardData(CF_UNICODETEXT, clipboardHandle) == nullptr) {
        onWindowsError("SetClipboardData");
    }
}

void setWindowsClipboardDataFiles() {

    std::vector<wchar_t> data;
    for (const auto& entry : fs::directory_iterator(main_filepath)) {
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