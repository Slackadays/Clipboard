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
#include "keyboard.hpp"
#include "all.hpp"

decltype(WlKeyboardSpec::listener) WlKeyboardSpec::listener {
        .keymap = &noHandler,
        .enter = &eventHandler<&WlKeyboard::onEnter>,
        .leave = &eventHandler<&WlKeyboard::onLeave>,
        .key = &noHandler,
        .modifiers = &noHandler,
        .repeat_info = &noHandler};

wl_keyboard* WlKeyboard::initKeyboard(const WlSeat& seat) {
    if (!seat.hasCapability(WL_SEAT_CAPABILITY_KEYBOARD)) {
        throw WlException("Seat ", seat.name(), " doesn't have Keyboard capabilities");
    }

    return wl_seat_get_keyboard(seat.value());
}

WlKeyboard::WlKeyboard(const WlSeat& seat) : WlObject<WlKeyboardSpec> {initKeyboard(seat)} {}

WlKeyboard::WlKeyboard(const WlRegistry& registry) : WlKeyboard {registry.get<WlSeat>()} {}

bool WlKeyboard::hasFocus(wl_surface* surface) const {
    return m_focus.contains(surface);
}

bool WlKeyboard::hasFocus(const WlSurface& surface) const {
    return hasFocus(surface.value());
}

std::uint32_t WlKeyboard::getFocusSerial(wl_surface* surface) const {
    return m_focus.at(surface);
}

std::uint32_t WlKeyboard::getFocusSerial(const WlSurface& surface) const {
    return getFocusSerial(surface.value());
}

void WlKeyboard::onEnter(std::uint32_t serial, wl_surface* surface, wl_array*) {
    m_focus.insert_or_assign(surface, serial);
}

void WlKeyboard::onLeave(std::uint32_t serial, wl_surface* surface) {
    m_focus.erase(surface);
}
