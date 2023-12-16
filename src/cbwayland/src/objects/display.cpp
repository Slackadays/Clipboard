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
#include "display.hpp"
#include "all.hpp"

#include <clipboard/logging.hpp>
#include <clipboard/utils.hpp>
#include <poll.h>

using namespace std::literals;

WlDisplay::WlDisplay() : WlObject<spec_t> {wl_display_connect(nullptr)} {}

void WlDisplay::throwIfError() const {
    if (wl_display_get_error(value()) != 0) {
        throw WlException("Fatal error in the Wayland display");
    }
}

void WlDisplay::roundtrip() const {
    throwIfError();

    if (wl_display_roundtrip(value()) < 0) {
        throw WlException("Error calling wl_display_roundtrip");
    }
}

void WlDisplay::dispatchPending() const {
    throwIfError();

    auto result = wl_display_dispatch_pending(value());
    if (result == -1) {
        throw WlException("Error while dispatching pending events from the default queue");
    }

    if (result == 0) {
        throw WlException("Tried to dispatch pending events, but none were pending");
    }
}

void WlDisplay::flush() const {
    throwIfError();

    while (true) {
        auto flushResult = wl_display_flush(value());
        if (flushResult != -1) {
            return;
        }

        if (errno != EAGAIN) {
            throw WlException("Error flushing display");
        }

        pollWithTimeout(POLLOUT);
    }
}

void WlDisplay::readEvents() const {
    throwIfError();

    if (wl_display_read_events(value()) == -1) {
        throw WlException("Error reading events received from the Wayland server");
    }
}

void WlDisplay::dispatch() const {
    throwIfError();
    wl_display_dispatch(value());
}

void WlDisplay::dispatchWithTimeout() const {
    throwIfError();

    if (wl_display_prepare_read(value()) == -1) {
        dispatchPending();
        return;
    }

    ArmedGuard guard {[&]() {
        wl_display_cancel_read(value());
    }};
    flush();
    pollWithTimeout(POLLIN);
    guard.disarm();

    readEvents();
    dispatchPending();
}

void WlDisplay::pollWithTimeout(short events) const {
    throwIfError();

    constexpr auto timeout = 5s;
    constexpr auto errorMask = POLLERR | POLLNVAL;

    pollUntilReturn([&]() -> std::optional<bool> {
        pollfd fds[] = {pollfd {.fd = wl_display_get_fd(value()), .events = events, .revents = 0}};
        auto result = poll(fds, 1, std::chrono::microseconds(timeout).count());
        if (result == 0) {
            throw WlException("Timed out waiting for event from the server");
        }
        if (result == -1) {
            throw WlException("Error waiting for event from the server");
        }

        if ((fds[0].revents & errorMask) != 0) {
            throw WlException("Error in connection to the server");
        }

        if ((fds[0].revents & events) != events) {
            return {};
        }

        return true;
    });
}

std::uint32_t WlDisplay::getSerial() const {
    throwIfError();

    WlCallback callback {*this};
    dispatchUntil([&]() { return callback.hasSerial(); });
    return callback.serial();
}
