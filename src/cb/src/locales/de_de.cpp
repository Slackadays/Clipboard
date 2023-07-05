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
    actions[Cut] = "";
    actions[Copy] = "";
    actions[Paste] = "";
    actions[Clear] = "";
    actions[Show] = "";
    actions[Edit] = "";
    actions[Add] = "";
    actions[Remove] = "";
    actions[Note] = "";
    actions[Swap] = "";
    actions[Status] = "";
    actions[Info] = "";
    actions[Load] = "";
    actions[Import] = "";
    actions[Export] = "";
    actions[History] = "";
    actions[Ignore] = "";
    actions[Search] = "";

    action_shortcuts[Cut] = "";
    action_shortcuts[Copy] = "";
    action_shortcuts[Paste] = "";
    action_shortcuts[Clear] = "";
    action_shortcuts[Show] = "";
    action_shortcuts[Edit] = "";
    action_shortcuts[Add] = "";
    action_shortcuts[Remove] = "";
    action_shortcuts[Note] = "";
    action_shortcuts[Swap] = "";
    action_shortcuts[Status] = "";
    action_shortcuts[Info] = "";
    action_shortcuts[Load] = "";
    action_shortcuts[Import] = "";
    action_shortcuts[Export] = "";
    action_shortcuts[History] = "";
    action_shortcuts[Ignore] = "";
    action_shortcuts[Search] = "";

    doing_action[Cut] = "";
    doing_action[Copy] = "";
    doing_action[Paste] = "";
    doing_action[Clear] = "";
    doing_action[Show] = "";
    doing_action[Edit] = "";
    doing_action[Add] = "";
    doing_action[Remove] = "";
    doing_action[Note] = "";
    doing_action[Swap] = "";
    doing_action[Status] = "";
    doing_action[Info] = "";
    doing_action[Load] = "";
    doing_action[Import] = "";
    doing_action[Export] = "";
    doing_action[History] = "";
    doing_action[Ignore] = "";
    doing_action[Search] = "";

    did_action[Cut] = "";
    did_action[Copy] = "";
    did_action[Paste] = "";
    did_action[Clear] = "";
    did_action[Show] = "";
    did_action[Edit] = "";
    did_action[Add] = "";
    did_action[Remove] = "";
    did_action[Note] = "";
    did_action[Swap] = "";
    did_action[Status] = "";
    did_action[Info] = "";
    did_action[Load] = "";
    did_action[Import] = "";
    did_action[Export] = "";
    did_action[History] = "";
    did_action[Ignore] = "";
    did_action[Search] = "";

    action_descriptions[Cut] = "";
    action_descriptions[Copy] = "";
    action_descriptions[Paste] = "";
    action_descriptions[Clear] = "";
    action_descriptions[Show] = "";
    action_descriptions[Edit] = "";
    action_descriptions[Add] = "";
    action_descriptions[Remove] = "";
    action_descriptions[Note] = "";
    action_descriptions[Swap] = "";
    action_descriptions[Status] = "";
    action_descriptions[Info] = "";
    action_descriptions[Load] = "";
    action_descriptions[Import] = "";
    action_descriptions[Export] = "";
    action_descriptions[History] = "";
    action_descriptions[Ignore] = "";
    action_descriptions[Search] = "";
}