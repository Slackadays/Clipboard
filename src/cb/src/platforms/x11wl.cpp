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
#include "../clipboard.hpp"
#include <clipboard/logging.hpp>
#include <cstring>
#include <dlfcn.h>
#include <optional>
#include <type_traits>

constexpr auto objectX11 = "libcbx11.so";
constexpr auto symbolGetX11Clipboard = "getX11Clipboard";
constexpr auto symbolSetX11Clipboard = "setX11Clipboard";

constexpr auto objectWayland = "libcbwayland.so";
constexpr auto symbolGetWaylandClipboard = "getWaylandClipboard";
constexpr auto symbolSetWaylandClipboard = "setWaylandClipboard";

const bool GUIClipboardSupportsCut = true;

using getClipboard_t = void* (*)(void*);
using setClipboard_t = bool (*)(void*);

static void x11wlClipboardFailure(const char* object) {
    if (auto required = envVarIsTrue("CLIPBOARD_REQUIREX11"); object == objectX11 && required) {
        indicator.detach();
        exit(EXIT_FAILURE);
    } else if (auto required = envVarIsTrue("CLIPBOARD_REQUIREWAYLAND"); object == objectWayland && required) {
        indicator.detach();
        exit(EXIT_FAILURE);
    }
}

template <typename proc_t, typename... args_t, typename return_t = std::invoke_result_t<proc_t, args_t...>>
static return_t dynamicCall(const char* object, const char* symbol, args_t... args) {
    auto objectHandle = dlopen(object, RTLD_LAZY | RTLD_NODELETE);
    if (objectHandle == nullptr) {
        debugStream << "Opening " << object << " to look for " << symbol << " failed, aborting operation" << std::endl;
        debugStream << "Specific error: " << dlerror() << std::endl;
        x11wlClipboardFailure(object);
        return return_t();
    }

    auto symbolHandle = dlsym(objectHandle, symbol);
    if (symbolHandle == nullptr) {
        debugStream << "Reading " << symbol << " from object " << object << " failed, aborting operation" << std::endl;
        debugStream << "Specific error: " << dlerror() << std::endl;
        x11wlClipboardFailure(object);
        return return_t();
    }

    debugStream << "Found " << symbol << " in " << object << ", calling it" << std::endl;
    auto funcPointer = reinterpret_cast<proc_t>(symbolHandle);
    return funcPointer(args...);
}

static ClipboardContent dynamicGetGUIClipboard(const char* object, const char* symbol, std::string& requested_mime) {
    auto mime_ptr = reinterpret_cast<void*>(&requested_mime);
    auto x11wlClipboardPtr = dynamicCall<getClipboard_t>(object, symbol, mime_ptr);
    if (x11wlClipboardPtr == nullptr) {
        return {};
    }

    std::unique_ptr<const ClipboardContent> x11wlClipboard {reinterpret_cast<const ClipboardContent*>(x11wlClipboardPtr)};
    return *x11wlClipboard;
}

static bool dynamicSetGUIClipboard(const char* object, const char* symbol, const ClipboardContent& clipboard) {
    WriteGuiContext context {
            .forker = forker,
            .clipboard = clipboard,
    };
    auto ptr = reinterpret_cast<void*>(&context);
    return dynamicCall<setClipboard_t>(object, symbol, ptr);
}

ClipboardContent getGUIClipboard(const std::string& requested_mime) {
    try {
        ClipboardContent clipboard;
        std::string mime(requested_mime);

        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard = dynamicGetGUIClipboard(objectX11, symbolGetX11Clipboard, mime);
        }

        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard = dynamicGetGUIClipboard(objectWayland, symbolGetWaylandClipboard, mime);
        }

        return clipboard;

    } catch (const std::exception& e) {
        debugStream << "Error getting clipboard data: " << e.what() << std::endl;
        return {};
    }
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    try {
        auto force_x11 = envVarIsTrue("CLIPBOARD_REQUIREX11");
        if (!dynamicSetGUIClipboard(objectWayland, symbolSetWaylandClipboard, clipboard) || (force_x11)) {
            debugStream << "Trying X11 clipboard now" << std::endl;
            if (!dynamicSetGUIClipboard(objectX11, symbolSetX11Clipboard, clipboard)) debugStream << "Setting X11 clipboard failed" << std::endl;
        }

    } catch (const std::exception& e) {
        debugStream << "Error setting clipboard data: " << e.what() << std::endl;
    }
}