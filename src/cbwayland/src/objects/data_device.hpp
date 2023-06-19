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

struct WlDataDeviceSpec {
    WL_SPEC_BASE(wl_data_device, 3)
    WL_SPEC_RELEASE(wl_data_device)
    WL_SPEC_LISTENER(wl_data_device)
};

class WlDataDevice : public WlObject<WlDataDeviceSpec> {
    friend WlDataDeviceSpec;

    bool m_receivedSelectionEvent {false};
    std::unique_ptr<WlDataOffer> m_bufferedOffer {};
    std::unique_ptr<WlDataOffer> m_selectionOffer {};

public:
    explicit WlDataDevice(const WlDataDeviceManager&, const WlSeat&);
    explicit WlDataDevice(const WlRegistry&);

    [[nodiscard]] inline bool receivedSelectionEvent() const { return m_receivedSelectionEvent; }
    [[nodiscard]] inline bool hasSelectionOffer() const { return m_selectionOffer != nullptr; }
    [[nodiscard]] inline std::unique_ptr<WlDataOffer> releaseSelectionOffer() { return std::move(m_selectionOffer); }

    void setSelection(const WlDataSource&, std::uint32_t serial) const;

private:
    void onDataOffer(wl_data_offer*);
    void onSelection(wl_data_offer*);
};
