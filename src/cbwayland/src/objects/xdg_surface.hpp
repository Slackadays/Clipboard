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

#include <wayland-xdg-shell.hpp>

#include <optional>

struct XdgSurfaceSpec {
    WL_SPEC_BASE(xdg_surface, 5)
    WL_SPEC_DESTROY(xdg_surface)
    WL_SPEC_LISTENER(xdg_surface)
};

class XdgSurface : public WlObject<XdgSurfaceSpec> {
    friend XdgSurfaceSpec;

    WlSurface& m_surface;
    std::unique_ptr<XdgToplevel> m_toplevel;
    std::unique_ptr<WlBuffer> m_pendingBuffer {};
    std::optional<std::tuple<int32_t, int32_t, int32_t, int32_t>> m_pendingDamage {};

public:
    explicit XdgSurface(const XdgWmBase&, WlSurface&);

    void setTitle(const char*) const;
    void scheduleAttach(std::unique_ptr<WlBuffer>&&);
    void scheduleDamage(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

private:
    void configure(std::uint32_t);
};
