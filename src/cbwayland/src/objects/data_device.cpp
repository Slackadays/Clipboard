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
#include "data_device.hpp"
#include "all.hpp"

#include <clipboard/logging.hpp>

wl_data_device_listener WlDataDeviceSpec::listener {
        .data_offer = &eventHandler<&WlDataDevice::onDataOffer>,
        .enter = &noHandler,
        .leave = &noHandler,
        .motion = &noHandler,
        .drop = &noHandler,
        .selection = &eventHandler<&WlDataDevice::onSelection>,
};

WlDataDevice::WlDataDevice(const WlDataDeviceManager& manager, const WlSeat& seat) : WlObject<spec_t> {wl_data_device_manager_get_data_device(manager.value(), seat.value())} {
    debugStream << "Created a data device for seat " << seat.name() << std::endl;
}

WlDataDevice::WlDataDevice(const WlRegistry& registry)
        : WlDataDevice {
                registry.get<WlDataDeviceManager>(),
                registry.get<WlSeat>(),
        } {}

void WlDataDevice::onDataOffer(wl_data_offer* offer) {
    if (offer == nullptr) {
        debugStream << "Received a null data offer, ignoring" << std::endl;
        return;
    }

    m_bufferedOffer = std::make_unique<WlDataOffer>(offer);
    debugStream << "Got a new offer" << std::endl;
}

void WlDataDevice::onSelection(wl_data_offer* offer) {
    m_receivedSelectionEvent = true;

    if (offer == nullptr) {
        debugStream << "Selection was cleared" << std::endl;
        m_bufferedOffer.reset();
        m_selectionOffer.reset();
        return;
    }

    if (!m_bufferedOffer) {
        debugStream << "Got a new selection but its offer wasn't initialized before, ignoring" << std::endl;
        return;
    }

    if (getValue(m_bufferedOffer) != offer) {
        debugStream << "Got a selection but its offer didn't match the one that was initialized earlier, ignoring" << std::endl;
        return;
    }

    m_selectionOffer.reset();
    m_selectionOffer.swap(m_bufferedOffer);
    debugStream << "Offer was promoted to selection" << std::endl;
}

void WlDataDevice::setSelection(const WlDataSource& source, std::uint32_t serial) const {
    wl_data_device_set_selection(value(), source.value(), serial);
}
