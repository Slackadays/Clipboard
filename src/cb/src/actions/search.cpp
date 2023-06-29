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

namespace PerformAction {

struct Result {
    std::string preview;
    std::string clipboard;
    unsigned long entry = 0;
    unsigned long score = 0;
    size_t hash;
};

void displaySearchResults(const std::vector<Result>& results) {
    auto available = thisTerminalSize();

    auto longestClipboardLength = (*std::max_element(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.clipboard.size() < b.clipboard.size(); })).clipboard.size();

    auto longestEntryLength = numberLength((*std::max_element(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.entry < b.entry; })).entry);

    stopIndicator();

    fprintf(stderr, "%s", formatColors("[info]┏━━[inverse] ").data());
    Message search_result_message = "[info][bold]Your search results[nobold]";
    fprintf(stderr, "%s", search_result_message().data());
    fprintf(stderr, "%s", formatColors(" [noinverse]━").data());
    auto usedSpace = (columnLength(search_result_message) - 2) + 9;
    if (usedSpace > available.columns) available.columns = usedSpace;
    int columns = available.columns - usedSpace;
    fprintf(stderr, "%s%s", repeatString("━", columns).data(), formatColors("┓[blank]\n").data());

    for (const auto& result : results) {
        fprintf(stderr,
                formatColors("[info]\033[%ldG┃\r┃ [bold]%*s%s[nobold]│ [bold]%*s%lu[nobold]│ [blank]").data(),
                available.columns,
                longestClipboardLength - result.clipboard.length(),
                "",
                result.clipboard.data(),
                longestEntryLength - numberLength(result.entry),
                "",
                result.entry);
        std::string preview = result.preview;
        std::erase(preview, '\n');
        auto widthRemaining = available.columns - (longestClipboardLength + longestEntryLength + 7);
        if (preview.length() > widthRemaining) {
            preview = preview.substr(0, widthRemaining - (preview.length() - columnLength(preview)));
        }
        fprintf(stderr, formatColors("[help]%s[blank]\n").data(), preview.data());
    }

    fprintf(stderr, "%s", formatColors("[info]┗━━▌").data());
    Message search_legend_message = "[bold]Clipboard[nobold]│ [bold]Entry[nobold]│[help] Result[info]";
    int cols = available.columns - (columnLength(search_legend_message) + 6);
    std::string bar2 = "▐" + repeatString("━", cols);
    fprintf(stderr, "%s", (search_legend_message() + bar2).data());
    fprintf(stderr, "%s", formatColors("┛[blank]\n").data());
}

void displaySearchJSON(const std::vector<Result>& results) {
    printf("[\n");
    for (size_t i = results.size() - 1; i > 0; i--) {
        printf("    {\n");
        printf("        \"clipboard\": \"%s\",\n", results.at(i).clipboard.data());
        printf("        \"entry\": %lu,\n", results.at(i).entry);
        printf("        \"preview\": \"%s\",\n", JSONescape(results.at(i).preview).data());
        printf("        \"score\": %lu\n", results.at(i).score);
        printf("    }%s\n", i == 1 ? "" : ",");
    }
    printf("]\n");
}

void searchInternal(std::function<void(const std::vector<Result>&)> nextStep) {
    if (copying.items.empty())
        error_exit(
                "%s",
                formatColors(
                        "[error][inverse] ✘ [noinverse] You need to enter something to search for. [help]⬤ Try entering a search term after the action, like [bold]cb search Foobar[nobold].[blank]\n"
                )
        );

    std::vector<std::string> queries;
    std::transform(copying.items.begin(), copying.items.end(), std::back_inserter(queries), [](const auto& item) { return item.string(); });

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
            } else if (std::smatch sm; std::regex_search(content, sm, std::regex(query))) { // then do a regex search of the content for the query
                result.score = 700;
                result.preview = content.substr(0, sm.position(0)) + "\033[1m" + sm.str(0) + "\033[22m" + content.substr(sm.position(0) + sm.length(0));
            } else if (size_t distance; content.size() < 1000 && (distance = levenshteinDistance(content, query)) < 25) { // then do a fuzzy search of the content for the query
                result.score = 600 - (distance * 20);
                result.preview = "\033[1m" + content + "\033[22m";
            }
        } catch (const std::regex_error& e) {
            error_exit(
                    formatColors("[error][inverse] ✘ [noinverse] CB couldn't process your query as regex. (Specific error: %s) [help]⬤ Try entering a valid regex instead, like [bold]cb search "
                                 "\"Foobar.*\"[nobold].[blank]\n"),
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
        for (auto entry = 0; entry < clipboard.entryIndex.size(); entry++) {
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

    if (results.empty())
        error_exit("%s", formatColors("[error][inverse] ✘ [noinverse] CB couldn't find anything matching your query.[blank] [help]⬤ Try searching for something else instead.[blank]\n"));

    std::sort(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.hash < two.hash; });
    results.erase(std::unique(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.hash == two.hash; }), results.end());

    std::sort(results.begin(), results.end(), [](const Result& one, const Result& two) { return one.score < two.score; });

    nextStep(results);
}

void search() {
    searchInternal(displaySearchResults);
}

void searchJSON() {
    searchInternal(displaySearchJSON);
}

} // namespace PerformAction
