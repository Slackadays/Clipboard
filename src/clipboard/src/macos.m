/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
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
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSString class], [NSAttributedString class], nil];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:[NSDictionary dictionary]]) {
        NSString *text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        return [text UTF8String];
    }
    return NULL;
}

char** fileContent() {
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSURL class], nil];
    if ([[NSPasteboard generalPasteboard] canReadObjectForClasses:classes options:[NSDictionary dictionary]]) {
        NSArray *files = [[NSPasteboard generalPasteboard] readObjectsForClasses:classes options:[NSDictionary dictionary]];
        int numberOfFiles = [files count];
        char** stringArray = malloc((numberOfFiles * sizeof(char*)) + 1);
        for (int i = 0; i < numberOfFiles; i++) {
            const char* filepath = [[[files objectAtIndex:i] path] UTF8String];
            char* Cfilepath = malloc(strlen(filepath) + 1);
            strcpy(Cfilepath, filepath);
            stringArray[i] = Cfilepath;
        }
        stringArray[numberOfFiles] = NULL;
        return stringArray;
    }
    return NULL;
}

void writeText(const char* text) {
    [[NSPasteboard generalPasteboard] clearContents];
    [[NSPasteboard generalPasteboard] setString:[NSString stringWithUTF8String:text] forType:NSPasteboardTypeString];
}

void writeFiles(const char** files) {
    [[NSPasteboard generalPasteboard] clearContents];
    NSMutableArray *fileArray = [[NSMutableArray alloc] init];
    for (int i = 0; files[i] != NULL; i++) {
        [fileArray addObject:[NSURL fileURLWithPath:[NSString stringWithUTF8String:files[i]]]];
    }
    [[NSPasteboard generalPasteboard] writeObjects:fileArray];
}