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

void addFiles() {
    if (path.holdsRawData()) {
        stopIndicator();
        fprintf(stderr,
                "%s",
                formatMessage("[error]❌ You can't add items to text. [blank]💡 [help]Try copying text first, or add "
                              "text instead.[blank]\n")
                        .data());
        exit(EXIT_FAILURE);
    }
    for (const auto& f : copying.items)
        copyItem(f);
}

void addData() {
    if (path.holdsRawData()) {
        std::string content;
        if (io_type == IOType::Pipe)
            content = pipedInContent();
        else
            content = copying.items.at(0).string();
        successes.bytes += writeToFile(path.data.raw, content, true);
    } else if (!fs::is_empty(path.data)) {
        stopIndicator();
        fprintf(stderr,
                "%s",
                formatMessage("[error]❌ You can't add text to items. [blank]💡 [help]Try copying text first, or add a "
                              "file instead.[blank]\n")
                        .data());
        exit(EXIT_FAILURE);
    } else {
        if (io_type == IOType::Pipe)
            pipeIn();
        else if (io_type == IOType::Text)
            successes.bytes += writeToFile(path.data.raw, copying.items.at(0).string());
    }
}

} // namespace PerformAction