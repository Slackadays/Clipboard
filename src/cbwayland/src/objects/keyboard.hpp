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

#include <map>

struct WlKeyboardSpec {
    WL_SPEC_BASE(wl_keyboard, 8)
    WL_SPEC_RELEASE(wl_keyboard)
    WL_SPEC_LISTENER(wl_keyboard)
};

class WlKeyboard : public WlObject<WlKeyboardSpec> {
    friend WlKeyboardSpec;

    std::map<wl_surface*, std::uint32_t> m_focus {};

    static wl_keyboard* initKeyboard(const WlSeat&);

public:
    explicit WlKeyboard(const WlSeat&);
    explicit WlKeyboard(const WlRegistry&);

    bool hasFocus(wl_surface*) const;
    bool hasFocus(const WlSurface&) const;

    std::uint32_t getFocusSerial(wl_surface*) const;
    std::uint32_t getFocusSerial(const WlSurface&) const;

private:
    void onEnter(std::uint32_t serial, wl_surface*, wl_array*);
    void onLeave(std::uint32_t serial, wl_surface*);
};
