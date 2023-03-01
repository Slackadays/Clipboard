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
#include <clipboard/gui.hpp>
#include <string_view>

std::string_view inferMIMEType(const std::string& temporaryContent) {
    std::string_view content(temporaryContent);

    // jpeg xl
    if (content.size() >= 12 && content.substr(0, 12) == "\x00\x00\x00\x0C\x4A\x58\x4C\x20\x0D\x0A\x87\x0A") return "image/jxl";

    // xml
    else if (content.size() >= 24 && content.substr(0, 24) == "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
        return "text/xml";

    // html
    else if (content.size() >= 15 && content.substr(0, 15) == "<!DOCTYPE html>")
        return "text/html";

    // png
    else if (content.size() >= 8 && content.substr(0, 8) == "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A")
        return "image/png";

    // jpeg
    else if (content.size() >= 3 && content.substr(0, 3) == "\xFF\xD8\xFF")
        return "image/jpeg";

    // gif
    else if (content.size() >= 6 && (content.substr(0, 6) == "GIF87a" || content.substr(0, 6) == "GIF89a"))
        return "image/gif";

    // webp
    else if (content.size() >= 12 && content.substr(0, 12) == "RIFF\x00\x00\x00\x00WEBPVP8 ")
        return "image/webp";

    // bmp
    else if (content.size() >= 2 && content.substr(0, 2) == "BM")
        return "image/bmp";

    // tiff
    else if (content.size() >= 4 && (content.substr(0, 4) == "II\x2A\x00" || content.substr(0, 4) == "MM\x00\x2A"))
        return "image/tiff";

    // zip
    else if (content.size() >= 4 && (content.substr(0, 4) == "PK\x03\x04" || content.substr(0, 4) == "PK\x05\x06" || content.substr(0, 4) == "PK\x07\x08"))
        return "application/zip";

    else
        return "text/plain";
}
