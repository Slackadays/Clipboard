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
#include "shm_pool.hpp"
#include "all.hpp"

WlShmPool::WlShmPool(const WlShm& shm, Fd&& fd, std::int32_t size) : WlObject<spec_t> {wl_shm_create_pool(shm.value(), fd.value(), size)}, m_fd {std::move(fd)}, m_size(size) {}

std::unique_ptr<WlShmPool> WlShmPool::fromMemfd(const WlShm& shm, std::int32_t size) {
    return std::make_unique<WlShmPool>(shm, Fd::memfd(size), size);
}

std::unique_ptr<WlShmPool> WlShmPool::fromMemfd(const WlRegistry& registry, std::int32_t size) {
    return fromMemfd(registry.get<WlShm>(), size);
}
