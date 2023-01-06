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
#pragma once

#include <variant>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

enum class ClipboardPathsAction {
    Copy,
    Cut
};

class ClipboardPaths {
private:
    ClipboardPathsAction m_action;
    std::vector<fs::path> m_paths;

public:
    ClipboardPaths(std::vector<fs::path>&& paths, ClipboardPathsAction action = ClipboardPathsAction::Copy)
        : m_action(action), m_paths(std::move(paths)) { }
    ClipboardPaths(std::vector<fs::path> const& paths, ClipboardPathsAction action = ClipboardPathsAction::Copy)
        : m_action(action), m_paths(paths) { }

    [[nodiscard]] inline ClipboardPathsAction action() const { return m_action; }
    [[nodiscard]] inline std::vector<fs::path> const& paths() const { return m_paths; }
};

enum class ClipboardContentType {
    Empty,
    Text,
    Paths
};

class ClipboardContent {
private:
    ClipboardContentType m_type;
    std::variant<std::nullptr_t, std::string, ClipboardPaths> m_data;

public:
    ClipboardContent() : m_type(ClipboardContentType::Empty), m_data(nullptr) { }

    ClipboardContent(std::string const& text) : m_type(ClipboardContentType::Text), m_data(text) { }
    ClipboardContent(std::string&& text) : m_type(ClipboardContentType::Text), m_data(std::move(text)) { }

    ClipboardContent(ClipboardPaths const& paths) : m_type(ClipboardContentType::Paths), m_data(paths) { }
    ClipboardContent(ClipboardPaths&& paths) : m_type(ClipboardContentType::Paths), m_data(std::move(paths)) { }

    ClipboardContent(std::vector<fs::path>&& paths, ClipboardPathsAction action = ClipboardPathsAction::Copy)
        : ClipboardContent(ClipboardPaths(std::move(paths), action)) { }
    ClipboardContent(std::vector<fs::path> const& paths, ClipboardPathsAction action = ClipboardPathsAction::Copy)
        : ClipboardContent(ClipboardPaths(paths, action)) { }

    [[nodiscard]] inline ClipboardContentType type() const { return m_type; }
    [[nodiscard]] inline std::string const& text() const { return std::get<std::string>(m_data); }
    [[nodiscard]] inline ClipboardPaths const& paths() const { return std::get<ClipboardPaths>(m_data); }
};

ClipboardContent getGUIClipboard();
void writeToGUIClipboard(ClipboardContent const& clipboard);
