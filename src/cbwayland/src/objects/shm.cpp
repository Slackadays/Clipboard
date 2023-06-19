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
#include "shm.hpp"
#include "all.hpp"

wl_shm_listener WlShmSpec::listener {
        .format = &eventHandler<&WlShm::onFormat>,
};

bool WlShm::supports(wl_shm_format format) const {
    return m_formats.contains(format);
}

void WlShm::onFormat(std::uint32_t format) {
    m_formats.insert(wl_shm_format(format));
}
