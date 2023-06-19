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

#include "forward.hpp"
#include "spec.hpp"

#include <optional>
#include <string>
#include <string_view>

struct WlSeatSpec {
    WL_SPEC_BASE(wl_seat, 7)
    WL_SPEC_RELEASE(wl_seat)
    WL_SPEC_LISTENER(wl_seat)
};

class WlSeat : public WlObject<WlSeatSpec> {
    friend WlSeatSpec;

    std::string m_name {"unnamed seat"};
    wl_seat_capability m_capabilities {static_cast<wl_seat_capability>(0)};

public:
    explicit WlSeat(obj_t* value) : WlObject<spec_t> {value} {}

    [[nodiscard]] inline std::string_view name() const { return m_name; }
    [[nodiscard]] bool hasCapability(wl_seat_capability) const;

private:
    void onName(const char*);
    void onCapability(std::uint32_t);
};
