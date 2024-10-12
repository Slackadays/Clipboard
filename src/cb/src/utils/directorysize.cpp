/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2024 Jackson Huff and other contributors on GitHub.com
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

#if defined(UNIX_OR_UNIX_LIKE)
#include <ftw.h>
#endif

size_t directoryOverhead(const fs::path& directory) {
#if defined(UNIX_OR_UNIX_LIKE)
    struct stat info;
    if (stat(directory.string().data(), &info) != 0) return 0;
    return info.st_size;
#else
    return 0;
#endif
}

size_t size = 0;

#if defined(UNIX_OR_UNIX_LIKE)
int ftwHandler(const char* fpath, const struct stat* sb, int typeflag) {
    size += sb->st_size;
    return 0;
}
#endif

size_t totalDirectorySize(const fs::path& directory) {
    size = directoryOverhead(directory);
#if defined(UNIX_OR_UNIX_LIKE)
    ftw(directory.string().data(), ftwHandler, 1);
#else
    for (const auto& entry : fs::recursive_directory_iterator(directory))
        try {
            size += entry.is_directory() ? directoryOverhead(entry) : entry.file_size();
        } catch (const fs::filesystem_error& e) {
            if (e.code() != std::errc::no_such_file_or_directory) throw e;
        }
#endif
    return size;
}