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
#include <latch>

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <format>
#include <io.h>
#define write _write
#define STDERR_FILENO 2
#endif

#if defined(__linuxx__)
#include <liburing.h>
int SQEsSubmitted = 0;
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#if (defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__posix__)) && !defined(__OpenBSD__)
#define USE_AIO 1
#include <aio.h>
#endif

namespace PerformAction {

void moveHistory() {
    size_t successful_entries = 0;
    std::vector<fs::path> absoluteEntryPaths;
    for (const auto& entry : copying.items) {
        try {
            unsigned long entryNum = std::stoul(entry.string());
            absoluteEntryPaths.emplace_back(path.entryPathFor(entryNum));
        } catch (fs::filesystem_error& e) {
            copying.failedItems.emplace_back(entry.string(), e.code());
            continue;
        } catch (...) {}
    }
    for (const auto& entry : absoluteEntryPaths) {
        path.makeNewEntry();
        fs::rename(entry, path.data);
        successful_entries++;
    }
    stopIndicator();
    fprintf(stderr, formatColors("[success][inverse] ✔ [noinverse] Queued up [bold]%lu[blank][success] entries[blank]\n").data(), successful_entries);
    if (clipboard_name == constants.default_clipboard_name) updateExternalClipboards(true);
}

void history() {
    if (!copying.items.empty()) {
        moveHistory();
        return;
    }
    std::vector<std::string> dates(path.entryIndex.size());

    std::atomic<size_t> atomicLongestDateLength = 0;

    auto now = std::chrono::system_clock::now();

    auto totalThreads = suitableThreadAmount();
    if (path.entryIndex.size() < totalThreads) totalThreads = path.entryIndex.size();

    auto entriesPerThread = path.entryIndex.size() / totalThreads;

    std::vector<std::thread> threads(totalThreads);

    auto dateWorker = [&](const unsigned long& start, const unsigned long& end) {
        struct stat dateInfo;
        std::string agoMessage;
        agoMessage.reserve(16);

        for (auto entry = start; entry < end; entry++) {
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
            stat(path.entryPathFor(entry).string().data(), &dateInfo);
            auto timeSince = now - std::chrono::system_clock::from_time_t(dateInfo.st_mtime);
            // format time like 1y 2d 3h 4m 5s
            auto years = std::chrono::duration_cast<std::chrono::years>(timeSince);
            auto days = std::chrono::duration_cast<std::chrono::days>(timeSince - years);
            auto hours = std::chrono::duration_cast<std::chrono::hours>(timeSince - days);
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timeSince - days - hours);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeSince - days - hours - minutes);
            if (years.count() > 0) agoMessage += std::to_string(years.count()) + "y ";
            if (days.count() > 0) agoMessage += std::to_string(days.count()) + "d ";
            if (hours.count() > 0) agoMessage += std::to_string(hours.count()) + "h ";
            if (minutes.count() > 0) agoMessage += std::to_string(minutes.count()) + "m ";
            agoMessage += std::to_string(seconds.count()) + "s";
            dates[entry] = agoMessage;

            if (agoMessage.length() > atomicLongestDateLength.load(std::memory_order_relaxed)) atomicLongestDateLength.store(agoMessage.length(), std::memory_order_relaxed);
            agoMessage.clear();
#else
            dates[entry] = "n/a";
            atomicLongestDateLength.store(3, std::memory_order_relaxed);
#endif
        }
    };

    for (size_t thread = 0; thread < totalThreads; thread++) {
        auto start = thread * entriesPerThread;
        auto end = start + entriesPerThread;
        if (thread == totalThreads - 1) end = path.entryIndex.size();
        threads[thread] = std::thread(dateWorker, start, end);
    }

    // for (auto& thread : threads)
    //     thread.join();

    // std::cout << "processing dates took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count() << "ms" << std::endl;
    // exit(0);

#if defined(__linuxx__)
    io_uring ring;
    io_uring_queue_init(128, &ring, IORING_SETUP_SQPOLL);
#endif

#if defined(USE_AIO)
    std::vector<std::shared_ptr<aiocb>> batchedAIOs;
#endif

