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
#pragma once

#include "forward.hpp"
#include "spec.hpp"

#include <memory>

struct WlBufferSpec {
    WL_SPEC_BASE(wl_buffer, 1)
    WL_SPEC_DESTROY(wl_buffer)
};

class WlBuffer : public WlObject<WlBufferSpec> {
private:
    std::unique_ptr<WlShmPool> m_shmPool;

public:
    WlBuffer(std::unique_ptr<WlShmPool>&&, std::int32_t offset, std::int32_t width, std::int32_t height, std::int32_t stride, wl_shm_format);

    static std::unique_ptr<WlBuffer> fromMemfd(const WlRegistry&, std::int32_t width, std::int32_t height, std::int32_t stride, wl_shm_format);
};
