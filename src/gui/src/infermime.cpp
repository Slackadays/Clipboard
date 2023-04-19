/*  The Clipboard Project - Cut, copy, and paste anything, anywhere, all from the terminal.
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
#include <clipboard/gui.hpp>
#include <optional>
#include <string_view>

using namespace std::string_view_literals;

std::optional<std::string_view> inferMIMEType(const std::string& temporaryContent) {
    std::string_view content(temporaryContent);
    auto beginning_matches = [&](const std::string_view& pattern) {
        if (content.size() < pattern.size()) return false;
        return content.substr(0, pattern.size()) == pattern;
    };

    // jpeg xl
    if (beginning_matches("\x00\x00\x00\x0C\x4A\x58\x4C\x20\x0D\x0A\x87\x0A"sv)) return "image/jxl";

    // xml
    if (beginning_matches("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"sv)) return "text/xml";

    // html
    if (beginning_matches("<!DOCTYPE html>"sv)) return "text/html";

    // png
    if (beginning_matches("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"sv)) return "image/png";

    // jpeg
    if (beginning_matches("\xFF\xD8\xFF"sv)) return "image/jpeg";

    // gif
    if (beginning_matches("GIF87a"sv) || beginning_matches("GIF89a"sv)) return "image/gif";

    // webp
    if (beginning_matches("RIFF\x00\x00\x00\x00WEBPVP8 "sv)) return "image/webp";

    // bmp
    if (beginning_matches("BM"sv)) return "image/bmp";

    // tiff
    if (beginning_matches("II\x2A\x00"sv) || beginning_matches("MM\x00\x2A"sv)) return "image/tiff";

    // zip
    if (beginning_matches("PK\x03\x04"sv) || beginning_matches("PK\x05\x06"sv) || beginning_matches("PK\x07\x08"sv)) return "application/zip";

    return std::nullopt;
}
