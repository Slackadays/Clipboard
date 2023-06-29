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

namespace PerformAction {

void exportClipboards() {
    std::vector<std::string> destinations;
    if (!copying.items.empty())
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(destinations), [](const auto& item) { return item.string(); });
    else {
        for (const auto& entry : fs::directory_iterator(global_path.temporary))
            destinations.emplace_back(entry.path().filename().string());
        for (const auto& entry : fs::directory_iterator(global_path.persistent))
            destinations.emplace_back(entry.path().filename().string());
    }

    fs::path exportDirectory(fs::current_path() / "Exported_Clipboards");

    try {
        if (fs::exists(exportDirectory)) fs::remove_all(exportDirectory);
        fs::create_directory(exportDirectory);
    } catch (const fs::filesystem_error& e) {
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] CB couldn't create the export directory. [help]⬤ Try checking if you have the right permissions or not.[blank]\n"));
    }

    auto exportClipboard = [&](const std::string& name) {
        try {
            Clipboard clipboard(name);
            clipboard.getLock();
            if (clipboard.isUnused()) return;
            fs::copy(clipboard, exportDirectory / name, copying.opts);
            fs::remove(exportDirectory / name / constants.metadata_directory / constants.lock_name);
            clipboard.releaseLock();
            successes.clipboards++;
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(name, e.code());
        }
    };

    for (const auto& name : destinations)
        exportClipboard(name);

    if (destinations.empty() || successes.clipboards == 0) {
        stopIndicator();
        fprintf(stderr, "%s", no_clipboard_contents_message().data());
        error_exit(clipboard_action_prompt(), clipboard_invocation, clipboard_invocation);
    }
}

} // namespace PerformAction