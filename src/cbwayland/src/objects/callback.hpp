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

struct WlCallbackSpec {
    WL_SPEC_BASE(wl_callback, 1)
    WL_SPEC_DESTROY(wl_callback)
    WL_SPEC_LISTENER(wl_callback)
};

class WlCallback : public WlObject<WlCallbackSpec> {
    friend WlCallbackSpec;

    std::optional<std::uint32_t> m_serial {};

public:
    explicit WlCallback(const WlDisplay&);

    [[nodiscard]] inline bool hasSerial() const { return m_serial.has_value(); }
    [[nodiscard]] inline std::uint32_t serial() const { return m_serial.value_or(0); }

private:
    void onDone(std::uint32_t);
};
