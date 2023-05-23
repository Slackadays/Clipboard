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

Message help_message = "[info]│This is the Clipboard Project %s (commit %s), the cut, copy, and paste system for the command line.[blank]\n"
                       "[info][bold]│Examples[blank]\n"
                       "[progress]│cb ct Nuclear_Launch_Codes.txt contactsfolder[blank] [help](This cuts the following items into the "
                       "default clipboard, 0.)[blank]\n"
                       "[progress]│clipboard cp1 dogfood.conf[blank] [help](This copies the following items into clipboard 1.)[blank]\n"
                       "[progress]│cb p1[blank] [help](This pastes clipboard 1.)[blank]\n"
                       "[progress]│cb sh4[blank] [help](This shows the contents of clipboard 4.)[blank]\n"
                       "[progress]│cb clr[blank] [help](This clears the contents of the default clipboard.)[blank]\n"
                       "[info]│You can also choose which clipboard you want to use by adding a number to the end, or "
                       "[bold]_[blank][info] to use a persistent clipboard.[blank]\n"
                       "[info][bold]│More Info[blank]\n"
                       "[info]│See the complete online documentation for CB at https://github.com/Slackadays/Clipboard.[blank]\n"
                       "[info]│Show this help screen anytime with [bold]cb -h[blank][info], [bold]cb "
                       "--help[blank][info], or[bold] cb help[blank][info].\n"
                       "[info]│You can also get more help in our Discord server at [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                       "[info]│Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                       "[info]│This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to "
                       "redistribute it under certain conditions.[blank]\n";
Message check_clipboard_status_message = "[info]All of your clipboards with content";
Message clipboard_item_one_contents_message = "[info]🔷 Here is the [bold]%i[blank][info] item in clipboard [bold]%s[blank][info]: [blank]\n";
Message clipboard_item_many_contents_message = "[info]Here are the items in clipboard [bold][help]%s[blank][info]";
Message clipboard_text_contents_message = "[info]🔷 Here are the first [bold]%i[blank][info] bytes in clipboard [bold]%s[blank][info]: [blank]";
Message no_clipboard_contents_message = "[info]🔷 There is currently nothing in the clipboard.[blank]\n";
Message clipboard_action_prompt = "[help]Add [bold]cut, copy, [blank][help]or[bold] paste[blank][help] to the end, "
                                  "like [bold]%s copy[blank][help] to get started, or if you need help, try "
                                  "[bold]%s -h[blank][help] to show the help screen.[blank]\n";
Message no_valid_action_message = "[error]❌ You did not specify a valid action ([bold]\"%s\"[blank][error]), or you forgot "
                                  "to include one. 💡 [help]Try using or adding [bold]cut, copy, [blank][help]or "
                                  "[bold]paste[blank][help] instead, like [bold]%s copy.[blank]\n";
Message choose_action_items_message = "[error]❌ You need to choose something to %s.[help] 💡 Try adding the items you want "
                                      "to %s to the end, like [bold]%s %s contacts.txt myprogram.cpp[blank]\n";
Message fix_redirection_action_message = "[error]❌ You can't use the [bold]%s[blank][error] action with redirection here. 💡 [help]Try removing "
                                         "[bold]%s[blank][help] or use [bold]%s[blank][help] instead, like [bold]%s %s[blank][help].\n";
Message redirection_no_items_message = "[error]❌ You can't specify items when you use redirection. 💡 [help]Try removing "
                                       "the items that come after [bold]%s [action].\n";
Message paste_success_message = "[success]✅ Pasted successfully[blank]\n";
Message clipboard_failed_one_message = "[error]❌ CB couldn't %s this item:[blank]\n";
Message clipboard_failed_many_message = "[error]❌ CB couldn't %s these items:[blank]\n";
Message and_more_fails_message = "[error]│ ...and [bold]%i[blank][error] more.[blank]\n";
Message and_more_items_message = "[info]│ ...and [bold]%i[blank][info] more.[blank]\n";
Message fix_problem_message = "[help]💡 See if you have the needed permissions, or\n"
                              "│ try double-checking the spelling of the files or what directory you're in.[blank]\n";
