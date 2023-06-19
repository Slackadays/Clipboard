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
#include "data_offer.hpp"
#include "all.hpp"

wl_data_offer_listener WlDataOfferSpec::listener {
        .offer = &eventHandler<&WlDataOffer::onOffer>,
        .source_actions = &noHandler,
        .action = &noHandler,
};

void WlDataOffer::onOffer(const char* mime) {
    m_mimeTypes.emplace(mime);
}

void WlDataOffer::receive(std::string_view mime, int fd) const {
    std::string mimeCopy {mime};
    wl_data_offer_receive(value(), mimeCopy.c_str(), fd);
}
