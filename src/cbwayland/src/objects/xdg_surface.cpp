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
#include "xdg_surface.hpp"
#include "all.hpp"

xdg_surface_listener XdgSurfaceSpec::listener {.configure = &eventHandler<&XdgSurface::configure>};

XdgSurface::XdgSurface(const XdgWmBase& base, WlSurface& surface)
        : WlObject {xdg_wm_base_get_xdg_surface(base.value(), surface.value())}
        , m_surface {surface}
        , m_toplevel {std::make_unique<XdgToplevel>(*this)} {}

void XdgSurface::configure(std::uint32_t serial) {
    auto didAction = false;

    if (m_pendingBuffer) {
        m_surface.attach(std::move(m_pendingBuffer));
        didAction = true;
    }

    if (m_pendingDamage) {
        auto args = m_pendingDamage.value();
        m_surface.damage(std::get<0>(args), std::get<1>(args), std::get<2>(args), std::get<3>(args));
        m_pendingDamage.reset();
        didAction = true;
    }

    xdg_surface_ack_configure(value(), serial);

    if (didAction) {
        m_surface.commit();
    }
}

void XdgSurface::setTitle(const char* title) const {
    m_toplevel->setTitle(title);
}

void XdgSurface::scheduleAttach(std::unique_ptr<WlBuffer>&& buffer) {
    m_pendingBuffer = std::move(buffer);
}

void XdgSurface::scheduleDamage(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    m_pendingDamage = {x, y, w, h};
}