Message not_enough_storage_message = "[error]❌ There won't be enough storage available to %s everything (%gMB to "
                                     "paste, %gMB available). 💡 [blank][help] Try double-checking what items you've "
                                     "selected or delete some files to free up space.[blank]\n";
Message item_already_exists_message = "[progress]🟡 The item [bold]%s[blank][progress] already exists here. Do you want to "
                                      "replace it? [help]Use [bold]all [blank][help]to replace all existing, or "
                                      "[bold]skip[blank][help] to replace nothing. [bold][(y)es/(n)o)/(a)ll/(s)kip] ";
Message bad_response_message = "[error]❌ Sorry, that wasn't a valid choice. Try again: [blank][help][bold][(y)es/(n)o)] ";
Message working_message = "\r[progress]🟡 %s... %s %s    [blank]";
Message cancelled_message = "[success]✅ Cancelled %s[blank]\n";
Message cancelled_with_progress_message = "[success]✅ Cancelled %s (%s in progress)[blank]\n";
Message byte_success_message = "[success]✅ %s %s[blank]\n";
Message one_item_success_message = "[success]✅ %s one item[blank]\n";
Message many_files_success_message = "[success]✅ %s %lu files[blank]\n";
Message many_directories_success_message = "[success]✅ %s %lu directories[blank]\n";
Message one_file_one_directory_success_message = "[success]✅ %s one file and one directory[blank]\n";
Message one_file_many_directories_success_message = "[success]✅ %s one file and %lu directories[blank]\n";
Message many_files_one_directory_success_message = "[success]✅ %s %lu files and one directory[blank]\n";
Message many_files_many_directories_success_message = "[success]✅ %s %lu files and %lu directories[blank]\n";
Message one_clipboard_success_message = "[success]✅ %s one clipboard[blank]\n";
Message many_clipboards_success_message = "[success]✅ %s %lu clipboards[blank]\n";
Message clipboard_name_message = "[info]Info for clipboard [bold][help]%s[blank]";
Message internal_error_message = "[error]❌ Internal error: %s\n│ This might be a bug, or you might be lacking "
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

    help_message = "[info]│Esto es Clipboard Project %s (commit %s), el sistema para cortar, copiar y pegar adentro del "
                   "terminal.[blank]\n"
                   "[info][bold]│Cómo usar[blank]\n"
                   "[progress]│cb cortar (cosa) [cosas][blank]\n"
                   "[progress]│cb copiar (cosa) [cosas][blank]\n"
                   "[progress]│cb pegar[blank]\n"
                   "[info][bold]│Ejemplos[blank]\n"
                   "[progress]│cb copiar cosas.conf[blank]\n"
                   "[progress]│cb cortar MisDocumentos.txt nuevacarpeta[blank]\n"
                   "[progress]│cb pegar[blank]\n"
                   "[info]│Muestra este mensaje de ayudar en cualquier tiempo que quieras con [bold]cb "
                   "-h[blank][info], [bold]cb --help[blank][info] o[bold] cb help[blank][info].\n"
                   "[info]│Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]│This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome "
                   "to redistribute it under certain conditions.[blank]\n";
    no_valid_action_message = "[error]❌ No especificaste ninguna acción válida o se te olvidó. 💡 [help]Inténta usar o añadir [bold]cortar, "
                              "copiar o pegar[blank][help] en su lugar, como [bold]%s copiar.[blank]\n";
    clipboard_item_many_contents_message = "[info]Aquí están las cosas primeras del "
                                           "portapapeles [bold][help]%s[blank][info]";
    no_clipboard_contents_message = "[info]🔷 No hay nada en CB en este momento.[blank]\n";
    clipboard_action_prompt = "[help]Añade [bold]cortar, copiar [blank][help]o[bold] pegar[blank][help] al final, como "
                              "[bold]%s copiar[blank][help] para comenzar, o si necesitas ayuda, haz "
                              "[bold]%s -h[blank][help] para mostrar el mensaje de ayudar.[blank]\n";
    choose_action_items_message = "[error]❌ Necesitas escoger una cosa para %s. 💡 [help] Inténta añadir las cosas que "
                                  "quieres %s al final, como [bold]%s %s contactos.txt miprograma.cpp[blank]\n";
    fix_redirection_action_message = "[error]❌ No se puede usar la acción [bold]%s[blank][error] con la redirección. 💡 [help]Inténta sacar "
                                     "[bold]%s[blank][help] o usa [bold]%s[blank][help] en su lugar, como [bold]%s %s[blank][help].\n";
    redirection_no_items_message = "[error]❌ No se pueden especificar las cosas con redirección. 💡 [help]Inténta sacar las "
                                   "cosas que siguen [bold]%s [acción].\n";
    paste_success_message = "[success]✅ Pegó con éxito[blank]\n";
    clipboard_failed_many_message = "[error]❌ CB no pudo %s estas cosas.[blank]\n";
    and_more_fails_message = "[error]│ ...y [bold]%i[blank][error] más.[blank]\n";
    and_more_items_message = "[info]│ ...y [bold]%i[blank][info] más.[blank]\n";
    fix_problem_message = "[help]💡 Verífica si tengas los permisos necesarios, o\n"
                          "│ vuelve a revisar el deletro de los archivos o la carpeta en que estás.[blank]\n";
    not_enough_storage_message = "[error]❌ No habrá espacio suficiente para %s todas tus cosas (%gMB a pegar, %gMB "
                                 "disponible). 💡 [blank][help] Vuelve a revisar las cosas que especificaste o saca "
                                 "algunas cosas para hacer más espacio.[blank]\n";
    many_files_success_message = "[success]✅ %s %lu archivos[blank]\n";
    many_directories_success_message = "[success]✅ %s %lu carpetas[blank]\n";
    many_files_many_directories_success_message = "[success]✅ %s %lu archivos y %lu carpetas[blank]\n";
    internal_error_message = "[error]❌ Error internal: %s[blank]\n";
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

    help_message = "[info]│Este é Clipboard Project %s (commit %s), o sistema de recortar, copiar e colar para a linha de "
                   "comando.[blank]\n"
                   "[info][bold]│Como utilizar[blank]\n"
                   "[progress]│cb recortar (item) [itens][blank]\n"
                   "[progress]│cb copiar (item) [itens][blank]\n"
                   "[progress]│cb colar[blank]\n"
                   "[info][bold]│Exemplos[blank]\n"
                   "[progress]│cb copiar ração.conf[blank]\n"
                   "[progress]│cb recortar Códigos_de_Lançamento_de_Mísseis.txt pastadecontatos[blank]\n"
                   "[progress]│cb colar[blank]\n"
                   "[info]│Você pode rever esta tela de instruções à qualquer momento com [bold]cb "
                   "-h[blank][info], [bold]cb --help[blank][info] ou[bold] cb help[blank][info].\n"
                   "[info]│Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]│Este programa vem com ABSOLUTAMENTE NENHUMA GARANTIA. Este é um software livre, e você é "
                   "bem-vindo a redistribuí-lo sob certas condições.[blank]\n";
    no_valid_action_message = "[error]❌ Você não especificou uma ação válida (\"%s\"), ou esqueceu de incluí-la. 💡 [help]Tente utilizar "
                              "[bold]recortar, copiar ou colar[blank][help], como em [bold]%s copiar.[blank]\n";
    no_clipboard_contents_message = "[error]❌ Você não especificou uma ação válida. 💡 [help]Tente adicionar [bold]recortar, copiar, or "
                                    "colar[blank][help] no final, como em [bold]clipboard copiar[blank][help]. Caso precise de ajuda, tente "
                                    "[bold]clipboard -h[blank][help] para mostrar a tela de instruções.[blank]\n";
    choose_action_items_message = "[error]❌ Você precisa especificar algo para %s. 💡 [help] Tenta adicionar os itens que você quer %s ao final, "
                                  "como em [bold]%s %s contatos.txt meuprograma.cpp[blank]\n";
    fix_redirection_action_message = "[error]❌ Você não pode [bold]%s[blank][error] com redirecionamento aqui. 💡 [help]Tente remover "
                                     "[bold]%s[blank][help] ou utilizar [bold]%s[blank][help], como em [bold]%s %s[blank][help].\n";
    redirection_no_items_message = "[error]❌ Você não pode especificar itens ao redirecionar. 💡 [help]Tente remover itens "
                                   "que vêm após [bold]%s [action].\n";
    paste_success_message = "[success]✅ Colado com sucesso[blank]\n";
    clipboard_failed_many_message = "[error]❌ CB não pôde %s esses itens.[blank]\n";
    and_more_fails_message = "[error]│ ...e mais [bold]%i[blank][error].[blank]\n";
    and_more_items_message = "[info]│ ...e mais [bold]%i[blank][info].[blank]\n";
    fix_problem_message = "[help]💡 Veja se você possui as permissões necessárias, ou\n"
                          "│ verifique a ortografia do arquivo ou diretório que voce está.[blank]\n";
    many_files_success_message = "[success]✅ %s %lu arquivos[blank]\n";
    many_directories_success_message = "[success]✅ %s %lu diretórios[blank]\n";
    many_files_many_directories_success_message = "[success]✅ %s %lu arquivos e %lu diretórios[blank]\n";
    internal_error_message = "[error]❌ Erro interno: %s\n│ Isso é provavelmente um bug.[blank]\n";
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

    help_message = "[info]│Clipboard Project %s (commit %s), komut satırı için, kesme, kopyalama ve yapıştırma sistemidir.[blank]\n"
                   "[info][bold]│Nasıl kullanılır[blank]\n"
                   "[progress]│cb kes (öğe) [öğeler][blank] [help](Bu öğe(leri) keser.)[blank]\n"
                   "[progress]│cb kopyala (öğe) [öğeler][blank] [help](Bu öğe(leri) kopyalar.)[blank]\n"
                   "[progress]│cb yapistir[blank] [help](Bu panodakileri yapıştırır.)[blank]\n"
                   "[progress]│cb goster[blank] [help](Bu panoda olan öğeleri gösterir.)[blank]\n"
                   "[progress]│cb temizle[blank] [help](Bu pano içerğini temizler.)[blank]\n"
                   "[info]│    Ben ise \"pano\" ismini kullanmanızı öneririm :)[blank]\n"
                   "[info]│Ayrıca kommutun sonuna bir sayı ekleyerek 10 farklı panodan birisini seçebilirsiniz.[blank]\n"
                   "[info][bold]│Örnekler[blank]\n"
                   "[progress]│pano ks Nükleer_Fırlatma_Kodları.txt kişilerklasörü[blank] [help](Bu verilen öğeleri öntanımlı "
                   "0. panoya keser)[blank]\n"
                   "[progress]│pano kp1 mama.conf[blank] [help](Bu verilen öğeleri 1. panoya kopyalar.)[blank]\n"
                   "[progress]│pano y1[blank] [help](Bu 1. panodakileri yapıştırır)[blank]\n"
                   "[progress]│pano go4[blank] [help](Bu 4. pano içeriğini gösterir, 4.)[blank]\n"
                   "[progress]│pano tmz[blank] [help](Bu öntanımlı panonun içeriğini temizler.)[blank]\n"
                   "[info]│Bu yardım ekranını herhangi bir zaman şu komutlardan birisiyle görebilirsiniz:[blank]\n"
                   "[info]│    [bold]cb -h[blank][info], [bold]cb --help[blank][info], ya da[bold] cb "
                   "help[blank][info].\n"
                   "[info]│Discord sunucumuzdan daha fazla yardım alabilirsiniz: [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                   "[info]│Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[info]│                                 GPLv3 altında lisanslanmıştır.[blank]\n"
                   "[info]│Bu program KESİNLİKLE HİÇBİR GARANTİ ile birlikte gelir. Bu ücretsiz bir yazılımdır ve belirli "
                   "koşullar altında yeniden dağıtabilirsiniz.[blank]\n";
    check_clipboard_status_message = "[info]Pano içeriği şunlardır:";
    clipboard_item_many_contents_message = "[help][bold]%s[blank][info] panoda bulunan";
    no_clipboard_contents_message = "[info]🔷 Panoda hiçbir şey yok.[blank]\n";
    clipboard_action_prompt = "[help]Başlamak için sonuna [bold]kes, kopyala, [blank][help]veya[bold] yapistir[blank][help] ekleyin, "
                              "[bold]%s kopyala[blank][help] gibi, veya yardıma ihtiyacın olursa yardım ekranını göstermek için "
                              "[bold]%s -h[blank][help]'i dene.[blank]\n";
    no_valid_action_message = "[error]❌ Geçerli bir işlem vermediniz (\"%s\") veya işlem vermeyi unuttunuz 💡 [help]Komutunuza [bold]cut, "
                              "copy, [blank][help]ya da [bold]paste[blank][help] eklemelisiniz, örneğin [bold]%s copy.[blank]\n";
    choose_action_items_message = "[error]❌ %s(ma/me) işlemi için bir öğe seçmeniz gerekmektedir. 💡 [help] %s işleminden sonra öğeler eklemeyi "
                                  "deneyiniz, örneğin [bold]%s %s contacts.txt myprogram.cpp[blank]\n";
    fix_redirection_action_message = "[error]❌ [bold]%s[blank][error](ma/me) işlemini burada yeniden yönlendirme ile kullanamazsın. [help] "
                                     "💡 [bold]%s[blank][help] işlemini silin veya [bold]%s[blank][help] işlemini kullanın, örneğin "
                                     "[bold]%s %s[blank][help].\n";
    redirection_no_items_message = "[error]❌ Yeniden yönlendirme işlemi yaparken öğe veremezsiniz. 💡 [help][bold]%s "
                                   "[action][blank][help]'dan sonra  gelen öğeleri siliniz.\n";
    paste_success_message = "[success]✅ Yapıştırma başarıyla tamamlandı[blank]\n";
    clipboard_failed_many_message = "[error]❌ %s(ma/me) işlemi şu öğeler için başarısız oldu:[blank]\n";
    and_more_fails_message = "[error]│ ...ve [bold]%i[blank][error] fazla.[blank]\n";
    and_more_items_message = "[info]│ ...ve [bold]%i[blank][info] fazla.[blank]\n";
    fix_problem_message = "[help]💡 Erişime ihtiyacınız varsa şuna bakın, veya\n"
                          "│ bulunduğunuz dizini veya girdiğiniz dosya isimlerini ikinci kez kontrol edin.[blank]\n";
    not_enough_storage_message = "[error]❌ Bütün öğelerinizi %s kadar yeterli bir alanınız yok (%gMB "
                                 "yapıştırılacak, %gMB boş). 💡 [blank][help] Hangi öğeleri seçtiğinizi ikinci kez kontrol "
                                 "etmeyi deneyin veya yer açmak için bazı dosyaları silin.[blank]\n";
    many_files_success_message = "[success]✅ %s %lu dosya[blank]\n";
    many_directories_success_message = "[success]✅ %s %lu dizin[blank]\n";
    many_files_many_directories_success_message = "[success]✅ %s %lu dosya ve %lu dizin[blank]\n";
    internal_error_message = "[error]❌ İçsel hata: %s\n│ Bu yüksek ihtimal bir hata veya bu sistemde erişim sorunu yaşıyorsunuz.[blank]\n";
}
