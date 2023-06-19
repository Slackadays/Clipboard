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
#include "callback.hpp"
#include "all.hpp"

decltype(WlCallbackSpec::listener) WlCallbackSpec::listener {
        .done = &eventHandler<&WlCallback::onDone>,
};

WlCallback::WlCallback(const WlDisplay& display) : WlObject<WlCallbackSpec> {wl_display_sync(display.value())} {}

void WlCallback::onDone(std::uint32_t serial) {
    m_serial = serial;
}
