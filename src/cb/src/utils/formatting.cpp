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

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__unix__)
struct termios tnormal;
#elif defined(_WIN32) || defined(_WIN64)
DWORD dwNormalMode = 0;
#endif

std::string formatColors(const std::string_view& oldStr, bool colorful) {
    std::string newStr;
    newStr.reserve(oldStr.size());
    for (size_t i = 0, lastAddedi = 0; i < oldStr.size(); i++) {
        while (i < oldStr.size() && oldStr[i] != '[')
            i++;

        newStr += oldStr.substr(lastAddedi, i - lastAddedi);

        if (i == oldStr.size()) break;

        auto j = oldStr.find(']', i + 1);
        if (j == std::string::npos) {
            newStr += '['; // no match, so just add the bracket
            break;
        }

        auto matches = [&](const std::string_view& key) {
            for (size_t k = 1; k < key.size() - 1; k++) // only compare the middle part
                if (key[k] != oldStr[i + k]) return false;
            return true;
        };

        bool matched = false;

        for (const auto& key : colors) {
            if (matches(key.first)) {
                if (colorful) newStr += key.second;
                i += key.first.length() - 1;
                matched = true;
                break;
            }
        }

        if (!matched) newStr += oldStr[i]; // no match, so just add the bracket

        lastAddedi = i + 1;
    }
    return newStr;
}

std::string makeControlCharactersVisible(const std::string_view& oldStr, size_t len) {
    std::string newStr;
    newStr.reserve(oldStr.size());

    if (len == 0) len = oldStr.size();

    // format characters such as \n and \r such that they appear as \n and \r surrounded by lightening terminal colors
    const std::array<std::pair<char, std::string_view>, 8> replacementCharacters {
            {{'\n', "\\n"}, {'\r', "\\r"}, {'\a', "\\a"}, {'\b', "\\b"}, {'\f', "\\f"}, {'\t', "\\t"}, {'\v', "\\v"}, {'\0', "\\0"}}};

    for (size_t i = 0; i < len - 1 && i < oldStr.size(); i++) {
        bool matched = false;
        for (const auto& [character, replacement] : replacementCharacters) {
            if (oldStr[i] == character) {
                newStr += "\033[2m" + std::string(replacement) + "\033[22m";
                matched = true;
                break;
            }
        }
        if (!matched) newStr += oldStr[i];
    }

    return newStr;
}

std::string JSONescape(const std::string_view& input) {
    std::string temp(input);

    for (size_t i = 0; i < temp.size(); i++) {
        switch (temp[i]) {
        case '"':
            temp.replace(i, 1, "\\\"");
            i++;
            break;
        case '\\':
            temp.replace(i, 1, "\\\\");
            i++;
            break;
        case '/':
            temp.replace(i, 1, "\\/");
            i++;
            break;
        case '\b':
            temp.replace(i, 1, "\\b");
            i++;
            break;
        case '\f':
            temp.replace(i, 1, "\\f");
            i++;
            break;
        case '\n':
            temp.replace(i, 1, "\\n");
            i++;
            break;
        case '\r':
            temp.replace(i, 1, "\\r");
            i++;
            break;
        case '\t':
            temp.replace(i, 1, "\\t");
            i++;
            break;
        default:
            if (temp[i] < 32) {
                std::stringstream ss;
                ss.imbue(std::locale::classic()); // disable locale formatting for numbers, so 1000 doesn't become 1,000
                ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)temp[i];
                temp.replace(i, 1, ss.str());
                i += 5;
            }
            break;
        }
    }

    return temp;
}

size_t columnLength(const std::string_view& message) {
    std::string temp(std::regex_replace(std::string(message), std::regex("[\\r\\n]|\\[[a-z]+\\]|\\\033\\[\\d+m"), ""));
    return temp.size() - std::count_if(temp.begin(), temp.end(), [](auto c) { return (c & 0xC0) == 0x80; }); // remove UTF-8 multibyte characters
}

std::string generatedEndbar() {
    static auto columns = thisTerminalSize().columns;
    return "\033[" + std::to_string(columns) + "G┃\r";
}

std::string repeatString(const std::string_view& character, const size_t& length) {
    std::string repeated;
    repeated.reserve(character.size() * length);
    for (int i = 0; i < length; i++)
        repeated += character;
    return repeated;
}

TerminalSize thisTerminalSize() {
    static TerminalSize temp(0, 0);
    if (temp.rows != 0 && temp.columns != 0) return temp;
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    temp = TerminalSize(csbi.srWindow.Bottom - csbi.srWindow.Top + 1, csbi.srWindow.Right - csbi.srWindow.Left + 1);
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    ioctl(STDERR_FILENO, TIOCGWINSZ, &w);
    temp = TerminalSize(w.ws_row, w.ws_col);
#endif
    if (temp.rows >= 5 && temp.columns >= 10) return temp;
    return TerminalSize(80, 24);
}

void makeTerminalRaw() {
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__unix__)
    struct termios tnew = tnormal;
    tnew.c_lflag &= ~(ICANON);
    tnew.c_lflag &= ~(ECHO);
    tnew.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tnew);
#elif defined(_WIN32) || defined(_WIN64)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), (dwNormalMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)));
#endif
}

void makeTerminalNormal() {
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__unix__)
    tcsetattr(STDIN_FILENO, TCSANOW, &tnormal);
#elif defined(_WIN32) || defined(_WIN64)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), dwNormalMode);
#endif
}

void setupTerminal() {
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__unix__)
    tcgetattr(STDIN_FILENO, &tnormal);
#elif defined(_WIN64) || defined(_WIN32)
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dwNormalMode);
#endif
}