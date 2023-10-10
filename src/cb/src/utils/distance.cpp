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
#include "../clipboard.hpp"

unsigned long levenshteinDistance(const std::string_view& one, const std::string_view& two) {
    if (one == two) return 0;

    if (one.empty()) return two.size();
    if (two.empty()) return one.size();

    std::vector<std::vector<size_t>> matrix(one.size() + 1, std::vector<size_t>(two.size() + 1));

    for (size_t i = 0; i <= one.size(); i++)
        matrix.at(i).at(0) = i;

    for (size_t j = 0; j <= two.size(); j++)
        matrix.at(0).at(j) = j;

    for (size_t i = 1; i <= one.size(); i++) {
        for (size_t j = 1; j <= two.size(); j++) {
            if (one.at(i - 1) == two.at(j - 1))
                matrix.at(i).at(j) = matrix.at(i - 1).at(j - 1);
            else
                matrix.at(i).at(j) = std::min({matrix.at(i - 1).at(j - 1), matrix.at(i - 1).at(j), matrix.at(i).at(j - 1)}) + 1;
        }
    }

    return matrix.at(one.size()).at(two.size());
};