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

ActionArray<std::string_view, 8> actions = {"cut", "copy", "paste", "clear", "show", "edit", "add", "remove"};

ActionArray<std::string_view, 8> action_shortcuts = {"ct", "cp", "p", "clr", "sh", "ed", "ad", "rm"};

ActionArray<std::string_view, 8> doing_action = {"Cutting", "Copying", "Pasting", "Clearing", "Editing", "Adding", "Removing"};

ActionArray<std::string_view, 8> did_action = {"Cut", "Copied", "Pasted", "Cleared", "Showed", "Edited", "Added", "Removed"};

Message help_message = "[blue]▏This is Clipboard %s (commit %s), the cut, copy, and paste system for the command line.[blank]\n"
                       "[blue][bold]▏How To Use[blank]\n"
                       "[orange]▏clipboard cut (item) [items][blank] [pink](This cuts an item or items.)[blank]\n"
                       "[orange]▏clipboard copy (item) [items][blank] [pink](This copies an item or items.)[blank]\n"
                       "[orange]▏clipboard paste[blank] [pink](This pastes a clipboard.)[blank]\n"
                       "[orange]▏clipboard show[blank] [pink](This shows what's in a clipboard.)[blank]\n"
                       "[orange]▏clipboard clear[blank] [pink](This clears a clipboard's contents.)[blank]\n"
                       "[blue]▏You can substitute \"cb\" for \"clipboard\" and use various shorthands for the actions to save "
                       "time.[blank]\n"
                       "[blue]▏You can also choose which clipboard you want to use by adding a number to the end, or "
                       "[bold]_[blank][blue] to use a persistent clipboard.[blank]\n"
                       "[blue][bold]▏Examples[blank]\n"
                       "[orange]▏cb ct Nuclear_Launch_Codes.txt contactsfolder[blank] [pink](This cuts the following items into the "
                       "default clipboard, 0.)[blank]\n"
                       "[orange]▏clipboard cp1 dogfood.conf[blank] [pink](This copies the following items into clipboard 1.)[blank]\n"
                       "[orange]▏cb p1[blank] [pink](This pastes clipboard 1.)[blank]\n"
                       "[orange]▏cb sh4[blank] [pink](This shows the contents of clipboard 4.)[blank]\n"
                       "[orange]▏cb clr[blank] [pink](This clears the contents of the default clipboard.)[blank]\n"
                       "[blue][bold]▏Configuration[blank]\n"
                       "[orange]▏CI: [pink]Set to make Clipboard overwrite existing items without a user prompt when pasting.[blank]\n"
                       "[orange]▏FORCE_COLOR: [pink]Set to make Clipboard always show color regardless of what you set NO_COLOR "
                       "to.[blank]\n"
                       "[orange]▏TMPDIR: [pink]Set to the directory that Clipboard (and other programs) will use to hold the items "
                       "you cut or copy into temporary clipboards.[blank]\n"
                       "[orange]▏CLIPBOARD_TMPDIR: [pink]Set to the directory that only Clipboard will use to hold the items you cut "
                       "or copy into temporary clipboards.[blank]\n"
                       "[orange]▏CLIPBOARD_PERSISTDIR: [pink]Set to the directory that only Clipboard will use to hold the items you "
                       "cut or copy into persistent clipboards.[blank]\n"
                       "[orange]▏CLIPBOARD_ALWAYS_PERSIST: [pink]Set to make Clipboard always use persistent clipboards.[blank]\n"
                       "[orange]▏CLIPBOARD_NOGUI: [pink]Set to disable GUI clipboard integration.[blank]\n"
                       "[orange]▏NO_COLOR: [pink]Set to make Clipboard not show color.[blank]\n"
                       "[blue]▏You can show this help screen anytime with [bold]clipboard -h[blank][blue], [bold]clipboard "
                       "--help[blank][blue], or[bold] clipboard help[blank][blue].\n"
                       "[blue]▏You can also get more help in our Discord server at [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                       "[blue]▏Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                       "[blue]▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to "
                       "redistribute it under certain conditions.[blank]\n";
