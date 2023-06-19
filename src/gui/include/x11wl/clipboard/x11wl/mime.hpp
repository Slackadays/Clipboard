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

#include "clipboard/gui.hpp"
#include "clipboard/logging.hpp"

#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

/**
 * Different options that can be attached to MIME Types to change how the system behaves when dealing with
 * them. Check the documentation of each specific option for more info.
 */
enum class MimeOption {
    /** Empty option, does nothing. Mostly here for convenience. */
    NoOption = 0,

    /**
     * A MIME Type with this option doesn't represent a concrete type, but rather a request for the owner of the
     * selection to return the best compatible type from the other list of types it supports.
     */
    ChooseBestType = 1 << 1,

    /**
     * The list of files should begin with the clipboard action (cut/copy) as the first line.
     * Has no effect when copying or pasting text.
     */
    IncludeAction = 1 << 2,

    /**
     * The list of files should be encoded as a file:// URI, escaping special characters with percent-encoding.
     * Has no effect when copying or pasting text.
     */
    EncodePaths = 1 << 3,
};

/**
 * Combines two MimeOption values into an aggregate value that encodes both of
 * them at the same time. Use hasFlag to extract the values later.
 */
MimeOption operator|(const MimeOption& a, const MimeOption& b);

/**
 * Checks if a MimeOption has a specific flag.
 */
bool hasFlag(const MimeOption& value, const MimeOption& flag);

/**
 * A possible MIME Type for the contents of the system GUI clipboard.
 * The MIME Type defines how the contents of the clipboard will be interpreted.
 */
class MimeType {
    unsigned int m_priority;
    std::string_view m_name;
    ClipboardContentType m_type;
    MimeOption m_options;

    static std::map<std::string_view, MimeType> s_typesByName;

    MimeType(unsigned int priority, std::string_view name, ClipboardContentType type, MimeOption options) : m_priority(priority), m_name(name), m_type(type), m_options(options) {}

    static decltype(s_typesByName) initializeTypes();

    [[nodiscard]] ClipboardContent decodeText(std::istream&) const;
    [[nodiscard]] ClipboardContent decodePaths(std::istream&) const;

    bool encode(const ClipboardContent&, std::ostream&) const;
    bool encode(const std::string&, std::ostream&) const;
    bool encode(const ClipboardPaths&, std::ostream&) const;

public:
    [[nodiscard]] inline std::string_view name() const { return m_name; }

    [[nodiscard]] inline bool isChooseBestType() const { return hasFlag(m_options, MimeOption::ChooseBestType); }
    [[nodiscard]] inline bool isIncludeAction() const { return hasFlag(m_options, MimeOption::IncludeAction); }
    [[nodiscard]] inline bool isEncodePaths() const { return hasFlag(m_options, MimeOption::EncodePaths); }

    /**
     * Checks if the contents of the clipboard can be converted to this MIME Type.
     */
    [[nodiscard]] bool supports(const ClipboardContent&) const;

    /** Decodes the contents of the system GUI clipboard using this MIME Type. */
    [[nodiscard]] ClipboardContent decode(std::istream&) const;

    /**
     * Performs an action for all MIME Types that support a given set of clipboard contents.
     */
    template <std::invocable<const MimeType&> func_t>
    static void forEachSupporting(const ClipboardContent& clipboard, func_t func);

    /** Finds a specific MIME type by name. */
    static std::optional<MimeType> find(std::string_view);

    /**
     * Finds the best MIME Type from a list of MIME Types.
     * The "best" type is defined as the one with the lowest priority.
     */
    template <std::ranges::range range_t, std::convertible_to<std::string_view> element_t = std::ranges::range_value_t<range_t>>
    static std::optional<MimeType> findBest(range_t range);

    /**
     * Encodes the contents of the clipboard according to a MIME Type requested by the receiver.
     * This will automatically attempt to select the best available matching MIME Type from the list of
     * known MIME Types.
     * @param clipboard Clipboard contents to be converted.
     * @param mime MIME Type requested by the receiver.
     * @param stream Stream to encode the contents of the clipboard to.
     * @return If the encoding process was successful or not. The process can fail for a variety of reasons, including
     *  incompatible MIME Types.
     */
    static bool encode(const ClipboardContent& clipboard, std::string_view mime, std::ostream& stream);

    /**
     * Decodes the contents of the system GUI clipboard, automatically selecting the best available
     * MIME Type from the list offered by the selection owner.
     * @param offeredTypes MIME Types offerred by the selection owner.
     * @param request Function that can be used to request a specific MIME Type from the selection owner.
     */
    template <typename range_t, typename request_t, typename preference_t>
        requires requires {
                     requires std::ranges::range<range_t>;
                     requires std::convertible_to<std::string_view, std::ranges::range_value_t<range_t>>;
                     requires std::same_as<std::istream&, std::invoke_result_t<request_t, const MimeType&>>;
                     requires std::same_as<std::string, preference_t>;
                 }
    static ClipboardContent decode(range_t offeredTypes, request_t request, preference_t preferredType = {});
};

template <std::ranges::range range_t, std::convertible_to<std::string_view> element_t>
std::optional<MimeType> MimeType::findBest(range_t range) {
    std::optional<MimeType> best {};
    for (auto&& target : range) {
        auto found = find(target);
        if (!found.has_value()) {
            continue;
        }

        if (best.has_value() && best->m_priority <= found->m_priority) {
            continue;
        }

        best.emplace(*found);
    }

    return best;
}

template <typename range_t, typename query_t, typename preference_t>
    requires requires {
                 requires std::ranges::range<range_t>;
                 requires std::convertible_to<std::string_view, std::ranges::range_value_t<range_t>>;
                 requires std::same_as<std::istream&, std::invoke_result_t<query_t, const MimeType&>>;
                 requires std::same_as<std::string, preference_t>;
             }
ClipboardContent MimeType::decode(range_t offeredTypes, query_t request, preference_t preferredType) {
    std::optional<MimeType> type {};

    debugStream << "Preferred MIME type: " << preferredType << std::endl;
    debugStream << "Preferred MIME type size: " << preferredType.size() << std::endl;

    if (preferredType.empty())
        type = MimeType::findBest(offeredTypes);
    else
        type = MimeType(0, preferredType, ClipboardContentType::Text, MimeOption::NoOption);

    if (!type) {
        debugStream << "No supported MIME Type, aborting" << std::endl;
        return {};
    }

    debugStream << "Chosen type: " << type->name() << std::endl;
    auto&& stream = request(*type);
    return type->decode(stream);
}

template <std::invocable<const MimeType&> func_t>
void MimeType::forEachSupporting(const ClipboardContent& clipboard, func_t func) {
    for (auto&& [key, value] : s_typesByName) {
        if (value.supports(clipboard)) {
            func(value);
        }
    }
}