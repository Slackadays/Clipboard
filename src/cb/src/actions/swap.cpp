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
#include "../clipboard.hpp"

namespace PerformAction {

void swap() {
    if (copying.items.size() > 1)
        error_exit(
                formatColors("[error][inverse] ✘ [noinverse] You can only swap one clipboard at a time. [help]Try making sure there's only one other clipboard specified, like [bold]%s swap "
                             "5[nobold] or [bold]%s swap3 0[nobold].[blank]\n"),
                clipboard_invocation,
                clipboard_invocation
        );

    std::string destination_name = copying.items.empty() ? std::string(constants.default_clipboard_name) : copying.items.at(0).string();

    if (destination_name == clipboard_name)
        error_exit(
                formatColors("[error][inverse] ✘ [noinverse] You can't swap a clipboard with itself. [help]Try choosing a different clipboard to swap with, like [bold]%s swap 5[nobold] or "
                             "[bold]%s swap3 0[nobold].[blank]\n"),
                clipboard_invocation,
                clipboard_invocation
        );

    Clipboard destination(destination_name);

    fs::path swapTargetSource(path.data);
    swapTargetSource.replace_extension("swap");

    fs::path swapTargetDestination(destination.data);
    swapTargetDestination.replace_extension("swap");

    try {
        fs::copy(destination.data, swapTargetSource, fs::copy_options::recursive);
        fs::copy(path.data, swapTargetDestination, fs::copy_options::recursive);

        fs::remove_all(path.data);
        fs::remove_all(destination.data);

        fs::rename(swapTargetSource, path.data);
        fs::rename(swapTargetDestination, destination.data);
    } catch (const fs::filesystem_error& e) {
        copying.failedItems.emplace_back(destination_name, e.code());
    }

    stopIndicator();

    if (!output_silent && !confirmation_silent)
        fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Swapped clipboard %s with %s[blank]\n").data(), clipboard_name.data(), destination_name.data());

    if (destination_name == constants.default_clipboard_name) updateExternalClipboards(true);
}

} // namespace PerformAction