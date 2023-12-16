/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    SPDX-License-Identifier: GPL-3.0-or-later
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

std::ostream& operator<<(std::ostream& stream, const ClipboardPathsAction& action) {
    if (action == ClipboardPathsAction::Copy) {
        stream << "copy";
    } else if (action == ClipboardPathsAction::Cut) {
        stream << "cut";
    } else {
        stream << "unknown";
    }

    return stream;
}

ClipboardPaths::ClipboardPaths(std::vector<fs::path>&& paths, ClipboardPathsAction action) : m_action(action), m_paths(std::move(paths)) {}

ClipboardPaths::ClipboardPaths(const std::vector<fs::path>& paths, ClipboardPathsAction action) : m_action(action), m_paths(paths) {}

ClipboardContent::ClipboardContent(const std::string& text, const std::string& mime) : m_type(ClipboardContentType::Text), mime_type(mime), m_data(text) {}

ClipboardContent::ClipboardContent(std::string&& text, const std::string& mime) : m_type(ClipboardContentType::Text), mime_type(mime), m_data(std::move(text)) {}

ClipboardContent::ClipboardContent(const ClipboardPaths& paths) : m_type(ClipboardContentType::Paths), mime_type("text/uri-list"), m_data(paths) {}

ClipboardContent::ClipboardContent(ClipboardPaths&& paths) : m_type(ClipboardContentType::Paths), mime_type("text/uri-list"), m_data(std::move(paths)) {}

ClipboardContent::ClipboardContent(std::vector<fs::path>&& paths, ClipboardPathsAction action) : ClipboardContent(ClipboardPaths(std::move(paths), action)) {}

ClipboardContent::ClipboardContent(const std::vector<fs::path>& paths, ClipboardPathsAction action) : ClipboardContent(ClipboardPaths(paths, action)) {}

ClipboardContent::ClipboardContent() : m_type(ClipboardContentType::Empty), m_data(nullptr) {}
