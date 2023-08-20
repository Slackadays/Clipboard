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
#include "clipboard.hpp"

#include "sounds/error.hpp"
#include "sounds/success.hpp"

#if defined(_WIN32) || defined(_WIN64)
#define STDIN_FILENO 0
#define read _read
#endif

bool stopIndicator(bool change_condition_variable) {
    IndicatorState expect = IndicatorState::Active;

    if (!change_condition_variable) return progress_state.exchange(IndicatorState::Cancel) == expect; // return true if the state was changed from Active to Cancel

    if (!progress_state.compare_exchange_strong(expect, IndicatorState::Done)) return false; // return false if the state was not changed from Active to Done

    cv.notify_one();
    indicator.join();
    return true;
}

void indicatorThread() {
    if (!is_tty.err || output_silent || progress_silent) return;

    bool hasFocus = true;

    makeTerminalRaw();

    fprintf(stderr, "\033[?25l"); // hide the cursor
    fflush(stderr);

    int columns = thisTerminalSize().columns;

    std::unique_lock<std::mutex> lock(m);

    auto start = std::chrono::steady_clock::now();

    bool readyToPollFocus = false;

    int step = 0;

    auto poll_focus = [&] {
        if (!readyToPollFocus) { // add a time duration check because on some slow systems over SSH, the reporting characters show up in the terminal
            if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(50))
                readyToPollFocus = true;
            else
                return;
            printf("\033[?1004h"); // enable focus tracking
            fflush(stdout);
        }
        std::array<char, 32> buf;
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

    auto display_progress = [&](const auto& formattedNum, const std::string_view& actionText = doing_action[action]) {
        if (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500)) {
            cv.wait_for(lock, std::chrono::milliseconds(17), [&] { return progress_state != IndicatorState::Active; });
            return;
        }
        std::string progressBar;
        if (step < 40) {
            progressBar += repeatString("█", step);
            progressBar += repeatString("▒", 39 - step);
        } else {
            progressBar += repeatString("▒", step - 40);
            progressBar += repeatString("█", 39 - (step - 40));
        }
        std::string formattedSeconds = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count()) + "s";
        fprintf(stderr, working_message().data(), actionText.data(), formattedNum, formattedSeconds.data(), progressBar.data());
        fprintf(stderr, "\033]0;%s (%s) - Clipboard\007", actionText.data(), formattedNum); // set the terminal title
        fflush(stderr);
        cv.wait_for(lock, std::chrono::milliseconds(17), [&] { return progress_state != IndicatorState::Active; });
    };

    while (clipboard_state == ClipboardState::Setup && progress_state == IndicatorState::Active) {
        display_progress("%?", "Setting up");

        if (is_tty.out) poll_focus();

        step == 79 ? step = 0 : step++;
    }

    auto itemsToProcess = [&] {
        return std::distance(fs::directory_iterator(path.data), fs::directory_iterator());
    };

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
            display_progress("?%");

        if (is_tty.out) poll_focus();

        step == 79 ? step = 0 : step++;
    }

    fprintf(stderr, "\033[?25h"); // restore the cursor
    fflush(stderr);
    if (is_tty.out) printf("\033[?1004l"); // disable focus tracking
    fflush(stdout);

    if (!hasFocus && clipboard_state != ClipboardState::Error && !envVarIsTrue("CLIPBOARD_NOAUDIO")) {
        std::valarray<short> samples(success_pcm_len / 2);
        for (size_t i = 0; i < success_pcm_len; i += 2)
            samples[i / 2] = static_cast<short>(success_pcm[i] | (success_pcm[i + 1] << 8));
        if (!playAsyncSoundEffect(samples)) printf("\007");
    } else if (clipboard_state == ClipboardState::Error && !envVarIsTrue("CLIPBOARD_NOAUDIO")) {
        std::valarray<short> samples(error_pcm_len / 2);
        for (size_t i = 0; i < error_pcm_len; i += 2)
            samples[i / 2] = static_cast<short>(error_pcm[i] | (error_pcm[i + 1] << 8));
        if (!playAsyncSoundEffect(samples)) printf("\007");
    }

    fflush(stdout);

    makeTerminalNormal();

    if (is_tty.out) printf("\r%*s\r", columns, "");
    fflush(stdout);
    fprintf(stderr, "\r%*s\r", columns, "");
    fflush(stderr);

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
    indicator = std::thread(indicatorThread);
}
