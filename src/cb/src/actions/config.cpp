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
void config() {
    // display the configuration for CB

    stopIndicator();
    fprintf(stderr, "%s", formatColors("[info]┏━━[inverse] ").data());
    fprintf(stderr, "%s", cb_config_message().data());
    fprintf(stderr, "%s", formatColors(" [noinverse][info]━").data());
    int columns = thisTerminalSize().columns - ((columnLength(clipboard_name_message) + 1));
    for (int i = 0; i < columns; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatColors("┓[blank]\n").data());

    // Clipbord editor
    fprintf(stderr, formatColors("[info]%s┃ Content editor: [help]%s[blank]\n").data(), generatedEndbar().data(), findUsableEditor() ? findUsableEditor().value().data() : "None");

    // Max history size
    fprintf(stderr, formatColors("[info]%s┃ Max history size: [help]%s[blank]\n").data(), generatedEndbar().data(), !maximumHistorySize.empty() ? maximumHistorySize.data() : "unlimited");

    // Locale
    fprintf(stderr, formatColors("[info]%s┃ Locale: [help]%s[blank]\n").data(), generatedEndbar().data(), !locale.empty() ? locale.data() : "default");

    // Temporary directory
    fprintf(stderr, formatColors("[info]%s┃ Temporary directory: [help]%s[blank]\n").data(), generatedEndbar().data(), global_path.temporary.string().data());

    // Persistent directory
    fprintf(stderr, formatColors("[info]%s┃ Persistent directory: [help]%s[blank]\n").data(), generatedEndbar().data(), global_path.persistent.string().data());

    // Custom persistent clipboards
    fprintf(stderr,
            formatColors("[info]%s┃ Custom persistent clipboards: [help]%s[blank]\n").data(),
            generatedEndbar().data(),
            getenv("CLIPBOARD_CUSTOM_PERSISTENT") ? getenv("CLIPBOARD_CUSTOM_PERSISTENT") : "none");

    // Audio
    fprintf(stderr, formatColors("[info]%s┃ Audio effects: [help]%s[blank]\n").data(), generatedEndbar().data(), envVarIsTrue("CLIPBOARD_NOAUDIO") ? "disabled" : "enabled");

    // GUI clipboard integration
    fprintf(stderr, formatColors("[info]%s┃ GUI clipboard integration: [help]%s[blank]\n").data(), generatedEndbar().data(), envVarIsTrue("CLIPBOARD_NOGUI") ? "disabled" : "enabled");

    // Remote clipboard integration
    fprintf(stderr, formatColors("[info]%s┃ Remote clipboard integration: [help]%s[blank]\n").data(), generatedEndbar().data(), envVarIsTrue("CLIPBOARD_NOREMOTE") ? "disabled" : "enabled");

    // Progress bar
    fprintf(stderr, formatColors("[info]%s┃ Progress bar: [help]%s[blank]\n").data(), generatedEndbar().data(), progress_silent ? "disabled" : "enabled");

    // Silent output
    fprintf(stderr, formatColors("[info]%s┃ Silent output: [help]%s[blank]\n").data(), generatedEndbar().data(), output_silent ? "enabled" : "disabled");

    // Theme
    fprintf(stderr, formatColors("[info]%s┃ Color theme: [help]%s[blank]\n").data(), generatedEndbar().data(), getenv("CLIPBOARD_THEME") ? getenv("CLIPBOARD_THEME") : "default");

    // Color output
    fprintf(stderr, formatColors("[info]%s┃ Color output: [help]%s[blank]\n").data(), generatedEndbar().data(), no_color ? "disabled" : "enabled");

    fprintf(stderr, "%s", formatColors("[info]┗").data());
    int cols = thisTerminalSize().columns;
    for (int i = 0; i < cols - 2; i++)
        fprintf(stderr, "━");
    fprintf(stderr, "%s", formatColors("┛[blank]\n").data());
}
} // namespace PerformAction