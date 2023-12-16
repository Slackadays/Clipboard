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

struct WlSurfaceSpec {
    WL_SPEC_BASE(wl_surface, 5)
    WL_SPEC_DESTROY(wl_surface)
};

class WlSurface : public WlObject<WlSurfaceSpec> {
    std::unique_ptr<WlBuffer> m_buffer;
    std::unique_ptr<XdgSurface> m_xdg;

public:
    explicit WlSurface(const WlCompositor&, const XdgWmBase&);
    explicit WlSurface(const WlRegistry&);

    void attach(std::unique_ptr<WlBuffer>&&);
    void damage(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) const;
    void commit() const;

    void setTitle(const char*) const;
    void scheduleAttach(std::unique_ptr<WlBuffer>&&);
    void scheduleDamage(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
};
