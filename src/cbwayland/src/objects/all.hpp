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

#include "buffer.hpp"
#include "callback.hpp"
#include "compositor.hpp"
#include "data_device.hpp"
#include "data_device_manager.hpp"
#include "data_offer.hpp"
#include "data_source.hpp"
#include "display.hpp"
#include "keyboard.hpp"
#include "registry.hpp"
#include "seat.hpp"
#include "shm.hpp"
#include "shm_pool.hpp"
#include "surface.hpp"
#include "xdg_surface.hpp"
#include "xdg_toplevel.hpp"
#include "xdg_wm_base.hpp"
