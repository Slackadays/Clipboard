void setLanguageES() {
colors = {
    {"red", "\033[38;5;196m"},
    {"green", "\033[38;5;40m"},
    {"yellow", "\033[38;5;214m"},
    {"blue", "\033[38;5;51m"},
    {"orange", "\033[38;5;208m"},
    {"pink", "\033[38;5;219m"},
    {"bold", "\033[1m"},
    {"blank", "\033[0m"}
};

actions = {
    {Action::Cut, "cut"},
    {Action::Copy, "copy"},
    {Action::Paste, "paste"},
    {Action::PipeIn, "pipe in"},
    {Action::PipeOut, "pipe out"},
};

doing_action = {
    {Action::Cut, "Cutting"},
    {Action::Copy, "Copying"},
    {Action::Paste, "Pasting"},
    {Action::PipeIn, "Piping in"},
    {Action::PipeOut, "Piping out"}
};

did_action = {
    {Action::Cut, "Cut"},
    {Action::Copy, "Copied"},
    {Action::Paste, "Pasted"},
    {Action::PipeIn, "Piped in"},
    {Action::PipeOut, "Piped out"}
};

help_message = "{blue}▏This is Clipboard %s, the {cut}, {copy}, and {paste} system for the command line.{blank}\n"
                "{blue}{bold}▏How To Use{blank}\n"
                "{orange}▏clipboard cut (item) [items]{blank}\n"
                "{orange}▏clipboard copy (item) [items]{blank}\n"
                "{orange}▏clipboard paste{blank}\n"
                "{blue}▏You can substitute \"cb\" for \"clipboard\" to save time.{blank}\n"
                "{blue}{bold}▏Examples{blank}\n"
                "{orange}▏clipboard copy dogfood.conf{blank}\n"
                "{orange}▏cb cut Nuclear_Launch_Codes.txt contactsfolder{blank}\n"
                "{orange}▏cb paste{blank}\n"
                "{blue}▏You can show this help screen anytime with {bold}clipboard -h{blank}{blue}, {bold}clipboard --help{blank}{blue}, or{bold} clipboard help{blank}{blue}.\n"
                "{blue}▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.{blank}\n"
                "{blue}▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.{blank}\n";
no_valid_action_message = "{red}╳ You did not specify a valid action, or you forgot to include one. {pink}Try using or adding {bold}cut, copy, or paste{blank}{pink} instead, like {bold}clipboard copy.{blank}\n";
no_action_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}%s, %s, or %s{blank}{pink} to the end, like {bold}clipboard %s{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
paste_success_message = "{green}√ Pasted successfully{blank}\n";
paste_fail_message = "{red}╳ Failed to paste{blank}\n";
clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items.{blank}\n";
and_more_message = "{red}▏ ...and %i more.{blank}\n";
fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                    "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
working_message = "{yellow}• %s...{blank}\r";
pipe_success_message = "{green}√ %s %i bytes{blank}\n";
one_item_success_message = "{green}√ %s %s{blank}\n";
multiple_files_success_message = "{green}√ %s %i files{blank}\n";
multiple_directories_success_message = "{green}√ %s %i directories{blank}\n";
multiple_files_directories_success_message = "{green}√ %s %i files and %i directories{blank}\n";
internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug.{blank}\n";
}

void setLanguagePT() {
colors = {
    {"red", "\033[38;5;196m"},
    {"green", "\033[38;5;40m"},
    {"yellow", "\033[38;5;214m"},
    {"blue", "\033[38;5;51m"},
    {"orange", "\033[38;5;208m"},
    {"pink", "\033[38;5;219m"},
    {"bold", "\033[1m"},
    {"blank", "\033[0m"}
};

actions = {
    {Action::Cut, "cut"},
    {Action::Copy, "copy"},
    {Action::Paste, "paste"},
    {Action::PipeIn, "pipe in"},
    {Action::PipeOut, "pipe out"},
};

doing_action = {
    {Action::Cut, "Cutting"},
    {Action::Copy, "Copying"},
    {Action::Paste, "Pasting"},
    {Action::PipeIn, "Piping in"},
    {Action::PipeOut, "Piping out"}
};

did_action = {
    {Action::Cut, "Cut"},
    {Action::Copy, "Copied"},
    {Action::Paste, "Pasted"},
    {Action::PipeIn, "Piped in"},
    {Action::PipeOut, "Piped out"}
};

help_message = "{blue}▏This is Clipboard %s, the {cut}, {copy}, and {paste} system for the command line.{blank}\n"
                "{blue}{bold}▏How To Use{blank}\n"
                "{orange}▏clipboard cut (item) [items]{blank}\n"
                "{orange}▏clipboard copy (item) [items]{blank}\n"
                "{orange}▏clipboard paste{blank}\n"
                "{blue}▏You can substitute \"cb\" for \"clipboard\" to save time.{blank}\n"
                "{blue}{bold}▏Examples{blank}\n"
                "{orange}▏clipboard copy dogfood.conf{blank}\n"
                "{orange}▏cb cut Nuclear_Launch_Codes.txt contactsfolder{blank}\n"
                "{orange}▏cb paste{blank}\n"
                "{blue}▏You can show this help screen anytime with {bold}clipboard -h{blank}{blue}, {bold}clipboard --help{blank}{blue}, or{bold} clipboard help{blank}{blue}.\n"
                "{blue}▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.{blank}\n"
                "{blue}▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.{blank}\n";
no_valid_action_message = "{red}╳ You did not specify a valid action, or you forgot to include one. {pink}Try using or adding {bold}cut, copy, or paste{blank}{pink} instead, like {bold}clipboard copy.{blank}\n";
no_action_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}%s, %s, or %s{blank}{pink} to the end, like {bold}clipboard %s{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
paste_success_message = "{green}√ Pasted successfully{blank}\n";
paste_fail_message = "{red}╳ Failed to paste{blank}\n";
clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items.{blank}\n";
and_more_message = "{red}▏ ...and %i more.{blank}\n";
fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                    "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
working_message = "{yellow}• %s...{blank}\r";
pipe_success_message = "{green}√ %s %i bytes{blank}\n";
one_item_success_message = "{green}√ %s %s{blank}\n";
multiple_files_success_message = "{green}√ %s %i files{blank}\n";
multiple_directories_success_message = "{green}√ %s %i directories{blank}\n";
multiple_files_directories_success_message = "{green}√ %s %i files and %i directories{blank}\n";
internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug.{blank}\n";
}