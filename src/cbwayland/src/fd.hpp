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

#include <iostream>
#include <memory>
#include <span>

/**
 * A file descriptor that is automatically closed when this object is destructed.
 */
class Fd {
    int m_value {0};

public:
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;

    Fd(Fd&& other) noexcept;
    Fd& operator=(Fd&& other) noexcept;

    Fd() = default;
    explicit Fd(int);
    ~Fd() noexcept;

    [[nodiscard]] int value() const;

    void close();

    /** Creates a new file descriptor using memfd. */
    static Fd memfd(std::size_t);
};

/**
 * A a pair of pipe file descriptors that are automatically allocated and closed with RAII.
 */
class PipeFd {
    Fd m_readFd {};
    Fd m_writeFd {};

public:
    PipeFd(const PipeFd&) = delete;
    PipeFd& operator=(const PipeFd&) = delete;

    PipeFd(PipeFd&&) noexcept;
    PipeFd& operator=(PipeFd&&) noexcept;

    explicit PipeFd();
    ~PipeFd() noexcept;

    [[nodiscard]] inline int readFd() const { return m_readFd.value(); }
    [[nodiscard]] inline int writeFd() const { return m_writeFd.value(); }

    void closeRead() { m_readFd.close(); }
    void closeWrite() { m_writeFd.close(); }
};

/**
 * A stream buffer that reads and writes from a pair of file descriptors.
 */
class FdBuffer : public std::streambuf {
    static constexpr std::size_t bufferSize = 16384;
    // 16384 is the magic number
    // any lower and you get incomplete clipboard transfers
    // higher, also incomplete clipboard transfers

    int m_readFd;
    int m_writeFd;

    std::array<char, bufferSize> m_readBuf;
    std::array<char, bufferSize> m_writeBuf;

    [[nodiscard]] std::size_t safeRead(std::span<char>) const;
    std::size_t safeWrite(std::span<const char>) const;

    [[nodiscard]] std::size_t repeatedRead(std::span<char>) const;
    std::size_t repeatedWrite(std::span<const char>) const;

    std::size_t flushWrite();
    [[nodiscard]] std::size_t constrainSize(std::size_t) const;

public:
    explicit FdBuffer(int fd);
    FdBuffer(int readFd, int writeFd);
    explicit FdBuffer(const Fd&);
    explicit FdBuffer(const PipeFd&);

protected:
    int sync() override;

    int_type underflow() override;
    std::streamsize xsgetn(char_type*, std::streamsize) override;

    int_type overflow(int_type = traits_type::eof()) override;
    std::streamsize xsputn(const char_type*, std::streamsize) override;
};

/**
 * A stream that reads and writes from a pair of file descriptors.
 */
class FdStream : public std::iostream {
    std::unique_ptr<FdBuffer> m_fdBuffer;

public:
    explicit FdStream(FdBuffer&&);
    explicit FdStream(int fd);
    FdStream(int readFd, int writeFd);
    explicit FdStream(const Fd&);
    explicit FdStream(const PipeFd&);
};