#if defined(__linux__) || defined(USE_AIO)
    constexpr size_t batchInterval = 1024 * 1024;
#else
    constexpr size_t batchInterval = 65536;
#endif

    auto longestEntryLength = numberLength(path.entryIndex.size() - 1);

    stopIndicator();
    auto available = thisTerminalSize();
    fprintf(stderr, "%s", formatColors("[info]┏━━[inverse] ").data());
    Message clipboard_history_message = "[bold]Entry history for clipboard [help] %s[nobold]";
    fprintf(stderr, clipboard_history_message().data(), clipboard_name.data());
    fprintf(stderr, "%s", formatColors(" [noinverse][info]━").data());
    auto usedSpace = (columnLength(clipboard_history_message) - 2) + clipboard_name.length() + 7;
    if (usedSpace > available.columns) available.columns = usedSpace;
    int columns = available.columns - usedSpace;
    fprintf(stderr, "%s%s", repeatString("━", columns).data(), formatColors("┓[blank]").data());

    std::string batchedMessage;
    // reserve enough contiguous memory for the entire batch, where the size is number of entries * line length, plus extra formatting characters
    // this prevents reallocations and thus helps prevent invalidation of the data pointer
    batchedMessage.reserve(path.entryIndex.size() * (available.columns + 64));
    size_t offset = 0;

    const std::array preformattedMessageParts = {
            formatColors("\n[info]\033[" + std::to_string(available.columns) + "G┃\r┃ [bold]"),
            formatColors("[nobold]│ [bold]"),
            formatColors("[nobold]│[help] ")};

    for (auto& thread : threads)
        thread.join();

    size_t longestDateLength = atomicLongestDateLength.load(std::memory_order_relaxed);

    for (long entry = path.entryIndex.size() - 1; entry >= 0; entry--) {
        path.setEntry(entry);

        if (batchedMessage.size() - offset > batchInterval) {
#if defined(__linuxx__)
            auto sqe = io_uring_get_sqe(&ring);
            auto rawByteAtOffset = batchedMessage.data() + offset;
            io_uring_prep_write(sqe, STDERR_FILENO, rawByteAtOffset, batchedMessage.size() - offset, 0);

            SQEsSubmitted += io_uring_submit(&ring);
            offset = batchedMessage.size();
#elif defined(USE_AIO)
            auto aio = std::make_shared<aiocb>();
            auto rawByteAtOffset = batchedMessage.data() + offset;
            aio->aio_fildes = STDERR_FILENO;
            aio->aio_buf = static_cast<void*>(rawByteAtOffset);
            aio->aio_nbytes = batchedMessage.size() - offset;
            aio_write(aio.get());

            offset = batchedMessage.size();
            batchedAIOs.emplace_back(aio);
#else
            auto ret = write(STDERR_FILENO, batchedMessage.data(), batchedMessage.size());
            batchedMessage.clear();
#endif
        }

        int widthRemaining = available.columns - (numberLength(entry) + longestEntryLength + longestDateLength + 7);

        batchedMessage += preformattedMessageParts[0] + std::string(longestEntryLength - numberLength(entry), ' ') + std::to_string(entry) + preformattedMessageParts[1]
                          + std::string(longestDateLength - dates.at(entry).length(), ' ') + dates.at(entry) + preformattedMessageParts[2];

        if (auto temp(fileContents(path.data.raw)); temp.has_value()) {
            auto content = std::move(temp.value());
            if (content.empty()) continue; // don't use holdsRawDataInCurrentEntry because we are reading anyway, so we can save on a syscall
            if (auto MIMEtype = inferMIMEType(content); MIMEtype.has_value())
                content = "\033[7m\033[1m " + std::string(MIMEtype.value()) + ", " + formatBytes(content.length()) + " \033[22m\033[27m";
            else
                content = makeControlCharactersVisible(content, available.columns);
            batchedMessage += content.substr(0, widthRemaining);
            continue;
        }

        for (bool first = true; const auto& entry : fs::directory_iterator(path.data)) {
            auto filename = entry.path().filename().string();
            if (filename == constants.data_file_name && entry.file_size() == 0) continue;

            if (widthRemaining <= 0) break;

            if (!first) {
                if (filename.length() <= widthRemaining - 2) {
                    batchedMessage += ", ";
                    widthRemaining -= 2;
                }
            }

            if (filename.length() <= widthRemaining) {
                if (entry.is_directory())
                    batchedMessage += "\033[4m" + filename + "\033[24m";
                else
                    batchedMessage += "\033[1m" + filename + "\033[22m";
                widthRemaining -= filename.length();
                first = false;
            }
        }
    }

