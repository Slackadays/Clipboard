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
        unsigned long entry = 0;
        unsigned long score = 0;
        size_t hash;
    };

    std::vector<Clipboard> targets;
    std::vector<Result> results;

    std::hash<std::string> hashString;
    std::hash<unsigned long> hashULong;

    if (all_option) {
        for (const auto& entry : fs::directory_iterator(global_path.temporary))
            if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsData()) targets.emplace_back(cb);
        for (const auto& entry : fs::directory_iterator(global_path.persistent))
            if (auto cb = Clipboard(entry.path().filename().string()); cb.holdsData()) targets.emplace_back(cb);
    } else
        targets.emplace_back(path);

    auto levenshteinDistance = [](const std::string_view& one, const std::string_view& two) -> unsigned long {
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

    // std::cerr << "distance between foo and fobobar is " << levenshteinDistance("foo", "fobobar") << std::endl;

    // exit(0);

    auto contentMatchRating = [&](const std::string& content, const std::string& query) -> std::optional<Result> {
        Result result;

        // check if the content matches the query
        try {
            if (content == query) {
                result.score = 1000;
                result.preview = "\033[1m" + content + "\033[22m";
            } else if (std::regex_match(content, std::regex(query))) { // then check if the content regex matches the query
                result.score = 800;
                result.preview = "\033[1m" + content + "\033[22m";
            } else if (size_t distance; content.size() < 1000 && (distance = levenshteinDistance(content, query)) < 25) { // then do a fuzzy search of the content for the query
                result.score = 700 - (distance * 20);
                result.preview = "\033[1m" + content + "\033[22m";
            } else if (std::smatch sm; std::regex_search(content, sm, std::regex(query))) { // then do a regex search of the content for the query
                result.score = 600;
                result.preview = content.substr(0, sm.position(0)) + "\033[1m" + sm.str(0) + "\033[22m" + content.substr(sm.position(0) + sm.length(0));
            }
        } catch (const std::regex_error& e) {
            error_exit(
                    formatMessage("[error]❌ CB couldn't process your query as regex. (Specific error: %s) 💡 [help]Try entering a valid regex instead, like [bold]cb search "
                                  "\"Foobar.*\"[blank][help].[blank]\n"),
                    std::string(e.what())
            );
        }

        if (result.score > 0) return result;

        return std::nullopt;
    };

    auto combineHashes = [](size_t one, size_t two) -> size_t {
        return one ^ (two + 0x9e3779b9 + (one << 6) + (one >> 2)); // from Boost
    };

    for (auto& clipboard : targets) {
        for (const auto& entry : clipboard.entryIndex) {
            auto adjustScoreByEntryPosition = [&](Result& result) {
                float multiplier = 1.0f - (static_cast<float>(entry) / (20.0f * static_cast<float>(clipboard.entryIndex.size())));
                float newScore = static_cast<float>(result.score) * multiplier;
                result.score = static_cast<unsigned long>(newScore);
            };
            clipboard.setEntry(entry);
            if (clipboard.holdsRawDataInCurrentEntry()) {
                for (const auto& query : queries) {
                    if (auto rating = contentMatchRating(fileContents(clipboard.data.raw), query); rating.has_value()) {
                        rating->clipboard = clipboard.name();
                        rating->entry = entry;
                        rating->hash = combineHashes(hashString(clipboard.name()), hashULong(entry));
                        adjustScoreByEntryPosition(rating.value());
                        results.emplace_back(rating.value());
                    }
                }
            } else {
                for (const auto& item : fs::directory_iterator(clipboard.data)) {
                    for (const auto& query : queries) {
                        if (auto rating = contentMatchRating(item.path().filename().string(), query); rating.has_value()) {
                            rating->clipboard = clipboard.name();
                            rating->entry = entry;
                            rating->hash = combineHashes(hashString(clipboard.name()), hashULong(entry));
                            adjustScoreByEntryPosition(rating.value());
                            results.emplace_back(rating.value());
                        }
                    }
                }
            }
        }
    }

    if (results.empty()) error_exit("%s", formatMessage("[error]❌ CB couldn't find anything matching your query.[blank] 💡 [help]Try searching for something else instead.[blank]\n"));

    std::sort(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.hash < two.hash; });
    results.erase(std::unique(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.hash == two.hash; }), results.end());

    std::sort(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.score < two.score; });

    auto available = thisTerminalSize();

    auto longestClipboardLength = (*std::max_element(targets.begin(), targets.end(), [](const auto& a, const auto& b) { return a.name().size() < b.name().size(); })).name().size();

    auto longestEntryLength = numberLength((*std::max_element(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.entry < b.entry; })).entry);

    stopIndicator();

    fprintf(stderr, "%s", formatMessage("[info]┍━┫ ").data());
    Message search_result_message = "[info]Your search results[blank]";
    fprintf(stderr, "%s", search_result_message().data());
    fprintf(stderr, "%s", formatMessage("[info] ┣").data());
    auto usedSpace = (search_result_message.rawLength() - 2) + 9;
    if (usedSpace > available.columns) available.columns = usedSpace;
    int columns = available.columns - usedSpace;
    std::string bar1;
    for (int i = 0; i < columns; i++)
        bar1 += "━";
    fprintf(stderr, "%s%s", bar1.data(), formatMessage("┑[blank]\n").data());

    for (const auto& result : results) {
        fprintf(stderr,
                formatMessage("[info]\033[%ldG│\r│ [bold]%*s%s[blank][info]│ [bold]%*s%lu[blank][info]│ [blank]").data(),
                available.columns,
                longestClipboardLength - result.clipboard.length(),
                "",
                result.clipboard.data(),
                longestEntryLength - numberLength(result.entry),
                "",
                result.entry);
        std::string preview = result.preview;
        std::erase(preview, '\n');
        int widthRemaining = available.columns - (longestClipboardLength + longestEntryLength);
        if (preview.length() > widthRemaining) {
            preview = preview.substr(0, widthRemaining - 3);
            preview += "...";
        }
        fprintf(stderr, formatMessage("[help]%s[blank]\n").data(), preview.data());
    }

    fprintf(stderr, "%s", formatMessage("[info]┕━┫ ").data());
    Message search_legend_message = "[bold]Clipboard[blank][info]│ [bold]Entry[blank][info]│[blank][help] Result[info]";
    int cols = available.columns - (search_legend_message.rawLength() + 3);
    std::string bar2 = " ┣";
    for (int i = 0; i < cols; i++)
        bar2 += "━";
    fprintf(stderr, "%s", (search_legend_message() + bar2).data());
    fprintf(stderr, "%s", formatMessage("┙[blank]\n").data());
}

} // namespace PerformAction
