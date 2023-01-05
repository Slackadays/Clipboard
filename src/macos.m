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
#include "macos.h"
#include <AppKit/AppKit.h>
#include <stdbool.h>

bool thisClipboardHoldsText() {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSString class], [NSAttributedString class], nil];
    NSDictionary *options = [NSDictionary dictionary];
    return [pasteboard canReadObjectForClasses:classes options:options];
}

char* getClipboardText() {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSString class], [NSAttributedString class], nil];
    NSDictionary *options = [NSDictionary dictionary];
    if ([pasteboard canReadObjectForClasses:classes options:options]) {
        //get the text from the clipboard
        NSString *text = [pasteboard stringForType:NSPasteboardTypeString];
        //convert the text to a C string
        const char *cString = [text UTF8String];
        //get the length of the C string
        int length = strlen(cString);
        //make a new C string
        char *newCString = malloc(length + 1);
        //copy the C string to the new C string
        strcpy(newCString, cString);
        return newCString;
    } else {
        return "";
    }
}

bool thisClipboardHoldsFiles() {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSURL class], nil];
    NSDictionary *options = [NSDictionary dictionary];
    return [pasteboard canReadObjectForClasses:classes options:options];
}

char** getClipboardFiles() {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSURL class], nil];
    NSDictionary *options = [NSDictionary dictionary];
    if ([pasteboard canReadObjectForClasses:classes options:options]) {
        NSArray *files = [pasteboard readObjectsForClasses:classes options:options];
        int numberOfFiles = [files count];
        char** newCStringArray = malloc(numberOfFiles * sizeof(char*));
        for (int i = 0; i < numberOfFiles; i++) {
            NSURL *file = [files objectAtIndex:i];
            const char* cString = [[file path] UTF8String];
            int length = strlen(cString);
            char* newCString = malloc(length + 1);
            strcpy(newCString, cString);
            newCStringArray[i] = newCString;
        }
        return newCStringArray;
    } else {
        return NULL;
    }
}