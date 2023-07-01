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
#include "clipboard.hpp"

EnumArray<std::string_view, 18> actions =
        {"cut", "copy", "paste", "clear", "show", "edit", "add", "remove", "note", "swap", "status", "info", "load", "import", "export", "history", "ignore", "search"};

EnumArray<std::string_view, 18> action_shortcuts = {"ct", "cp", "p", "clr", "sh", "ed", "ad", "rm", "nt", "sw", "st", "in", "ld", "imp", "ex", "hs", "ig", "sr"};
EnumArray<std::string_view, 18> doing_action = {
        "Cutting",
        "Copying",
        "Pasting",
        "Clearing",
        "Showing",
        "Editing",
        "Adding",
        "Removing",
        "Noting",
        "Swapping",
        "Checking status",
        "Showing info",
        "Loading",
        "Importing",
        "Exporting",
        "Getting history",
        "Ignoring",
        "Searching"};

EnumArray<std::string_view, 18> did_action = {
        "Cut",
        "Copied",
        "Pasted",
        "Cleared",
        "Showed",
        "Edited",
        "Added",
        "Removed",
        "Noted",
        "Swapped",
        "Checked status",
        "Showed info",
        "Loaded",
        "Imported",
        "Exported",
        "Got history",
        "Ignored",
        "Searched"};

EnumArray<std::string_view, 18> action_descriptions = {
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
        "Search for items in a clipboard."};

Message help_message = "[info]┃ This is the Clipboard Project %s (commit %s), the cut, copy, and paste system for the command line.[blank]\n"
                       "[info][bold]┃ Examples[blank]\n"
                       "[progress]┃ cb ct Nuclear_Launch_Codes.txt contactsfolder[blank] [help](This cuts the following items into the "
                       "default clipboard, 0.)[blank]\n"
                       "[progress]┃ clipboard cp1 dogfood.conf[blank] [help](This copies the following items into clipboard 1.)[blank]\n"
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
Message check_clipboard_status_message = "[info][bold]All of your clipboards with content[nobold]";
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

void setLanguageES() {
    actions[Action::Cut] = "cortar";
    actions[Action::Copy] = "copiar";
    actions[Action::Paste] = "pegar";
    actions[Action::Clear] = "quitar";
    actions[Action::Show] = "mostrar";

    action_shortcuts[Action::Cut] = "ct";
    action_shortcuts[Action::Copy] = "cp";
    action_shortcuts[Action::Paste] = "p";
    action_shortcuts[Action::Clear] = "qt";
    action_shortcuts[Action::Show] = "ms";

    doing_action[Action::Cut] = "Cortando";
    doing_action[Action::Copy] = "Copiando";
    doing_action[Action::Paste] = "Pegando";

    did_action[Action::Cut] = "Cortó";
    did_action[Action::Copy] = "Copió";
    did_action[Action::Paste] = "Pegó";

    help_message = "[info]┃ Esto es Clipboard Project %s (commit %s), el sistema para cortar, copiar y pegar adentro del "
                   "terminal.[blank]\n"
                   "[info][bold]┃ Cómo usar[blank]\n"
                   "[progress]┃ cb cortar (cosa) [cosas][blank]\n"
                   "[progress]┃ cb copiar (cosa) [cosas][blank]\n"
                   "[progress]┃ cb pegar[blank]\n"
                   "[info][bold]┃ Ejemplos[blank]\n"
                   "[progress]┃ cb copiar cosas.conf[blank]\n"
                   "[progress]┃ cb cortar MisDocumentos.txt nuevacarpeta[blank]\n"
                   "[progress]┃ cb pegar[blank]\n"
                   "[info]┃ Muestra este mensaje de ayudar en cualquier tiempo que quieras con [bold]cb "
                   "-h[nobold], [bold]cb --help[nobold] o[bold] cb help[nobold].\n"
                   "[info][bold]┃ All Actions Available[blank]\n"
                   "%s"
                   "[info]┃ Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]┃ This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome "
                   "to redistribute it under certain conditions.[blank]\n";
    no_valid_action_message = "[error][inverse] ✘ [noinverse] No especificaste ninguna acción válida o se te olvidó. [help]⬤ Inténta usar o añadir [bold]cortar, "
                              "copiar o pegar[nobold] en su lugar, como [bold]%s copiar.[blank]\n";
    clipboard_item_many_contents_message = "[inverse][bold][info] Aquí están las cosas primeras del "
                                           "portapapeles [bold][help] %s[nobold] [noinverse]";
    no_clipboard_contents_message = "[info]┃ No hay nada en CB en este momento.[blank]\n";
    clipboard_action_prompt = "[help]Añade [bold]cortar, copiar [nobold]o[bold] pegar[nobold] al final, como "
                              "[bold]%s copiar[nobold] para comenzar, o si necesitas ayuda, haz "
                              "[bold]%s -h[nobold] para mostrar el mensaje de ayudar.[blank]\n";
    choose_action_items_message = "[error][inverse] ✘ [noinverse] Necesitas escoger una cosa para %s. [help]⬤ Inténta añadir las cosas que "
                                  "quieres %s al final, como [bold]%s %s contactos.txt miprograma.cpp[blank]\n";
    fix_redirection_action_message = "[error][inverse] ✘ [noinverse] No se puede usar la acción [bold]%s[blank][error] con la redirección. [help]⬤ Inténta sacar "
                                     "[bold]%s[nobold] o usa [bold]%s[nobold] en su lugar, como [bold]%s %s[nobold].\n";
    redirection_no_items_message = "[error][inverse] ✘ [noinverse] No se pueden especificar las cosas con redirección. [help]⬤ Inténta sacar las "
                                   "cosas que siguen [bold]%s [acción].\n";
    paste_success_message = "[success][inverse] ✔ [noinverse] Pegó con éxito[blank]\n";
    clipboard_failed_many_message = "[error][inverse] ✘ [noinverse] CB no pudo %s estas cosas.[blank]\n";
    and_more_fails_message = "[error][inverse] ✘ [noinverse] ...y [bold]%i[nobold] más.[blank]\n";
    and_more_items_message = "[info]┃ ...y [bold]%i[nobold] más.[blank]\n";
    fix_problem_message = "[help]⬤ Verífica si tengas los permisos necesarios, o\n"
                          "┃ vuelve a revisar el deletro de los archivos o la carpeta en que estás.[blank]\n";
    not_enough_storage_message = "[error][inverse] ✘ [noinverse] No habrá espacio suficiente para %s todas tus cosas (%gMB a pegar, %gMB "
                                 "disponible). [help]⬤ Vuelve a revisar las cosas que especificaste o saca "
                                 "algunas cosas para hacer más espacio.[blank]\n";
    many_files_success_message = "[success][inverse] ✔ [noinverse] %s %lu archivos[blank]\n";
    many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu carpetas[blank]\n";
    many_files_many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu archivos y %lu carpetas[blank]\n";
    internal_error_message = "[error][inverse] ✘ [noinverse] Error internal: %s[blank]\n";
}

