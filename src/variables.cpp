/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
    Copyright (C) 2022 Jackson Huff and other contributors on GitHub.com
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
#include <filesystem>
#include <vector>
#include <string_view>
#include <string>
#include <array>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include "clipboard.hpp"

bool use_perma_clip = false;
bool use_safe_copy = true;
fs::path main_filepath;
fs::path temporary_filepath;
fs::path persistent_filepath;
fs::path original_files_path;
fs::path home_directory;
fs::copy_options opts = fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks;
std::vector<fs::path> items;
std::vector<std::pair<std::string, std::error_code>> failedItems;
std::string clipboard_name = "0";

std::condition_variable cv;
std::mutex m;
std::jthread indicator; //If this fails to compile, then you need C++20!

unsigned int output_length = 0;
std::atomic<unsigned long> files_success = 0;
std::atomic<unsigned long> directories_success = 0;
std::atomic<unsigned long long> bytes_success = 0;

bool stdin_is_tty = true;
bool stdout_is_tty = true;
bool stderr_is_tty = true;

std::array<std::pair<std::string_view, std::string_view>, 8> colors = {{
    {"{red}", "\033[38;5;196m"},
    {"{green}", "\033[38;5;40m"},
    {"{yellow}", "\033[38;5;214m"},
    {"{blue}", "\033[38;5;51m"},
    {"{orange}", "\033[38;5;208m"},
    {"{pink}", "\033[38;5;219m"},
    {"{bold}", "\033[1m"},
    {"{blank}", "\033[0m"}
}};

Action action;