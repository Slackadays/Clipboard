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
#include "buffer.hpp"
#include "all.hpp"

WlBuffer::WlBuffer(std::unique_ptr<WlShmPool>&& pool, std::int32_t offset, std::int32_t width, std::int32_t height, std::int32_t stride, wl_shm_format format)
        : WlObject<spec_t> {wl_shm_pool_create_buffer(getValue(pool), offset, width, height, stride, format)}
        , m_shmPool {std::move(pool)} {}

std::unique_ptr<WlBuffer> WlBuffer::fromMemfd(const WlRegistry& registry, std::int32_t width, std::int32_t height, std::int32_t stride, wl_shm_format format) {
    if (!registry.get<WlShm>().supports(format)) {
        throw WlException("wl_shm doesn't support format ", format);
    }

    auto size = stride * height;
    return std::make_unique<WlBuffer>(WlShmPool::fromMemfd(registry, size), 0, width, height, stride, format);
}
