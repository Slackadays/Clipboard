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

#if defined(X11_AVAILABLE)
ClipboardContent getX11ClipboardInternal();
#endif

#if defined(WAYLAND_AVAILABLE)
ClipboardContent getWaylandClipboardInternal() { return ClipboardContent(); }
#endif

ClipboardContent getGUIClipboard() {
    #if defined(X11_AVAILABLE)
    //try {
        return getX11ClipboardInternal();
    //} catch (X11Exception const& e) { //X11Exception is not available from bringing getGUIClipboard over
    //    debugStream << "Error getting data from X11: " << e.what() << std::endl;
    //    return {};
    //}
    #endif
    #if defined(WAYLAND_AVAILABLE)
    return getWaylandClipboardInternal();
    #endif
}

void writeToGUIClipboard(ClipboardContent& clipboard) {
    
}