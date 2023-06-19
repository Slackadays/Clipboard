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
#include <clipboard/x11wl/mime.hpp>

#include <clipboard/logging.hpp>
#include <clipboard/utils.hpp>

using namespace std::literals;

using MimeOptionU = std::underlying_type_t<MimeOption>;

MimeOption operator|(const MimeOption& a, const MimeOption& b) {
    return static_cast<MimeOption>(static_cast<MimeOptionU>(a) | static_cast<MimeOptionU>(b));
}

bool hasFlag(const MimeOption& value, const MimeOption& flag) {
    auto valueUnderlying = static_cast<MimeOptionU>(value);
    auto flagUnderlying = static_cast<MimeOptionU>(flag);

    return (valueUnderlying & flagUnderlying) != (MimeOptionU {});
}

decltype(MimeType::s_typesByName) MimeType::s_typesByName {initializeTypes()};

decltype(MimeType::s_typesByName) MimeType::initializeTypes() {
    decltype(s_typesByName) result {};

    auto insert = [&](const char* name, ClipboardContentType type, MimeOption options = MimeOption::NoOption) {
        result.insert({name, MimeType {static_cast<unsigned int>(result.size()), name, type, options}});
    };

    insert("x-special/gnome-copied-files", ClipboardContentType::Paths, MimeOption::IncludeAction | MimeOption::EncodePaths);
    insert("application/x-kde-cutselection", ClipboardContentType::Paths, MimeOption::IncludeAction | MimeOption::EncodePaths);
    insert("text/uri-list", ClipboardContentType::Paths, MimeOption::EncodePaths);
    insert("image/jxl", ClipboardContentType::Binary);
    insert("image/png", ClipboardContentType::Binary);
    insert("image/jpeg", ClipboardContentType::Binary);
    insert("image/bmp", ClipboardContentType::Binary);
    insert("image/gif", ClipboardContentType::Binary);
    insert("image/tiff", ClipboardContentType::Binary);
    insert("image/gif", ClipboardContentType::Binary);
    insert("image/webp", ClipboardContentType::Binary);
    insert("audio/mpeg", ClipboardContentType::Binary);
    insert("audio/ogg", ClipboardContentType::Binary);
    insert("audio/wav", ClipboardContentType::Binary);
    insert("video/mp4", ClipboardContentType::Binary);
    insert("video/webm", ClipboardContentType::Binary);
    insert("application/zip", ClipboardContentType::Binary);
    insert("application/pdf", ClipboardContentType::Binary);
    insert("text/plain;charset=utf-8", ClipboardContentType::Text);
    insert("UTF8_STRING", ClipboardContentType::Text);
    insert("text/plain", ClipboardContentType::Text);
    insert("STRING", ClipboardContentType::Text);
    insert("TEXT", ClipboardContentType::Text);
    insert("GTK_TEXT_BUFFER_CONTENTS", ClipboardContentType::Text);

    return result;
}

std::optional<MimeType> MimeType::find(std::string_view name) {
    auto&& it = s_typesByName.find(name);
    if (it == s_typesByName.end()) {
        return {};
    }

    return it->second;
}

bool MimeType::supports(const ClipboardContent& content) const {
    static auto type = (content.type() == ClipboardContentType::Text || content.type() == ClipboardContentType::Binary) ? inferMIMEType(content.text()) : std::nullopt;

    if (type.has_value() && type.value() == m_name) {
        return true;
    }

    if (m_type == content.type()) {
        return true;
    }

    if (m_type == ClipboardContentType::Text && content.type() == ClipboardContentType::Paths) {
        return true;
    }

    return false;
}

ClipboardContent MimeType::decode(std::istream& stream) const {
    if (m_type == ClipboardContentType::Text || m_type == ClipboardContentType::Binary) {
        return decodeText(stream);
    }

    if (m_type == ClipboardContentType::Paths) {
        return decodePaths(stream);
    }

    debugStream << "Unknown clipboard content type, ignoring decode request" << std::endl;
    return {};
}

ClipboardContent MimeType::decodePaths(std::istream& stream) const {
    ClipboardPathsAction action = ClipboardPathsAction::Copy;
    std::vector<fs::path> paths {};
    while (!stream.eof()) {
        std::string line;
        std::getline(stream, line);

        if (line.empty()) {
            continue;
        }

        if (isIncludeAction()) {
            if (line == "copy"sv) {
                action = ClipboardPathsAction::Copy;
                continue;
            }

            if (line == "cut"sv) {
                action = ClipboardPathsAction::Cut;
                continue;
            }
        }

        if (isEncodePaths()) {
            if (line.starts_with("file://"sv)) {
                line.erase(0, "file://"sv.size());
                line = urlDecode(line);
            }
        }

        std::erase(line, '\r'); // Sanitize carriage returns from KDE

        paths.emplace_back(line);
    }

    debugStream << "Read " << paths.size() << " paths with action " << action << std::endl;

    return {std::move(paths), action};
}

ClipboardContent MimeType::decodeText(std::istream& stream) const {
    std::ostringstream strStream;
    strStream << stream.rdbuf();
    auto str = std::move(strStream).str();

    debugStream << "Read " << str.size() << " characters from the system" << std::endl;

    return {std::move(str)};
}

bool MimeType::encode(const ClipboardContent& clipboard, std::ostream& stream) const {
    if (!supports(clipboard)) {
        debugStream << "Clipboard is incompatible with " << m_name << ", refusing to encode" << std::endl;
        return false;
    }

    if (clipboard.type() == ClipboardContentType::Text || clipboard.type() == ClipboardContentType::Binary) {
        return encode(clipboard.text(), stream);
    }

    if (clipboard.type() == ClipboardContentType::Paths) {
        return encode(clipboard.paths(), stream);
    }

    debugStream << "Unknown clipboard content type, refusing to encode" << std::endl;
    return false;
}

bool MimeType::encode(const std::string& text, std::ostream& stream) const {
    stream << text;
    return true;
}

bool MimeType::encode(const ClipboardPaths& paths, std::ostream& stream) const {
    if (isIncludeAction()) {
        if (paths.action() == ClipboardPathsAction::Cut) {
            stream << "cut" << std::endl;
        } else {
            stream << "copy" << std::endl;
        }
    }

    bool first = true;
    for (auto&& path : paths.paths()) {
        if (!first) {
            stream << std::endl;
        }

        if (isEncodePaths()) {
            stream << "file://" << urlEncode(path.string());
        } else {
            stream << path.string();
        }

        first = false;
    }

    return true;
}

bool MimeType::encode(const ClipboardContent& clipboard, std::string_view mime, std::ostream& stream) {
    auto type = find(mime);
    if (!type.has_value()) {
        debugStream << "Request MIME Type " << mime << " not recognized, refusing" << std::endl;
        return false;
    }

    if (type->isChooseBestType()) {

        auto bestType = std::find_if(s_typesByName.begin(), s_typesByName.end(), [&](auto&& entry) -> bool {
            auto&& value = entry.second;
            return value.supports(clipboard) && !value.isChooseBestType();
        });

        if (bestType == s_typesByName.end()) {
            throw SimpleException("Unable to find proper target");
        }

        type.emplace(bestType->second);
    }

    return type->encode(clipboard, stream);
}