void setLanguagePT() {
    actions[Action::Cut] = "recortar";
    actions[Action::Copy] = "copiar";
    actions[Action::Paste] = "colar";

    doing_action[Action::Cut] = "Recortando";
    doing_action[Action::Copy] = "Copiando";
    doing_action[Action::Paste] = "Colando";

    did_action[Action::Cut] = "Recortou";
    did_action[Action::Copy] = "Copiou";
    did_action[Action::Paste] = "Colou";

    help_message = "[info]┃ Este é Clipboard Project %s (commit %s), o sistema de recortar, copiar e colar para a linha de "
                   "comando.[blank]\n"
                   "[info][bold]┃ Como utilizar[blank]\n"
                   "[progress]┃ cb recortar (item) [itens][blank]\n"
                   "[progress]┃ cb copiar (item) [itens][blank]\n"
                   "[progress]┃ cb colar[blank]\n"
                   "[info][bold]┃ Exemplos[blank]\n"
                   "[progress]┃ cb copiar ração.conf[blank]\n"
                   "[progress]┃ cb recortar Códigos_de_Lançamento_de_Mísseis.txt pastadecontatos[blank]\n"
                   "[progress]┃ cb colar[blank]\n"
                   "[info]┃ Você pode rever esta tela de instruções à qualquer momento com [bold]cb "
                   "-h[nobold], [bold]cb --help[nobold] ou[bold] cb help[nobold].\n"
                   "[info][bold]┃ All Actions Available[blank]\n"
                   "%s"
                   "[info]┃ Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]┃ Este programa vem com ABSOLUTAMENTE NENHUMA GARANTIA. Este é um software livre, e você é "
                   "bem-vindo a redistribuí-lo sob certas condições.[blank]\n";
    no_valid_action_message = "[error][inverse] ✘ [noinverse] Você não especificou uma ação válida (\"%s\"), ou esqueceu de incluí-la. [help]⬤ Tente utilizar "
                              "[bold]recortar, copiar ou colar[nobold], como em [bold]%s copiar.[blank]\n";
    no_clipboard_contents_message = "[error][inverse] ✘ [noinverse] Você não especificou uma ação válida. [help]⬤ Tente adicionar [bold]recortar, copiar, or "
                                    "colar[nobold] no final, como em [bold]clipboard copiar[nobold]. Caso precise de ajuda, tente "
                                    "[bold]clipboard -h[nobold] para mostrar a tela de instruções.[blank]\n";
    choose_action_items_message = "[error][inverse] ✘ [noinverse] Você precisa especificar algo para %s. [help]⬤ Tenta adicionar os itens que você quer %s ao final, "
                                  "como em [bold]%s %s contatos.txt meuprograma.cpp[blank]\n";
    fix_redirection_action_message = "[error][inverse] ✘ [noinverse] Você não pode [bold]%s[blank][error] com redirecionamento aqui. [help]⬤ Tente remover "
                                     "[bold]%s[nobold] ou utilizar [bold]%s[nobold], como em [bold]%s %s[nobold].\n";
    redirection_no_items_message = "[error][inverse] ✘ [noinverse] Você não pode especificar itens ao redirecionar. [help]⬤ Tente remover itens "
                                   "que vêm após [bold]%s [action].\n";
    paste_success_message = "[success][inverse] ✔ [noinverse] Colado com sucesso[blank]\n";
    clipboard_failed_many_message = "[error][inverse] ✘ [noinverse] CB não pôde %s esses itens.[blank]\n";
    and_more_fails_message = "[error][inverse] ✘ [noinverse] ...e mais [bold]%i[nobold].[blank]\n";
    and_more_items_message = "[info]┃ ...e mais [bold]%i[nobold].[blank]\n";
    fix_problem_message = "[help]⬤ Veja se você possui as permissões necessárias, ou\n"
                          "┃ verifique a ortografia do arquivo ou diretório que voce está.[blank]\n";
    many_files_success_message = "[success][inverse] ✔ [noinverse] %s %lu arquivos[blank]\n";
    many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu diretórios[blank]\n";
    many_files_many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu arquivos e %lu diretórios[blank]\n";
    internal_error_message = "[error][inverse] ✘ [noinverse] Erro interno: %s\n┃ Isso é provavelmente um bug.[blank]\n";
}

