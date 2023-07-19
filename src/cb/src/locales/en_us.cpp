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

EnumArray<std::string_view, 20> actions = {"cut",    "copy", "paste", "clear",  "show",   "edit",    "add",    "remove", "note", "swap",
                                           "status", "info", "load",  "import", "export", "history", "ignore", "search", "undo", "redo"};

EnumArray<std::string_view, 20> action_shortcuts = {"ct", "cp", "p", "clr", "sh", "ed", "ad", "rm", "nt", "sw", "st", "in", "ld", "imp", "ex", "hs", "ig", "sr", "u", "r"};

EnumArray<std::string_view, 20> doing_action = {"Cutting",         "Copying",      "Pasting", "Clearing",  "Showing",   "Editing",         "Adding",   "Removing",  "Noting",  "Swapping",
                                                "Checking status", "Showing info", "Loading", "Importing", "Exporting", "Getting history", "Ignoring", "Searching", "Undoing", "Redoing"};

EnumArray<std::string_view, 20> did_action = {"Cut",         "Copied", "Pasted",   "Cleared",  "Showed",      "Edited",  "Added",    "Removed", "Noted", "Swapped", "Checked status",
                                              "Showed info", "Loaded", "Imported", "Exported", "Got history", "Ignored", "Searched", "Undid",   "Redid"};

EnumArray<std::string_view, 20> action_descriptions = {
        "Cut items into a clipboard.",
        "Copy items into a clipboard.",
        "Paste items from a clipboard.",
        "Clear a clipboard.",
        "Show the contents of a clipboard.",
        "Edit the contents of a clipboard.",
        "Add items to a clipboard.",
        "Remove items from a clipboard.",
        "Add a note to a clipboard.",
        "Swap the contents of multiple clipboards.",
        "Check the status of a clipboard.",
        "Show info about a clipboard.",
        "Load a clipboard into another clipboard.",
        "Import a clipboard from a file.",
        "Export a clipboard to a file.",
        "Show the history of a clipboard.",
        "Ignore types of content in a clipboard.",
        "Search for items in a clipboard.",
        "Placeholder: DO NOT USE",
        "Placeholder: DO NOT USE"};

Message help_message = "[info]┃ This is the Clipboard Project %s (commit %s), the cut, copy, and paste system for the command line.[blank]\n"
                       "[info][bold]┃ Examples[blank]\n"
                       "[progress]┃ cb ct Nuclear_Launch_Codes.txt contactsfolder[blank] [help](This cuts the following items into the "
                       "default clipboard, 0.)[blank]\n"
                       "[progress]┃ cb cp1 dogfood.conf[blank] [help](This copies the following items into clipboard 1.)[blank]\n"
                       "[progress]┃ cb p1[blank] [help](This pastes clipboard 1.)[blank]\n"
                       "[progress]┃ cb sh4[blank] [help](This shows the contents of clipboard 4.)[blank]\n"
                       "[progress]┃ cb clr[blank] [help](This clears the contents of the default clipboard.)[blank]\n"
                       "[info]┃ You can also choose which clipboard you want to use by adding a number to the end, or "
                       "[bold]_[nobold] to use a persistent clipboard.[blank]\n"
                       "[info][bold]┃ More Info[blank]\n"
                       "[info]┃ See the complete online documentation for CB at https://github.com/Slackadays/Clipboard.[blank]\n"
                       "[info]┃ Show this help screen anytime with [bold]cb -h[nobold], [bold]cb "
                       "--help[nobold], or[bold] cb help[nobold].\n"
                       "[info]┃ You can also get more help in our Discord server at [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                       "[info][bold]┃ All Actions Available[blank]\n"
                       "%s"
                       "[info]┃ Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                       "[info]┃ This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to "
                       "redistribute it under certain conditions.[blank]\n";
Message check_clipboard_status_message = "[info][bold]All your clipboards with content[nobold]";
Message clipboard_item_one_contents_message = "[info]┃ Here is the [bold]%i[nobold] item in clipboard [bold]%s[nobold]: [blank]\n";
Message clipboard_item_many_contents_message = "[inverse][bold][info] Here are the items in clipboard [bold][help] %s [nobold][noinverse]";
Message clipboard_text_contents_message = "[info]┃ Here are the first [bold]%i[nobold] bytes in clipboard [bold]%s[nobold]: [blank]";
Message no_clipboard_contents_message = "[info]┃ There is currently nothing in the clipboard.[blank]\n";
Message clipboard_action_prompt = "[help]┃ Add [bold]cut, copy, [nobold]or[bold] paste[nobold] to the end, "
                                  "like [bold]%s copy[nobold] to get started, or if you need help, try "
                                  "[bold]%s -h[nobold] to show the help screen.[blank]\n";
