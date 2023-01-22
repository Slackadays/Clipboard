/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
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

#include "objects/all.hpp"
#include "fd.hpp"

#include <clipboard/gui.hpp>
#include <clipboard/logging.hpp>
#include "clipboard/posix/mime.hpp"
#include <clipboard/fork.hpp>

#include <exception>

class SimpleWindow {
    static constexpr auto width = 1;
    static constexpr auto height = 1;
    static constexpr auto stride = width * 4;
    static constexpr auto format = wl_shm_format::WL_SHM_FORMAT_XRGB8888;

    WlDisplay const& m_display;
    WlSurface m_surface;
    WlKeyboard m_keyboard;

public:
    explicit SimpleWindow(WlDisplay const& display, WlRegistry const& registry)
        : m_display { display }
        , m_surface { registry }
        , m_keyboard { registry} {

        m_surface.setTitle("Clipboard");

        auto buffer = WlBuffer::fromMemfd(
            registry,
            width,
            height,
            stride,
            format
        );
        m_surface.scheduleAttach(std::move(buffer));
        m_surface.scheduleDamage(0, 0, width, height);
        m_surface.commit();
    }

    std::uint32_t waitForFocus() const {
        m_display.dispatchUntil([&]() {
            return m_keyboard.hasFocus(m_surface);
        });
        return m_keyboard.getFocusSerial(m_surface);
    }
};

class PasteDaemon {
    ClipboardContent const& m_clipboard;
    WlDisplay m_display;
    WlRegistry m_registry;
    WlDataDevice m_dataDevice;
    WlDataSource m_dataSource;

public:
    explicit PasteDaemon(ClipboardContent const& clipboard)
        : m_clipboard { clipboard }
        , m_display()
        , m_registry { m_display }
        , m_dataDevice { m_registry }
        , m_dataSource { m_registry } {

        MimeType::forEachSupporting(m_clipboard, [&](auto&& x) {
            m_dataSource.offer(x.name());
        });

        m_dataSource.sendCallback([&](std::string_view mime, Fd&& fd) {
            FdStream stream { fd };
            MimeType::encode(
                m_clipboard,
                mime,
                stream
            );
        });
    }

    void run() {
        {
            SimpleWindow window { m_display, m_registry };
            auto serial = window.waitForFocus();
            m_dataDevice.setSelection(m_dataSource, serial);
        }

        while (!m_dataSource.isCancelled())
            m_display.dispatch();
    }
};

static ClipboardContent getWaylandClipboardInternal() {
    WlDisplay display;
    WlRegistry registry { display };
    SimpleWindow window { display, registry };
    WlDataDevice dataDevice { registry };

    display.dispatchUntil([&]() {
        return dataDevice.receivedSelectionEvent();
    });

    auto offer = dataDevice.releaseSelectionOffer();
    if (!offer) {
        return {};
    }

    std::vector<std::string_view> offeredTypes;
    offer->forEachMimeType([&](auto&& type) {
        offeredTypes.emplace_back(type);
    });

    PipeFd pipe;
    FdStream stream { pipe };
    auto request = [&](MimeType const& type) -> std::istream& {
        offer->receive(type.name(), pipe.writeFd());
        display.roundtrip();
        pipe.closeWrite();
        return stream;
    };

    return MimeType::decode(offeredTypes, request);
}

static void setWaylandClipboardInternal(WriteGuiContext const& context) {
    context.forker.fork([&]() {
        PasteDaemon daemon { context.clipboard };
        daemon.run();
    });
}

extern "C" {
    extern void* getWaylandClipboard() noexcept {
        try {
            auto clipboard = std::make_unique<ClipboardContent>(getWaylandClipboardInternal());
            return clipboard.release();
        } catch (std::exception const& e) {
            debugStream << "Error getting clipboard data: " << e.what() << std::endl;
            return nullptr;
        } catch (...) {
            debugStream << "Unknown error getting clipboard data" << std::endl;
            return nullptr;
        }
    }

    extern void setWaylandClipboard(void* ptr) noexcept {
        try {
            WriteGuiContext const& context = *reinterpret_cast<WriteGuiContext const*>(ptr);
            setWaylandClipboardInternal(context);
        } catch (std::exception const& e) {
            debugStream << "Error setting clipboard data: " << e.what() << std::endl;
        } catch (...) {
            debugStream << "Unknown error setting clipboard data" << std::endl;
        }
    }
}

