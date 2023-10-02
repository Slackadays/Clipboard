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
#include "clipboard.hpp"
#include <climits>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
#define STDIN_FILENO 0
#define read _read
#include "platforms/windows.hpp"
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#endif

bool isARemoteSession() {
    if (getenv("SSH_CLIENT") || getenv("SSH_TTY") || getenv("SSH_CONNECTION")) return true;
    return false;
}

ClipboardContent getRemoteClipboard() {
    if (!isARemoteSession() || !is_tty.out) return {};

    std::string response;

    auto requestAndReadResponse = [&] {
        printf("\033]52;c;?\007");
        fflush(stdout);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

#if defined(_WIN32) || defined(_WIN64)
        DWORD bytesAvailable = 0;
        PeekNamedPipe(GetStdHandle(STD_INPUT_HANDLE), nullptr, 0, nullptr, &bytesAvailable, nullptr);
        if (bytesAvailable < 8) return;
#endif

        std::array<char, 65536> buffer;
        size_t n = 0;
        while ((n = read(STDIN_FILENO, buffer.data(), buffer.size())) > 8)
            response += std::string(buffer.data(), n);
    };

    stopIndicator();

    makeTerminalRaw();

    requestAndReadResponse();

    makeTerminalNormal();

    startIndicator();

    // std::cerr << "response: " << response << std::endl;

    // remove terminal control characters
    response = response.substr(response.find_last_of(';') + 1);
    response = response.substr(0, response.size() - 2); // remove the \007 character and something before it

    if (response.empty()) return {};

    // std::cerr << "response character 2 ID: " << static_cast<int>(response.at(1)) << std::endl;
    // std::cerr << "response: " << response << std::endl;
    // std::cerr << "response size: " << response.size() << std::endl;

    auto fromBase64 = [](const std::string_view& content) {
        static_assert(CHAR_BIT == 8);
        constexpr std::string_view convertToChar("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
        std::string output;
        output.reserve(content.size() * 3 / 4);
        for (size_t i = 0; i < content.size(); i += 4) {
            auto first = content.at(i);
            auto second = content.at(i + 1);
            auto byte = static_cast<char>((convertToChar.find(first) << 2) | (convertToChar.find(second) >> 4));
            output += byte;
            if (i + 2 < content.size() && content.at(i + 2) != '=') {
                auto third = content.at(i + 2);
                byte = static_cast<char>(((convertToChar.find(second) & 0x0F) << 4) | (convertToChar.find(third) >> 2));
                output += byte;
                if (i + 3 < content.size() && content.at(i + 3) != '=') {
                    auto fourth = content.at(i + 3);
                    byte = static_cast<char>(((convertToChar.find(third) & 0x03) << 6) | convertToChar.find(fourth));
                    output += byte;
                }
            }
        }
        return output;
    };

    // std::cerr << "content: " << fromBase64(response) << std::endl;
    // std::cerr << "content size: " << fromBase64(response).size() << std::endl;

    return ClipboardContent(fromBase64(response));
}

void convertFromGUIClipboard(const std::string& text) {
    if (fs::exists(path.data.raw) && (fileContents(path.data.raw).value() == text || text.size() == 4096 && fileContents(path.data.raw).value().size() > 4096))
        return; // check if 4096b long because remote clipboard is up to 4096b long
    auto regexes = path.ignoreRegexes();
    for (const auto& regex : regexes)
        if (std::regex_match(text, regex)) return;
    path.makeNewEntry();
    writeToFile(path.data.raw, text);
}

void convertFromGUIClipboard(const ClipboardPaths& clipboard) {
    auto regexes = path.ignoreRegexes();
    auto paths = clipboard.paths();
    for (const auto& regex : regexes)
        for (auto&& path : paths)
            if (std::regex_match(path.filename().string(), regex)) paths.erase(std::find(paths.begin(), paths.end(), path));

    // Only clear the temp directory if all files in the clipboard are outside the temp directory
    // This avoids the situation where we delete the very files we're trying to copy
    auto filesHaveChanged = std::all_of(paths.begin(), paths.end(), [](auto& path) {
        auto filename = path.filename().empty() ? path.parent_path().filename() : path.filename();
        // check if the filename of the provided path does not exist in the temp directory
        if (!fs::exists(::path.data / filename)) return true;

        // check if the file sizes are different if it's not a directory
        if (!fs::is_directory(path) && fs::file_size(path) != fs::file_size(::path.data / filename)) return true;

        // check if the file contents are different if it's not a directory
        if (!fs::is_directory(path) && fileContents(path).value() != fileContents(::path.data / filename)) return true;

        return false;
    });

    auto eligibleForCopying = std::all_of(paths.begin(), paths.end(), [](auto& path) {
        if (!fs::exists(path)) return false;
        return true;
    });

    if (filesHaveChanged && eligibleForCopying && !paths.empty()) path.makeNewEntry();

    for (auto&& path : paths) {
        if (!fs::exists(path)) continue;

        auto target = ::path.data / path.filename();

        if (fs::exists(target) && fs::equivalent(path, target)) continue;

        try {
            fs::copy(path, target, copying.opts | fs::copy_options::create_hard_links);
        } catch (const fs::filesystem_error& e) {
            try {
                fs::copy(path, target, copying.opts);
            } catch (const fs::filesystem_error& e) {} // Give up
        }
    }

    if (clipboard.action() == ClipboardPathsAction::Cut) {
        std::ofstream originalFiles {path.metadata.originals};
        for (auto&& path : paths)
            originalFiles << path.string() << std::endl;
    }
}

ClipboardContent thisClipboard() {
    Clipboard default_cb(std::string(constants.default_clipboard_name));
    if (fs::exists(default_cb.metadata.originals) && GUIClipboardSupportsCut) {
        std::vector<fs::path> files;

        for (const auto& line : fileLines(default_cb.metadata.originals))
            files.emplace_back(line);

        return {std::move(files), ClipboardPathsAction::Cut};
    }

    if (!copying.buffer.empty()) return {copying.buffer, copying.mime};

    if (default_cb.holdsRawDataInCurrentEntry()) return {fileContents(default_cb.data.raw).value(), std::string(inferMIMEType(fileContents(default_cb.data.raw).value()).value_or("text/plain"))};

    if (!copying.items.empty()) {
        std::vector<fs::path> paths;

        paths.assign(fs::directory_iterator(default_cb.data), fs::directory_iterator {});

        return ClipboardContent(ClipboardPaths(std::move(paths)));
    }

    return {};
}

void syncWithRemoteClipboard(bool force) {
    using enum ClipboardContentType;
    if ((!isAClearingAction() && clipboard_name == constants.default_clipboard_name && clipboard_entry == constants.default_clipboard_entry && action != Action::Status)
        || force) { // exclude Status because it does this manually
        ClipboardContent content;
        if (envVarIsTrue("CLIPBOARD_NOREMOTE")) return;
        content = getRemoteClipboard();
        if (content.type() == Text) {
            convertFromGUIClipboard(content.text());
            copying.mime = !content.mime().empty() ? content.mime() : inferMIMEType(content.text()).value_or("text/plain");
        }
    }
}

void syncWithGUIClipboard(bool force) {
    using enum ClipboardContentType;
    if ((!isAClearingAction() && clipboard_name == constants.default_clipboard_name && clipboard_entry == constants.default_clipboard_entry && action != Action::Status)
        || force) { // exclude Status because it does this manually
        ClipboardContent content;
        if (envVarIsTrue("CLIPBOARD_NOGUI")) return;
        content = getGUIClipboard(preferred_mime);
        if (content.type() == Text) {
            convertFromGUIClipboard(content.text());
            copying.mime = !content.mime().empty() ? content.mime() : inferMIMEType(content.text()).value_or("text/plain");
        } else if (content.type() == Paths) {
            convertFromGUIClipboard(content.paths());
            copying.mime = "text/uri-list";
        }
    }
}

void syncWithExternalClipboards(bool force) {
    using enum ClipboardContentType;
    if ((!isAClearingAction() && clipboard_name == constants.default_clipboard_name && clipboard_entry == constants.default_clipboard_entry && action != Action::Status)
        || force) { // exclude Status because it does this manually
        ClipboardContent content;
        if (!envVarIsTrue("CLIPBOARD_NOREMOTE")) content = getRemoteClipboard();
        if (content.type() == Empty && !envVarIsTrue("CLIPBOARD_NOGUI")) content = getGUIClipboard(preferred_mime);
        if (content.type() == Text) {
            convertFromGUIClipboard(content.text());
            copying.mime = !content.mime().empty() ? content.mime() : inferMIMEType(content.text()).value_or("text/plain");
        } else if (content.type() == Paths) {
            convertFromGUIClipboard(content.paths());
            copying.mime = !content.mime().empty() ? content.mime() : "text/uri-list";
        }
        available_mimes = content.availableTypes();
    }
}

void writeToRemoteClipboard(const ClipboardContent& content) {
    if (!isARemoteSession()) return;
    if (content.type() != ClipboardContentType::Text) {
        printf("\033]52;c;\007");
        fflush(stdout);
        return;
    }
    auto toBase64 = [](const std::string_view& content) {
        static_assert(CHAR_BIT == 8);
        constexpr std::string_view convertToChar("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
        std::string output;
        output.reserve(4 * ((content.size() + 2) / 3));
        for (size_t i = 0; i < content.size(); i += 3) {
            auto first = static_cast<unsigned char>(content.at(i));
            output += convertToChar.at(first >> 2);
            if (i + 1 < content.size()) {
                auto second = static_cast<unsigned char>(content.at(i + 1));
                output += convertToChar.at(((first & 0x03) << 4) | (second >> 4));
                if (i + 2 < content.size()) {
                    auto third = static_cast<unsigned char>(content.at(i + 2));
                    output += convertToChar.at(((second & 0x0F) << 2) | (third >> 6));
                    output += convertToChar.at(third & 0x3F);
                } else {
                    output += convertToChar.at((second & 0x0F) << 2);
                    output += "=";
                }
            } else {
                output += convertToChar.at((first & 0x03) << 4);
                output += "==";
            }
        }
        return output;
    };
    printf("\033]52;c;\007"); // clear clipboard first
    if (auto term = getenv("TERM"); term && !strcmp(term, "xterm-kitty")) {
        for (size_t i = 0; i < content.text().size(); i += 4096) // kitty has a limit of 4096 bytes per write
            printf("\033]52;c;%s\007", toBase64(content.text().substr(i, 4096)).data());
    } else
        printf("\033]52;c;%s\007", toBase64(content.text()).data());
    fflush(stdout);
}

void updateExternalClipboards(bool force) {
    if ((isAWriteAction() && clipboard_name == constants.default_clipboard_name) || force) { // only update GUI clipboard on write operations
        auto thisContent = thisClipboard();
        if (!envVarIsTrue("CLIPBOARD_NOGUI")) writeToGUIClipboard(thisContent);
        if (!envVarIsTrue("CLIPBOARD_NOREMOTE")) writeToRemoteClipboard(thisContent);
    }
}

void setupGUIClipboardDaemon() {
    if (envVarIsTrue("CLIPBOARD_NOGUI")) return;

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    auto pid = fork();
    if (pid > 0) return;
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }
    if (chdir("/") < 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

#if defined(__linux__)
    // check if there is already a cb daemon by checking /proc for a process which has an exe symlink entry that points to a binary called "cb" and which does not have stdin or stdout file descriptors

    try {
        for (const auto& entry : fs::directory_iterator("/proc")) {
            if (!entry.is_directory()) continue;
            auto exe = entry.path() / "exe";
            if (!fs::exists(exe)) continue;
            auto exeTarget = fs::read_symlink(exe);
            if (exeTarget.filename() != "cb") continue;
            auto fd = entry.path() / "fd";
            if (fs::exists(fd / "0") || fs::exists(fd / "1") || fs::exists(fd / "2")) continue;
            // found a cb daemon
            exit(EXIT_SUCCESS);
        }
    } catch (...) {}

    // std::cerr << "Starting cb daemon" << std::endl;
#endif
#elif defined(_WIN32) | defined(_WIN64)

#endif
    while (fs::exists(path)) {
        path.getLock();
        syncWithGUIClipboard(true);
        path.releaseLock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        path = Clipboard(std::string(constants.default_clipboard_name));
    }

    exit(EXIT_SUCCESS);
}