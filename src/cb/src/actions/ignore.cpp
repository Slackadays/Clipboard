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

void ignoreRegex() {
    std::vector<std::string> regexes;
    if (io_type == IOType::Pipe)
        regexes.emplace_back(pipedInContent());
    else
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(regexes), [](const auto& item) { return item.string(); });

    if (regexes.empty()) {
        if (fs::exists(path.metadata.ignore) && !fs::is_empty(path.metadata.ignore)) {
            std::vector<std::string> ignorePatterns(fileLines(path.metadata.ignore));

            if (is_tty.out) {
                stopIndicator();
                fprintf(stderr, "%s", formatColors("[info]┃ Ignore patterns for this clipboard: [help]").data());
                for (const auto& pattern : ignorePatterns)
                    fprintf(stderr, "%s%s", pattern.data(), pattern != ignorePatterns.back() ? ", " : "");
                fprintf(stderr, "%s", formatColors("[blank]\n").data());
            } else {
                for (const auto& pattern : ignorePatterns)
                    printf("%s%s", pattern.data(), pattern != ignorePatterns.back() ? ", " : "");
            }
        } else {
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[info]┃ There are no ignore patterns for this clipboard.[blank]\n").data());
        }
        return;
    }

    if (regexes.size() == 1 && (regexes.at(0) == "" || regexes.at(0) == "\n")) {
        fs::remove(path.metadata.ignore);
        if (output_silent || confirmation_silent) return;
        stopIndicator();
        fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Removed ignore patterns\n").data());
        exit(EXIT_SUCCESS);
    }

    for (const auto& pattern : regexes) {
        try {
            volatile auto test = std::regex(pattern); // volatile makes sure this otherwise unused variable isn't optimized out
        } catch (const std::regex_error& e) {
            error_exit(
                    formatColors(
                            "[error][inverse] ✘ [noinverse] The regex pattern you provided [bold](\"%s\")[blank][error] is invalid with error %s [help]⬤ Try using a different one instead.[blank]\n"
                    ),
                    pattern,
                    std::string(e.what())
            );
        }
    }

    std::string writeToFileContent;
    for (const auto& pattern : regexes)
        writeToFileContent += pattern + "\n";

    writeToFile(path.metadata.ignore, writeToFileContent);

    stopIndicator();
    fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Saved ignore patterns [bold]").data());
    for (const auto& pattern : regexes) {
        fprintf(stderr, "%s", pattern.data());
        if (pattern != regexes.back()) fprintf(stderr, ", ");
    }
    fprintf(stderr, "%s", formatColors("[blank]\n").data());
    path.applyIgnoreRegexes();
    exit(EXIT_SUCCESS);
}

} // namespace PerformAction