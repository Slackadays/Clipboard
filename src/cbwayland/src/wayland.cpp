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

#include "fd.hpp"
#include "objects/all.hpp"

#include "clipboard/x11wl/mime.hpp"
#include <clipboard/fork.hpp>
#include <clipboard/gui.hpp>
#include <clipboard/logging.hpp>

#include <exception>

class SimpleWindow {
    static constexpr auto width = 1;
    static constexpr auto height = 1;
    static constexpr auto stride = width * 4;
    static constexpr auto format = wl_shm_format::WL_SHM_FORMAT_XRGB8888;

    const WlDisplay& m_display;
    WlSurface m_surface;
    WlKeyboard m_keyboard;

public:
    explicit SimpleWindow(const WlDisplay& display, const WlRegistry& registry) : m_display {display}, m_surface {registry}, m_keyboard {registry} {

        m_surface.setTitle("Clipboard");

        auto buffer = WlBuffer::fromMemfd(registry, width, height, stride, format);
        m_surface.scheduleAttach(std::move(buffer));
        m_surface.scheduleDamage(0, 0, width, height);
        m_surface.commit();
    }

    std::uint32_t waitForFocus() const {
        m_display.dispatchUntil([&]() { return m_keyboard.hasFocus(m_surface); });
        return m_keyboard.getFocusSerial(m_surface);
    }
};

class PasteDaemon {
    const ClipboardContent& m_clipboard;
    WlDisplay m_display;
    WlRegistry m_registry;
    WlDataDevice m_dataDevice;
    WlDataSource m_dataSource;

public:
    explicit PasteDaemon(const ClipboardContent& clipboard) : m_clipboard {clipboard}, m_display(), m_registry {m_display}, m_dataDevice {m_registry}, m_dataSource {m_registry} {

        MimeType::forEachSupporting(m_clipboard, [&](auto&& x) { m_dataSource.offer(x.name()); });

        m_dataSource.sendCallback([&](std::string_view mime, Fd&& fd) {
            FdStream stream {fd};
            MimeType::encode(m_clipboard, mime, stream);
        });
    }

    void run() {
        {
            SimpleWindow window {m_display, m_registry};
            auto serial = window.waitForFocus();
            m_dataDevice.setSelection(m_dataSource, serial);
        }

        kill(getppid(), SIGUSR1);

        while (!m_dataSource.isCancelled())
            m_display.dispatch();
    }
};

static ClipboardContent getWaylandClipboardInternal(const std::string& requested_mime) {
    WlDisplay display;
    WlRegistry registry {display};
    SimpleWindow window {display, registry};
    WlDataDevice dataDevice {registry};

    display.dispatchUntil([&]() { return dataDevice.receivedSelectionEvent(); });

    auto offer = dataDevice.releaseSelectionOffer();
    if (!offer) {
        return {};
    }

    std::vector<std::string_view> offeredTypes;
    offer->forEachMimeType([&](auto&& type) { offeredTypes.emplace_back(type); });

    PipeFd pipe;
    FdStream stream {pipe};
    auto request = [&](const MimeType& type) -> std::istream& {
        offer->receive(type.name(), pipe.writeFd());
        display.roundtrip();
        pipe.closeWrite();
        return stream;
    };

    auto content = MimeType::decode(offeredTypes, request, requested_mime);

    std::vector<std::string> mimes(offeredTypes.begin(), offeredTypes.end());

    content.makeTypesAvailable(mimes);

    return content;
}

static bool setWaylandClipboardInternal(const WriteGuiContext& context) {
    context.forker.fork([&]() {
        PasteDaemon daemon {context.clipboard};
        daemon.run();
    });
    return waitForSuccessSignal();
}

extern "C" {
extern void* getWaylandClipboard(void* ptr) noexcept {
    try {
        std::string requested_mime(*reinterpret_cast<std::string*>(ptr));
        auto clipboard = std::make_unique<ClipboardContent>(getWaylandClipboardInternal(requested_mime));
        return clipboard.release();
    } catch (const std::exception& e) {
        debugStream << "Error getting clipboard data: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        debugStream << "Unknown error getting clipboard data" << std::endl;
        return nullptr;
    }
}

extern bool setWaylandClipboard(void* ptr) noexcept {
    try {
        const WriteGuiContext& context = *reinterpret_cast<const WriteGuiContext*>(ptr);
        return setWaylandClipboardInternal(context);
    } catch (const std::exception& e) {
        debugStream << "Error setting clipboard data: " << e.what() << std::endl;
        return false;
    } catch (...) {
        debugStream << "Unknown error setting clipboard data" << std::endl;
        return false;
    }
}
}
