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

using enum Action;

void setLanguageTR() {
    actions[Cut] = "kes";
    actions[Copy] = "kopyala";
    actions[Paste] = "yapistir";
    actions[Clear] = "temizle";
    actions[Show] = "goster";
    actions[Edit] = "duzenle";
    actions[Add] = "ekle";
    actions[Remove] = "cikar";
    actions[Note] = "not";
    actions[Swap] = "degistir";
    actions[Status] = "durum";
    actions[Info] = "bilgi";
    actions[Load] = "yukle";
    actions[Import] = "aktar";
    actions[Export] = "daktar";
    actions[History] = "gecmis";
    actions[Ignore] = "yoksay";
    actions[Search] = "ara";

    action_shortcuts[Cut] = "ks";
    action_shortcuts[Copy] = "kp";
    action_shortcuts[Paste] = "y";
    action_shortcuts[Clear] = "tmz";
    action_shortcuts[Show] = "go";
    action_shortcuts[Edit] = "dzn";
    action_shortcuts[Add] = "ek";
    action_shortcuts[Remove] = "ck";
    action_shortcuts[Note] = "nt";
    action_shortcuts[Swap] = "dgt";
    action_shortcuts[Status] = "drm";
    action_shortcuts[Info] = "blg";
    action_shortcuts[Load] = "ylk";
    action_shortcuts[Import] = "akt";
    action_shortcuts[Export] = "dakt";
    action_shortcuts[History] = "gms";
    action_shortcuts[Ignore] = "ysy";
    action_shortcuts[Search] = "ara";

    doing_action[Cut] = "Kesiliyor";
    doing_action[Copy] = "Kopyalanıyor";
    doing_action[Paste] = "Yapıştırılıyor";
    doing_action[Clear] = "Temizleniyor";
    doing_action[Show] = "Gösteriliyor";
    doing_action[Edit] = "Düzenleniyor";
    doing_action[Add] = "Ekleniyor";
    doing_action[Remove] = "Çıkarılıyor";
    doing_action[Note] = "Not ekleniyor";
    doing_action[Swap] = "Değiştiriliyor";
    doing_action[Status] = "Durum gösteriliyor";
    doing_action[Info] = "Bilgi gösteriliyor";
    doing_action[Load] = "Yükleniyor";
    doing_action[Import] = "İçeri aktarılıyor";
    doing_action[Export] = "Dışarı aktarılıyor";
    doing_action[History] = "Geçmiş gösteriliyor";
    doing_action[Ignore] = "Yoksayılıyor";
    doing_action[Search] = "Aranıyor";

    did_action[Cut] = "Kesildi";
    did_action[Copy] = "Kopyalandı";
    did_action[Paste] = "Yapıştırıldı";
    did_action[Clear] = "Temizlendi";
    did_action[Show] = "Gösterildi";
    did_action[Edit] = "Düzenlendi";
    did_action[Add] = "Eklendi";
    did_action[Remove] = "Çıkarıldı";
    did_action[Note] = "Not eklendi";
    did_action[Swap] = "Değiştirildi";
    did_action[Status] = "Durum gösterildi";
    did_action[Info] = "Bilgi gösterildi";
    did_action[Load] = "Yüklendi";
    did_action[Import] = "İçeri aktarıldı";
    did_action[Export] = "Dışarı aktarıldı";
    did_action[History] = "Geçmiş gösterildi";
    did_action[Ignore] = "Yoksayıldı";
    did_action[Search] = "Aranıyor";

    action_descriptions[Cut] = "Panodaki öğeyi keser.";
    action_descriptions[Copy] = "Panodaki öğeyi kopyalar.";
    action_descriptions[Paste] = "Panoya öğe ekler.";
    action_descriptions[Clear] = "Panodaki öğeyi temizler.";
    action_descriptions[Show] = "Panodaki öğeleri gösterir.";
    action_descriptions[Edit] = "Panodaki öğeyi düzenler.";
    action_descriptions[Add] = "Panoya öğe ekler.";
    action_descriptions[Remove] = "Panodaki öğeyi çıkarır.";
    action_descriptions[Note] = "Panoya not ekler.";
    action_descriptions[Swap] = "Panodaki öğeleri değiştirir.";
    action_descriptions[Status] = "Panodaki öğelerin durumunu gösterir.";
    action_descriptions[Info] = "Panonun bilgilerini gösterir.";
    action_descriptions[Load] = "Panoyu yükler.";
    action_descriptions[Import] = "Panoya öğe ekler.";
    action_descriptions[Export] = "Panodaki öğeleri dışarı aktarır.";
    action_descriptions[History] = "Panonun geçmişini gösterir.";
    action_descriptions[Ignore] = "Panodaki öğeyi yoksayar.";
    action_descriptions[Search] = "Panodaki öğeleri arar.";

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