Message check_clipboard_status_message = "[blue]• There are items in these clipboards:\n";
Message clipboard_item_one_contents_message = "[blue]• Here is the [bold]%i[blank][blue] item in clipboard [bold]%s[blank][blue]: [blank]\n";
Message clipboard_item_many_contents_message = "[blue]• Here are the [bold]%i[blank][blue] items in clipboard [bold]%s[blank][blue]: [blank]\n";
Message clipboard_item_too_many_contents_message = "[blue]• Here are the first [bold]%i[blank][blue] items in clipboard [bold]%s[blank][blue]: [blank]\n";
Message clipboard_text_contents_message = "[blue]• Here are the first [bold]%i[blank][blue] bytes in clipboard [bold]%s[blank][blue]: [blank]\n";
Message no_clipboard_contents_message = "[blue]• There is currently nothing in the clipboard.[blank]\n";
Message clipboard_action_prompt = "[pink]Add [bold]cut, copy, [blank][pink]or[bold] paste[blank][pink] to the end, "
                                  "like [bold]clipboard copy[blank][pink] to get started, or if you need help, try "
                                  "[bold]clipboard -h[blank][pink] to show the help screen.[blank]\n";
Message no_valid_action_message = "[red]❌ You did not specify a valid action ([bold]\"%s\"[blank][red]), or you forgot "
                                  "to include one. [pink]Try using or adding [bold]cut, copy, [blank][pink]or "
                                  "[bold]paste[blank][pink] instead, like [bold]clipboard copy.[blank]\n";
Message choose_action_items_message = "[red]❌ You need to choose something to %s.[pink] Try adding the items you want "
                                      "to %s to the end, like [bold]clipboard %s contacts.txt myprogram.cpp[blank]\n";
Message fix_redirection_action_message = "[red]❌ You can't use the [bold]%s[blank][red] action with redirection here. [pink]Try removing "
                                         "[bold]%s[blank][pink] or use [bold]%s[blank][pink] instead, like [bold]clipboard %s[blank][pink].\n";
Message redirection_no_items_message = "[red]❌ You can't specify items when you use redirection. [pink]Try removing "
                                       "the items that come after [bold]clipboard [action].\n";
Message paste_success_message = "[green]✅ Pasted successfully[blank]\n";
Message clipboard_failed_one_message = "[red]❌ Clipboard couldn't %s this item:[blank]\n";
Message clipboard_failed_many_message = "[red]❌ Clipboard couldn't %s these items:[blank]\n";
Message and_more_fails_message = "[red]▏ ...and [bold]%i[blank][red] more.[blank]\n";
Message and_more_items_message = "[blue]▏ ...and [bold]%i[blank][blue] more.[blank]\n";
Message fix_problem_message = "[pink]▏ See if you have the needed permissions, or\n"
                              "▏ try double-checking the spelling of the files or what directory you're in.[blank]\n";
Message not_enough_storage_message = "[red]❌ There won't be enough storage available to paste everything (%gMB to "
                                     "paste, %gMB available).[blank][pink] Try double-checking what items you've "
                                     "selected or delete some files to free up space.[blank]\n";
Message item_already_exists_message = "[yellow]🟡 The item [bold]%s[blank][yellow] already exists here. Do you want to "
                                      "replace it? [pink]Use [bold]all [blank][pink]to replace all existing, or "
                                      "[bold]skip[blank][pink] to replace nothing. [bold][(y)es/(n)o)/(a)ll/(s)kip] ";
