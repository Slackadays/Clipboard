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
#include "clipboard.hpp"

int main(int argc, char* argv[]) {
    try {
        setupHandlers();

        setupVariables(argc, argv);

        setupTerminal();

        setLocale();

        setClipboardAttributes();

        setFlags();

        startIndicator();

        setFilepaths();

        action = getAction();

        if (auto pos = std::find_if(arguments.begin(), arguments.end(), [](const auto& arg) { return arg == "--"; }); pos != arguments.end()) arguments.erase(pos);

        copying.items.assign(arguments.begin(), arguments.end());

        io_type = getIOType();

        verifyAction();

        if (action != Action::Info) path.getLock();

        syncWithExternalClipboards();

        ignoreItemsPreemptively(copying.items);

        checkForNoItems();

        if (needsANewEntry()) path.makeNewEntry();

        (clipboard_state.exchange(ClipboardState::Action), cv.notify_one());

        (fs::create_directories(global_path.temporary), fs::create_directories(global_path.persistent));

        deduplicate(copying.items);

        checkItemSize(totalItemSize());

        performAction();

        if (isAWriteAction()) path.applyIgnoreRegexes();

        copying.mime = getMIMEType();

        updateExternalClipboards();

        if (!copying.failedItems.empty()) clipboard_state = ClipboardState::Error;

        stopIndicator();

        deduplicate(copying.failedItems);

        showFailures();

        showSuccesses();

        path.trimHistoryEntries();
    } catch (const std::exception& e) {
        clipboard_state = ClipboardState::Error;
        stopIndicator();
        fprintf(stderr, internal_error_message().data(), e.what());
        exit(EXIT_FAILURE);
    }
    if (clipboard_state == ClipboardState::Error)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}