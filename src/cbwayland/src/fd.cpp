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
#include "fd.hpp"
#include "exception.hpp"

#include <cstring>

#include <sys/mman.h>
#include <unistd.h>

#include <clipboard/logging.hpp>

Fd::Fd(Fd&& other) noexcept {
    *this = std::move(other);
}

Fd& Fd::operator=(Fd&& other) noexcept {
    close();
    std::swap(m_value, other.m_value);
    return *this;
}

Fd::Fd(int value) : m_value(value) {}

int Fd::value() const {
    if (m_value <= 0) {
        throw WlException("Tried to get the value of an invalid file descriptor");
    }

    return m_value;
}

Fd::~Fd() noexcept {
    close();
}

Fd Fd::memfd(std::size_t size) {
    auto fd = memfd_create("Clipboard", 0);
    if (fd < 0) {
        throw WlException("Error allocating memfd");
    }
    if (ftruncate(fd, size)) {
        throw WlException("Error truncating memfd");
    }

    debugStream << "Created temporary file descriptor " << fd << std::endl;
    return Fd {fd};
}

void Fd::close() {
    if (m_value <= 0) {
        return;
    }

    debugStream << "Closing file descriptor " << m_value << std::endl;
    ::close(m_value);
    m_value = 0;
}

PipeFd::PipeFd(PipeFd&& other) noexcept {
    *this = std::move(other);
}

PipeFd& PipeFd::operator=(PipeFd&& other) noexcept {
    PipeFd::~PipeFd();
    std::swap(m_readFd, other.m_readFd);
    std::swap(m_writeFd, other.m_writeFd);
    return *this;
}

PipeFd::PipeFd() {
    int fds[2];
    if (pipe(fds) == -1) {
        throw WlException("Error creating pipe");
    }

    m_readFd = Fd {fds[0]};
    m_writeFd = Fd {fds[1]};
    debugStream << "Created a new pipe with read end " << fds[0] << " and write end " << fds[1] << std::endl;
}

PipeFd::~PipeFd() noexcept {
    closeRead();
    closeWrite();
}

FdStream::FdStream(FdBuffer&& buffer) : m_fdBuffer {std::make_unique<FdBuffer>(buffer)} {
    rdbuf(m_fdBuffer.get());
}

FdStream::FdStream(int fd) : FdStream(FdBuffer {fd}) {}
FdStream::FdStream(int readFd, int writeFd) : FdStream(FdBuffer {readFd, writeFd}) {}
FdStream::FdStream(const Fd& fd) : FdStream(FdBuffer {fd}) {}
FdStream::FdStream(const PipeFd& fd) : FdStream(FdBuffer {fd}) {}

FdBuffer::FdBuffer(int readFd, int writeFd) : m_readFd {readFd}, m_writeFd {writeFd} {

    setg(&m_readBuf.front(), &m_readBuf.back(), &m_readBuf.back());
    setp(&m_writeBuf.front(), &m_writeBuf.back());
}

FdBuffer::FdBuffer(int fd) : FdBuffer {fd, fd} {}
FdBuffer::FdBuffer(const Fd& fd) : FdBuffer {fd.value()} {}
FdBuffer::FdBuffer(const PipeFd& fd) : FdBuffer {fd.readFd(), fd.writeFd()} {}

std::size_t FdBuffer::safeRead(std::span<char> span) const {
    if (span.empty()) {
        throw WlException("Tried to read a nonpositive number of bytes");
    }

    auto result = read(m_readFd, span.data(), constrainSize(span.size()));
    if (result < 0) {
        throw WlException("Error calling read()");
    }

    return result;
}

std::size_t FdBuffer::safeWrite(std::span<const char> span) const {
    if (span.empty()) {
        throw WlException("Tried to write a nonpositive number of bytes");
    }

    auto result = write(m_writeFd, span.data(), constrainSize(span.size()));
    if (result < 0) {
        throw WlException("Error calling write()");
    }

    return result;
}

std::size_t FdBuffer::constrainSize(std::size_t size) const {
    return std::min(size, static_cast<std::size_t>(SSIZE_MAX));
}

std::size_t FdBuffer::repeatedRead(std::span<char> span) const {
    std::size_t total = 0;
    while (!span.empty()) {
        auto result = safeRead(span);
        if (result == 0) {
            break;
        }

        total += result;
        span = span.subspan(result);
    }

    return total;
}

std::size_t FdBuffer::repeatedWrite(std::span<const char> span) const {
    std::size_t total = 0;
    while (!span.empty()) {
        auto result = safeWrite(span);
        if (result == 0) {
            break;
        }

        total += result;
        span = span.subspan(result);
    }

    return total;
}

std::size_t FdBuffer::flushWrite() {
    std::size_t result = 0;
    if (pptr() > pbase()) {
        result += repeatedWrite({pbase(), pptr()});
        setp(m_writeBuf.data(), m_writeBuf.data() + m_writeBuf.size());
    }

    return result;
}

int FdBuffer::sync() {
    flushWrite();
    return 0;
}

FdBuffer::int_type FdBuffer::underflow() {
    if (gptr() < egptr()) {
        throw WlException("Expected gptr() = egptr() during call to underflow()");
    }

    auto count = repeatedRead(m_readBuf);
    setg(&m_readBuf.front(), &m_readBuf.front(), m_readBuf.data() + count);

    if (count == 0) {
        return traits_type::eof();
    }
    return m_readBuf[0];
}

std::streamsize FdBuffer::xsgetn(char_type* output, std::streamsize count) {
    if (count <= 0) {
        throw WlException("Tried reading a nonpositive number of bytes");
    }

    std::streamsize totalRead = 0;

    auto bufAvailable = egptr() - gptr();
    if (bufAvailable > 0) {
        auto bufRead = std::min(bufAvailable, count);
        std::memcpy(output, gptr(), bufRead);

        output += bufRead;
        totalRead += bufRead;
        gbump(bufRead);
    }

    totalRead += repeatedRead({output, static_cast<std::size_t>(count - totalRead)});
    return totalRead;
}

FdBuffer::int_type FdBuffer::overflow(int_type ch) {
    flushWrite();

    if (ch != traits_type::eof()) {
        repeatedWrite({reinterpret_cast<char*>(&ch), 1});
    }

    return 1;
}

std::streamsize FdBuffer::xsputn(const char_type* input, std::streamsize count) {
    flushWrite();
    return repeatedWrite({input, static_cast<std::size_t>(count)});
}
