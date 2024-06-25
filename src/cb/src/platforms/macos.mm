/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    SPDX-License-Identifier: GPL-3.0-or-later
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
#import <AppKit/AppKit.h>
#import <string>
#import <vector>
#import <iostream>
#import <memory>
#import "../clipboard.hpp"

const bool GUIClipboardSupportsCut = false;

static std::vector<std::string> fileContent() {
    NSArray *classes = @[ [NSURL class] ];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:nil]) {
        NSArray *files = [[NSPasteboard generalPasteboard] readObjectsForClasses:classes options:nil];
        std::vector<std::string> stringArray;
        for (NSURL *fileURL in files) {
            stringArray.push_back([[fileURL path] UTF8String]);
        }
        return stringArray;
    }
    return {};
}

static std::string textContent() {
    NSArray *classes = @[ [NSString class], [NSAttributedString class] ];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:nil]) {
        NSString *text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        return [text UTF8String];
    }
    return "";
}

ClipboardContent getGUIClipboard(const std::string& requested_mime) {
    (void)requested_mime;
    auto files = fileContent();
    if (files.size() > 0) {
        std::vector<fs::path> fileVector;
        for (auto &file : files) {
            fileVector.push_back(file);
        }
        ClipboardPaths paths(fileVector);
        return ClipboardContent(paths);
    } else if (auto text = textContent(); text != "") {
        return ClipboardContent(text);
    }
    return ClipboardContent();
}

void writeToGUIClipboard(const ClipboardContent& clipboard) {
    [[NSPasteboard generalPasteboard] clearContents];
    if (clipboard.type() == ClipboardContentType::Text || clipboard.type() == ClipboardContentType::Binary) {
        [[NSPasteboard generalPasteboard] setString:@(clipboard.text().c_str()) forType:NSPasteboardTypeString];
    } else if (clipboard.type() == ClipboardContentType::Paths) {
        NSMutableArray *fileArray = [NSMutableArray new];
        for (auto const& path : clipboard.paths().paths()) {
            [fileArray addObject:[NSURL fileURLWithPath:@(path.c_str())]];
        }
        [[NSPasteboard generalPasteboard] writeObjects:fileArray];
    }
}

bool playAsyncSoundEffect(const std::valarray<short>& samples) {
    (void)samples;
    return false;
}