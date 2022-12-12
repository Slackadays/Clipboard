void setLanguageES() {
actions[Action::Cut] = "cut";
actions[Action::Copy] = "copy";
actions[Action::Paste] = "paste";
actions[Action::PipeIn] = "pipe in";
actions[Action::PipeOut] = "pipe out";

doing_action[Action::Cut] = "Cutting";
doing_action[Action::Copy] = "Copying";
doing_action[Action::Paste] = "Pasting";
doing_action[Action::PipeIn] = "Piping in";
doing_action[Action::PipeOut] = "Piping out";

did_action[Action::Cut] = "Cut";
did_action[Action::Copy] = "Copied";
did_action[Action::Paste] = "Pasted";
did_action[Action::PipeIn] = "Piped in";
did_action[Action::PipeOut] = "Piped out";

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
no_clipboard_contents_message = "{red}╳ You did not specify an action. {pink}Try adding {bold}%s, %s, or %s{blank}{pink} to the end, like {bold}clipboard %s{blank}{pink}. If you need more help, try {bold}clipboard -h{blank}{pink} to show the help screen.{blank}\n";
choose_action_items_message = "{red}╳ You need to choose something to %s.{pink} Try adding the items you want to %s to the end, like {bold}clipboard %s contacts.txt myprogram.cpp{blank}\n";
fix_redirection_action_message = "{red}╳ You can't use the {bold}%s{blank}{red} action with redirection here. {pink}Try removing {bold}%s{blank}{pink} or use {bold}%s{blank}{pink} instead, like {bold}clipboard %s{blank}{pink}.\n";
redirection_no_items_message = "{red}╳ You can't specify items when you use redirection. {pink}Try removing the items that come after {bold}clipboard [action].\n";
paste_success_message = "{green}✓ Pasted successfully{blank}\n";
paste_fail_message = "{red}╳ Failed to paste{blank}\n";
clipboard_failed_message = "{red}╳ Clipboard couldn't %s these items.{blank}\n";
and_more_fails_message = "{red}▏ ...and {bold}%i{blank}{red} more.{blank}\n";
and_more_items_message = "{blue}▏ ...and {bold}%i{blank}{blue} more.{blank}\n";
fix_problem_message = "{pink}▏ See if you have the needed permissions, or\n"
                    "▏ try double-checking the spelling of the files or what directory you're in.{blank}\n";
pipe_success_message = "{green}✓ %s %i bytes{blank}\n";
one_item_success_message = "{green}✓ %s %s{blank}\n";
multiple_files_success_message = "{green}✓ %s %i files{blank}\n";
multiple_directories_success_message = "{green}✓ %s %i directories{blank}\n";
multiple_files_directories_success_message = "{green}✓ %s %i files and %i directories{blank}\n";
internal_error_message = "{red}╳ Internal error: %s\n▏ This is probably a bug.{blank}\n";
}

void setLanguagePT() {
actions[Action::Cut] = "recortar";
actions[Action::Copy] = "copiar";
actions[Action::Paste] = "colar";
actions[Action::PipeIn] = "direcionar para dentro";
actions[Action::PipeOut] = "direcionar para fora";

doing_action[Action::Cut] = "Recortando";
doing_action[Action::Copy] = "Copiando";
doing_action[Action::Paste] = "Colando";
doing_action[Action::PipeIn] = "Direcionando para dentro";
doing_action[Action::PipeOut] = "Direcionando para fora";

did_action[Action::Cut] = "Recortou";
did_action[Action::Copy] = "Copiou";
did_action[Action::Paste] = "Colou";
did_action[Action::PipeIn] = "Direcionou para dentro";
did_action[Action::PipeOut] = "Direcionou para fora";

help_message = "{blue}▏Este é Clipboard %s, o sistema de recortar, copiar e colar para a linha de comando.{blank}\n"
                "{blue}{bold}▏Como utilizar{blank}\n"
                "{orange}▏clipboard recortar (item) [itens]{blank}\n"
                "{orange}▏clipboard copiar (item) [itens]{blank}\n"
                "{orange}▏clipboard colar{blank}\n"
                "{blue} Você pode utilizar \"cb\" ao invés de \"clipboard\" para ganhar tempo.{blank}\n"
                "{blue}{bold}▏Exemplos{blank}\n"
                "{orange}▏clipboard copiar ração.conf{blank}\n"
                "{orange}▏cb recortar Códigos_de_Lançamento_de_Mísseis.txt pastadecontatos{blank}\n"
                "{orange}▏cb colar{blank}\n"
                "{blue}▏Você pode rever esta tela de instruções à qualquer momento com {bold}clipboard -h{blank}{blue}, {bold}clipboard --help{blank}{blue} ou{bold} clipboard help{blank}{blue}.\n"
                "{blue}▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.{blank}\n"
                "{blue}▏Este programa vem com ABSOLUTAMENTE NENHUMA GARANTIA. Este é um software livre, e você é bem-vindo a redistribuí-lo sob certas condições.{blank}\n";
no_valid_action_message = "{red}╳ Você não especificou uma ação válida, ou esqueceu de incluí-la. {pink}Tente utilizar {bold}recortar, copiar ou colar{blank}{pink}, como em {bold}clipboard copiar.{blank}\n";
no_clipboard_contents_message = "{red}╳ Você não especificou uma ação válida. {pink}Tente adicionar {bold}%s, %s, or %s{blank}{pink} no final, como em {bold}clipboard %s{blank}{pink}. Caso precise de ajuda, tente {bold}clipboard -h{blank}{pink} para mostrar a tela de instruções.{blank}\n";
choose_action_items_message = "{red}╳ Você precisa especificar algo para %s.{pink} Tenta adicionar os itens que você quer %s ao final, como em {bold}clipboard %s contatos.txt meuprograma.cpp{blank}\n";
fix_redirection_action_message = "{red}╳ Você não pode {bold}%s{blank}{red} com redirecionamento aqui. {pink}Tente remover {bold}%s{blank}{pink} ou utilizar {bold}%s{blank}{pink}, como em {bold}clipboard %s{blank}{pink}.\n";
redirection_no_items_message = "{red}╳ Você não pode especificar itens ao redirecionar. {pink}Tente remover itens que vêm após {bold}clipboard [action].\n";
paste_success_message = "{green}✓ Colado com sucesso{blank}\n";
paste_fail_message = "{red}╳ Não foi possível colar{blank}\n";
clipboard_failed_message = "{red}╳ Clipboard não pôde %s esses itens.{blank}\n";
and_more_fails_message = "{red}▏ ...e mais {bold}%i{blank}{red}.{blank}\n";
and_more_items_message = "{blue}▏ ...e mais {bold}%i{blank}{blue}.{blank}\n";
fix_problem_message = "{pink}▏ Veja se você possui as permissões necessárias, ou\n"
                    "▏ verifique a ortografia do arquivo ou diretório que voce está.{blank}\n";
pipe_success_message = "{green}✓ %s %i bytes{blank}\n";
one_item_success_message = "{green}✓ %s %s{blank}\n";
multiple_files_success_message = "{green}✓ %s %i arquivos{blank}\n";
multiple_directories_success_message = "{green}✓ %s %i diretórios{blank}\n";
multiple_files_directories_success_message = "{green}✓ %s %i arquivos e %i diretórios{blank}\n";
internal_error_message = "{red}╳ Erro interno: %s\n▏ Isso é provavelmente um bug.{blank}\n";
}