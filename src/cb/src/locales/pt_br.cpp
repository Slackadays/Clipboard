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

void setLanguagePT() {
    actions[Cut] = "recortar";
    actions[Copy] = "copiar";
    actions[Paste] = "colar";
    actions[Clear] = "limpar";
    actions[Show] = "mostrar";
    actions[Edit] = "editar";
    actions[Add] = "adicionar";
    actions[Remove] = "remover";
    actions[Note] = "notar";
    actions[Swap] = "trocar";
    actions[Status] = "estado";
    actions[Info] = "info";
    actions[Load] = "carregar";
    actions[Import] = "importar";
    actions[Export] = "exportar";
    actions[History] = "historia";
    actions[Ignore] = "ignorar";
    actions[Search] = "buscar";

    doing_action[Cut] = "Recortando";
    doing_action[Copy] = "Copiando";
    doing_action[Paste] = "Colando";
    doing_action[Clear] = "Limpando";
    doing_action[Show] = "Mostrando";
    doing_action[Edit] = "Editando";
    doing_action[Add] = "Adicionando";
    doing_action[Remove] = "Removendo";
    doing_action[Note] = "Notando";
    doing_action[Swap] = "Trocando";
    doing_action[Status] = "Checando estado";
    doing_action[Info] = "Checando info";
    doing_action[Load] = "Carregando";
    doing_action[Import] = "Importando";
    doing_action[Export] = "Exportando";
    doing_action[History] = "Checando história";
    doing_action[Ignore] = "Ignorando";
    doing_action[Search] = "Buscando";

    did_action[Cut] = "Recortou";
    did_action[Copy] = "Copiou";
    did_action[Paste] = "Colou";
    did_action[Clear] = "Limpou";
    did_action[Show] = "Mostrou";
    did_action[Edit] = "Editou";
    did_action[Add] = "Adicionou";
    did_action[Remove] = "Removeu";
    did_action[Note] = "Notou";
    did_action[Swap] = "Trocado";
    did_action[Status] = "Checado estado";
    did_action[Info] = "Checado info";
    did_action[Load] = "Carregado";
    did_action[Import] = "Importado";
    did_action[Export] = "Exportado";
    did_action[History] = "Checado história";
    did_action[Ignore] = "Ignorado";
    did_action[Search] = "Buscado";

    action_descriptions[Cut] = "Recorta um item ou itens do clipboard.";
    action_descriptions[Copy] = "Copia um item ou itens para o clipboard.";
    action_descriptions[Paste] = "Cola o conteúdo do clipboard.";
    action_descriptions[Clear] = "Limpa o conteúdo do clipboard.";
    action_descriptions[Show] = "Mostra o conteúdo do clipboard.";
    action_descriptions[Edit] = "Edita um item ou itens do clipboard.";
    action_descriptions[Add] = "Adiciona um item ou itens ao clipboard.";
    action_descriptions[Remove] = "Remove um item ou itens do clipboard.";
    action_descriptions[Note] = "Adiciona uma nota a um item ou itens do clipboard.";
    action_descriptions[Swap] = "Troca a posição de dois itens do clipboard.";
    action_descriptions[Status] = "Checa o estado do clipboard.";
    action_descriptions[Info] = "Checa a informação do clipboard.";
    action_descriptions[Load] = "Carrega um arquivo para o clipboard.";
    action_descriptions[Import] = "Importa um arquivo para o clipboard.";
    action_descriptions[Export] = "Exporta um arquivo do clipboard.";
    action_descriptions[History] = "Checa a história do clipboard.";
    action_descriptions[Ignore] = "Ignora um item ou itens do clipboard.";
    action_descriptions[Search] = "Busca um item ou itens do clipboard.";

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