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
#include <AppKit/AppKit.h>

const char* textContent() {
    NSArray *classes = @[ [NSString class], [NSAttributedString class] ];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:nil]) {
        NSString *text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        char* textContent = strdup([text UTF8String]);
        return textContent;
    }
    return "";
}

char** fileContent() {
    NSArray *classes = @[ [NSURL class] ];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:nil]) {
        NSArray *files = [[NSPasteboard generalPasteboard] readObjectsForClasses:classes options:nil];
        int numberOfFiles = [files count];
        char** stringArray = malloc((numberOfFiles + 1) * sizeof(char*));
        for (unsigned i = 0; i < numberOfFiles; i++) {
            stringArray[i] = strdup([[files[i] path] UTF8String]);
        }
        stringArray[numberOfFiles] = NULL;
        return stringArray;
    }
    return NULL;
}

void clearContent() {
    [[NSPasteboard generalPasteboard] clearContents];
}

void writeText(const char* text) {
    [[NSPasteboard generalPasteboard] setString:@(text) forType:NSPasteboardTypeString];
}

void writeFiles(const char** files) {
    NSMutableArray *fileArray = [NSMutableArray new];
    for (int i = 0; files[i] != NULL; i++) {
        [fileArray addObject:[NSURL fileURLWithPath:@(files[i])]];
    }
    [[NSPasteboard generalPasteboard] writeObjects:fileArray];
}