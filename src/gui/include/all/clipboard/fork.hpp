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
#pragma once

#include <clipboard/logging.hpp>
#include <clipboard/utils.hpp>
#include <functional>
#include <vector>

#if defined(HAVE_FORK)
#include <signal.h>
#include <unistd.h>
#endif

/**
 * Manages all operations related to process forking.
 */
class Forker {
    using callback_t = std::function<void()>;
    std::vector<callback_t> m_nonForkCallbacks {};
    std::vector<callback_t> m_forkCallbacks {};

public:
    /**
     * Registers a callback to be executed in a forked process, after the process forks but
     * before the business logic is executed.
     */
    void atFork(callback_t&&);

    /**
     * Registers a callback to be executed if a requested fork isn't performed due to external factors
     * preventing the process from forking (e.g. environment variable or debug configuration).
     */
    void atNonFork(callback_t&&);

    /**
     * Forks the process. In the original/parent process, this function exits immediately after
     * forking and does nothing. In the forked/child process, the function specified as a parameter
     * is invoked, any exceptions that it throws are swallowed, and the process exits immediately
     * after without returning control to the upstream stack.
     *
     * If any callbacks were registered with atFork, they'll be executed in the forked/child process
     * before the parameter function is invoked.
     */
    template <std::invocable func_t>
    void fork(func_t func) const;
};

#if defined(HAVE_FORK)
template <std::invocable func_t>
void Forker::fork(func_t func) const {
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    bool noFork = envVarIsTrue("CLIPBOARD_NO_FORK");
    if (!noFork && ::fork() != 0) {
        debugStream << "Successfully forked process" << std::endl;
        return;
    }

    debugStream << "We are the forked process, hijacking operation" << std::endl;

    try {
        if (noFork) {
            for (auto&& callback : m_nonForkCallbacks) {
                callback();
            }
        } else {
            for (auto&& callback : m_forkCallbacks) {
                callback();
            }
        }

        func();
    } catch (const std::exception& e) {
        debugStream << "Error during fork operation: " << e.what() << std::endl;
        kill(getppid(), SIGUSR2);
    } catch (...) {
        debugStream << "Unknown error during fork operation" << std::endl;
        kill(getppid(), SIGUSR2);
    }

    // Always exit no matter what happens, to prevent the forked daemon
    // from returning control to the stack frames above and overwriting the
    // non-forked original process' work
    std::_Exit(EXIT_SUCCESS);
}

bool waitForSuccessSignal();
#else
template <std::invocable func_t>
void Forker::fork(func_t func) const {
    static_assert(AssertFalse<sizeof(func)>::value, "This platform doesn't support fork()");
}
#endif