#if defined(__linuxx__)
    auto sqe = io_uring_get_sqe(&ring);
    auto rawByteAtOffset = batchedMessage.data() + offset;
    io_uring_prep_write(sqe, STDERR_FILENO, rawByteAtOffset, batchedMessage.size() - offset, 0);

    // block until all writes are done
    io_uring_submit_and_wait(&ring, SQEsSubmitted + 1);
    io_uring_queue_exit(&ring);
#elif defined(USE_AIO)
    auto rawByteAtOffset = batchedMessage.data() + offset;
    auto aio = std::make_shared<aiocb>();
    aio->aio_fildes = STDERR_FILENO;
    aio->aio_buf = static_cast<void*>(rawByteAtOffset);
    aio->aio_nbytes = batchedMessage.size() - offset;
    aio_write(aio.get());
    batchedAIOs.emplace_back(aio);

    for (const auto& aio : batchedAIOs) {
        const std::array<const aiocb*, 1> aio_list = {aio.get()};
        aio_suspend(aio_list.data(), aio_list.size(), nullptr);
    }
#else
    auto ret = write(STDERR_FILENO, batchedMessage.data(), batchedMessage.size());
#endif

    fputs(formatColors("[info]\n┗━━▌").data(), stderr);
    Message status_legend_message = "[help]Text, \033[1mFiles\033[22m, \033[4mDirectories\033[24m, \033[7m\033[1m Data \033[22m\033[27m[info]";
    usedSpace = columnLength(status_legend_message) + 6;
    if (usedSpace > available.columns) available.columns = usedSpace;
    auto cols = available.columns - usedSpace;
    std::string bar2 = "▐" + repeatString("━", cols);
    fputs((status_legend_message() + bar2).data(), stderr);
    fputs(formatColors("┛[blank]\n").data(), stderr);
}

void historyJSON() {
    printf("{\n");
    for (unsigned long entry = 0; entry < path.entryIndex.size(); entry++) {
        path.setEntry(entry);
        printf("    \"%lu\": {\n", entry);
        printf("        \"date\": %zu,\n", static_cast<size_t>(fs::last_write_time(path.data).time_since_epoch().count()));
        printf("        \"content\": ");
        if (path.holdsRawDataInCurrentEntry()) {
            std::string content(fileContents(path.data.raw).value());
            if (auto type = inferMIMEType(content); type.has_value()) {
                printf("{\n");
                printf("            \"dataType\": \"%s\",\n", type.value().data());
                printf("            \"dataSize\": %zd,\n", content.length());
                printf("            \"path\": \"%s\"\n", JSONescape(path.data.raw.string()).data());
                printf("        }");
            } else {
                printf("\"%s\"", JSONescape(content).data());
            }
        } else if (path.holdsDataInCurrentEntry()) {
            printf("[\n");
            std::vector<fs::path> itemsInPath(fs::directory_iterator(path.data), fs::directory_iterator());
            for (const auto& entry : itemsInPath) {
                printf("            {\n");
                printf("                \"filename\": \"%s\",\n", JSONescape(entry.filename().string()).data());
                printf("                \"path\": \"%s\",\n", JSONescape(entry.string()).data());
                printf("                \"isDirectory\": %s\n", fs::is_directory(entry) ? "true" : "false");
                printf("            }%s\n", entry == itemsInPath.back() ? "" : ",");
            }
            printf("\n        ]");
        } else {
            printf("null");
        }
        printf("\n    }%s\n", entry == path.entryIndex.size() - 1 ? "" : ",");
    }
    printf("}\n");
}

} // namespace PerformAction
