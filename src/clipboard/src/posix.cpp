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
#include "clipboard.hpp"
#include <clipboard/logging.hpp>
#include <dlfcn.h>
#include <optional>
#include <type_traits>

constexpr auto objectX11 = "libclipboardx11.so";
constexpr auto symbolGetX11Clipboard = "getX11Clipboard";
constexpr auto symbolSetX11Clipboard = "setX11Clipboard";

constexpr auto objectWayland = "libclipboardwayland.so";
constexpr auto symbolGetWaylandClipboard = "getWaylandClipboard";
constexpr auto symbolSetWaylandClipboard = "setWaylandClipboard";

using getClipboard_t = void* (*)();
using setClipboard_t = void (*)(void*);

static void posixClipboardFailure(const char* object) {
    if (bool required = getenv("CLIPBOARD_REQUIREX11"); object == objectX11 && required) {
        indicator.detach();
        exit(EXIT_FAILURE);
    } else if (bool required = getenv("CLIPBOARD_REQUIREWAYLAND"); object == objectWayland && required) {
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
        posixClipboardFailure(object);
        return return_t();
    }

    auto symbolHandle = dlsym(objectHandle, symbol);
    if (symbolHandle == nullptr) {
        debugStream << "Reading " << symbol << " from object " << object << " failed, aborting operation" << std::endl;
        debugStream << "Specific error: " << dlerror() << std::endl;
        posixClipboardFailure(object);
        return return_t();
    }

    debugStream << "Found " << symbol << " in " << object << ", calling it" << std::endl;
    auto funcPointer = reinterpret_cast<proc_t>(symbolHandle);
    return funcPointer(args...);
}

static ClipboardContent dynamicGetGUIClipboard(const char* object, const char* symbol) {
    auto posixClipboardPtr = dynamicCall<getClipboard_t>(object, symbol);
    if (posixClipboardPtr == nullptr) {
        return {};
    }

    std::unique_ptr<const ClipboardContent> posixClipboard {reinterpret_cast<const ClipboardContent*>(posixClipboardPtr)};
    return *posixClipboard;
}

static void dynamicSetGUIClipboard(const char* object, const char* symbol, const ClipboardContent& clipboard) {
    WriteGuiContext context {
            .forker = forker,
            .clipboard = clipboard,
    };
    auto ptr = reinterpret_cast<void*>(&context);
    dynamicCall<setClipboard_t>(object, symbol, ptr);
}

ClipboardContent getGUIClipboard() {
    try {
        ClipboardContent clipboard;

        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard = dynamicGetGUIClipboard(objectX11, symbolGetX11Clipboard);
        }

        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard = dynamicGetGUIClipboard(objectWayland, symbolGetWaylandClipboard);
        }

        return clipboard;

    } catch (const std::exception& e) {
        debugStream << "Error getting clipboard data: " << e.what() << std::endl;
        return {};
    }
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    try {
        dynamicSetGUIClipboard(objectX11, symbolSetX11Clipboard, clipboard);
        dynamicSetGUIClipboard(objectWayland, symbolSetWaylandClipboard, clipboard);

    } catch (const std::exception& e) {
        debugStream << "Error setting clipboard data: " << e.what() << std::endl;
    }
}