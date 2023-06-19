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
#pragma once

#include <clipboard/fork.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

enum class ClipboardPathsAction { Copy, Cut };

std::ostream& operator<<(std::ostream&, const ClipboardPathsAction&);

class ClipboardPaths {
private:
    ClipboardPathsAction m_action;
    std::vector<fs::path> m_paths;

public:
    ClipboardPaths(std::vector<fs::path>&&, ClipboardPathsAction = ClipboardPathsAction::Copy);
    ClipboardPaths(const std::vector<fs::path>& paths, ClipboardPathsAction = ClipboardPathsAction::Copy);

    [[nodiscard]] inline ClipboardPathsAction action() const { return m_action; }
    [[nodiscard]] inline const std::vector<fs::path>& paths() const { return m_paths; }
};

enum class ClipboardContentType { Empty, Text, Paths, Binary };

class ClipboardContent {
private:
    ClipboardContentType m_type = ClipboardContentType::Empty;
    std::string mime_type;
    std::vector<std::string> available_types;
    std::variant<std::nullptr_t, std::string, ClipboardPaths> m_data;

public:
    ClipboardContent();

    ClipboardContent(const std::string&, const std::string& = "text/plain");
    ClipboardContent(std::string&&, const std::string& = "text/plain");

    ClipboardContent(const ClipboardPaths&);
    ClipboardContent(ClipboardPaths&&);

    ClipboardContent(std::vector<fs::path>&&, ClipboardPathsAction = ClipboardPathsAction::Copy);
    ClipboardContent(const std::vector<fs::path>&, ClipboardPathsAction = ClipboardPathsAction::Copy);

    inline void makeTypesAvailable(const std::vector<std::string>& types) { available_types = types; }

    [[nodiscard]] inline ClipboardContentType type() const { return m_type; }
    [[nodiscard]] inline const std::string& text() const { return std::get<std::string>(m_data); }
    [[nodiscard]] inline const ClipboardPaths& paths() const { return std::get<ClipboardPaths>(m_data); }
    [[nodiscard]] inline const std::string& mime() const { return mime_type; }
    [[nodiscard]] inline const std::vector<std::string>& availableTypes() const { return available_types; }
};

/**
 * Object that's passed through the C interface to System GUI
 * implementations on Write calls.
 */
struct WriteGuiContext {
    const Forker& forker;
    const ClipboardContent& clipboard;
};

extern std::optional<std::string_view> inferMIMEType(const std::string_view& content);
extern std::optional<std::string_view> inferFileExtension(const std::string_view& content);