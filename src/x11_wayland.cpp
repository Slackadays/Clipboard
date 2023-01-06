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
#include "gui.hpp"
#include "logging.hpp"

#if defined(X11_AVAILABLE)
ClipboardContent getX11ClipboardInternal();
void setX11ClipboardInternal(ClipboardContent const&);
#endif

#if defined(WAYLAND_AVAILABLE)
ClipboardContent getWaylandClipboardInternal() { return {}; }
void setWaylandClipboardInternal(ClipboardContent const&) { }
#endif

ClipboardContent getGUIClipboard() {
    try {
        ClipboardContent clipboard;

        #if defined(X11_AVAILABLE)
        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard = getX11ClipboardInternal();
        }
        #endif

        #if defined(WAYLAND_AVAILABLE)
        if (clipboard.type() == ClipboardContentType::Empty) {
            clipboard =  getWaylandClipboardInternal();
        }
        #endif

        return clipboard;

    } catch (std::exception const& e) {
        debugStream << "Error getting clipboard data: " << e.what() << std::endl;
        return {};
    }
}

void writeToGUIClipboard(ClipboardContent const& clipboard) {
    try {
        #if defined(X11_AVAILABLE)
        setX11ClipboardInternal(clipboard);
        #endif

        #if defined(WAYLAND_AVAILABLE)
        setWaylandClipboardInternal(clipboard);
        #endif

    } catch (std::exception const& e) {
        debugStream << "Error setting clipboard data: " << e.what() << std::endl;
    }
}