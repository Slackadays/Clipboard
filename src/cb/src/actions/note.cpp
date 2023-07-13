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

void noteText() {
    if (copying.items.size() >= 1) {
        if (copying.items.at(0).string() == "") {
            fs::remove(path.metadata.notes);
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Removed note[blank]\n").data());
        } else {
            fs::remove(path.metadata.notes);
            for (size_t i = 0; i < copying.items.size(); i++) {
                writeToFile(path.metadata.notes, copying.items.at(i).string(), true);
                if (i != copying.items.size() - 1) writeToFile(path.metadata.notes, " ", true);
            }
            if (output_silent || confirmation_silent) return;
            stopIndicator();
            fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved note \"%s\"[blank]\n").data(), fileContents(path.metadata.notes).value().data());
        }
    } else if (copying.items.empty()) {
        if (fs::is_regular_file(path.metadata.notes)) {
            std::string content(fileContents(path.metadata.notes).value());
            if (is_tty.out) {
                stopIndicator();
                printf(formatColors("[info]┃ Note for this clipboard: %s[blank]\n").data(), content.data());
            } else
                printf(formatColors("%s").data(), content.data());
        } else {
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[info]┃ There is no note for this clipboard.[blank]\n").data());
        }
    } else
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] You can't add multiple items to a note. [help]⬤ Try providing a single piece of text instead.[blank]\n"));
}

void notePipe() {
    std::string content(pipedInContent());
    writeToFile(path.metadata.notes, content);
    if (output_silent || confirmation_silent) return;
    stopIndicator();
    fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Saved note \"%s\"[blank]\n").data(), content.data());
    exit(EXIT_SUCCESS);
}

} // namespace PerformAction