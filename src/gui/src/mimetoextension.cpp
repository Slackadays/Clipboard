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

std::optional<std::string_view> MIMETypeToExtension(const std::string_view& type) {
    // jpeg
    if (type == "image/jpeg") return "jpg";

    // png
    if (type == "image/png") return "png";

    // gif
    if (type == "image/gif") return "gif";

    // bmp
    if (type == "image/bmp") return "bmp";

    // svg
    if (type == "image/svg+xml") return "svg";

    // html
    if (type == "text/html") return "html";

    // jpeg xl
    if (type == "image/jxl") return "jxl";

    // jpeg 2000
    if (type == "image/jp2") return "jp2";

    // deb
    if (type == "application/vnd.debian.binary-package") return "deb";

    // rpm
    if (type == "application/x-rpm") return "rpm";

    // zip
    if (type == "application/zip") return "zip";

    // tar
    if (type == "application/x-tar") return "tar";

    // tar.gz
    if (type == "application/gzip") return "tar.gz";

    // tar.xz
    if (type == "application/x-xz") return "tar.xz";

    // tar.bz2
    if (type == "application/x-bzip2") return "tar.bz2";

    // mkv
    if (type == "video/x-matroska") return "mkv";

    // mp4
    if (type == "video/mp4") return "mp4";

    // mp3
    if (type == "audio/mpeg") return "mp3";

    // ogg
    if (type == "audio/ogg") return "ogg";

    // wav
    if (type == "audio/wav") return "wav";

    // webm
    if (type == "video/webm") return "webm";

    // pdf
    if (type == "application/pdf") return "pdf";

    // docx
    if (type == "application/vnd.openxmlformats-officedocument.wordprocessingml.document") return "docx";

    // odt
    if (type == "application/vnd.oasis.opendocument.text") return "odt";

    // doc
    if (type == "application/msword") return "doc";

    // exe
    if (type == "application/x-msdownload") return "exe";

    // tiff
    if (type == "image/tiff") return "tiff";

    // ico
    if (type == "image/vnd.microsoft.icon") return "ico";

    // txt
    if (type == "text/plain") return "txt";

    return std::nullopt;
}