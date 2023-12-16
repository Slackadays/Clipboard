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
#include "clipboard.hpp"

void setTheme(const std::string_view& theme) {
    if (theme == "light") {
        colors = {
                {{"[error]", "\033[38;5;196m"},
                 {"[success]", "\033[38;5;22m"},
                 {"[progress]", "\033[38;5;202m"},
                 {"[info]", "\033[38;5;20m"},
                 {"[help]", "\033[38;5;200m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    } else if (theme == "amber") {
        colors = {
                {{"[error]", "\033[38;5;202m"},
                 {"[success]", "\033[38;5;220m"},
                 {"[progress]", "\033[38;5;214m"},
                 {"[info]", "\033[38;5;222m"},
                 {"[help]", "\033[38;5;226m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    } else if (theme == "green") {
        colors = {
                {{"[error]", "\033[38;5;112m"},
                 {"[success]", "\033[38;5;41m"},
                 {"[progress]", "\033[38;5;48m"},
                 {"[info]", "\033[38;5;154m"},
                 {"[help]", "\033[38;5;46m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    } else if (theme == "darkhighcontrast") {
        colors = {
                {{"[error]", "\033[38;5;218m"},
                 {"[success]", "\033[38;5;157m"},
                 {"[progress]", "\033[38;5;230m"},
                 {"[info]", "\033[38;5;195m"},
                 {"[help]", "\033[38;5;225m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    } else if (theme == "lighthighcontrast") {
        colors = {
                {{"[error]", "\033[38;5;124m"},
                 {"[success]", "\033[38;5;22m"},
                 {"[progress]", "\033[38;5;58m"},
                 {"[info]", "\033[38;5;18m"},
                 {"[help]", "\033[38;5;161m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    } else if (theme == "ansi") {
        colors = {
                {{"[error]", "\033[38;5;1m"},
                 {"[success]", "\033[38;5;2m"},
                 {"[progress]", "\033[38;5;3m"},
                 {"[info]", "\033[38;5;4m"},
                 {"[help]", "\033[38;5;5m"},
                 {"[bold]", "\033[1m"},
                 {"[nobold]", "\033[22m"},
                 {"[inverse]", "\033[7m"},
                 {"[noinverse]", "\033[27m"},
                 {"[blank]", "\033[0m"}}};
    }
}