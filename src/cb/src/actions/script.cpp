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
#if defined(UNIX_OR_UNIX_LIKE)
#include <sys/wait.h>
#endif

namespace PerformAction {
void script() {
    if (io_type == IOType::File) {
        if (copying.items.size() > 1) {
            error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] You can only set one script file to run. [help]⬤ Try providing a single script file instead.[blank]\n"));
            return;
        }
        if (copying.items.empty()) {
            stopIndicator();
            if (fs::is_regular_file(path.metadata.script)) {
                fprintf(stderr, formatColors("[info]┃ Here is this clipboard's current script: [help]%s[blank]\n").data(), fileContents(path.metadata.script).value().data());
            } else {

                fprintf(stderr,
                        formatColors("[error][inverse] ✘ [noinverse] There is currently no script set for this clipboard. [help]⬤ To set a script, add it to the end, like [bold]%s %s "
                                     "myscript.sh[nobold], or specify it as an argument, like [bold]%s %s \"echo Hello World!\".[blank]\n")
                                .data(),
                        clipboard_invocation.data(),
                        actions[action].data(),
                        clipboard_invocation.data(),
                        actions[action].data());
            }
            return;
        }
        fs::remove(path.metadata.script);
        fs::copy(copying.items.at(0), path.metadata.script);
        fs::permissions(path.metadata.script, fs::perms::owner_exec, fs::perm_options::add);
        if (output_silent || confirmation_silent) return;
        stopIndicator();
        fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved script \"%s\"[blank]\n").data(), fileContents(path.metadata.script).value().data());
    } else if (io_type == IOType::Text) {
        if (copying.items.at(0).string() == "") {
            fs::remove(path.metadata.script);
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Removed script[blank]\n").data());
        } else {
            for (size_t i = 0; i < copying.items.size(); i++) {
                copying.buffer += copying.items.at(i).string();
                if (i != copying.items.size() - 1) copying.buffer += " ";
            }
            fs::remove(path.metadata.script);
            writeToFile(path.metadata.script, copying.buffer);
            fs::permissions(path.metadata.script, fs::perms::owner_exec, fs::perm_options::add);
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved script \"%s\"[blank]\n").data(), fileContents(path.metadata.script).value().data());
        }
    }
}

} // namespace PerformAction

void runClipboardScript() {
    if (!fs::is_regular_file(path.metadata.script)) return;

    static bool secondRun = false;

    fs::path currentPath = fs::current_path();

    fs::current_path(path.data);

    auto execute = [&](const std::string_view& timing) {
        // Set the CLIPBOARD_ACTION environment variable to the action that was performed
        int res = setenv("CLIPBOARD_ACTION", actions[action].data(), 1);
        if (res != 0) fprintf(stderr, "%s", formatColors("[error][inverse] ✘ [noinverse] Failed to set the CLIPBOARD_ACTION environment variable[blank]\n").data());

        // Set the CLIPBOARD_SCRIPT_TIMING environment variable to "before" or "after" depending on the timing
        res = setenv("CLIPBOARD_SCRIPT_TIMING", timing.data(), 1);
        if (res != 0) fprintf(stderr, "%s", formatColors("[error][inverse] ✘ [noinverse] Failed to set the CLIPBOARD_SCRIPT_TIMING environment variable[blank]\n").data());

        res = system(path.metadata.script.string().c_str());
        if (res != 0) {
            res = WEXITSTATUS(res);
            fprintf(stderr, formatColors("[error][inverse] ✘ [noinverse] Failed to run the clipboard script (returned exit code [bold]%d[nobold])[blank]\n").data(), res);
        }
    };

    if (!secondRun)
        execute("before");
    else
        execute("after");

    fs::current_path(currentPath);

    secondRun = true;
}