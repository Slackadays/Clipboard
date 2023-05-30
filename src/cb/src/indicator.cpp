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
#include "clipboard.hpp"

#if defined(_WIN32) || defined(_WIN64)
#define STDIN_FILENO 0
#define read _read
#endif

bool stopIndicator(bool change_condition_variable) {
    IndicatorState expect = IndicatorState::Active;

    if (!change_condition_variable) return progress_state.exchange(IndicatorState::Cancel) == expect;

    if (!progress_state.compare_exchange_strong(expect, IndicatorState::Done)) return false;

    cv.notify_one();
    indicator.join();
    return true;
}

void setupIndicator() {
    if (!is_tty.err || output_silent || progress_silent) return;

    bool hasFocus = true;

    makeTerminalRaw();

    fprintf(stderr, "\033]0;%s - Clipboard\007", doing_action[action].data()); // set the terminal title
    fprintf(stderr, "\033[?25l");                                              // hide the cursor
    if (is_tty.out) printf("\033[?1004h");                                     // enable focus tracking
    fflush(stdout);
    fflush(stderr);

    std::unique_lock<std::mutex> lock(m);
    int output_length = 0;
    const std::array<std::string_view, 22> spinner_steps {"╸         ", "━         ", "╺╸        ", " ━        ", " ╺╸       ", "  ━       ", "  ╺╸      ", "   ━      ",
                                                          "   ╺╸     ", "    ━     ", "    ╺╸    ", "     ━    ", "     ╺╸   ", "      ━   ", "      ╺╸  ", "       ━  ",
                                                          "       ╺╸ ", "        ━ ", "        ╺╸", "         ━", "         ╺", "          "};
    int step = 0;
    auto poll_focus = [&] {
        std::array<char, 16> buf;
#if defined(_WIN32) || defined(_WIN64)
        DWORD bytesAvailable = 0;
        PeekNamedPipe(GetStdHandle(STD_INPUT_HANDLE), nullptr, 0, nullptr, &bytesAvailable, nullptr);
        if (bytesAvailable < 3) return;
#endif
        if (read(STDIN_FILENO, buf.data(), buf.size()) >= 3) {
            if (buf.at(0) == '\033' && buf.at(1) == '[' && buf.at(2) == 'I') hasFocus = true;
            if (buf.at(0) == '\033' && buf.at(1) == '[' && buf.at(2) == 'O') hasFocus = false;
        }
    };
    auto display_progress = [&](const auto& formattedNum) {
        output_length = fprintf(stderr, working_message().data(), doing_action[action].data(), formattedNum, spinner_steps.at(step).data());
        fflush(stderr);
        cv.wait_for(lock, std::chrono::milliseconds(20), [&] { return progress_state != IndicatorState::Active; });
    };
    auto itemsToProcess = [&] {
        return std::distance(fs::directory_iterator(path.data), fs::directory_iterator());
    };
    while (clipboard_state == ClipboardState::Setup && progress_state == IndicatorState::Active) {
        display_progress("");
        step == 21 ? step = 0 : step++;
    }
    static size_t items_size = action_is_one_of(Action::Cut, Action::Copy) ? copying.items.size() : itemsToProcess();
    if (items_size == 0) items_size++;
    auto percent_done = [&] {
        return std::to_string(((successes.files + successes.directories + copying.failedItems.size()) * 100) / items_size) + "%";
    };
    while (clipboard_state == ClipboardState::Action && progress_state == IndicatorState::Active) {
        if (io_type == IOType::File)
            display_progress(percent_done().data());
        else if (io_type == IOType::Pipe)
            display_progress(formatBytes(successes.bytes.load(std::memory_order_relaxed)).data());
        else
            display_progress("");

        if (is_tty.out) poll_focus();

        step == 21 ? step = 0 : step++;
    }

    fprintf(stderr, "\033[?25h");          // restore the cursor
    if (is_tty.out) printf("\033[?1004l"); // disable focus tracking
    if (!hasFocus) printf("\007");         // play a bell sound if the terminal doesn't have focus
    fflush(stdout);
    fprintf(stderr, "\r%*s\r", output_length, "");
    fflush(stderr);

    makeTerminalNormal();

    if (progress_state == IndicatorState::Cancel) {
        if (io_type == IOType::File)
            fprintf(stderr, cancelled_with_progress_message().data(), actions[action].data(), percent_done().data());
        else if (io_type == IOType::Pipe)
            fprintf(stderr, cancelled_with_progress_message().data(), actions[action].data(), formatBytes(successes.bytes.load(std::memory_order_relaxed)).data());
        else
            fprintf(stderr, cancelled_message().data(), actions[action].data());
        fflush(stderr);
        path.releaseLock();
        _exit(EXIT_FAILURE);
    }
    fflush(stderr);
}

void startIndicator() { // If cancelled, leave cancelled
    IndicatorState expect = IndicatorState::Done;
    progress_state.compare_exchange_strong(expect, IndicatorState::Active);
    indicator = std::thread(setupIndicator);
}