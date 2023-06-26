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

#include <chrono>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <variant>

#include <clipboard/logging.hpp>

/*
 * Helper class that contains either an owned std::string or a char const* with static
 * lifetime that doesn't have to be deleted. This can help avoid unnecessary copies.
 */
class StringOrLiteral {
private:
    std::variant<std::string, const char*> m_data;

public:
    /*implicit*/ StringOrLiteral(std::string&& data) : m_data(std::move(data)) {}
    /*implicit*/ StringOrLiteral(const char* data) : m_data(data) {}

    operator const char*() const;
    operator std::string_view() const;
};

/*
 * Base class for simple exceptions that only contain a message.
 */
class SimpleException : public std::exception {
private:
    StringOrLiteral m_message;

public:
    explicit SimpleException(StringOrLiteral&& message) : m_message(std::move(message)) {}

    template <typename... Args>
    explicit SimpleException(Args&&... args);

    [[nodiscard]] const char* what() const noexcept override { return m_message; }
};

/**
 * Helper class that will call a function at the end of the current scope
 * unless it is disarmed. Useful to clean up when errors occur.
 */
template <std::invocable guard_t>
class ArmedGuard {
    bool m_armed {true};
    guard_t m_guard;

public:
    ArmedGuard(ArmedGuard&&) = delete;
    ArmedGuard(const ArmedGuard&) = delete;

    ArmedGuard& operator=(ArmedGuard&&) = delete;
    ArmedGuard& operator=(const ArmedGuard&) = delete;

    explicit ArmedGuard(guard_t guard) : m_guard {guard} {}

    inline void disarm() { m_armed = false; }

    ~ArmedGuard() {
        if (m_armed) {
            m_guard();
            m_armed = false;
        }
    }
};

/**
 * Class with a dummy template argument that always evaluates to false in static contexts.
 * Can be used to e.g. static_assert on a template argument to ensure a given template specialization
 * fails at compile-time.
 */
template <auto T>
struct AssertFalse : std::false_type {};

/**
 * Decodes a percent-encoded string.
 * Does not strip any other URL elements (e.g. file:// is left as-is if included)
 */
std::string urlDecode(std::string_view);

/**
 * Encodes a string with percent-encoding suitable for URLs.
 */
std::string urlEncode(std::string_view);

/**
 * Checks if an environment variable is defined and has a true-ish value.
 * True-ish values include "1", "ON", "Y", "TRUE", and so on.
 */
bool envVarIsTrue(const std::string_view& name);

/**
 * Keeps calling a function until it returns a value, using an exponential backoff scheme
 * between each call and with a timeout to ensure operations don't run for too long.
 */
template <typename func_t>
auto pollUntilReturn(func_t func) -> typename std::invoke_result_t<func_t>::value_type {
    using namespace std::literals;
    using optional_t = typename std::invoke_result_t<func_t>;

    constexpr auto maxEventPollTime = 10s;
    constexpr auto startEventPollBackoff = 1ms;
    constexpr auto eventPollBackoffMultiplier = 2;
    constexpr auto maxEventPollBackoffTime = 500ms;

    const auto startTime = std::chrono::steady_clock::now();
    auto backoffTime = startEventPollBackoff;

    optional_t result;
    while (!(result = func()).has_value()) {
        debugStream << "No pollUntilReturn data, sleeping" << std::endl;

        const auto time = std::chrono::steady_clock::now() - startTime;
        if (time >= maxEventPollTime) {
            debugStream << "Timeout during pollUntilReturn" << std::endl;
            throw SimpleException("Timeout during pollUntilReturn");
        }

        std::this_thread::sleep_for(backoffTime);
        backoffTime = eventPollBackoffMultiplier * backoffTime;
        if (backoffTime > maxEventPollBackoffTime) {
            backoffTime = maxEventPollBackoffTime;
        }
    }

    debugStream << "pollUntilReturn finished successfully, got a result" << std::endl;
    return result.value();
}

template <typename... Args>
SimpleException::SimpleException(Args&&... args) : m_message {""} {
    std::ostringstream message;
    (message << ... << args);
    m_message = std::move(message).str();
}