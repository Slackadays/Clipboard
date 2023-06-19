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

#include "../fd.hpp"
#include "forward.hpp"
#include "spec.hpp"

struct WlShmPoolSpec {
    WL_SPEC_BASE(wl_shm_pool, 1)
    WL_SPEC_DESTROY(wl_shm_pool)
};

class WlShmPool : public WlObject<WlShmPoolSpec> {
    Fd m_fd;
    std::int32_t m_size;

public:
    explicit WlShmPool(const WlShm&, Fd&&, std::int32_t size);

    static std::unique_ptr<WlShmPool> fromMemfd(const WlShm&, std::int32_t size);
    static std::unique_ptr<WlShmPool> fromMemfd(const WlRegistry&, std::int32_t size);
};
