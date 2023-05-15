/*  The Clipboard Project - Cut, copy, and paste anything, anywhere, all from the terminal.
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
    if (copying.items.size() == 1) {
        if (copying.items.at(0).string() == "") {
            fs::remove(path.metadata.notes);
            if (output_silent) return;
            stopIndicator();
            fprintf(stderr, "%s", formatMessage("[success]✅ Removed note\n").data());
        } else {
            writeToFile(path.metadata.notes, copying.items.at(0).string());
            if (output_silent) return;
            stopIndicator();
            fprintf(stderr, formatMessage("[success]✅ Saved note \"%s\"\n").data(), copying.items.at(0).string().data());
        }
    } else if (copying.items.empty()) {
        if (fs::is_regular_file(path.metadata.notes)) {
            std::string content(fileContents(path.metadata.notes));
            if (is_tty.out)
                fprintf(stdout, formatMessage("[info]🔷 Note for this clipboard: %s\n").data(), content.data());
            else
                fprintf(stdout, formatMessage("%s").data(), content.data());
        } else {
            fprintf(stderr, "%s", formatMessage("[info]🔷 There is no note for this clipboard.[blank]\n").data());
        }
    } else {
        stopIndicator();
        fprintf(stderr, "%s", formatMessage("[error]❌ You can't add multiple items to a note. 💡 [blank][help]Try providing a single piece of text instead.[blank]\n").data());
        exit(EXIT_FAILURE);
    }
}

void notePipe() {
    std::string content(pipedInContent());
    writeToFile(path.metadata.notes, content);
    if (output_silent) return;
    stopIndicator();
    fprintf(stderr, formatMessage("[success]✅ Saved note \"%s\"\n").data(), content.data());
    exit(EXIT_SUCCESS);
}

} // namespace PerformAction