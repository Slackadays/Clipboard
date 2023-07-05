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

void setLanguageFR() {
    actions[Cut] = "couper";
    actions[Copy] = "copier";
    actions[Paste] = "coller";
    actions[Clear] = "effacer";
    actions[Show] = "afficher";
    actions[Edit] = "editer";
    actions[Add] = "ajouter";
    actions[Remove] = "supprimer";
    actions[Note] = "noter";
    actions[Swap] = "echanger";
    actions[Status] = "statut";
    actions[Info] = "information";
    actions[Load] = "charger";
    actions[Import] = "importer";
    actions[Export] = "exporter";
    actions[History] = "historique";
    actions[Ignore] = "ignorer";
    actions[Search] = "rechercher";

    action_shortcuts[Cut] = "ct";
    action_shortcuts[Copy] = "cp";
    action_shortcuts[Paste] = "p";
    action_shortcuts[Clear] = "clr";
    action_shortcuts[Show] = "sh";
    action_shortcuts[Edit] = "ed";
    action_shortcuts[Add] = "ad";
    action_shortcuts[Remove] = "rm";
    action_shortcuts[Note] = "nt";
    action_shortcuts[Swap] = "sw";
    action_shortcuts[Status] = "st";
    action_shortcuts[Info] = "in";
    action_shortcuts[Load] = "ld";
    action_shortcuts[Import] = "imp";
    action_shortcuts[Export] = "ex";
    action_shortcuts[History] = "hs";
    action_shortcuts[Ignore] = "ig";
    action_shortcuts[Search] = "sr";

    doing_action[Cut] = "En train de couper";
    doing_action[Copy] = "En train de copier";
    doing_action[Paste] = "En train de coller";
    doing_action[Clear] = "Effacement";
    doing_action[Show] = "Affichage";
    doing_action[Edit] = "Edition";
    doing_action[Add] = "Ajout";
    doing_action[Remove] = "Suppression";
    doing_action[Note] = "En train de noter";
    doing_action[Swap] = "Echange";
    doing_action[Status] = "Vérification du statut";
    doing_action[Info] = "Affichage des informations";
    doing_action[Load] = "Chargement";
    doing_action[Import] = "Importation";
    doing_action[Export] = "Exportation";
    doing_action[History] = "Affichage de l'historique";
    doing_action[Ignore] = "Ignorer";
    doing_action[Search] = "Recherche";

    did_action[Cut] = "Coupé";
    did_action[Copy] = "Copié";
    did_action[Paste] = "Collé";
    did_action[Clear] = "Effacé";
    did_action[Show] = "Affiché";
    did_action[Edit] = "Edité";
    did_action[Add] = "Ajouté";
    did_action[Remove] = "Supprimé";
    did_action[Note] = "Noté";
    did_action[Swap] = "Echangé";
    did_action[Status] = "Statut vérifié";
    did_action[Info] = "Informations affichées";
    did_action[Load] = "Chargé";
    did_action[Import] = "Importé";
    did_action[Export] = "Exporté";
    did_action[History] = "Historique affiché";
    did_action[Ignore] = "Ignoré";
    did_action[Search] = "Recherché";

    action_descriptions[Cut] = "Couper des éléments dans un presse-papiers";
    action_descriptions[Copy] = "Copier des éléments dans un presse-papiers";
    action_descriptions[Paste] = "Coller des éléments depuis un presse-papiers";
    action_descriptions[Clear] = "Effacer un presse-papiers";
    action_descriptions[Show] = "Afficher le contenu d'un presse-papiers";
    action_descriptions[Edit] = "Editer le contenu d'un presse-papiers";
    action_descriptions[Add] = "Ajouter des éléments dans un presse-papiers";
    action_descriptions[Remove] = "Supprimer des éléments d'un presse-papiers";
    action_descriptions[Note] = "Ajouter une note à un presse-papiers";
    action_descriptions[Swap] = "Echanger le contenu de plusieurs presse-papiers";
    action_descriptions[Status] = "Afficher le statut d'un presse-papiers";
    action_descriptions[Info] = "Afficher les informations d'un presse-papiers";
    action_descriptions[Load] = "Charger un presse-papiers dans un autre presse-papiers";
    action_descriptions[Import] = "Importer un presse-papiers depuis un fichier";
    action_descriptions[Export] = "Exporter un presse-papiers dans un fichier";
    action_descriptions[History] = "Afficher l'historique d'un presse-papiers";
    action_descriptions[Ignore] = "Ignorer des types de contenu d'un presse-papiers";
    action_descriptions[Search] = "Rechercher des éléments dans un presse-papiers";
}
