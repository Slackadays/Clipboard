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
    ClipboardPaths(ClipboardPathsAction action, std::vector<fs::path>&& paths)
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
    ClipboardContent(std::string&& text) : m_type(ClipboardContentType::Text), m_data(std::move(text)) { }
    ClipboardContent(ClipboardPaths&& paths) : m_type(ClipboardContentType::Paths), m_data(std::move(paths)) { }
    ClipboardContent(ClipboardPathsAction action, std::vector<fs::path>&& paths)
        : ClipboardContent(ClipboardPaths(action, std::move(paths))) { }

    [[nodiscard]] inline ClipboardContentType type() const { return m_type; }
    [[nodiscard]] inline std::string const& text() { return std::get<std::string>(m_data); }
    [[nodiscard]] inline ClipboardPaths const& paths() { return std::get<ClipboardPaths>(m_data); }
};
