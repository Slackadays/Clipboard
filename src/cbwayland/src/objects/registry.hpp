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

#include <map>
#include <memory>

struct WlRegistrySpec {
    WL_SPEC_BASE(wl_registry, 1)
    WL_SPEC_DESTROY(wl_registry)
    WL_SPEC_LISTENER(wl_registry)
};

class WlRegistry : public WlObject<WlRegistrySpec> {
    friend WlRegistrySpec;

    struct BoundObject {
        std::uint32_t name;
        std::string_view interface;
        std::shared_ptr<void> object;
    };

    const WlDisplay& m_display;
    std::map<std::uint32_t, BoundObject> m_boundObjectsByName {};
    std::multimap<std::string_view, BoundObject> m_boundObjectsByInterface {};

    template <IsWlObject T>
    void bind(std::uint32_t name, std::uint32_t version);

public:
    explicit WlRegistry(const WlDisplay& display);

    /**
     * Gets a C++-managed global Wayland object bound in this registry.
     * @tparam T Type of the Wayland object to get.
     * @throws WlException If there's no object of that type bound in the registry.
     */
    template <IsWlObject T>
    const T& get() const;

private:
    void onGlobal(std::uint32_t name, const char* interface, std::uint32_t version);
    void onGlobalRemove(std::uint32_t name);
};

template <IsWlObject T>
const T& WlRegistry::get() const {
    std::string_view name {T::spec_t::interface.name};

    auto found = m_boundObjectsByInterface.find(name);
    if (found == m_boundObjectsByInterface.end()) {
        throw WlException("Tried to use global ", name, " but it wasn't bound by registry");
    }

    return *std::static_pointer_cast<T>(found->second.object);
}
