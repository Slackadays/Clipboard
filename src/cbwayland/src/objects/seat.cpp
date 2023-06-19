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
#include "seat.hpp"
#include "all.hpp"

wl_seat_listener WlSeatSpec::listener {
        .capabilities = &eventHandler<&WlSeat::onCapability>,
        .name = &eventHandler<&WlSeat::onName>,
};

void WlSeat::onName(const char* name) {
    m_name = name;
}

void WlSeat::onCapability(std::uint32_t capabilities) {
    m_capabilities = static_cast<wl_seat_capability>(capabilities);
}

bool WlSeat::hasCapability(wl_seat_capability query) const {
    return (m_capabilities & query) != 0;
}
