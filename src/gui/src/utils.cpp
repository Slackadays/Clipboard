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
#include <clipboard/utils.hpp>

#include <algorithm>
#include <iomanip>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std::literals;

StringOrLiteral::operator const char*() const {
    return std::visit(
            [](auto&& data) -> const char* {
                using T = std::decay_t<decltype(data)>;
                if constexpr (std::is_same_v<char const*, T>)
                    return data;
                else
                    return data.c_str();
            },
            m_data
    );
}

StringOrLiteral::operator std::string_view() const {
    return std::visit([](auto&& data) -> std::string_view { return {data}; }, m_data);
}

std::string urlDecode(std::string_view value) {
    auto tryConvertByte = [](const std::string& str) -> std::optional<char> {
        std::size_t pos = 0;
        unsigned long result;
        try {
            result = std::stoul(str, &pos, 16);
        } catch (std::invalid_argument&) {
            return {};
        }

        if (pos != 2) {
            return {};
        }

        return static_cast<char>(result);
    };

    std::vector<char> result;

    for (std::size_t i = 0; i < value.size(); i++) {
        if (value[i] != '%' || i >= value.size() - 2) {
            result.push_back(value[i]);
            continue;
        }

        auto possibleByte = tryConvertByte({&value[i + 1], 2});
        if (possibleByte.has_value()) {
            result.push_back(possibleByte.value());
            i += 2;
        } else {
            result.push_back('%');
        }
    }

    return {result.begin(), result.end()};
}

std::string urlEncode(std::string_view value) {
    static const std::set<char> allowedCharacters {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

                                                   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

                                                   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_', '.', '~', '/'};

    std::stringstream result;
    for (auto&& c : value) {
        if (allowedCharacters.contains(c)) {
            result << c;
            continue;
        }

        result << "%" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<std::uint64_t>(static_cast<std::uint8_t>(c));
    }

    return result.str();
}