Message bad_response_message = "[red]❌ Sorry, that wasn't a valid choice. Try again: [blank][pink][bold][(y)es/(n)o)] ";
Message working_message = "\r[yellow]🟡 %s... %llu%s %s[blank]";
Message cancelled_message = "[green]✅ Cancelled %s[blank]\n";
Message byte_success_message = "[green]✅ %s %llu bytes[blank]\n";
Message one_item_success_message = "[green]✅ %s one item[blank]\n";
Message many_files_success_message = "[green]✅ %s %lu files[blank]\n";
Message many_directories_success_message = "[green]✅ %s %lu directories[blank]\n";
Message one_file_one_directory_success_message = "[green]✅ %s one file and one directory[blank]\n";
Message one_file_many_directories_success_message = "[green]✅ %s one file and %lu directories[blank]\n";
Message many_files_one_directory_success_message = "[green]✅ %s %lu files and one directory[blank]\n";
Message many_files_many_directories_success_message = "[green]✅ %s %lu files and %lu directories[blank]\n";
Message internal_error_message = "[red]❌ Internal error: %s\n▏ This is probably a bug, or you might be lacking "
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

    help_message = "[blue]▏Esto es Clipboard %s (commit %s), el sistema para cortar, copiar y pegar adentro del "
                   "terminal.[blank]\n"
                   "[blue][bold]▏Cómo usar[blank]\n"
                   "[orange]▏clipboard cortar (cosa) [cosas][blank]\n"
                   "[orange]▏clipboard copiar (cosa) [cosas][blank]\n"
                   "[orange]▏clipboard pegar[blank]\n"
                   "[blue]▏Reemplaza \"cb\" por \"clipboard\" para que gastes menos tiempo.[blank]\n"
                   "[blue][bold]▏Ejemplos[blank]\n"
                   "[orange]▏clipboard copiar cosas.conf[blank]\n"
                   "[orange]▏cb cortar MisDocumentos.txt nuevacarpeta[blank]\n"
                   "[orange]▏cb pegar[blank]\n"
                   "[blue]▏Muestra este mensaje de ayudar en cualquier tiempo que quieras con [bold]clipboard "
                   "-h[blank][blue], [bold]clipboard --help[blank][blue] o[bold] clipboard help[blank][blue].\n"
                   "[blue]▏Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[blue]▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome "
                   "to redistribute it under certain conditions.[blank]\n";
    no_valid_action_message = "[red]❌ No especificaste ninguna acción válida o se te olvidó. [pink]Inténta usar o añadir [bold]cortar, "
                              "copiar o pegar[blank][pink] en su lugar, como [bold]clipboard copiar.[blank]\n";
    clipboard_item_many_contents_message = "[blue]• Aquí están las [bold]%i[blank][blue] cosas primeras del "
                                           "portapapeles [bold]%s[blank][blue]: [blank]\n";
    no_clipboard_contents_message = "[blue]• No hay nada en Clipboard en este momento.[blank]\n";
    clipboard_action_prompt = "[pink]Añade [bold]cortar, copiar [blank][pink]o[bold] pegar[blank][pink] al final, como "
                              "[bold]clipboard copiar[blank][pink] para comenzar, o si necesitas ayuda, haz "
                              "[bold]clipboard -h[blank][pink] para mostrar el mensaje de ayudar.[blank]\n";
    choose_action_items_message = "[red]❌ Necesitas escoger una cosa para %s.[pink] Inténta añadir las cosas que "
                                  "quieres %s al final, como [bold]clipboard %s contactos.txt miprograma.cpp[blank]\n";
    fix_redirection_action_message = "[red]❌ No se puede usar la acción [bold]%s[blank][red] con la redirección. [pink]Inténta sacar "
                                     "[bold]%s[blank][pink] o usa [bold]%s[blank][pink] en su lugar, como [bold]clipboard %s[blank][pink].\n";
    redirection_no_items_message = "[red]❌ No se pueden especificar las cosas con redirección. [pink]Inténta sacar las "
                                   "cosas que siguen [bold]clipboard [acción].\n";
    paste_success_message = "[green]✅ Pegó con éxito[blank]\n";
    clipboard_failed_many_message = "[red]❌ Clipboard no pudo %s estas cosas.[blank]\n";
    and_more_fails_message = "[red]▏ ...y [bold]%i[blank][red] más.[blank]\n";
    and_more_items_message = "[blue]▏ ...y [bold]%i[blank][blue] más.[blank]\n";
    fix_problem_message = "[pink]▏ Verífica si tengas los permisos necesarios, o\n"
                          "▏ vuelve a revisar el deletro de los archivos o la carpeta en que estás.[blank]\n";
    not_enough_storage_message = "[red]❌ No habrá espacio suficiente para pegar todas tus cosas (%gMB a pegar, %gMB "
                                 "disponible).[blank][pink] Vuelve a revisar las cosas que especificaste o saca "
                                 "algunas cosas para hacer más espacio.[blank]\n";
    byte_success_message = "[green]✅ %s %llu bytes[blank]\n";
    many_files_success_message = "[green]✅ %s %lu archivos[blank]\n";
    many_directories_success_message = "[green]✅ %s %lu carpetas[blank]\n";
    many_files_many_directories_success_message = "[green]✅ %s %lu archivos y %lu carpetas[blank]\n";
    internal_error_message = "[red]❌ Error internal: %s[blank]\n";
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

    help_message = "[blue]▏Este é Clipboard %s (commit %s), o sistema de recortar, copiar e colar para a linha de "
                   "comando.[blank]\n"
                   "[blue][bold]▏Como utilizar[blank]\n"
                   "[orange]▏clipboard recortar (item) [itens][blank]\n"
                   "[orange]▏clipboard copiar (item) [itens][blank]\n"
                   "[orange]▏clipboard colar[blank]\n"
                   "[blue] Você pode utilizar \"cb\" ao invés de \"clipboard\" para ganhar tempo.[blank]\n"
                   "[blue][bold]▏Exemplos[blank]\n"
                   "[orange]▏clipboard copiar ração.conf[blank]\n"
                   "[orange]▏cb recortar Códigos_de_Lançamento_de_Mísseis.txt pastadecontatos[blank]\n"
                   "[orange]▏cb colar[blank]\n"
                   "[blue]▏Você pode rever esta tela de instruções à qualquer momento com [bold]clipboard "
                   "-h[blank][blue], [bold]clipboard --help[blank][blue] ou[bold] clipboard help[blank][blue].\n"
                   "[blue]▏Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[blue]▏Este programa vem com ABSOLUTAMENTE NENHUMA GARANTIA. Este é um software livre, e você é "
                   "bem-vindo a redistribuí-lo sob certas condições.[blank]\n";
    no_valid_action_message = "[red]❌ Você não especificou uma ação válida (\"%s\"), ou esqueceu de incluí-la. [pink]Tente utilizar "
                              "[bold]recortar, copiar ou colar[blank][pink], como em [bold]clipboard copiar.[blank]\n";
    no_clipboard_contents_message = "[red]❌ Você não especificou uma ação válida. [pink]Tente adicionar [bold]recortar, copiar, or "
                                    "colar[blank][pink] no final, como em [bold]clipboard copiar[blank][pink]. Caso precise de ajuda, tente "
                                    "[bold]clipboard -h[blank][pink] para mostrar a tela de instruções.[blank]\n";
    choose_action_items_message = "[red]❌ Você precisa especificar algo para %s.[pink] Tenta adicionar os itens que você quer %s ao final, "
                                  "como em [bold]clipboard %s contatos.txt meuprograma.cpp[blank]\n";
    fix_redirection_action_message = "[red]❌ Você não pode [bold]%s[blank][red] com redirecionamento aqui. [pink]Tente remover "
                                     "[bold]%s[blank][pink] ou utilizar [bold]%s[blank][pink], como em [bold]clipboard %s[blank][pink].\n";
    redirection_no_items_message = "[red]❌ Você não pode especificar itens ao redirecionar. [pink]Tente remover itens "
                                   "que vêm após [bold]clipboard [action].\n";
    paste_success_message = "[green]✅ Colado com sucesso[blank]\n";
    clipboard_failed_many_message = "[red]❌ Clipboard não pôde %s esses itens.[blank]\n";
    and_more_fails_message = "[red]▏ ...e mais [bold]%i[blank][red].[blank]\n";
    and_more_items_message = "[blue]▏ ...e mais [bold]%i[blank][blue].[blank]\n";
    fix_problem_message = "[pink]▏ Veja se você possui as permissões necessárias, ou\n"
                          "▏ verifique a ortografia do arquivo ou diretório que voce está.[blank]\n";
    byte_success_message = "[green]✅ %s %llu bytes[blank]\n";
    many_files_success_message = "[green]✅ %s %lu arquivos[blank]\n";
    many_directories_success_message = "[green]✅ %s %lu diretórios[blank]\n";
    many_files_many_directories_success_message = "[green]✅ %s %lu arquivos e %lu diretórios[blank]\n";
    internal_error_message = "[red]❌ Erro interno: %s\n▏ Isso é provavelmente um bug.[blank]\n";
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

    help_message = "[blue]▏Clipboard %s (commit %s), komut satırı için, kesme, kopyalama ve yapıştırma sistemidir.[blank]\n"
                   "[blue][bold]▏Nasıl kullanılır[blank]\n"
                   "[orange]▏clipboard kes (öğe) [öğeler][blank] [pink](Bu öğe(leri) keser.)[blank]\n"
                   "[orange]▏clipboard kopyala (öğe) [öğeler][blank] [pink](Bu öğe(leri) kopyalar.)[blank]\n"
                   "[orange]▏clipboard yapistir[blank] [pink](Bu panodakileri yapıştırır.)[blank]\n"
                   "[orange]▏clipboard goster[blank] [pink](Bu panoda olan öğeleri gösterir.)[blank]\n"
                   "[orange]▏clipboard temizle[blank] [pink](Bu pano içerğini temizler.)[blank]\n"
                   "[blue]▏İşlemlerde uzun uzun yazarak zaman kaybetmemek için \"clipboard\" yerine \"cb\" kullanarak "
                   "kısaltabilirsiniz.[blank]\n"
                   "[blue]▏    Ben ise \"pano\" ismini kullanmanızı öneririm :)[blank]\n"
                   "[blue]▏Ayrıca kommutun sonuna bir sayı ekleyerek 10 farklı panodan birisini seçebilirsiniz.[blank]\n"
                   "[blue][bold]▏Örnekler[blank]\n"
                   "[orange]▏pano ks Nükleer_Fırlatma_Kodları.txt kişilerklasörü[blank] [pink](Bu verilen öğeleri öntanımlı "
                   "0. panoya keser)[blank]\n"
                   "[orange]▏pano kp1 mama.conf[blank] [pink](Bu verilen öğeleri 1. panoya kopyalar.)[blank]\n"
                   "[orange]▏pano y1[blank] [pink](Bu 1. panodakileri yapıştırır)[blank]\n"
                   "[orange]▏pano go4[blank] [pink](Bu 4. pano içeriğini gösterir, 4.)[blank]\n"
                   "[orange]▏pano tmz[blank] [pink](Bu öntanımlı panonun içeriğini temizler.)[blank]\n"
                   "[blue]▏Bu yardım ekranını herhangi bir zaman şu komutlardan birisiyle görebilirsiniz:[blank]\n"
                   "[blue]▏    [bold]clipboard -h[blank][blue], [bold]clipboard --help[blank][blue], ya da[bold] clipboard "
                   "help[blank][blue].\n"
                   "[blue]▏Discord sunucumuzdan daha fazla yardım alabilirsiniz: [bold]https://discord.gg/J6asnc3pEG[blank]\n"
                   "[blue]▏Copyright (C) 2023 Jackson Huff. Licensed under the GPLv3.[blank]\n"
                   "[blue]▏                                 GPLv3 altında lisanslanmıştır.[blank]\n"
                   "[blue]▏Bu program KESİNLİKLE HİÇBİR GARANTİ ile birlikte gelir. Bu ücretsiz bir yazılımdır ve belirli "
                   "koşullar altında yeniden dağıtabilirsiniz.[blank]\n";
    check_clipboard_status_message = "[blue]• Pano içeriği şunlardır:\n";
    clipboard_item_many_contents_message = "[blue]• [bold]%s[blank][blue] panoda bulunan ilk [bold]%i[blank][blue] öğe: [blank]\n";
    no_clipboard_contents_message = "[blue]• Panoda hiçbir şey yok.[blank]\n";
    clipboard_action_prompt = "[pink]Başlamak için sonuna [bold]kes, kopyala, [blank][pink]veya[bold] yapistir[blank][pink] ekleyin, "
                              "[bold]clipboard kopyala[blank][pink] gibi, veya yardıma ihtiyacın olursa yardım ekranını göstermek için "
                              "[bold]clipboard -h[blank][pink]'i dene.[blank]\n";
    no_valid_action_message = "[red]❌ Geçerli bir işlem vermediniz (\"%s\") veya işlem vermeyi unuttunuz [pink]Komutunuza [bold]cut, "
                              "copy, [blank][pink]ya da [bold]paste[blank][pink] eklemelisiniz, örneğin [bold]clipboard copy.[blank]\n";
    choose_action_items_message = "[red]❌ %s(ma/me) işlemi için bir öğe seçmeniz gerekmektedir.[pink] %s işleminden sonra öğeler eklemeyi "
                                  "deneyiniz, örneğin [bold]clipboard %s contacts.txt myprogram.cpp[blank]\n";
    fix_redirection_action_message = "[red]❌ [bold]%s[blank][red](ma/me) işlemini burada yeniden yönlendirme ile kullanamazsın. [pink] "
                                     "[bold]%s[blank][pink] işlemini silin veya [bold]%s[blank][pink] işlemini kullanın, örneğin "
                                     "[bold]clipboard %s[blank][pink].\n";
    redirection_no_items_message = "[red]❌ Yeniden yönlendirme işlemi yaparken öğe veremezsiniz. [pink][bold]clipboard "
                                   "[action][blank][pink]'dan sonra  gelen öğeleri siliniz.\n";
    paste_success_message = "[green]✅ Yapıştırma başarıyla tamamlandı[blank]\n";
    clipboard_failed_many_message = "[red]❌ %s(ma/me) işlemi şu öğeler için başarısız oldu:[blank]\n";
    and_more_fails_message = "[red]▏ ...ve [bold]%i[blank][red] fazla.[blank]\n";
    and_more_items_message = "[blue]▏ ...ve [bold]%i[blank][blue] fazla.[blank]\n";
    fix_problem_message = "[pink]▏ Erişime ihtiyacınız varsa şuna bakın, veya\n"
                          "▏ bulunduğunuz dizini veya girdiğiniz dosya isimlerini ikinci kez kontrol edin.[blank]\n";
    not_enough_storage_message = "[red]❌ Bütün öğelerinizi yapıştırabileceğin kadar yeterli bir alanınız yok (%gMB "
                                 "yapıştırılacak, %gMB boş).[blank][pink] Hangi öğeleri seçtiğinizi ikinci kez kontrol "
                                 "etmeyi deneyin veya yer açmak için bazı dosyaları silin.[blank]\n";
    byte_success_message = "[green]✅ %s %llu bayt[blank]\n";
    many_files_success_message = "[green]✅ %s %lu dosya[blank]\n";
    many_directories_success_message = "[green]✅ %s %lu dizin[blank]\n";
    many_files_many_directories_success_message = "[green]✅ %s %lu dosya ve %lu dizin[blank]\n";
    internal_error_message = "[red]❌ İçsel hata: %s\n▏ Bu yüksek ihtimal bir hata veya bu sistemde erişim sorunu yaşıyorsunuz.[blank]\n";
}
