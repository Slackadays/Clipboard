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
#include "../clipboard.hpp"

namespace PerformAction {

void search() {
    if (copying.items.empty())
        error_exit("%s", formatMessage("[error]❌ You need to enter something to search for. 💡 [help]Try entering a search term after the action, like [bold]cb search Foobar[blank][help].[blank]\n"));

    std::vector<std::string> queries;
    std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(queries), [](const auto& item) { return item.string(); });

    struct Result {
        std::string preview;
        std::string clipboard;
        unsigned long entry;
        unsigned long score;
    };

    std::vector<Result> results;

    std::vector<Clipboard> targets;

    if (all_option) {
        for (const auto& entry : fs::directory_iterator(global_path.temporary))
            if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsData()) targets.emplace_back(cb);
        for (const auto& entry : fs::directory_iterator(global_path.persistent))
            if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsData()) targets.emplace_back(cb);
    } else
        targets.emplace_back(path);

    auto levenshteinDistance = [](const std::string& one, const std::string& two) -> unsigned long {
        if (one == two) return 0;

        if (one.empty()) return two.size();

        if (two.empty()) return one.size();

        std::vector<std::vector<unsigned long>> matrix(one.size() + 1, std::vector<unsigned long>(two.size() + 1));

        for (unsigned long i = 0; i <= one.size(); i++)
            matrix.at(i).at(0) = i;

        for (unsigned long j = 0; j <= two.size(); j++)
            matrix.at(0).at(j) = j;

        for (unsigned long i = 1; i <= one.size(); i++) {
            for (unsigned long j = 1; j <= two.size(); j++) {
                if (one.at(i - 1) == two.at(j - 1))
                    matrix.at(i).at(j) = matrix.at(i - 1).at(j - 1);
                else
                    matrix.at(i).at(j) = std::min({matrix.at(i - 1).at(j - 1), matrix.at(i - 1).at(j), matrix.at(i).at(j - 1)}) + 1;
            }
        }

        return matrix.at(one.size()).at(two.size());
    };

    // std::cerr << "distance between foo and fobobar is " << levenshteinDistance("foo", "fobobar") << std::endl;

    // exit(0);

    auto contentMatchRating = [&](const std::string& content, const std::string& query) -> std::optional<Result> {
        Result result;

        // check if the content matches the query

        if (content == query) {
            result.score = 1000;
            result.preview = "\033[1m" + content + "\033[0m";
            return result;
        }
        // then check if the content regex matches the query

        else if (std::regex_match(content, std::regex(query))) {
            result.score = 800;
            result.preview = "\033[1m" + content + "\033[0m";
            return result;
        }
        // then do a regex search of the content for the query

        else if (std::smatch sm; std::regex_search(content, sm, std::regex(query))) {
            result.score = 600;
            result.preview = content.substr(0, sm.position(0)) + "\033[1m" + sm.str(0) + "\033[0m" + content.substr(sm.position(0) + sm.length(0));
            return result;
        }

        // then do a fuzzy search of the content for the query

        else if (content.size() < 1000)
            if (auto distance = levenshteinDistance(content, query); distance < 100) {
                result.score = 400 - distance;
                result.preview = "\033[1m" + content + "\033[0m";
                return result;
            }

        return std::nullopt;
    };

    for (auto& clipboard : targets) {
        for (const auto& entry : clipboard.entryIndex) {
            clipboard.setEntry(entry);
            if (clipboard.holdsRawDataInCurrentEntry()) {
                for (const auto& query : queries) {
                    if (auto rating = contentMatchRating(fileContents(clipboard.data.raw), query); rating.has_value()) {
                        rating->clipboard = clipboard.name();
                        rating->entry = entry;
                        results.emplace_back(rating.value());
                    }
                }
            } else {
                for (const auto& item : fs::directory_iterator(clipboard.data)) {
                    for (const auto& query : queries) {
                        if (auto rating = contentMatchRating(item.path().filename().string(), query); rating.has_value()) {
                            rating->clipboard = clipboard.name();
                            rating->entry = entry;
                            results.emplace_back(rating.value());
                        }
                    }
                }
            }
        }
    }

    std::sort(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.score > two.score; });

    stopIndicator();

    for (const auto& result : results) {
        std::cout << "clipboard " << result.clipboard << " entry " << result.entry << " score " << result.score << " preview " << result.preview << std::endl;
    }
}

} // namespace PerformAction