Message no_valid_action_message = "[error][inverse] ✘ [noinverse] You did not specify a valid action ([bold]\"%s\"[blank][error]), or you forgot "
                                  "to include one. [help]⬤ Try using or adding [bold]cut, copy, [nobold]or "
                                  "[bold]paste[nobold] instead, like [bold]%s copy,[nobold] or do the [bold]help[nobold] action for a complete list of them.[blank]\n";
Message no_valid_action_with_candidate_message = "[error][inverse] ✘ [noinverse] You did not specify a valid action ([bold]\"%s\"[blank][error]), or you forgot "
                                                 "to include one. [help]⬤ Did you mean [bold]%s %s%s[nobold] instead?[blank]\n";
Message choose_action_items_message = "[error][inverse] ✘ [noinverse] You need to choose something to %s.[help] ⬤ Try adding the items you want "
                                      "to %s to the end, like [bold]%s %s contacts.txt myprogram.cpp[blank]\n";
Message fix_redirection_action_message = "[error][inverse] ✘ [noinverse] You can't use the [bold]%s[blank][error] action with redirection here. [help]⬤ Try removing "
                                         "[bold]%s[nobold] or use [bold]%s[nobold] instead, like [bold]%s %s[nobold].\n";
Message redirection_no_items_message = "[error][inverse] ✘ [noinverse] You can't specify items when you use redirection. [help]⬤ Try removing "
                                       "the items that come after [bold]%s [action].\n";
Message paste_success_message = "[success][inverse] ✔ [noinverse] Pasted successfully[blank]\n";
Message clipboard_failed_one_message = "[error][inverse] ✘ [noinverse] CB couldn't %s this item:[blank]\n";
Message clipboard_failed_many_message = "[error][inverse] ✘ [noinverse] CB couldn't %s these items:[blank]\n";
Message and_more_fails_message = "[error][inverse] ✘ [noinverse] ...and [bold]%i[nobold] more.[blank]\n";
Message and_more_items_message = "[info]┃ ...and [bold]%i[nobold] more.[blank]\n";
Message fix_problem_message = "[help]⬤ See if you have the needed permissions, or\n"
                              "┃ try double-checking the spelling of the files or what directory you're in.[blank]\n";
Message not_enough_storage_message = "[error][inverse] ✘ [noinverse] There won't be enough storage available to %s everything (%gMB to "
                                     "paste, %gMB available). [help]⬤ Try double-checking what items you've "
                                     "selected or delete some files to free up space.[blank]\n";
Message item_already_exists_message = "[progress]⬤ The item [bold]%s[blank][progress] already exists here. Do you want to "
                                      "replace it? [help]Use [bold]all [nobold]to replace all existing, or "
                                      "[bold]skip[nobold] to replace nothing. [bold][(y)es/(n)o)/(a)ll/(s)kip] ";
Message bad_response_message = "[error][inverse] ✘ [noinverse] Sorry, that wasn't a valid choice. Try again: [bold][(y)es/(n)o)] ";
Message working_message = "\r[progress]⬤ %s... %s, %s elapsed %s    [blank]";
Message cancelled_message = "[success][inverse] ✔ [noinverse] Cancelled %s[blank]\n";
Message cancelled_with_progress_message = "[success][inverse] ✔ [noinverse] Cancelled %s (%s in progress)[blank]\n";
Message byte_success_message = "[success][inverse] ✔ [noinverse] %s %s[blank]\n";
Message one_item_success_message = "[success][inverse] ✔ [noinverse] %s one item[blank]\n";
Message many_files_success_message = "[success][inverse] ✔ [noinverse] %s %lu files[blank]\n";
Message many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu directories[blank]\n";
Message one_file_one_directory_success_message = "[success][inverse] ✔ [noinverse] %s one file and one directory[blank]\n";
Message one_file_many_directories_success_message = "[success][inverse] ✔ [noinverse] %s one file and %lu directories[blank]\n";
Message many_files_one_directory_success_message = "[success][inverse] ✔ [noinverse] %s %lu files and one directory[blank]\n";
Message many_files_many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu files and %lu directories[blank]\n";
Message one_clipboard_success_message = "[success][inverse] ✔ [noinverse] %s one clipboard[blank]\n";
Message many_clipboards_success_message = "[success][inverse] ✔ [noinverse] %s %lu clipboards[blank]\n";
Message clipboard_name_message = "[info][bold]Info for clipboard [help] %s[nobold]";
Message internal_error_message = "[error][inverse] ✘ [noinverse] Internal error: %s\n┃ This might be a bug, or you might be lacking "
                                 "permissions on this system.[blank]\n";