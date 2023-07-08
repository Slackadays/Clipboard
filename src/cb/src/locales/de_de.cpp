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

void setLanguageDE() {
    actions[Cut] = "Ausschneiden";
    actions[Copy] = "Kopieren";
    actions[Paste] = "Einfügen";
    actions[Clear] = "Leeren";
    actions[Show] = "Zeigen";
    actions[Edit] = "Bearbeiten";
    actions[Add] = "Hinzufügen";
    actions[Remove] = "Entfernen";
    actions[Note] = "Notiz";
    actions[Swap] = "Wechseln";
    actions[Status] = "Status";
    actions[Info] = "Info";
    actions[Load] = "Laden";
    actions[Import] = "Importieren";
    actions[Export] = "Exportieren";
    actions[History] = "Verlauf";
    actions[Ignore] = "Ignorieren";
    actions[Search] = "Suchen";

    action_shortcuts[Cut] = "Auss";
    action_shortcuts[Copy] = "Kopi";
    action_shortcuts[Paste] = "Einf";
    action_shortcuts[Clear] = "Leer";
    action_shortcuts[Show] = "Zeig";
    action_shortcuts[Edit] = "Bear";
    action_shortcuts[Add] = "Hinzf";
    action_shortcuts[Remove] = "Entf";
    action_shortcuts[Note] = "Not";
    action_shortcuts[Swap] = "Wech";
    action_shortcuts[Status] = "St";
    action_shortcuts[Info] = "In";
    action_shortcuts[Load] = "Lad";
    action_shortcuts[Import] = "Imp";
    action_shortcuts[Export] = "Exp";
    action_shortcuts[History] = "Ver";
    action_shortcuts[Ignore] = "Ign";
    action_shortcuts[Search] = "Such";

    doing_action[Cut] = "Schneidet aus";
    doing_action[Copy] = "Kopiert";
    doing_action[Paste] = "Fügt ein";
    doing_action[Clear] = "Leert";
    doing_action[Show] = "Zeigt";
    doing_action[Edit] = "Bearbeitet";
    doing_action[Add] = "Fügt hinzu";
    doing_action[Remove] = "Entfernt";
    doing_action[Note] = "Notiert";
    doing_action[Swap] = "Wechselt";
    doing_action[Status] = "Checkt Status";
    doing_action[Info] = "Zeigt Info";
    doing_action[Load] = "Läd";
    doing_action[Import] = "Importiert";
    doing_action[Export] = "Exportiert";
    doing_action[History] = "Lade Verlauf";
    doing_action[Ignore] = "Ignoriert";
    doing_action[Search] = "Suche";

    did_action[Cut] = "Ausgeschnitten";
    did_action[Copy] = "Kopiert";
    did_action[Paste] = "Eingefügt";
    did_action[Clear] = "Geleert";
    did_action[Show] = "Gezeigt";
    did_action[Edit] = "Bearbeitet";
    did_action[Add] = "Hinzugefügt";
    did_action[Remove] = "Entfernt";
    did_action[Note] = "Notiert";
    did_action[Swap] = "Gewechselt";
    did_action[Status] = "Status gecheckt";
    did_action[Info] = "Info gezeigt";
    did_action[Load] = "Geladen";
    did_action[Import] = "Importiert";
    did_action[Export] = "Exportiert";
    did_action[History] = "Verlauf geladen";
    did_action[Ignore] = "Ignoriert";
    did_action[Search] = "Gesucht";

    // action_descriptions[Cut] = "";
    action_descriptions[Copy] = "Kopiert Objekte in die Zwischenablage";
    // action_descriptions[Paste] = "";
    // action_descriptions[Clear] = "";
    // action_descriptions[Show] = "";
    // action_descriptions[Edit] = "";
    // action_descriptions[Add] = "";
    // action_descriptions[Remove] = "";
    // action_descriptions[Note] = "";
    // action_descriptions[Swap] = "";
    // action_descriptions[Status] = "";
    // action_descriptions[Info] = "";
    // action_descriptions[Load] = "";
    // action_descriptions[Import] = "";
    // action_descriptions[Export] = "";
    // action_descriptions[History] = "";
    // action_descriptions[Ignore] = "";
    // action_descriptions[Search] = "";
}
