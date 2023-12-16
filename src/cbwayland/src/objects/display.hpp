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
#pragma once

#include "forward.hpp"
#include "spec.hpp"

#include <chrono>

struct WlDisplaySpec {
    WL_SPEC_BASE(wl_display, 1)
    static constexpr auto deleter = &wl_display_disconnect;
};

class WlDisplay : public WlObject<WlDisplaySpec> {

    void throwIfError() const;
    void flush() const;
    void dispatchPending() const;
    void readEvents() const;
    void pollWithTimeout(short events) const;

public:
    explicit WlDisplay();

    void roundtrip() const;
    void dispatch() const;
    void dispatchWithTimeout() const;

    /**
     * Loops dispatch() until a certain predicate is met.
     * Throws if the operation takes too long.
     */
    template <std::predicate predicate_t>
    void dispatchUntil(predicate_t predicate) const;

    /**
     * Gets the next event serial from the Wayland server.
     * Requires a back-and-forth between the client and the server and may
     * dispatch other events in the meantime.
     */
    std::uint32_t getSerial() const;
};

template <std::predicate predicate_t>
void WlDisplay::dispatchUntil(predicate_t predicate) const {
    using namespace std::literals;
    constexpr auto maxWaitTime = 5s;

    throwIfError();

    auto start = std::chrono::steady_clock::now();
    while (!predicate()) {
        dispatchWithTimeout();

        const auto time = std::chrono::steady_clock::now() - start;
        if (time > maxWaitTime) {
            throw WlException("Timed out waiting for the Wayland server to reply");
        }
    }
}
