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

#include <iostream>
#include <streambuf>

#if defined(NDEBUG)
class NullBuffer : public std::streambuf {
public:
    int overflow(int c) override { return c; }
};

static NullBuffer nullBuffer;
static std::ostream nullStream(&nullBuffer);
static std::ostream& debugStream = nullStream;
#else
static std::ostream& debugStream = std::cerr;
#endif
