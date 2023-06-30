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

void importClipboards() {
    fs::path importDirectory;
    if (copying.items.empty())
        importDirectory = fs::current_path() / "Exported_Clipboards";
    else
        importDirectory = copying.items.at(0);

    if (!fs::exists(importDirectory))
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] The directory you're trying to import from doesn't exist. [help]⬤ Try choosing a different one instead.[blank]\n"));

    if (!fs::is_directory(importDirectory))
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] The directory you're trying to import from isn't a directory. [help]⬤ Try choosing a different one instead.[blank]\n"));

    for (const auto& entry : fs::directory_iterator(importDirectory)) {
        if (!entry.is_directory())
            copying.failedItems.emplace_back(entry.path().filename().string(), std::make_error_code(std::errc::not_a_directory));
        else {
            try {
                auto target = (isPersistent(entry.path().filename().string()) ? global_path.persistent : global_path.temporary) / entry.path().filename();
                if (fs::exists(target)) {
                    using enum CopyPolicy;
                    switch (copying.policy) {
                    case SkipAll:
                        continue;
                    case ReplaceAll:
                        fs::copy(entry.path(), target, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        successes.clipboards++;
                        break;
                    default:
                        stopIndicator();
                        copying.policy = userDecision(entry.path().filename().string());
                        startIndicator();
                        if (copying.policy == ReplaceOnce || copying.policy == ReplaceAll) {
                            fs::copy(entry.path(), target, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                            successes.clipboards++;
                        }
                        break;
                    }
                } else {
                    fs::copy(entry.path(), target, fs::copy_options::recursive);
                    successes.clipboards++;
                }
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(entry.path().filename().string(), e.code());
            }
        }
    }
}

} // namespace PerformAction