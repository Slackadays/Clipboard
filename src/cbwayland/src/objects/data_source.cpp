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
#include "data_source.hpp"
#include "all.hpp"

#include <clipboard/logging.hpp>

wl_data_source_listener WlDataSourceSpec::listener {
        .target = &noHandler,
        .send = &eventHandler<&WlDataSource::onSend>,
        .cancelled = &eventHandler<&WlDataSource::onCancelled>,
        .dnd_drop_performed = &noHandler,
        .dnd_finished = &noHandler,
        .action = &noHandler,
};

WlDataSource::WlDataSource(const WlDataDeviceManager& dataDeviceManager) : WlObject<WlDataSourceSpec> {wl_data_device_manager_create_data_source(dataDeviceManager.value())} {}

WlDataSource::WlDataSource(const WlRegistry& registry) : WlDataSource {registry.get<WlDataDeviceManager>()} {}

void WlDataSource::offer(std::string_view mime) const {
    std::string mimeCopy {mime};
    wl_data_source_offer(value(), mimeCopy.c_str());
}

void WlDataSource::onSend(const char* rawMime, std::int32_t rawFd) {
    std::string_view mime {rawMime};
    Fd fd {rawFd};
    if (m_sendCallback) {
        m_sendCallback(mime, std::move(fd));
    }
}

void WlDataSource::onCancelled() {
    m_isCancelled = true;
    debugStream << "Data source was cancelled" << std::endl;
}

void WlDataSource::sendCallback(std::function<sendCallback_t>&& callback) {
    m_sendCallback = std::move(callback);
}
