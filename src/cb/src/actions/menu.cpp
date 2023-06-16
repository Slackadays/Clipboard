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
#include "../clipboard.hpp"

namespace PerformAction {

Action DisplayMenuAndGetAction() {
    auto available = thisTerminalSize();
    fprintf(stderr, "%s", formatMessage("[info]┍━┫ ").data());
    Message choose_action_message = "[info]Choose an action below[blank]";
    fprintf(stderr, "%s", choose_action_message().data());
    fprintf(stderr, "%s", formatMessage("[info] ┣").data());
    auto usedSpace = (choose_action_message.columnLength() - 2) + 9;
    if (usedSpace > available.columns) available.columns = usedSpace;
    int columns = available.columns - usedSpace;
    std::string bar1;
    for (int i = 0; i < columns; i++)
        bar1 += "━";
    fprintf(stderr, "%s%s", bar1.data(), formatMessage("┑[blank]\n").data());

    auto sortedActions = actions;
    std::sort(sortedActions.begin(), sortedActions.end());

    auto longestActionLength = (*std::max_element(sortedActions.begin(), sortedActions.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); })).size();

    auto longestShortcutLength = (*std::max_element(action_shortcuts.begin(), action_shortcuts.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); })).size();

    for (auto& action : sortedActions) {
        auto description = action_descriptions[static_cast<Action>(std::find(actions.begin(), actions.end(), action) - actions.begin())];
        auto shortcut = action_shortcuts[static_cast<Action>(std::find(actions.begin(), actions.end(), action) - actions.begin())];
        int widthRemaining = available.columns - (action.length() + 5 + longestActionLength);
        fprintf(stderr,
                formatMessage("[info]\033[%ldG│\r│ [bold]%*s%s, %*s%s[blank][info]│ [help]%s[blank]\n").data(),
                available.columns,
                longestActionLength - action.length(),
                "",
                action.data(),
                longestShortcutLength - shortcut.length(),
                "",
                shortcut.data(),
                description.data());
    }

    fprintf(stderr, "%s", formatMessage("[info]┕━┫ ").data());
    Message user_input_message = "[info]Your choice: ";
    fprintf(stderr, "%s", user_input_message().data());
    fprintf(stderr, formatMessage("%*s┣┙[bold]").data(), available.columns - (user_input_message.columnLength() + 6), "");
    fprintf(stderr, "\033[%ldG", user_input_message.columnLength() + 5);
    std::string input;
    std::getline(std::cin, input);
    fprintf(stderr, "%s", formatMessage("[blank]").data());

    auto action = getActionByFunction([&](const Action& action) { return actions[action] == input || action_shortcuts[action] == input; });
    if (!action) {
        fprintf(stderr, "%s", formatMessage("[error]❌ The action you entered doesn't exist.[help] 💡 Try entering the full name of the action, or its shortcut.[blank]\n").data());
        return DisplayMenuAndGetAction();
    }

    return action.value();
}

void menu() {
    stopIndicator();
    fprintf(stderr, "%s", formatMessage("[info]Welcome to CB![blank]\n").data());

    auto selectedAction = DisplayMenuAndGetAction();

    std::cout << "you selected: " << actions[selectedAction] << std::endl;
}

} // namespace PerformAction