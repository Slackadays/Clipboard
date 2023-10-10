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
#include <fstream>

std::optional<std::string> fileContents(const fs::path& path) {
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    errno = 0;
    int fd = open(path.string().data(), O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT)
            return std::nullopt;
        else
            throw std::runtime_error("Couldn't open file " + path.string() + ": " + std::strerror(errno));
    }
    std::string contents;
#if defined(__linux__) || defined(__FreeBSD__)
    std::array<char, 65536> buffer;
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    std::array<char, 16384> buffer;
#else
    std::array<char, PIPE_BUF> buffer;
#endif
    ssize_t bytes_read;
    errno = 0;
    while ((bytes_read = read(fd, buffer.data(), buffer.size())) > 0) {
        contents.append(buffer.data(), bytes_read);
        if (bytes_read < buffer.size() && errno == 0) break; // check if we reached EOF early and not due to an error
    }
    close(fd);
    return contents;
#else
    std::stringstream buffer;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return std::nullopt;
    buffer << file.rdbuf();
    return buffer.str();
#endif
}

std::vector<std::string> fileLines(const fs::path& path) {
    std::vector<std::string> lines;
    std::ifstream input_file(path, std::ios::binary);
    for (std::string line; !input_file.eof();) {
        std::getline(input_file, line);
        if (!line.empty()) lines.emplace_back(line);
    }
    return lines;
}

size_t writeToFile(const fs::path& path, const std::string& content, bool append) {
    std::ofstream file(path, append ? std::ios::app : std::ios::trunc | std::ios::binary);
    file << content;
    return content.size();
}