void setLanguageTR() {
    actions[Action::Cut] = "kes";
    actions[Action::Copy] = "kopyala";
    actions[Action::Paste] = "yapistir";
    actions[Action::Show] = "goster";
    actions[Action::Clear] = "temizle";

    action_shortcuts[Action::Cut] = "ks";
    action_shortcuts[Action::Copy] = "kp";
    action_shortcuts[Action::Paste] = "y";
    action_shortcuts[Action::Clear] = "tmz";
    action_shortcuts[Action::Show] = "go";

    doing_action[Action::Cut] = "Kesiliyor";
    doing_action[Action::Copy] = "Kopyalanıyor";
    doing_action[Action::Paste] = "Yapıştırılıyor";

    did_action[Action::Cut] = "Kesildi";
    did_action[Action::Copy] = "Kopyalandı";
    did_action[Action::Paste] = "Yapıştırıldı";

    help_message = "[info]┃ Clipboard Project %s (commit %s), komut satırı için, kesme, kopyalama ve yapıştırma sistemidir.[blank]\n"
                   "[info][bold]┃ Nasıl kullanılır[blank]\n"
                   "[progress]┃ cb kes (öğe) [öğeler][blank] [help](Bu öğe(leri) keser.)[blank]\n"
                   "[progress]┃ cb kopyala (öğe) [öğeler][blank] [help](Bu öğe(leri) kopyalar.)[blank]\n"
                   "[progress]┃ cb yapistir[blank] [help](Bu panodakileri yapıştırır.)[blank]\n"
                   "[progress]┃ cb goster[blank] [help](Bu panoda olan öğeleri gösterir.)[blank]\n"
                   "[progress]┃ cb temizle[blank] [help](Bu pano içerğini temizler.)[blank]\n"
                   "[info]┃    Ben ise \"pano\" ismini kullanmanızı öneririm :)[blank]\n"
                   "[info]┃ Ayrıca kommutun sonuna bir sayı ekleyerek 10 farklı panodan birisini seçebilirsiniz.[blank]\n"
                   "[info][bold]┃ Örnekler[blank]\n"
                   "[progress]┃ pano ks Nükleer_Fırlatma_Kodları.txt kişilerklasörü[blank] [help](Bu verilen öğeleri öntanımlı "
                   "0. panoya keser)[blank]\n"
                   "[progress]┃ pano kp1 mama.conf[blank] [help](Bu verilen öğeleri 1. panoya kopyalar.)[blank]\n"
                   "[progress]┃ pano y1[blank] [help](Bu 1. panodakileri yapıştırır)[blank]\n"
                   "[progress]┃ pano go4[blank] [help](Bu 4. pano içeriğini gösterir, 4.)[blank]\n"
                   "[progress]┃ pano tmz[blank] [help](Bu öntanımlı panonun içeriğini temizler.)[blank]\n"
                   "[info]┃ Bu yardım ekranını herhangi bir zaman şu komutlardan birisiyle görebilirsiniz:[blank]\n"
                   "[info]┃    [bold]cb -h[nobold], [bold]cb --help[nobold], ya da[bold] cb "
                   "help[nobold].\n"
                   "[info][bold]┃ All Actions Available[blank]\n"
                   "%s"
                   "[info]┃ Discord sunucumuzdan daha fazla yardım alabilirsiniz: [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                   "[info]┃ Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]┃                                 GPLv3 altında lisanslanmıştır.[blank]\n"
                   "[info]┃ Bu program KESİNLİKLE HİÇBİR GARANTİ ile birlikte gelir. Bu ücretsiz bir yazılımdır ve belirli "
                   "koşullar altında yeniden dağıtabilirsiniz.[blank]\n";
    check_clipboard_status_message = "[info][bold]Pano içeriği şunlardır[nobold]";
    clipboard_item_many_contents_message = "[inverse][help][bold] %s [info] panoda bulunan[nobold] [noinverse]";
    no_clipboard_contents_message = "[info]┃ Panoda hiçbir şey yok.[blank]\n";
    clipboard_action_prompt = "[help]Başlamak için sonuna [bold]kes, kopyala, [nobold]veya[bold] yapistir[nobold] ekleyin, "
                              "[bold]%s kopyala[nobold] gibi, veya yardıma ihtiyacın olursa yardım ekranını göstermek için "
                              "[bold]%s -h[nobold]'i dene.[blank]\n";
    no_valid_action_message = "[error][inverse] ✘ [noinverse] Geçerli bir işlem vermediniz (\"%s\") veya işlem vermeyi unuttunuz [help]⬤ Komutunuza [bold]cut, "
                              "copy, [nobold]ya da [bold]paste[nobold] eklemelisiniz, örneğin [bold]%s copy.[blank]\n";
    choose_action_items_message = "[error][inverse] ✘ [noinverse] %s(ma/me) işlemi için bir öğe seçmeniz gerekmektedir. [help]⬤ %s işleminden sonra öğeler eklemeyi "
                                  "deneyiniz, örneğin [bold]%s %s contacts.txt myprogram.cpp[blank]\n";
    fix_redirection_action_message = "[error][inverse] ✘ [noinverse] [bold]%s[blank][error](ma/me) işlemini burada yeniden yönlendirme ile kullanamazsın. [help] "
                                     "⬤ [bold]%s[nobold] işlemini silin veya [bold]%s[nobold] işlemini kullanın, örneğin "
                                     "[bold]%s %s[nobold].\n";
    redirection_no_items_message = "[error][inverse] ✘ [noinverse] Yeniden yönlendirme işlemi yaparken öğe veremezsiniz. [help]⬤[bold]%s "
                                   "[action][nobold]'dan sonra  gelen öğeleri siliniz.\n";
    paste_success_message = "[success][inverse] ✔ [noinverse] Yapıştırma başarıyla tamamlandı[blank]\n";
    clipboard_failed_many_message = "[error][inverse] ✘ [noinverse] %s(ma/me) işlemi şu öğeler için başarısız oldu:[blank]\n";
    and_more_fails_message = "[error][inverse] ✘ [noinverse] ...ve [bold]%i[nobold] fazla.[blank]\n";
    and_more_items_message = "[info]┃ ...ve [bold]%i[nobold] fazla.[blank]\n";
    fix_problem_message = "[help]⬤ Erişime ihtiyacınız varsa şuna bakın, veya\n"
                          "┃ bulunduğunuz dizini veya girdiğiniz dosya isimlerini ikinci kez kontrol edin.[blank]\n";
    not_enough_storage_message = "[error][inverse] ✘ [noinverse] Bütün öğelerinizi %s kadar yeterli bir alanınız yok (%gMB "
                                 "yapıştırılacak, %gMB boş). [help]⬤ Hangi öğeleri seçtiğinizi ikinci kez kontrol "
                                 "etmeyi deneyin veya yer açmak için bazı dosyaları silin.[blank]\n";
    many_files_success_message = "[success][inverse] ✔ [noinverse] %s %lu dosya[blank]\n";
    many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu dizin[blank]\n";
    many_files_many_directories_success_message = "[success][inverse] ✔ [noinverse] %s %lu dosya ve %lu dizin[blank]\n";
    internal_error_message = "[error][inverse] ✘ [noinverse] İçsel hata: %s\n┃ Bu yüksek ihtimal bir hata veya bu sistemde erişim sorunu yaşıyorsunuz.[blank]\n";
}
