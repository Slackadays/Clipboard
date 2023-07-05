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

void setLanguageES() {
    actions[Cut] = "cortar";
    actions[Copy] = "copiar";
    actions[Paste] = "pegar";
    actions[Clear] = "quitar";
    actions[Show] = "mostrar";
    actions[Edit] = "editar";
    actions[Add] = "anadir";
    actions[Remove] = "sacar";
    actions[Note] = "notar";
    actions[Swap] = "cambiar";
    actions[Status] = "estado";
    actions[Info] = "info";
    actions[Load] = "cargar";
    actions[Import] = "importar";
    actions[Export] = "exportar";
    actions[History] = "historia";
    actions[Ignore] = "ignorar";
    actions[Search] = "buscar";

    action_shortcuts[Cut] = "ct";
    action_shortcuts[Copy] = "cp";
    action_shortcuts[Paste] = "p";
    action_shortcuts[Clear] = "qt";
    action_shortcuts[Show] = "ms";
    action_shortcuts[Edit] = "ed";
    action_shortcuts[Add] = "ad";
    action_shortcuts[Remove] = "rm";
    action_shortcuts[Note] = "nt";
    action_shortcuts[Swap] = "cm";
    action_shortcuts[Status] = "st";
    action_shortcuts[Info] = "in";
    action_shortcuts[Load] = "cg";
    action_shortcuts[Import] = "im";
    action_shortcuts[Export] = "ex";
    action_shortcuts[History] = "hs";
    action_shortcuts[Ignore] = "ig";
    action_shortcuts[Search] = "bs";

    doing_action[Cut] = "Cortando";
    doing_action[Copy] = "Copiando";
    doing_action[Paste] = "Pegando";
    doing_action[Clear] = "Quitando";
    doing_action[Show] = "Mostrando";
    doing_action[Edit] = "Editando";
    doing_action[Add] = "Añadiendo";
    doing_action[Remove] = "Sacando";
    doing_action[Note] = "Notando";
    doing_action[Swap] = "Cambiando";
    doing_action[Status] = "Mostrando estado";
    doing_action[Info] = "Mostrando información";
    doing_action[Load] = "Cargando";
    doing_action[Import] = "Importando";
    doing_action[Export] = "Exportando";
    doing_action[History] = "Mostrando historia";
    doing_action[Ignore] = "Ignorando";
    doing_action[Search] = "Buscando";

    did_action[Cut] = "Cortó";
    did_action[Copy] = "Copió";
    did_action[Paste] = "Pegó";
    did_action[Clear] = "Quitó";
    did_action[Show] = "Mostró";
    did_action[Edit] = "Editó";
    did_action[Add] = "Añadió";
    did_action[Remove] = "Sacó";
    did_action[Note] = "Notó";
    did_action[Swap] = "Cambio";
    did_action[Status] = "Mostró estado";
    did_action[Info] = "Mostró información";
    did_action[Load] = "Cargó";
    did_action[Import] = "Importó";
    did_action[Export] = "Exportó";
    did_action[History] = "Mostró historia";
    did_action[Ignore] = "Ignoró";
    did_action[Search] = "Buscó";

    action_descriptions[Cut] = "Corta un archivo o carpeta.";
    action_descriptions[Copy] = "Copia un archivo o carpeta.";
    action_descriptions[Paste] = "Pega un archivo o carpeta.";
    action_descriptions[Clear] = "Quita un portapapeles.";
    action_descriptions[Show] = "Muestra el contenido de un portapapeles.";
    action_descriptions[Edit] = "Edita el contenido de un portapapeles.";
    action_descriptions[Add] = "Añade cosas a un portapapeles.";
    action_descriptions[Remove] = "Saca cosas de un portapapeles.";
    action_descriptions[Note] = "Añade una nota a un portapapeles.";
    action_descriptions[Swap] = "Cambia dos portapapeles.";
    action_descriptions[Status] = "Muestra el estado de un portapapeles.";
    action_descriptions[Info] = "Muestra información sobre un portapapeles.";
    action_descriptions[Load] = "Carga un portapapeles a otro.";
    action_descriptions[Import] = "Importa un portapapeles a un archivo.";
    action_descriptions[Export] = "Exporta un portapapeles a un archivo.";
    action_descriptions[History] = "Muestra la historia de un portapapeles.";
    action_descriptions[Ignore] = "Ignora contenido en un portapapeles.";
    action_descriptions[Search] = "Busca contenido en un portapapeles.";

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