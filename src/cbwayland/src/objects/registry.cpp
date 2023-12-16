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
#include "registry.hpp"
#include "all.hpp"

#include <clipboard/logging.hpp>

wl_registry_listener WlRegistrySpec::listener {
        .global = &eventHandler<&WlRegistry::onGlobal>,
        .global_remove = &eventHandler<&WlRegistry::onGlobalRemove>,
};

WlRegistry::WlRegistry(const WlDisplay& display) : WlObject<spec_t> {wl_display_get_registry(display.value())}, m_display {display} {

    // Ensure all globals are bound before exiting the constructor
    m_display.roundtrip();
}

template <IsWlObject T>
void WlRegistry::bind(std::uint32_t name, std::uint32_t version) {
    using spec = T::spec_t;

    std::string_view interfaceName {spec::interface.name};
    auto chosenVersion = std::min(spec::version, version);

    auto&& existing = m_boundObjectsByName.find(name);
    if (existing != m_boundObjectsByName.end()) {
        debugStream << "Tried to bind global " << name << " with interface " << interfaceName << " version " << chosenVersion << " but it was already bound to "
                    << existing->second.interface << ", ignoring" << std::endl;
        return;
    }

    auto voidPtr = wl_registry_bind(value(), name, &spec::interface, chosenVersion);
    if (voidPtr == nullptr) {
        throw WlException("Unable to bind global ", name, " with interface ", interfaceName, " version ", chosenVersion);
    }

    auto rawPtr = reinterpret_cast<spec::obj_t*>(voidPtr);
    auto sharedPtr = std::make_shared<T>(rawPtr);

    BoundObject boundObject {.name = name, .interface {interfaceName}, .object = std::static_pointer_cast<void>(sharedPtr)};
    m_boundObjectsByName.insert({boundObject.name, boundObject});
    m_boundObjectsByInterface.insert({boundObject.interface, boundObject});

    debugStream << "Bound global " << name << " with interface " << interfaceName << " version " << chosenVersion << std::endl;

    // Roundtrip to ensure the bound global is fully initialized
    m_display.roundtrip();
}

void WlRegistry::onGlobal(std::uint32_t name, const char* interface, std::uint32_t version) {
    debugStream << "Got global " << name << " of type " << interface << " version " << version << std::endl;

    std::string_view interfaceName {interface};
    if (interfaceName == WlDataDeviceManager::spec_t::interface.name) {
        bind<WlDataDeviceManager>(name, version);
    } else if (interfaceName == WlSeat::spec_t::interface.name) {
        bind<WlSeat>(name, version);
    } else if (interfaceName == WlCompositor::spec_t::interface.name) {
        bind<WlCompositor>(name, version);
    } else if (interfaceName == WlShm::spec_t::interface.name) {
        bind<WlShm>(name, version);
    } else if (interfaceName == WlShm::spec_t::interface.name) {
        bind<WlShm>(name, version);
    } else if (interfaceName == XdgWmBase::spec_t::interface.name) {
        bind<XdgWmBase>(name, version);
    }
}

void WlRegistry::onGlobalRemove(std::uint32_t name) {
    debugStream << "Global " << name << " has been removed" << std::endl;

    auto it = m_boundObjectsByName.find(name);
    if (it == m_boundObjectsByName.end()) {
        return;
    }

    BoundObject boundObject {it->second};
    m_boundObjectsByName.erase(boundObject.name);

    auto&& [start, end] = m_boundObjectsByInterface.equal_range(boundObject.interface);
    for (auto&& it = start; it != end; it++) {
        if (it->second.name == boundObject.name) {
            m_boundObjectsByInterface.erase(it);
        }
    }
}
