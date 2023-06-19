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

#include "../fd.hpp"
#include "forward.hpp"
#include "spec.hpp"

#include <functional>

struct WlDataSourceSpec {
    WL_SPEC_BASE(wl_data_source, 3)
    WL_SPEC_DESTROY(wl_data_source)
    WL_SPEC_LISTENER(wl_data_source)
};

class WlDataSource : public WlObject<WlDataSourceSpec> {
    friend WlDataSourceSpec;

public:
    using sendCallback_t = void(std::string_view, Fd&&);

private:
    bool m_isCancelled {false};
    std::function<sendCallback_t> m_sendCallback;

public:
    explicit WlDataSource(const WlDataDeviceManager&);
    explicit WlDataSource(const WlRegistry&);

    [[nodiscard]] inline bool isCancelled() const { return m_isCancelled; }

    void sendCallback(std::function<sendCallback_t>&&);
    void offer(std::string_view) const;

private:
    void onSend(const char* mime, std::int32_t fd);
    void onCancelled();
};
