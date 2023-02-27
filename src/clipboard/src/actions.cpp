/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
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
#include "clipboard.hpp"
#include <regex>

namespace PerformAction {
void copyItem(const fs::path& f) {
    auto actuallyCopyItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
        if (fs::is_directory(f)) {
            auto target = f.filename().empty() ? f.parent_path().filename() : f.filename();
            fs::create_directories(path.main / target);
            fs::copy(f, path.main / target, copying.opts);
            successes.directories++;
        } else {
            fs::copy(f, path.main / f.filename(), use_regular_copy ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
            successes.files++;
        }
        if (action == Action::Cut) writeToFile(path.original_files, fs::absolute(f).string() + "\n", true);
    };
    try {
        actuallyCopyItem();
    } catch (const fs::filesystem_error& e) {
        if (!copying.use_safe_copy && e.code() == std::errc::cross_device_link) {
            try {
                actuallyCopyItem(true);
            } catch (const fs::filesystem_error& e) {
                copying.failedItems.emplace_back(f.string(), e.code());
            }
        } else {
            copying.failedItems.emplace_back(f.string(), e.code());
        }
    }
}

void copy() {
    for (const auto& f : copying.items)
        copyItem(f);
}

void copyText() {
    copying.buffer = copying.items.at(0).string();
    writeToFile(path.data, copying.buffer);

    if (!output_silent) {
        printf(replaceColors("[success]✅ %s text \"[bold]%s[blank][success]\"[blank]\n").data(), did_action[action].data(), copying.buffer.data());
    }

    if (action == Action::Cut) writeToFile(path.original_files, path.data.string());
    successes.bytes = 0; // temporarily disable the bytes success message
}

void paste() {
    for (const auto& f : fs::directory_iterator(path.main)) {
        auto pasteItem = [&](const bool use_regular_copy = copying.use_safe_copy) {
            if (fs::exists(fs::current_path() / f.path().filename()) && fs::equivalent(f, fs::current_path() / f.path().filename())) {
                if (fs::is_directory(f))
                    successes.directories++;
                else
                    successes.files++;
                return;
            }
            if (fs::is_directory(f)) {
                fs::copy(f, fs::current_path() / f.path().filename(), copying.opts);
                successes.directories++;
            } else {
                fs::copy(f, fs::current_path() / f.path().filename(), use_regular_copy ? copying.opts : copying.opts | fs::copy_options::create_hard_links);
                successes.files++;
            }
        };
        try {
            if (fs::exists(fs::current_path() / f.path().filename())) {
                switch (copying.policy) {
                case CopyPolicy::SkipAll:
                    break;
                case CopyPolicy::ReplaceAll:
                    pasteItem();
                    break;
                default:
                    stopIndicator();
                    copying.policy = userDecision(f.path().filename().string());
                    startIndicator();
                    if (copying.policy == CopyPolicy::ReplaceOnce || copying.policy == CopyPolicy::ReplaceAll) {
                        pasteItem();
                    }
                    break;
                }
            } else {
                pasteItem();
            }
        } catch (const fs::filesystem_error& e) {
            if (!copying.use_safe_copy) {
                try {
                    pasteItem(true);
                } catch (const fs::filesystem_error& e) {
                    copying.failedItems.emplace_back(f.path().filename().string(), e.code());
                }
            } else {
                copying.failedItems.emplace_back(f.path().filename().string(), e.code());
            }
        }
    }
    removeOldFiles();
}

void pipeIn() {
    copying.buffer = pipedInContent();
    writeToFile(path.data, copying.buffer);
    if (action == Action::Cut) writeToFile(path.original_files, path.data.string());
}

void pipeOut() {
    for (const auto& entry : fs::recursive_directory_iterator(path.main)) {
        std::string content(fileContents(entry.path()));
#if !defined(_WIN32) && !defined(_WIN64)
        int len = write(fileno(stdout), content.data(), content.size());
        if (len < 0) throw std::runtime_error("write() failed");
#elif defined(_WIN32) || defined(_WIN64)
        fwrite(content.data(), sizeof(char), content.size(), stdout);
#endif
        fflush(stdout);
        successes.bytes += content.size();
    }
    removeOldFiles();
}

void clear() {
    if (fs::is_empty(path.main)) printf("%s", no_clipboard_contents_message().data());
    clearTempDirectory(true);
}

void show() {
    stopIndicator();
    if (fs::is_directory(path.main) && !fs::is_empty(path.main)) {
        TerminalSize termSpaceRemaining(getTerminalSize());
        if (fs::is_regular_file(path.data)) {
            std::string content(fileContents(path.data));
            content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
            printf(clipboard_text_contents_message().data(), std::min(static_cast<size_t>(250), content.size()), clipboard_name.data());
            printf(replaceColors("[bold][info]%s\n[blank]").data(), content.substr(0, 250).data());
            if (content.size() > 250) {
                printf(and_more_items_message().data(), content.size() - 250);
            }
            return;
        }
        size_t total_items = 0;

        for (auto dummy : fs::directory_iterator(path.main))
            total_items++;

        size_t rowsAvailable = termSpaceRemaining.accountRowsFor(clipboard_item_many_contents_message().length());
        rowsAvailable -= 3;
        printf(total_items > rowsAvailable ? clipboard_item_too_many_contents_message().data() : clipboard_item_many_contents_message().data(),
               std::min(rowsAvailable, total_items),
               clipboard_name.data());
        auto it = fs::directory_iterator(path.main);
        for (size_t i = 0; i < std::min(rowsAvailable, total_items); i++) {

            printf(replaceColors("[info]▏ [bold][help]%s[blank]\n").data(), it->path().filename().string().data());

            if (i == rowsAvailable - 1 && total_items > rowsAvailable) printf(and_more_items_message().data(), total_items - rowsAvailable);

            it++;
        }
    } else {
        printf(no_clipboard_contents_message().data(), actions[Action::Cut].data(), actions[Action::Copy].data(), actions[Action::Paste].data(), actions[Action::Copy].data());
    }
}

void edit() {}

void addFiles() {
    if (fs::is_regular_file(path.data)) {
        fprintf(stderr,
                "%s",
                replaceColors("[error]❌ You can't add items to text. [blank][help]Try copying text first, or add "
                              "text instead.[blank]\n")
                        .data());
        return;
    }
    for (const auto& f : copying.items)
        copyItem(f);
}

void addData() {
    if (fs::is_regular_file(path.data)) {
        std::string content(pipedInContent());
        successes.bytes += content.size();
        writeToFile(path.data, content, true);
    } else if (!fs::is_empty(path.main)) {
        fprintf(stderr,
                "%s",
                replaceColors("[error]❌ You can't add text to items. [blank][help]Try copying text first, or add a "
                              "file instead.[blank]\n")
                        .data());
    } else {
        pipeIn();
    }
}

void addText() {
    writeToFile(path.data, copying.items.at(0).string(), true);
    successes.bytes += copying.items.at(0).string().size();
}

void removeFiles() {
    if (fs::is_regular_file(path.data)) {
        fprintf(stderr, "%s", replaceColors("[error]❌ You can't remove items from text. [blank][help]Try copying files first, or remove text instead.[blank]\n").data());
        return;
    }
    for (const auto& item : copying.items) {
        try {
            if (fs::is_directory(path.main / item)) {
                fs::remove_all(path.main / item);
                successes.directories++;
            } else if (fs::is_regular_file(path.main / item)) {
                fs::remove(path.main / item);
                successes.files++;
            } else {
                copying.failedItems.emplace_back(item.string(), std::make_error_code(std::errc::no_such_file_or_directory));
            }
        } catch (const fs::filesystem_error& e) {
            copying.failedItems.emplace_back(item.string(), e.code());
        }
    }
}

void removeRegex() {
    std::regex regex(io_type == IOType::Text ? copying.items.at(0).string() : pipedInContent());
    if (fs::is_regular_file(path.data)) {
        std::string content(fileContents(path.data));
        size_t oldLength = content.size();
        content = std::regex_replace(content, regex, "");
        successes.bytes += oldLength - content.size();
        if (oldLength != content.size())
            writeToFile(path.data, content);
        else
            fprintf(stderr,
                    "%s",
                    replaceColors(
                            "[error]❌ Clipboard couldn't match your pattern against anything. [blank][help]Try using a different pattern instead or check what's stored.[blank]\n"
                    )
                            .data());
    } else {
        for (const auto& entry : fs::directory_iterator(path.main)) {
            if (std::regex_match(entry.path().filename().string(), regex)) {
                try {
                    if (fs::is_directory(entry.path())) {
                        fs::remove_all(entry.path());
                        successes.directories++;
                    } else {
                        fs::remove(entry.path());
                        successes.files++;
                    }
                } catch (const fs::filesystem_error& e) {
                    copying.failedItems.emplace_back(entry.path().filename().string(), e.code());
                }
            }
        }
        if (successes.directories == 0 && successes.files == 0)
            fprintf(stderr,
                    "%s",
                    replaceColors(
                            "[error]❌ Clipboard couldn't match your pattern against anything. [blank][help]Try using a different pattern instead or check what's stored.[blank]\n"
                    )
                            .data());
    }
}

void noteText() {
    if (copying.items.size() == 1) {
        if (copying.items.at(0).string() == "") {
            fs::remove(path.notes);
            fprintf(stdout, "%s", replaceColors("[success]✅ Removed note\n").data());
        } else {
            writeToFile(path.notes, copying.items.at(0).string());
            fprintf(stdout, replaceColors("[success]✅ Saved note \"%s\"\n").data(), copying.items.at(0).string().data());
        }
    } else if (copying.items.empty()) {
        if (fs::is_regular_file(path.notes)) {
            std::string content(fileContents(path.notes));
            fprintf(stdout, replaceColors("[info]• Note for this clipboard: %s\n").data(), content.data());
        } else {
            fprintf(stderr, "%s", replaceColors("[info]• There is no note for this clipboard.[blank]\n").data());
        }
    } else {
        fprintf(stderr, "%s", replaceColors("[error]❌ You can't add multiple items to a note. [blank][help]Try providing a single piece of text instead.[blank]\n").data());
    }
}

void swap() {}
} // namespace PerformAction