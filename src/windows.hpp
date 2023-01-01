#pragma once
#include <filesystem>
#include <vector>
#include <string_view>
#include "clipboard.hpp"

namespace fs = std::filesystem;

template<typename char_t>
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
void getWindowsClipboardDataFiles(void* clipboardPointer);
void getWindowsClipboardDataPipe(void* clipboardPointer);
void setWindowsClipboardDataPipe();
void setWindowsClipboardDataFiles();