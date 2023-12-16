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
#include "surface.hpp"
#include "all.hpp"

WlSurface::WlSurface(const WlCompositor& compositor, const XdgWmBase& xdgWmBase)
        : WlObject<spec_t> {wl_compositor_create_surface(compositor.value())}
        , m_xdg {std::make_unique<XdgSurface>(xdgWmBase, *this)} {}

WlSurface::WlSurface(const WlRegistry& registry) : WlSurface {registry.get<WlCompositor>(), registry.get<XdgWmBase>()} {}

void WlSurface::attach(std::unique_ptr<WlBuffer>&& buffer) {
    m_buffer = std::move(buffer);
    wl_surface_attach(value(), getValue(m_buffer), 0, 0); // 0, 0 required by protocol
}

void WlSurface::damage(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) const {
    wl_surface_damage(value(), x, y, width, height);
}

void WlSurface::commit() const {
    wl_surface_commit(value());
}

void WlSurface::setTitle(const char* title) const {
    m_xdg->setTitle(title);
}

void WlSurface::scheduleAttach(std::unique_ptr<WlBuffer>&& buffer) {
    m_xdg->scheduleAttach(std::move(buffer));
}

void WlSurface::scheduleDamage(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    m_xdg->scheduleDamage(x, y, w, h);
}
