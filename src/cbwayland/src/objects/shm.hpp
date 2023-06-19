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

#include <set>

struct WlShmSpec {
    WL_SPEC_BASE(wl_shm, 1)
    WL_SPEC_DESTROY(wl_shm)
    WL_SPEC_LISTENER(wl_shm)
};

class WlShm : public WlObject<WlShmSpec> {
    friend WlShmSpec;

    std::set<wl_shm_format> m_formats {};

public:
    explicit WlShm(obj_t* value) : WlObject<spec_t> {value} {}
    [[nodiscard]] bool supports(wl_shm_format) const;

private:
    void onFormat(std::uint32_t);
};
