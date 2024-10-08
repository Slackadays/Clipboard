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
#include <openssl/sha.h>

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
    path.applyIgnoreRules();
    exit(EXIT_SUCCESS);
}

void ignoreSecret() {
    std::vector<std::string> secrets;
    if (io_type == IOType::Pipe)
        secrets.emplace_back(pipedInContent());
    else
        std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(secrets), [](const auto& item) { return item.string(); });

    if (secrets.empty()) {
        if (fs::exists(path.metadata.ignore_secret) && !fs::is_empty(path.metadata.ignore_secret)) {
            std::vector<std::string> ignoreSecrets(fileLines(path.metadata.ignore_secret));

            if (is_tty.out) {
                stopIndicator();
                fprintf(stderr, "%s", formatColors("[info]┃ Secret hashes (SHA512) to ignore for this clipboard: [help]").data());
                for (const auto& secret : ignoreSecrets)
                    fprintf(stderr, "%s%s", secret.data(), secret != ignoreSecrets.back() ? ", " : "");
                fprintf(stderr, "%s", formatColors("[blank]\n").data());
            } else {
                for (const auto& secret : ignoreSecrets)
                    printf("%s%s", secret.data(), secret != ignoreSecrets.back() ? ", " : "");
            }
        } else {
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[info]┃ There are no secrets to ignore for this clipboard.[blank]\n").data());
        }
        return;
    }

    if (secrets.size() == 1 && (secrets.at(0) == "" || secrets.at(0) == "\n")) {
        if (!userIsARobot()) {
            stopIndicator();
            fprintf(stderr, "%s", formatColors("[progress]⬤ Are you sure you want to remove all secrets to ignore? [help]This action is irreversible. [bold][y(es)/n(o)] ").data());
            std::string decision;
            std::getline(std::cin, decision);
            fprintf(stderr, "%s", formatColors("[blank]").data());
            if (decision.substr(0, 1) != "y" && decision.substr(0, 1) != "Y") return;
            startIndicator();
        }
        fs::remove(path.metadata.ignore_secret);
        if (output_silent || confirmation_silent) return;
        stopIndicator();
        fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Removed secrets to ignore[blank]\n").data());
        exit(EXIT_SUCCESS);
    }

    std::string writeToFileContent;
    for (const auto& secret : secrets) {
        std::array<unsigned char, SHA512_DIGEST_LENGTH> hash;
        SHA512(reinterpret_cast<const unsigned char*>(secret.data()), secret.size(), hash.data());
        std::stringstream ss;
        for (const auto& byte : hash)
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        writeToFileContent += ss.str() + "\n";
    }

    writeToFile(path.metadata.ignore_secret, writeToFileContent);

    stopIndicator();
    fprintf(stderr, "%s", formatColors("[success][inverse] ✔ [noinverse] Saved ignore secrets [bold]").data());
    for (const auto& secret : secrets) {
        fprintf(stderr, "%s", secret.data());
        if (secret != secrets.back()) fprintf(stderr, ", ");
    }
    fprintf(stderr, "%s", formatColors("[blank]\n").data());
    path.applyIgnoreRules();
    exit(EXIT_SUCCESS);
}

void ignore() {
    if (secret_selection)
        ignoreSecret();
    else
        ignoreRegex();
}

} // namespace PerformAction