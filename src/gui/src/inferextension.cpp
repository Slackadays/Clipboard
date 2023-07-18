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
#include <clipboard/gui.hpp>
#include <optional>
#include <string_view>

using namespace std::string_view_literals;

/* many of these signatures are automatically generated from https://www.garykessler.net/library/file_sigs.html by this python script:

import json

# Load the JSON array from file_sigs.json
with open("file_sigs.json", "r") as f:
    file_sigs = json.load(f)

# Convert each file signature to C++ code
cpp_code_array = []
for signature in file_sigs:
    cpp_code = ""
    cpp_code += "    // " + signature["File description"].lower() + "\n"
    cpp_code += '    if (header_is("' + "".join([f"\\x{c}" for c in signature["Header (hex)"].split()]) + '"sv'
    if signature["Header offset"] != '0':
        cpp_code += ', ' + signature["Header offset"]
    if signature["File extension"] != "(none)":
        # get first extension of format ONE|TWO|THREE, or just the extension if there is only one
        cpp_code += ')) return "' + signature["File extension"].split("|")[0].lower() + '";\n'
    else:
        cpp_code += ')) return "data";\n'
    cpp_code += "\n"
    cpp_code_array.append(cpp_code)

# Join all the C++ code together in reverse order (because the file signatures are sorted alphabetcially, which excludes longer codes from getting matched)
cpp_code = ""
for code in reversed(cpp_code_array):
    cpp_code += code

print(cpp_code)

*/

std::optional<std::string_view> inferFileExtension(const std::string_view& content) {
    auto header_is = [&](const std::string_view& pattern, const size_t offset = 0) {
        if (content.size() < (pattern.size() + offset)) return false;
        for (size_t i = 0; i < pattern.size(); i++) // DON'T use a substr comparison because the stdlib uses a 3-way comparison (super slow)
            if (content[i + offset] != pattern[i]) [[likely]]
                return false; // more likely to be false than true because there can only be one match out of hundreds
            else [[unlikely]]
                continue;
        return true;
    };

    // dos system driver
    if (header_is("\xFF\xFF\xFF\xFF"sv)) return "sys";

    // msinfo file
    if (header_is("\xFF\xFE\x23\x00\x6C\x00\x69\x00"sv)) return "mof";

    // utf-32-ucs-4 file
    if (header_is("\xFF\xFE\x00\x00"sv)) return "data";

    // utf-32-ucs-2 file
    if (header_is("\xFF\xFE"sv)) return "data";

    // windows registry file
    if (header_is("\xFF\xFE"sv)) return "reg";

    // mpeg-2 aac audio
    if (header_is("\xFF\xF9"sv)) return "aac";

    // mpeg-4 aac audio
    if (header_is("\xFF\xF1"sv)) return "aac";

    // jpeg-exif-spiff images
    if (header_is("\xFF\xD8\xFF"sv)) return "jfif";

    // generic jpeg image file
    if (header_is("\xFF\xD8"sv)) return "jpe";

    // wordperfect text and graphics
    if (header_is("\xFF\x57\x50\x43"sv)) return "wp";

    // keyboard driver file
    if (header_is("\xFF\x4B\x45\x59\x42\x20\x20\x20"sv)) return "sys";

    // windows international code page
    if (header_is("\xFF\x46\x4F\x4E\x54"sv)) return "cpi";

    // quickreport report
    if (header_is("\xFF\x0A\x00"sv)) return "qrp";

    // works for windows spreadsheet
    if (header_is("\xFF\x00\x02\x00\x04\x04\x05\x54"sv)) return "wks";

    // windows executable
    if (header_is("\xFF"sv)) return "sys";

    // utf-16-ucs-2 file
    if (header_is("\xFE\xFF"sv)) return "data";

    // symantex ghost image file
    if (header_is("\xFE\xEF"sv)) return "gho";

    // javakeystore
    if (header_is("\xFE\xED\xFE\xED"sv)) return "data";

    // os x abi mach-o binary (64-bit)
    if (header_is("\xFE\xED\xFA\xCF"sv)) return "data";

    // os x abi mach-o binary (32-bit)
    if (header_is("\xFE\xED\xFA\xCE"sv)) return "data";

    // powerpoint presentation subheader_6
    if (header_is("\xFD\xFF\xFF\xFF\x43\x00\x00\x00"sv, 512)) return "ppt";

    // excel spreadsheet subheader_7
    if (header_is("\xFD\xFF\xFF\xFF\x29"sv, 512)) return "xls";

    // excel spreadsheet subheader_6
    if (header_is("\xFD\xFF\xFF\xFF\x28"sv, 512)) return "xls";

    // excel spreadsheet subheader_5
    if (header_is("\xFD\xFF\xFF\xFF\x23"sv, 512)) return "xls";

    // excel spreadsheet subheader_4
    if (header_is("\xFD\xFF\xFF\xFF\x22"sv, 512)) return "xls";

    // developer studio subheader
    if (header_is("\xFD\xFF\xFF\xFF\x20"sv, 512)) return "opt";

    // excel spreadsheet subheader_3
    if (header_is("\xFD\xFF\xFF\xFF\x1F"sv, 512)) return "xls";

    // powerpoint presentation subheader_5
    if (header_is("\xFD\xFF\xFF\xFF\x1C\x00\x00\x00"sv, 512)) return "ppt";

    // excel spreadsheet subheader_2
    if (header_is("\xFD\xFF\xFF\xFF\x10"sv, 512)) return "xls";

    // powerpoint presentation subheader_4
    if (header_is("\xFD\xFF\xFF\xFF\x0E\x00\x00\x00"sv, 512)) return "ppt";

    // visual studio solution subheader
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "suo";

    // quickbooks portable company file
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "qbm";

    // microsoft outlook-exchange message
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "msg";

    // ms publisher file subheader
    if (header_is("\xFD\xFF\xFF\xFF\x02"sv, 512)) return "pub";

    // thumbs.db subheader
    if (header_is("\xFD\xFF\xFF\xFF"sv, 512)) return "db";

    // ms publisher subheader
    if (header_is("\xFD\x37\x7A\x58\x5A\x00"sv, 512)) return "pub";

    // xz archive
    if (header_is("\xFD\x37\x7A\x58\x5A\x00"sv)) return "xz";

    // bitcoin-qt blockchain block file
    if (header_is("\xF9\xBE\xB4\xD9"sv)) return "dat";

    // fat32 file allocation table_2
    if (header_is("\xF8\xFF\xFF\x0F\xFF\xFF\xFF\xFF"sv)) return "data";

    // fat32 file allocation table_1
    if (header_is("\xF8\xFF\xFF\x0F\xFF\xFF\xFF\x0F"sv)) return "data";

    // fat16 file allocation table
    if (header_is("\xF8\xFF\xFF\xFF"sv)) return "data";

    // fat12 file allocation table
    if (header_is("\xF0\xFF\xFF"sv)) return "data";

    // youtube timed text (subtitle) file
    if (header_is("\xEF\xBB\xBF\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E"sv)) return "ytt";

    // windows script component (utf-8)_2
    if (header_is("\xEF\xBB\xBF\x3C\x3F"sv)) return "wsc";

    // windows script component (utf-8)_1
    if (header_is("\xEF\xBB\xBF\x3C"sv)) return "wsf";

    // utf-8 file
    if (header_is("\xEF\xBB\xBF"sv)) return "data";

    // redhat package manager
    if (header_is("\xED\xAB\xEE\xDB"sv)) return "rpm";

    // word document subheader
    if (header_is("\xEC\xA5\xC1\x00"sv, 512)) return "doc";

    // bitlocker boot sector (win7)
    if (header_is("\xEB\x58\x90\x2D\x46\x56\x45\x2D"sv)) return "data";

    // bitlocker boot sector (vista)
    if (header_is("\xEB\x52\x90\x2D\x46\x56\x45\x2D"sv)) return "data";

    // gem raster file
    if (header_is("\xEB\x3C\x90\x2A"sv)) return "img";

    // windows executable file_3
    if (header_is("\xEB"sv)) return "com";

    // windows executable file_2
    if (header_is("\xE9"sv)) return "com";

    // windows executable file_1
    if (header_is("\xE8"sv)) return "com";

    // ms onenote note
    if (header_is("\xE4\x52\x5C\x7B\x8C\xD8\xA7\x4D"sv)) return "one";

    // win98 password file
    if (header_is("\xE3\x82\x85\x96"sv)) return "pwl";

    // amiga icon
    if (header_is("\xE3\x10\x00\x01\x00\x00\x00\x00"sv)) return "info";

    // efax file
    if (header_is("\xDC\xFE"sv)) return "efx";

    // corel color palette
    if (header_is("\xDC\xDC"sv)) return "cpl";

    // word 2.0 file
    if (header_is("\xDB\xA5\x2D\x00"sv)) return "doc";

    // windows graphics metafile
    if (header_is("\xD7\xCD\xC6\x9A"sv)) return "wmf";

    // windump (winpcap) capture file
    if (header_is("\xD4\xC3\xB2\xA1"sv)) return "data";

    // aol history|typed url files
    if (header_is("\xD4\x2A"sv)) return "arl";

    // winpharoah filter file
    if (header_is("\xD2\x0A\x00\x00"sv)) return "ftr";

    // msworks text document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "wps";

    // visio file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "vsd";

    // spss output file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "spo";

    // visual studio solution user options file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "sou";

    // revit project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "rvt";

    // ms publisher file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "pub";

    // developer studio file options file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "opt";

    // arcmap gis project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "mxd";

    // minitab data file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "mtw";

    // microsoft installer patch
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "msp";

    // microsoft installer package
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "msi";

    // microsoft common console document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "msc";

    // msworks database file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "db";

    // lotus-ibm approach 97 file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "apr";

    // access project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "adp";

    // caseware working papers
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "ac_";

    // microsoft office document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "doc";

    // os x abi mach-o binary (64-bit reverse)
    if (header_is("\xCF\xFA\xED\xFE"sv)) return "data";

    // outlook express e-mail folder
    if (header_is("\xCF\xAD\x12\xFE"sv)) return "dbx";

    // perfect office document
    if (header_is("\xCF\x11\xE0\xA1\xB1\x1A\xE1\x00"sv)) return "doc";

    // os x abi mach-o binary (32-bit reverse)
    if (header_is("\xCE\xFA\xED\xFE"sv)) return "data";

    // java cryptography extension keystore
    if (header_is("\xCE\xCE\xCE\xCE"sv)) return "jceks";

    // acronis true image_2
    if (header_is("\xCE\x24\xB9\xA2\x20\x00\x00\x00"sv)) return "tib";

    // nav quarantined virus file
    if (header_is("\xCD\x20\xAA\xAA\x02\x00\x00\x00"sv)) return "data";

    // nokia phone backup file
    if (header_is("\xCC\x52\x33\xFC\xE9\x2C\x18\x48\xAF\xE3\x36\x30\x1A\x39\x40\x06"sv)) return "nbu";

    // java bytecode
    if (header_is("\xCA\xFE\xBA\xBE"sv)) return "class";

    // jeppesen flitelog file
    if (header_is("\xC8\x00\x79\x00"sv)) return "lbk";

    // adobe encapsulated postscript
    if (header_is("\xC5\xD0\xD3\xC6"sv)) return "eps";

    // ms agent character file
    if (header_is("\xC3\xAB\xCD\xAB"sv)) return "acs";

    // palm desktop datebook
    if (header_is("\xBE\xBA\xFE\xCA\x0F\x50\x61\x6C\x6D\x53\x47\x20\x44\x61\x74\x61"sv)) return "dat";

    // ms write file_3
    if (header_is("\xBE\x00\x00\x00\xAB"sv)) return "wri";

    // installshield script
    if (header_is("\xB8\xC9\x0C\x00"sv)) return "ins";

    // windows calendar
    if (header_is("\xB5\xA2\xB0\xB3\xB3\xB0\xA5\xB5"sv)) return "cal";

    // acronis true image_1
    if (header_is("\xB4\x6E\x68\x44"sv)) return "tib";

    // pcx bitmap
    if (header_is("\xB1\x68\xDE\x3A"sv)) return "dcx";

    // win95 password file
    if (header_is("\xB0\x4D\x46\x43"sv)) return "pwl";

    // bgblitz position database file
    if (header_is("\xAC\xED\x00\x05\x73\x72\x00\x12"sv)) return "pdb";

    // java serialization data
    if (header_is("\xAC\xED"sv)) return "data";

    // powerpoint presentation subheader_3
    if (header_is("\xA0\x46\x1D\xF0"sv, 512)) return "ppt";

    // quicken data
    if (header_is("\xAC\x9E\xBD\x8F\x00\x00"sv)) return "qdf";

    // khronos texture file
    if (header_is("\xAB\x4B\x54\x58\x20\x31\x31\xBB\x0D\x0A\x1A\x0A"sv)) return "ktx";

    // access data ftk evidence
    if (header_is("\xA9\x0D\x00\x00\x00\x00\x00\x00"sv)) return "dat";

    // extended tcpdump (libpcap) capture file
    if (header_is("\xA1\xB2\xCD\x34"sv)) return "data";

    // tcpdump (libpcap) capture file
    if (header_is("\xA1\xB2\xC3\xD4"sv)) return "data";

    // outlook address file
    if (header_is("\x9C\xCB\xCB\x8D\x13\x75\xD2\x11"sv)) return "wab";

    // pgp public keyring
    if (header_is("\x99\x01"sv)) return "pkr";

    // gpg public keyring
    if (header_is("\x99"sv)) return "gpg";

    // jbog2 image file
    if (header_is("\x97\x4A\x42\x32\x0D\x0A\x1A\x0A"sv)) return "jb2";

    // pgp secret keyring_2
    if (header_is("\x95\x01"sv)) return "skr";

    // pgp secret keyring_1
    if (header_is("\x95\x00"sv)) return "skr";

    // hamarsoft compressed archive
    if (header_is("\x91\x33\x48\x46"sv)) return "hap";

    // ms answer wizard
    if (header_is("\x8A\x01\x09\x00\x00\x00\xE1\x08"sv)) return "aw";

    // png image
    if (header_is("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"sv)) return "png";

    // wordperfect text
    if (header_is("\x81\xCD\xAB"sv)) return "wpf";

    // outlook express address book (win95)
    if (header_is("\x81\x32\x84\xC1\x85\x05\xD0\x11"sv)) return "wab";

    // kodak cineon image
    if (header_is("\x80\x2A\x5F\xD7"sv)) return "cin";

    // dreamcast audio
    if (header_is("\x80\x00\x00\x20\x03\x12\x04"sv)) return "adx";

    // relocatable object code
    if (header_is("\x80"sv)) return "obj";

    // elf executable
    if (header_is("\x7F\x45\x4C\x46"sv)) return "data";

    // digital watchdog dw-tp-500g audio
    if (header_is("\x7E\x74\x2C\x01\x50\x70\x02\x4D\x52"sv)) return "img";

    // easy street draw diagram file
    if (header_is("\x7E\x45\x53\x44\x77\xF6\x85\x3E\xBF\x6A\xD2\x11\x45\x61\x73\x79\x20\x53\x74\x72\x65\x65\x74\x20\x44\x72\x61\x77"sv)) return "esd";

    // corel paint shop pro image
    if (header_is("\x7E\x42\x4B\x00"sv)) return "psp";

    // huskygram poem or singer embroidery
    if (header_is("\x7C\x4B\xC3\x74\xE1\xC8\x53\xA4\x79\xB9\x01\x1D\xFC\x4F\xDD\x13"sv)) return "csd";

    // rich text format
    if (header_is("\x7B\x5C\x72\x74\x66\x31"sv)) return "rtf";

    // ms winmobile personal note
    if (header_is("\x7B\x5C\x70\x77\x69"sv)) return "pwi";

    // google drive drawing link
    if (header_is("\x7B\x22\x75\x72\x6C\x22\x3A\x20\x22\x68\x74\x74\x70\x73\x3A\x2F"sv)) return "gdraw";

    // windows application log
    if (header_is("\x7B\x0D\x0A\x6F\x20"sv)) return "lgc";

    // zoombrowser image index
    if (header_is("\x7A\x62\x65\x78"sv)) return "info";

    // extensible archive file
    if (header_is("\x78\x61\x72\x21"sv)) return "xar";

    // macos x image file
    if (header_is("\x78\x01\x73\x0D\x62\x62\x60"sv)) return "dmg";

    // web open font format
    if (header_is("\x77\x4F\x46\x46"sv)) return "woff";

    // web open font format 2
    if (header_is("\x77\x4F\x46\x32"sv)) return "woff2";

    // qimage filter
    if (header_is("\x76\x32\x30\x30\x33\x2E\x31\x30"sv)) return "flt";

    // openexr bitmap image
    if (header_is("\x76\x2F\x31\x01"sv)) return "exr";

    // tape archive
    if (header_is("\x75\x73\x74\x61\x72"sv, 257)) return "tar";

    // truetype font
    if (header_is("\x74\x72\x75\x65\x00"sv)) return "ttf";

    // pathway map file
    if (header_is("\x74\x42\x4D\x50\x4B\x6E\x57\x72"sv, 60)) return "prc";

    // powerbasic debugger symbols
    if (header_is("\x73\x7A\x65\x7A"sv)) return "pdb";

    // cals raster bitmap
    if (header_is("\x73\x72\x63\x64\x6F\x63\x69\x64"sv)) return "cal";

    // stl (stereolithography) file
    if (header_is("\x73\x6F\x6C\x69\x64"sv)) return "stl";

    // palmos supermemo
    if (header_is("\x73\x6D\x5F"sv)) return "pdb";

    // allegro generic packfile (uncompressed)
    if (header_is("\x73\x6C\x68\x2E"sv)) return "dat";

    // allegro generic packfile (compressed)
    if (header_is("\x73\x6C\x68\x21"sv)) return "dat";

    // realmedia metafile
    if (header_is("\x72\x74\x73\x70\x3A\x2F\x2F"sv)) return "ram";

    // sonic foundry acid music file
    if (header_is("\x72\x69\x66\x66"sv)) return "ac";

    // winnt registry file
    if (header_is("\x72\x65\x67\x66"sv)) return "dat";

    // 1password 4 cloud keychain encrypted data
    if (header_is("\x6F\x70\x64\x61\x74\x61\x30\x31"sv)) return "data";

    // sms text (sim)
    if (header_is("\x6F\x3C"sv)) return "data";

    // multibit bitcoin wallet information
    if (header_is("\x6D\x75\x6C\x74\x69\x42\x69\x74\x2E\x69\x6E\x66\x6F"sv)) return "info";

    // internet explorer v11 tracking protection list
    if (header_is("\x6D\x73\x46\x69\x6C\x74\x65\x72\x4C\x69\x73\x74"sv)) return "tpl";

    // quicktime movie_6
    if (header_is("\x73\x6B\x69\x70"sv, 4)) return "mov";

    // quicktime movie_5
    if (header_is("\x70\x6E\x6F\x74"sv, 4)) return "mov";

    // quicktime movie_4
    if (header_is("\x77\x69\x64\x65"sv, 4)) return "mov";

    // quicktime movie_3
    if (header_is("\x6D\x64\x61\x74"sv, 4)) return "mov";

    // quicktime movie_2
    if (header_is("\x66\x72\x65\x65"sv, 4)) return "mov";

    // quicktime movie_1
    if (header_is("\x6D\x6F\x6F\x76"sv, 4)) return "mov";

    // skype user data file
    if (header_is("\x6C\x33\x33\x6C"sv)) return "dbb";

    // macos icon file
    if (header_is("\x69\x63\x6E\x73"sv)) return "icns";

    // win server 2003 printer spool file
    if (header_is("\x68\x49\x00\x00"sv)) return "shd";

    // gimp file
    if (header_is("\x67\x69\x6d\x70\x20\x78\x63\x66"sv)) return "xcf";

    // win2000-xp printer spool file
    if (header_is("\x67\x49\x00\x00"sv)) return "shd";

    // quicktime movie_7
    if (header_is("\x66\x74\x79\x70\x71\x74\x20\x20"sv, 4)) return "mov";

    // mpeg-4 video-quicktime file
    if (header_is("\x66\x74\x79\x70\x6D\x70\x34\x32"sv, 4)) return "m4v";

    // iso base media file (mpeg-4) v1
    if (header_is("\x66\x74\x79\x70\x69\x73\x6F\x6D"sv, 4)) return "mp4";

    // mpeg-4 video file_2
    if (header_is("\x66\x74\x79\x70\x4D\x53\x4E\x56"sv, 4)) return "mp4";

    // iso media-mpeg v4-itunes avc-lc
    if (header_is("\x66\x74\x79\x70\x4D\x34\x56\x20"sv, 4)) return "flv";

    // apple lossless audio codec file
    if (header_is("\x66\x74\x79\x70\x4D\x34\x41\x20"sv, 4)) return "m4a";

    // mpeg-4 video file_1
    if (header_is("\x66\x74\x79\x70\x33\x67\x70\x35"sv, 4)) return "mp4";

    // free lossless audio codec file
    if (header_is("\x66\x4C\x61\x43\x00\x00\x00\x22"sv)) return "flac";

    // winnt printer spool file
    if (header_is("\x66\x49\x00\x00"sv)) return "shd";

    // macintosh encrypted disk image (v2)
    if (header_is("\x65\x6E\x63\x72\x63\x64\x73\x61"sv)) return "dmg";

    // ms visual studio workspace file
    if (header_is("\x64\x73\x77\x66\x69\x6C\x65"sv)) return "dsw";

    // audacity audio file
    if (header_is("\x64\x6E\x73\x2E"sv)) return "au";

    // dalvik (android) executable file
    if (header_is("\x64\x65\x78\x0A"sv)) return "dex";

    // torrent file
    if (header_is("\x64\x38\x3A\x61\x6E\x6E\x6F\x75\x6E\x63\x65"sv)) return "torrent";

    // intel proset-wireless profile
    if (header_is("\x64\x00\x00\x00"sv)) return "p10";

    // photoshop custom shape
    if (header_is("\x63\x75\x73\x68\x00\x00\x00\x02"sv)) return "csh";

    // virtual pc hd image
    if (header_is("\x63\x6F\x6E\x65\x63\x74\x69\x78"sv)) return "vhd";

    // macintosh encrypted disk image (v1)
    if (header_is("\x63\x64\x73\x61\x65\x6E\x63\x72"sv)) return "dmg";

    // apple core audio file
    if (header_is("\x63\x61\x66\x66"sv)) return "caf";

    // binary property list (plist)
    if (header_is("\x62\x70\x6C\x69\x73\x74"sv)) return "data";

    // uuencoded base64 file
    if (header_is("\x62\x65\x67\x69\x6E\x2D\x62\x61\x73\x65\x36\x34"sv)) return "b64";

    // uuencoded file
    if (header_is("\x62\x65\x67\x69\x6E"sv)) return "data";

    // compressed archive file
    if (header_is("\x60\xEA"sv)) return "arj";

    // encase case file
    if (header_is("\x5F\x43\x41\x53\x45\x5F"sv)) return "cas";

    // jar archive
    if (header_is("\x5F\x27\xA8\x89"sv)) return "jar";

    // husqvarna designer
    if (header_is("\x5D\xFC\xC8\x00"sv)) return "hus";

    // lotus ami pro document_2
    if (header_is("\x5B\x76\x65\x72\x5D"sv)) return "sam";

    // winamp playlist
    if (header_is("\x5B\x70\x6C\x61\x79\x6C\x69\x73\x74\x5D"sv)) return "pls";

    // flight simulator aircraft configuration
    if (header_is("\x5B\x66\x6C\x74\x73\x69\x6D\x2E"sv)) return "cfg";

    // microsoft code page translation file
    if (header_is("\x5B\x57\x69\x6E\x64\x6F\x77\x73"sv)) return "cpx";

    // vocaltec voip media file
    if (header_is("\x5B\x56\x4D\x44\x5D"sv)) return "vmd";

    // lotus ami pro document_1
    if (header_is("\x5B\x56\x45\x52\x5D"sv)) return "sam";

    // dial-up networking file
    if (header_is("\x5B\x50\x68\x6F\x6E\x65\x5D"sv)) return "dun";

    // visual c++ workbench info file
    if (header_is("\x5B\x4D\x53\x56\x43"sv)) return "vcw";

    // ms exchange configuration file
    if (header_is("\x5B\x47\x65\x6E\x65\x72\x61\x6C"sv)) return "ecf";

    // macromedia shockwave flash
    if (header_is("\x5A\x57\x53"sv)) return "swf";

    // zoo compressed archive
    if (header_is("\x5A\x4F\x4F\x20"sv)) return "zoo";

    // ms publisher
    if (header_is("\x58\x54"sv)) return "bdr";

    // smpte dpx file (little endian)
    if (header_is("\x58\x50\x44\x53"sv)) return "dpx";

    // xpcom libraries
    if (header_is("\x58\x50\x43\x4F\x4D\x0A\x54\x79"sv)) return "xpt";

    // packet sniffer files
    if (header_is("\x58\x43\x50\x00"sv)) return "cap";

    // exchange e-mail
    if (header_is("\x58\x2D"sv)) return "eml";

    // lotus wordpro file
    if (header_is("\x57\x6F\x72\x64\x50\x72\x6F"sv)) return "lwp";

    // winzip compressed archive
    if (header_is("\x57\x69\x6E\x5A\x69\x70"sv, 29152)) return "zip";

    // wordstar for windows file
    if (header_is("\x57\x53\x32\x30\x30\x30"sv)) return "ws2";

    // walkman mp3 file
    if (header_is("\x57\x4D\x4D\x50"sv)) return "dat";

    // riff webp
    if (header_is("\x57\x45\x42\x50"sv, 8)) return "webp";

    // riff windows audio
    if (header_is("\x57\x41\x56\x45\x66\x6D\x74\x20"sv, 8)) return "wav";

    // spss template
    if (header_is("\x57\x04\x00\x00\x53\x50\x53\x53\x20\x74\x65\x6D\x70\x6C\x61\x74"sv)) return "sct";

    // mapinfo interchange format file
    if (header_is("\x56\x65\x72\x73\x69\x6F\x6E\x20"sv)) return "mif";

    // visual basic user-defined control file
    if (header_is("\x56\x45\x52\x53\x49\x4F\x4E\x20"sv)) return "ctl";

    // visual c precompiled header
    if (header_is("\x56\x43\x50\x43\x48\x30"sv)) return "pch";

    // measurement data format file
    if (header_is("\x55\x6E\x46\x69\x6E\x4D\x46"sv)) return "mf4";

    // ufo capture map file
    if (header_is("\x55\x46\x4F\x4F\x72\x62\x69\x74"sv)) return "dat";

    // ufa compressed archive
    if (header_is("\x55\x46\x41\xC6\xD2\xC1"sv)) return "ufa";

    // unicode extensions
    if (header_is("\x55\x43\x45\x58"sv)) return "uce";

    // gnu info reader file
    if (header_is("\x54\x68\x69\x73\x20\x69\x73\x20"sv)) return "info";

    // wii-gamecube
    if (header_is("\x54\x48\x50\x00"sv)) return "thp";

    // supercalc worksheet
    if (header_is("\x53\x75\x70\x65\x72\x43\x61\x6C"sv)) return "cal";

    // stuffit compressed archive
    if (header_is("\x53\x74\x75\x66\x66\x49\x74\x20"sv)) return "sit";

    // szdd file format
    if (header_is("\x53\x5A\x44\x44\x88\xF0\x27\x33"sv)) return "data";

    // qbasic szdd file
    if (header_is("\x53\x5A\x20\x88\xF0\x27\x33\xD1"sv)) return "data";

    // db2 conversion file
    if (header_is("\x53\x51\x4C\x4F\x43\x4F\x4E\x56"sv)) return "cnv";

    // sqlite database file
    if (header_is("\x53\x51\x4C\x69\x74\x65\x20\x66\x6F\x72\x6D\x61\x74\x20\x33\x00"sv)) return "db";

    // multibit bitcoin blockchain file
    if (header_is("\x53\x50\x56\x42"sv)) return "spvb";

    // storagecraft shadownprotect backup file
    if (header_is("\x53\x50\x46\x49\x00"sv)) return "spf";

    // smartdraw drawing file
    if (header_is("\x53\x4D\x41\x52\x54\x44\x52\x57"sv)) return "sdr";

    // stuffit archive
    if (header_is("\x53\x49\x54\x21\x00"sv)) return "sit";

    // flexible image transport system (fits) file
    if (header_is("\x53\x49\x4D\x50\x4C\x45\x20\x20\x3D\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x54"sv)) return "fits";

    // sietronics cpi xrd document
    if (header_is("\x53\x49\x45\x54\x52\x4F\x4E\x49"sv)) return "cpi";

    // harvard graphics presentation
    if (header_is("\x53\x48\x4F\x57"sv)) return "shw";

    // smpte dpx (big endian)
    if (header_is("\x53\x44\x50\x58"sv)) return "sdpx";

    // img software bitmap
    if (header_is("\x53\x43\x4D\x49"sv)) return "img";

    // underground audio
    if (header_is("\x53\x43\x48\x6C"sv)) return "ast";

    // windows prefetch
    if (header_is("\x53\x43\x43\x41"sv, 4)) return "pf";

    // generic e-mail_1
    if (header_is("\x52\x65\x74\x75\x72\x6E\x2D\x50"sv)) return "eml";

    // winrar compressed archive
    if (header_is("\x52\x61\x72\x21\x1A\x07\x00"sv)) return "rar";

    // winnt netmon capture file
    if (header_is("\x52\x54\x53\x53"sv)) return "cap";

    // riff windows midi
    if (header_is("\x52\x4D\x49\x44\x64\x61\x74\x61"sv, 8)) return "rmi";

    // resource interchange file format
    if (header_is("\x52\x49\x46\x46"sv)) return "avi";

    // 4x movie video
    if (header_is("\x52\x49\x46\x46"sv)) return "4xm";

    // micrografx designer graphic
    if (header_is("\x52\x49\x46\x46"sv)) return "ds4";

    // video cd mpeg movie
    if (header_is("\x52\x49\x46\x46"sv)) return "dat";

    // coreldraw document
    if (header_is("\x52\x49\x46\x46"sv)) return "cdr";

    // corel presentation exchange metadata
    if (header_is("\x52\x49\x46\x46"sv)) return "cmx";

    // windows animated cursor
    if (header_is("\x52\x49\x46\x46"sv)) return "ani";

    // antenna data file
    if (header_is("\x52\x45\x56\x4E\x55\x4D\x3A\x2C"sv)) return "ad";

    // winnt registry-registry undo files
    if (header_is("\x52\x45\x47\x45\x44\x49\x54"sv)) return "reg";

    // r saved work space
    if (header_is("\x52\x44\x58\x32\x0A"sv)) return "rdata";

    // shareaza (p2p) thumbnail
    if (header_is("\x52\x41\x5A\x41\x54\x44\x42\x31"sv)) return "dat";

    // outlook-exchange message subheader
    if (header_is("\x52\x00\x6F\x00\x6F\x00\x74\x00\x20\x00\x45\x00\x6E\x00\x74\x00\x72\x00\x79\x00"sv, 512)) return "msg";

    // quicken data file
    if (header_is("\x51\x57\x20\x56\x65\x72\x2E\x20"sv)) return "abd";

    // riff qualcomm purevoice
    if (header_is("\x51\x4C\x43\x4D\x66\x6D\x74\x20"sv, 8)) return "qcp";

    // qcow disk image
    if (header_is("\x51\x46\x49"sv)) return "qemu";

    // quicken data
    if (header_is("\x51\x45\x4C\x20"sv, 92)) return "qel";

    // parrot video encapsulation
    if (header_is("\x50\x61\x56\x45"sv)) return "data";

    // puffer encrypted archive
    if (header_is("\x50\x55\x46\x58"sv)) return "puf";

    // dreamcast sound format
    if (header_is("\x50\x53\x46\x12"sv)) return "dsf";

    // microsoft windows user state migration tool
    if (header_is("\x50\x4D\x4F\x43\x43\x4D\x4F\x43"sv)) return "pmoccmoc";

    // norton disk doctor undo file
    if (header_is("\x50\x4E\x43\x49\x55\x4E\x44\x4F"sv)) return "dat";

    // windows program manager group file
    if (header_is("\x50\x4D\x43\x43"sv)) return "grp";

    // pksfx self-extracting archive
    if (header_is("\x50\x4B\x53\x70\x58"sv, 526)) return "zip";

    // pklite archive
    if (header_is("\x50\x4B\x4C\x49\x54\x45"sv, 30)) return "zip";

    // pkzip archive_3
    if (header_is("\x50\x4B\x07\x08"sv)) return "zip";

    // pkzip archive_2
    if (header_is("\x50\x4B\x05\x06"sv)) return "zip";

    // java archive_2
    if (header_is("\x50\x4B\x03\x04\x14\x00\x08\x00"sv)) return "jar";

    // ms office 2007 documents
    if (header_is("\x50\x4B\x03\x04\x14\x00\x06\x00"sv)) return "docx";

    // zlock pro encrypted zip
    if (header_is("\x50\x4B\x03\x04\x14\x00\x01\x00"sv)) return "zip";

    // open publication structure ebook
    if (header_is("\x50\x4B\x03\x04\x0A\x00\x02\x00"sv)) return "epub";

    // exact packager models
    if (header_is("\x50\x4B\x03\x04"sv)) return "xpt";

    // xml paper specification file
    if (header_is("\x50\x4B\x03\x04"sv)) return "xps";

    // mozilla browser archive
    if (header_is("\x50\x4B\x03\x04"sv)) return "xpi";

    // windows media compressed skin file
    if (header_is("\x50\x4B\x03\x04"sv)) return "wmz";

    // staroffice spreadsheet
    if (header_is("\x50\x4B\x03\x04"sv)) return "sxc";

    // openoffice documents
    if (header_is("\x50\x4B\x03\x04"sv)) return "sxc";

    // microsoft open xml paper specification
    if (header_is("\x50\x4B\x03\x04"sv)) return "oxps";

    // opendocument template
    if (header_is("\x50\x4B\x03\x04"sv)) return "odt";

    // kword document
    if (header_is("\x50\x4B\x03\x04"sv)) return "kwd";

    // google earth session file
    if (header_is("\x50\x4B\x03\x04"sv)) return "kmz";

    // java archive_1
    if (header_is("\x50\x4B\x03\x04"sv)) return "jar";

    // ms office open xml format document
    if (header_is("\x50\x4B\x03\x04"sv)) return "docx";

    // macos x dashboard widget
    if (header_is("\x50\x4B\x03\x04"sv)) return "zip";

    // android package
    if (header_is("\x50\x4B\x03\x04"sv)) return "apk";

    // pkzip archive_1
    if (header_is("\x50\x4B\x03\x04"sv)) return "zip";

    // chromagraph graphics card bitmap
    if (header_is("\x50\x49\x43\x54\x00\x08"sv)) return "img";

    // pgp disk image
    if (header_is("\x50\x47\x50\x64\x4D\x41\x49\x4E"sv)) return "pgd";

    // pestpatrol data-scan strings
    if (header_is("\x50\x45\x53\x54"sv)) return "dat";

    // pax password protected bitmap
    if (header_is("\x50\x41\x58"sv)) return "pax";

    // windows memory dump
    if (header_is("\x50\x41\x47\x45\x44\x55"sv)) return "dmp";

    // quake archive file
    if (header_is("\x50\x41\x43\x4B"sv)) return "pak";

    // portable graymap graphic
    if (header_is("\x50\x35\x0A"sv)) return "pgm";

    // quicken quickfinder information file
    if (header_is("\x50\x00\x00\x00\x20\x00\x00\x00"sv)) return "idx";

    // visio-displaywrite 4 text file
    if (header_is("\x4F\x7B"sv)) return "dw4";

    // ogg vorbis codec compressed file
    if (header_is("\x4F\x67\x67\x53\x00\x02\x00\x00"sv)) return "oga";

    // opentype font
    if (header_is("\x4F\x54\x54\x4F\x00"sv)) return "otf";

    // psion series 3 database
    if (header_is("\x4F\x50\x4C\x44\x61\x74\x61\x62"sv)) return "dbf";

    // 1password 4 cloud keychain
    if (header_is("\x4F\x50\x43\x4C\x44\x41\x54"sv)) return "attachment";

    // agent newsreader character map
    if (header_is("\x4E\x61\x6D\x65\x3A\x20"sv)) return "cod";

    // national imagery transmission format file
    if (header_is("\x4E\x49\x54\x46\x30"sv)) return "ntf";

    // nes sound file
    if (header_is("\x4E\x45\x53\x4D\x1A\x01"sv)) return "nsf";

    // ms windows journal
    if (header_is("\x4E\x42\x2A\x00"sv)) return "jnt";

    // tomtom traffic data
    if (header_is("\x4E\x41\x56\x54\x52\x41\x46\x46"sv)) return "dat";

    // vmapsource gps waypoint database
    if (header_is("\x4D\x73\x52\x63\x66"sv)) return "gdb";

    // windows media player playlist
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x57\x69\x6E\x64\x6F\x77\x73\x20\x4D\x65\x64\x69\x61\x20\x50\x6C\x61\x79\x65\x72\x20\x2D\x2D\x20"sv, 84)) return "wpl";

    // visual studio .net file
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x56\x69\x73\x75\x61\x6C"sv)) return "sln";

    // ms c++ debugging symbols file
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x43\x2F\x43\x2B\x2B\x20"sv)) return "pdb";

    // zonealam data file
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00\x04\x00\x00\x00\xFF\xFF"sv)) return "zap";

    // audition graphic filter
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "flt";

    // directshow filter
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "ax";

    // acrobat plug-in
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "api";

    // windows virtual device drivers
    if (header_is("\x4D\x5A"sv)) return "vxd";

    // visualbasic application
    if (header_is("\x4D\x5A"sv)) return "vbx";

    // screen saver
    if (header_is("\x4D\x5A"sv)) return "scr";

    // ole object library
    if (header_is("\x4D\x5A"sv)) return "olb";

    // activex-ole custom control
    if (header_is("\x4D\x5A"sv)) return "ocx";

    // font file
    if (header_is("\x4D\x5A"sv)) return "fon";

    // control panel application
    if (header_is("\x4D\x5A"sv)) return "cpl";

    // library cache file
    if (header_is("\x4D\x5A"sv)) return "ax";

    // ms audio compression manager driver
    if (header_is("\x4D\x5A"sv)) return "acm";

    // windows-dos executable file
    if (header_is("\x4D\x5A"sv)) return "com";

    // milestones project management file_2
    if (header_is("\x4D\x56\x32\x43"sv)) return "mls";

    // milestones project management file_1
    if (header_is("\x4D\x56\x32\x31\x34"sv)) return "mls";

    // cd stomper pro label file
    if (header_is("\x4D\x56"sv)) return "dsn";

    // yamaha piano
    if (header_is("\x4D\x54\x68\x64"sv)) return "pcs";

    // midi sound file
    if (header_is("\x4D\x54\x68\x64"sv)) return "mid";

    // sony compressed voice file
    if (header_is("\x4D\x53\x5F\x56\x4F\x49\x43\x45"sv)) return "cdr";

    // microsoft windows imaging format
    if (header_is("\x4D\x53\x57\x49\x4D"sv)) return "wim";

    // health level-7 data (pipe delimited) file
    if (header_is("\xD\x53\x48\x7C\x5E\x7E\x5C\x26\x7C"sv)) return "hl7";

    // ole-spss-visual c++ library file
    if (header_is("\x4D\x53\x46\x54\x02\x00\x01\x00"sv)) return "tlb";

    // ms access snapshot viewer file
    if (header_is("\x4D\x53\x43\x46"sv)) return "snp";

    // powerpoint packaged presentation
    if (header_is("\x4D\x53\x43\x46"sv)) return "ppz";

    // onenote package
    if (header_is("\x4D\x53\x43\x46"sv)) return "onepkg";

    // microsoft cabinet file
    if (header_is("\x4D\x53\x43\x46"sv)) return "cab";

    // vmware bios state file
    if (header_is("\x4D\x52\x56\x4E"sv)) return "nvram";

    // yamaha synthetic music mobile application format
    if (header_is("\x4D\x4D\x4D\x44\x00\x00"sv)) return "mmf";

    // tiff file_4
    if (header_is("\x4D\x4D\x00\x2B"sv)) return "tif";

    // tiff file_3
    if (header_is("\x4D\x4D\x00\x2A"sv)) return "tif";

    // skype localization data file
    if (header_is("\x4D\x4C\x53\x57"sv)) return "mls";

    // milestones project management file
    if (header_is("\x4D\x49\x4C\x45\x53"sv)) return "mls";

    // windows dump file
    if (header_is("\x4D\x44\x4D\x50\x93\xA7"sv)) return "dmp";

    // targetexpress target file
    if (header_is("\x4D\x43\x57\x20\x54\x65\x63\x68\x6E\x6F\x67\x6F\x6C\x69\x65\x73"sv)) return "mte";

    // mar compressed archive
    if (header_is("\x4D\x41\x72\x30\x00"sv)) return "mar";

    // matlab v5 workspace
    if (header_is("\x4D\x41\x54\x4C\x41\x42\x20\x35\x2E\x30\x20\x4D\x41\x54\x2D\x66\x69\x6C\x65"sv)) return "mat";

    // microsoft-msn marc archive
    if (header_is("\x4D\x41\x52\x43"sv)) return "mar";

    // mozilla archive
    if (header_is("\x4D\x41\x52\x31\x00"sv)) return "mar";

    // merriam-webster pocket dictionary
    if (header_is("\x4D\x2D\x57\x20\x50\x6F\x63\x6B"sv)) return "pdb";

    // logical file evidence format
    if (header_is("\x4C\x56\x46\x09\x0D\x0A\xFF\x00"sv)) return "e01";

    // deluxepaint animation
    if (header_is("\x4C\x50\x46\x20\x00\x01"sv)) return "anm";

    // ea interchange format file (iff)_2
    if (header_is("\x4C\x49\x53\x54"sv)) return "iff";

    // windows help file_3
    if (header_is("\x4C\x4E\x02\x00"sv)) return "gid";

    // tajima emboridery
    if (header_is("\x4C\x41\x3A"sv)) return "dst";

    // ms coff relocatable object code
    if (header_is("\x4C\x01"sv)) return "obj";

    // windows shortcut file
    if (header_is("\x4C\x00\x00\x00\x01\x14\x02\x00"sv)) return "lnk";

    // kwaj (compressed) file
    if (header_is("\x4B\x57\x41\x4A\x88\xF0\x27\xD1"sv)) return "data";

    // win9x printer spool file
    if (header_is("\x4B\x49\x00\x00"sv)) return "shd";

    // kgb archive
    if (header_is("\x4B\x47\x42\x5F\x61\x72\x63\x68"sv)) return "kgb";

    // vmware 4 virtual disk
    if (header_is("\x4B\x44\x4D"sv)) return "vmdk";

    // aol art file_2
    if (header_is("\x4A\x47\x04\x0E"sv)) return "jg";

    // aol art file_1
    if (header_is("\x4A\x47\x03\x0E"sv)) return "jg";

    // jarcs compressed archive
    if (header_is("\x4A\x41\x52\x43\x53\x00"sv)) return "jar";

    // inter@ctive pager backup (blackberry file
    if (header_is("\x49\x6E\x74\x65\x72\x40\x63\x74\x69\x76\x65\x20\x50\x61\x67\x65"sv)) return "ipd";

    // inno setup uninstall log
    if (header_is("\x49\x6E\x6E\x6F\x20\x53\x65\x74"sv)) return "dat";

    // ms compiled html help file
    if (header_is("\x49\x54\x53\x46"sv)) return "chi";

    // ms reader ebook
    if (header_is("\x49\x54\x4F\x4C\x49\x54\x4C\x53"sv)) return "lit";

    // install shield compressed file
    if (header_is("\x49\x53\x63\x28"sv)) return "cab";

    // windows 7 thumbnail_2
    if (header_is("\x49\x4D\x4D\x4D\x15\x00\x00\x00"sv)) return "db";

    // tiff file_2
    if (header_is("\x49\x49\x2A\x00"sv)) return "tif";

    // canon raw file
    if (header_is("\x49\x49\x1A\x00\x00\x00\x48\x45"sv)) return "crw";

    // sprint music store audio
    if (header_is("\x49\x44\x33\x03\x00\x00\x00"sv)) return "koz";

    // mp3 audio file
    if (header_is("\x49\x44\x33"sv)) return "mp3";

    // tiff file_1
    if (header_is("\x49\x20\x49"sv)) return "tif";

    // harvard graphics presentation file
    if (header_is("\x48\x48\x47\x42\x31"sv)) return "sh3";

    // sas transport dataset
    if (header_is("\x48\x45\x41\x44\x45\x52\x20\x52\x45\x43\x4F\x52\x44\x2A\x2A\x2A"sv)) return "xpt";

    // sap powerbuilder integrated development environment file
    if (header_is("\x48\x44\x52\x2A\x50\x6F\x77\x65\x72\x42\x75\x69\x6C\x64\x65\x72"sv)) return "pbd";

    // genetec video archive
    if (header_is("\x47\x65\x6E\x65\x74\x65\x63\x20\x4F\x6D\x6E\x69\x63\x61\x73\x74"sv)) return "g64";

    // show partner graphics file
    if (header_is("\x47\x58\x32"sv)) return "gx2";

    // general regularly-distributed information (gridded) binary
    if (header_is("\x47\x52\x49\x42"sv)) return "grb";

    // gimp pattern file
    if (header_is("\x47\x50\x41\x54"sv)) return "pat";

    // gif file
    if (header_is("\x47\x49\x46\x38"sv)) return "gif";

    // generic e-mail_2
    if (header_is("\x46\x72\x6F\x6D"sv)) return "eml";

    // shockwave flash player
    if (header_is("\x46\x57\x53"sv)) return "swf";

    // dakx compressed audio
    if (header_is("\x46\x4F\x52\x4D\x00"sv)) return "dax";

    // audio interchange file
    if (header_is("\x46\x4F\x52\x4D\x00"sv)) return "aiff";

    // ea interchange format file (iff)_1
    if (header_is("\x46\x4F\x52\x4D"sv)) return "iff";

    // iff anim file
    if (header_is("\x46\x4F\x52\x4D"sv)) return "anm";

    // flash video file
    if (header_is("\x46\x4C\x56"sv)) return "flv";

    // ntfs mft (file)
    if (header_is("\x46\x49\x4C\x45"sv)) return "data";

    // fiasco database definition file
    if (header_is("\x46\x44\x42\x48\x00"sv)) return "fdb";

    // ms fax cover sheet
    if (header_is("\x46\x41\x58\x43\x4F\x56\x45\x52"sv)) return "cpe";

    // quickbooks backup
    if (header_is("\x45\x86\x00\x00\x06\x00"sv)) return "qbb";

    // windows vista event log
    if (header_is("\x45\x6C\x66\x46\x69\x6C\x65\x00"sv)) return "evtx";

    // encase evidence file format v2
    if (header_is("\x45\x56\x46\x32\x0D\x0A\x81"sv)) return "ex01";

    // expert witness compression format
    if (header_is("\x45\x56\x46\x09\x0D\x0A\xFF\x00"sv)) return "e01";

    // ms document imaging file
    if (header_is("\x45\x50"sv)) return "mdi";

    // dsd storage facility audio file
    if (header_is("\x44\x53\x44\x20"sv)) return "dsf";

    // easyrecovery saved state file
    if (header_is("\x45\x52\x46\x53\x53\x41\x56\x45"sv)) return "dat";

    // apple iso 9660-hfs hybrid cd image
    if (header_is("\x45\x52\x02\x00\x00"sv)) return "iso";

    // videovcd-vcdimager file
    if (header_is("\x45\x4E\x54\x52\x59\x56\x43\x44"sv)) return "vcd";

    // elite plus commander game file
    if (header_is("\x45\x4C\x49\x54\x45\x20\x43\x6F"sv)) return "cdr";

    // dvd info file
    if (header_is("\x44\x56\x44"sv)) return "ifo";

    // dvr-studio stream file
    if (header_is("\x44\x56\x44"sv)) return "dvr";

    // dst compression
    if (header_is("\x44\x53\x54\x62"sv)) return "dst";

    // amiga disk file
    if (header_is("\x44\x4F\x53"sv)) return "adf";

    // amiga diskmasher compressed archive
    if (header_is("\x44\x4D\x53\x21"sv)) return "dms";

    // palm zire photo database
    if (header_is("\x44\x42\x46\x48"sv)) return "db";

    // dax compressed cd image
    if (header_is("\x44\x41\x58\x00"sv)) return "dax";

    // poweriso direct-access-archive image
    if (header_is("\x44\x41\x41\x00\x00\x00\x00\x00"sv)) return "daa";

    // creative voice
    if (header_is("\x43\x72\x65\x61\x74\x69\x76\x65\x20\x56\x6F\x69\x63\x65\x20\x46"sv)) return "voc";

    // google chromium patch update
    if (header_is("\x43\x72\x4F\x44"sv)) return "crx";

    // google chrome extension
    if (header_is("\x43\x72\x32\x34"sv)) return "crx";

    // ie history file
    if (header_is("\x43\x6C\x69\x65\x6E\x74\x20\x55"sv)) return "dat";

    // whereisit catalog
    if (header_is("\x43\x61\x74\x61\x6C\x6F\x67\x20"sv)) return "ctf";

    // calculux indoor lighting project file
    if (header_is("\x43\x61\x6C\x63\x75\x6C\x75\x78\x20\x49\x6E\x64\x6F\x6F\x72\x20"sv)) return "cin";

    // shockwave flash file
    if (header_is("\x43\x57\x53"sv)) return "swf";

    // crush compressed archive
    if (header_is("\x43\x52\x55\x53\x48\x20\x76"sv)) return "cru";

    // win9x registry hive
    if (header_is("\x43\x52\x45\x47"sv)) return "dat";

    // corel photopaint file_2
    if (header_is("\x43\x50\x54\x46\x49\x4C\x45"sv)) return "cpt";

    // corel photopaint file_1
    if (header_is("\x43\x50\x54\x37\x46\x49\x4C\x45"sv)) return "cpt";

    // vmware 3 virtual disk
    if (header_is("\x43\x4F\x57\x44"sv)) return "vmdk";

    // com+ catalog
    if (header_is("\x43\x4F\x4D\x2B"sv)) return "clb";

    // corel binary metafile
    if (header_is("\x43\x4D\x58\x31"sv)) return "clb";

    // windows 7 thumbnail
    if (header_is("\x43\x4D\x4D\x4D\x15\x00\x00\x00"sv)) return "db";

    // compressed iso cd image
    if (header_is("\x43\x49\x53\x4F"sv)) return "cso";

    // riff cd audio
    if (header_is("\x43\x44\x44\x41\x66\x6D\x74\x20"sv, 8)) return "cda";

    // iso-9660 cd disc image
    if (header_is("\x43\x44\x30\x30\x31"sv)) return "iso";

    // wordperfect dictionary
    if (header_is("\x43\x42\x46\x49\x4C\x45"sv)) return "cbd";

    // ea interchange format file (iff)_3
    if (header_is("\x43\x41\x54\x20"sv)) return "iff";

    // ragtime document
    if (header_is("\x43\x23\x2B\x44\xA4\x43\x4D\xA5"sv)) return "rtd";

    // blink compressed archive
    if (header_is("\x42\x6C\x69\x6E\x6B"sv)) return "bli";

    // puffer ascii encrypted archive
    if (header_is("\x42\x65\x67\x69\x6E\x20\x50\x75\x66\x66\x65\x72"sv)) return "apuf";

    // mac disk image (bz2 compressed)
    if (header_is("\x42\x5A\x68"sv)) return "dmg";

    // bzip2 compressed archive
    if (header_is("\x42\x5A\x68"sv)) return "bz2";

    // better portable graphics
    if (header_is("\x42\x50\x47\xFB"sv)) return "bpg";

    // palmpilot resource file
    if (header_is("\x42\x4F\x4F\x4B\x4D\x4F\x42\x49"sv)) return "prc";

    // bitmap image
    if (header_is("\x42\x4D"sv)) return "bmp";

    // speedtouch router firmware
    if (header_is("\x42\x4C\x49\x32\x32\x33"sv)) return "bin";

    // vcard
    if (header_is("\x42\x45\x47\x49\x4E\x3A\x56\x43"sv)) return "vcf";

    // google chrome dictionary file
    if (header_is("\x42\x44\x69\x63"sv)) return "bdic";

    // ntfs mft (baad)
    if (header_is("\x42\x41\x41\x44"sv)) return "data";

    // freearc compressed file
    if (header_is("\x41\x72\x43\x01"sv)) return "arc";

    // riff windows audio
    if (header_is("\x41\x56\x49\x20\x4C\x49\x53\x54"sv, 8)) return "avi";

    // avg6 integrity database
    if (header_is("\x41\x56\x47\x36\x5F\x49\x6E\x74"sv)) return "dat";

    // aol personal file cabinet
    if (header_is("\x41\x4F\x4C\x56\x4D\x31\x30\x30"sv)) return "org";

    // aol address book index
    if (header_is("\x41\x4F\x4C\x49\x4E\x44\x45\x58"sv)) return "abi";

    // aol client preferences-settings file
    if (header_is("\x41\x4F\x4C\x49\x44\x58"sv)) return "ind";

    // aol user configuration
    if (header_is("\x41\x4F\x4C\x44\x42"sv)) return "idx";

    // aol address book
    if (header_is("\x41\x4F\x4C\x44\x42"sv)) return "aby";

    // aol and aim buddy list
    if (header_is("\x41\x4F\x4C\x20\x46\x65\x65\x64"sv)) return "bag";

    // aol config files
    if (header_is("\x41\x4F\x4C"sv)) return "abi";

    // harvard graphics symbol graphic
    if (header_is("\x41\x4D\x59\x4F"sv)) return "syw";

    // aol parameter-info files
    if (header_is("\x41\x43\x53\x44"sv)) return "data";

    // steganos virtual secure drive
    if (header_is("\x41\x43\x76"sv)) return "sle";

    // generic autocad drawing
    if (header_is("\x41\x43\x31\x30"sv)) return "dwg";

    // analog box (abox) circuit files
    if (header_is("\x41\x42\x6F\x78"sv)) return "abox2";

    // endnote library file
    if (header_is("\x40\x40\x40\x20\x00\x00\x40\x40\x40\x40"sv, 32)) return "enl";

    // windows help file_2
    if (header_is("\x3F\x5F\x03\x00"sv)) return "gid";

    // quatro pro for windows 7.0
    if (header_is("\x3E\x00\x03\x00\xFE\xFF\x09\x00\x06"sv, 24)) return "wb3";

    // base85 file
    if (header_is("\x3C\x7E\x36\x3C\x5C\x25\x5F\x30\x67\x53\x71\x68\x3B"sv)) return "b85";

    // gps exchange (v1.1)
    if (header_is("\x3C\x67\x70\x78\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E"sv)) return "gpx";

    // adobe framemaker
    if (header_is("\x3C\x4D\x61\x6B\x65\x72\x46\x69"sv)) return "fm";

    // google earth keyhole overlay file
    if (header_is("\x3C\x4B\x65\x79\x68\x6F\x6C\x65\x3E"sv)) return "eta";

    // csound music
    if (header_is("\x3C\x43\x73\x6F\x75\x6E\x64\x53\x79\x6E\x74\x68\x65\x73\x69\x7A"sv)) return "csd";

    // picasa movie project file
    if (header_is("\x3C\x43\x54\x72\x61\x6E\x73\x54\x69\x6D\x65\x6C\x69\x6E\x65\x3E"sv)) return "mxf";

    // mmc snap-in control file
    if (header_is(
                "\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E\x30\x22\x3F\x3E\x0D\x0A\x3C\x4D\x4D\x43\x5F\x43\x6F\x6E\x73\x6F\x6C\x65\x46\x69\x6C\x65\x20\x43\x6F\x6E\x73\x6F\x6C\x65\x56\x65\x72\x73\x69\x6F\x6E\x3D\x22"sv
        ))
        return "msc";

    // user interface language
    if (header_is("\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E\x30\x22\x3F\x3E"sv)) return "xml";

    // windows visual stylesheet
    if (header_is("\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D"sv)) return "manifest";

    // windows script component
    if (header_is("\x3C\x3F"sv)) return "wsc";

    // aol html mail
    if (header_is("\x3C\x21\x64\x6F\x63\x74\x79\x70"sv)) return "dci";

    // biztalk xml-data reduced schema
    if (header_is("\x3C"sv)) return "xdr";

    // advanced stream redirector
    if (header_is("\x3C"sv)) return "asx";

    // surfplan kite project file
    if (header_is("\x3A\x56\x45\x52\x53\x49\x4F\x4E"sv)) return "sle";

    // photoshop image
    if (header_is("\x38\x42\x50\x53"sv)) return "psd";

    // zisofs compressed file
    if (header_is("\x37\xE4\x53\x96\xC9\xDB\xD6\x07"sv)) return "data";

    // 7-zip compressed file
    if (header_is("\x37\x7A\xBC\xAF\x27\x1C"sv)) return "7z";

    // tcpdump capture file
    if (header_is("\x34\xCD\xB2\xA1"sv)) return "data";

    // pfaff home embroidery
    if (header_is("\x32\x03\x10\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\x00"sv)) return "pcs";

    // ms write file_2
    if (header_is("\x32\xBE"sv)) return "wri";

    // ms write file_1
    if (header_is("\x31\xBE"sv)) return "wri";

    // cpio archive
    if (header_is("\x30\x37\x30\x37\x30"sv)) return "data";

    // national transfer format map
    if (header_is("\x30\x31\x4F\x52\x44\x4E\x41\x4E"sv)) return "ntf";

    // windows media audio-video file
    if (header_is("\x30\x26\xB2\x75\x8E\x66\xCF\x11"sv)) return "asf";

    // genealogical data communication (gedcom) file
    if (header_is("\x30\x20\x48\x45\x41\x44"sv)) return "ged";

    // windows event viewer file
    if (header_is("\x30\x00\x00\x00\x4C\x66\x4C\x65"sv)) return "evt";

    // ms security catalog file
    if (header_is("\x30"sv)) return "cat";

    // thunderbird-mozilla mail summary file
    if (header_is("\x2F\x2F\x20\x3C\x21\x2D\x2D\x20\x3C\x6D\x64\x62\x3A\x6D\x6F\x72\x6B\x3A\x7A"sv)) return "msf";

    // next-sun microsystems audio file
    if (header_is("\x2E\x73\x6E\x64"sv)) return "au";

    // realaudio streaming media
    if (header_is("\x2E\x72\x61\xFD\x00"sv)) return "ra";

    // realaudio file
    if (header_is("\x2E\x52\x4D\x46\x00\x00\x00\x12"sv)) return "ra";

    // realmedia streaming media
    if (header_is("\x2E\x52\x4D\x46"sv)) return "rm";

    // realplayer video file (v11+)
    if (header_is("\x2E\x52\x45\x43"sv)) return "ivr";

    // compressed archive
    if (header_is("\x2D\x6C\x68"sv, 2)) return "lha";

    // symantec wise installer log
    if (header_is("\x2A\x2A\x2A\x20\x20\x49\x6E\x73"sv)) return "log";

    // binhex 4 compressed archive
    if (header_is("\x28\x54\x68\x69\x73\x20\x66\x69"sv)) return "hqx";

    // fuzzy bitmap (fbm) file
    if (header_is("\x25\x62\x69\x74\x6D\x61\x70"sv)) return "fbm";

    // pdf file
    if (header_is("\x25\x50\x44\x46"sv)) return "pdf";

    // postscript file
    if (header_is("\x25\x21\x50\x53\x2D\x41\x64\x6F\x62\x65\x2D"sv)) return "ps";

    // encapsulated postscript file
    if (header_is("\x25\x21\x50\x53\x2D\x41\x64\x6F"sv)) return "eps";

    // spss data file
    if (header_is("\x24\x46\x4C\x32\x40\x28\x23\x29"sv)) return "sav";

    // brother-babylock-bernina home embroidery
    if (header_is("\x23\x50\x45\x53\x30"sv)) return "pes";

    // brother-babylock-bernina home embroidery
    if (header_is("\x23\x50\x45\x43\x30\x30\x30\x31"sv)) return "pec";

    // nvidia scene graph binary file
    if (header_is("\x23\x4E\x42\x46"sv)) return "nbf";

    // vbscript encoded script
    if (header_is("\x23\x40\x7E\x5E"sv)) return "vbe";

    // radiance high dynamic range image file
    if (header_is("\x23\x3F\x52\x41\x44\x49\x41\x4E"sv)) return "hdr";

    // skype audio compression
    if (header_is("\x23\x21\x53\x49\x4C\x4B\x0A"sv)) return "sil";

    // adaptive multi-rate acelp codec (gsm)
    if (header_is("\x23\x21\x41\x4D\x52"sv)) return "amr";

    // google earth keyhole placemark file
    if (header_is("\x23\x20\x54\x68\x69\x73\x20\x69\x73\x20\x61\x6E\x20\x4B\x65\x79"sv)) return "eta";

    // ms developer studio project file
    if (header_is("\x23\x20\x4D\x69\x63\x72\x6F\x73"sv)) return "dsp";

    // vmware 4 virtual disk description
    if (header_is("\x23\x20\x44\x69\x73\x6B\x20\x44"sv)) return "vmdk";

    // cerius2 file
    if (header_is("\x23\x20"sv)) return "msi";

    // microsoft outlook exchange offline storage folder
    if (header_is("\x21\x42\x44\x4E"sv)) return "ost";

    // unix archiver (ar)-ms program library common object file format (coff)
    if (header_is("\x21\x3C\x61\x72\x63\x68\x3E\x0A"sv)) return "lib";

    // ain compressed archive
    if (header_is("\x21\x12"sv)) return "ain";

    // noaa raster navigation chart (rnc) file
    if (header_is("\x21\x0D\x0A\x43\x52\x52\x2F\x54\x68\x69\x73\x20\x65\x6C\x65\x63"sv)) return "bsb";

    // mapinfo sea chart
    if (header_is("\x21"sv)) return "bsb";

    // compressed tape archive_2
    if (header_is("\x1F\xA0"sv)) return "tar.z";

    // compressed tape archive_1
    if (header_is("\x1F\x9D\x90"sv)) return "tar.z";

    // synology router configuration backup file
    if (header_is("\x1F\x8B\x08\x00"sv)) return "dss";

    // vlc player skin file
    if (header_is("\x1F\x8B\x08"sv)) return "vlt";

    // gzip archive file
    if (header_is("\x1F\x8B\x08"sv)) return "gz";

    // wordstar version 5.0-6.0 document
    if (header_is("\x1D\x7D"sv)) return "ws";

    // runtime software disk image
    if (header_is("\x1A\x52\x54\x53\x20\x43\x4F\x4D"sv)) return "dat";

    // matroska stream file_2
    if (header_is("\x1A\x45\xDF\xA3\x93\x42\x82\x88"sv)) return "mkv";

    // matroska stream file_1
    if (header_is("\x1A\x45\xDF\xA3"sv)) return "mkv";

    // webm video file
    if (header_is("\x1A\x45\xDF\xA3"sv)) return "webm";

    // winpharoah capture file
    if (header_is("\x1A\x35\x01\x00"sv)) return "eth";

    // compressed archive file
    if (header_is("\x1A\x0B"sv)) return "pak";

    // lh archive (old vers.-type 5)
    if (header_is("\x1A\x09"sv)) return "arc";

    // lh archive (old vers.-type 4)
    if (header_is("\x1A\x08"sv)) return "arc";

    // lh archive (old vers.-type 3)
    if (header_is("\x1A\x04"sv)) return "arc";

    // lh archive (old vers.-type 2)
    if (header_is("\x1A\x03"sv)) return "arc";

    // lh archive (old vers.-type 1)
    if (header_is("\x1A\x02"sv)) return "arc";

    // lotus notes database
    if (header_is("\x1A\x00\x00\x04\x00\x00"sv)) return "nsf";

    // lotus notes database template
    if (header_is("\x1A\x00\x00"sv)) return "ntf";

    // windows prefetch file
    if (header_is("\x11\x00\x00\x00\x53\x43\x43\x41"sv)) return "pf";

    // easy cd creator 5 layout file
    if (header_is("\x10\x00\x00\x00"sv)) return "cl5";

    // sibelius music - score
    if (header_is("\x0F\x53\x49\x42\x45\x4C\x49\x55\x53"sv)) return "sib";

    // powerpoint presentation subheader_2
    if (header_is("\x0F\x00\xE8\x03"sv, 512)) return "ppt";

    // deskmate worksheet
    if (header_is("\x0E\x57\x4B\x53"sv)) return "wks";

    // nero cd compilation
    if (header_is("\x0E\x4E\x65\x72\x6F\x49\x53\x4F"sv)) return "nri";

    // deskmate document
    if (header_is("\x0D\x44\x4F\x43"sv)) return "doc";

    // monochrome picture tiff bitmap
    if (header_is("\x0C\xED"sv)) return "mp";

    // multibit bitcoin wallet file
    if (header_is("\x0A\x16\x6F\x72\x67\x2E\x62\x69\x74\x63\x6F\x69\x6E\x2E\x70\x72"sv)) return "wallet";

    // zsoft paintbrush file_3
    if (header_is("\x0A\x05\x01\x01"sv)) return "pcx";

    // zsoft paintbrush file_2
    if (header_is("\x0A\x03\x01\x01"sv)) return "pcx";

    // zsoft paintbrush file_1
    if (header_is("\x0A\x02\x01\x01"sv)) return "pcx";

    // excel spreadsheet subheader_1
    if (header_is("\x09\x08\x10\x00\x00\x06\x05\x00"sv, 512)) return "xls";

    // dbase iv or dbfast configuration file
    if (header_is("\x08"sv)) return "db";

    // designtools 2d design file
    if (header_is("\x07\x64\x74\x32\x64\x64\x74\x64"sv)) return "dtd";

    // skincrafter skin
    if (header_is("\x07\x53\x4B\x46"sv)) return "skf";

    // generic drawing programs
    if (header_is("\x07"sv)) return "drw";

    // material exchange format
    if (header_is("\x06\x0E\x2B\x34\x02\x05\x01\x01\x0D\x01\x02\x01\x01\x02"sv)) return "mxf";

    // adobe indesign
    if (header_is("\x06\x06\xED\xF5\xD8\x1D\x46\xE5\xBD\x31\xEF\xE7\xFE\x74\xB7\x1D"sv)) return "indd";

    // info2 windows recycle bin_2
    if (header_is("\x05\x00\x00\x00"sv)) return "data";

    // info2 windows recycle bin_1
    if (header_is("\x04\x00\x00\x00"sv)) return "data";

    // dbase iv file
    if (header_is("\x04"sv)) return "db4";

    // digital speech standard (v3)
    if (header_is("\x03\x64\x73\x73"sv)) return "dss";

    // approach index file
    if (header_is("\x03\x00\x00\x00\x41\x50\x50\x52"sv)) return "adx";

    // nokia pc suite content copier file
    if (header_is("\x03\x00\x00\x00"sv)) return "nfc";

    // quicken price history
    if (header_is("\x03\x00\x00\x00"sv)) return "qph";

    // dbase iii file
    if (header_is("\x03"sv)) return "db3";

    // mapinfo native data format
    if (header_is("\x03"sv)) return "dat";

    // digital speech standard file
    if (header_is("\x02\x64\x73\x73"sv)) return "dss";

    // micrografx vector graphic file
    if (header_is("\x01\xFF\x02\x04\x03\x02"sv)) return "drw";

    // silicon graphics rgb bitmap
    if (header_is("\x01\xDA\x01\x01\x00\x03"sv)) return "rgb";

    // novell lanalyzer capture file
    if (header_is("\x01\x10"sv)) return "tr1";

    // sql data base
    if (header_is("\x01\x0F\x00\x00"sv)) return "mdf";

    // the bat! message base index
    if (header_is("\x01\x01\x47\x19\xA4\x00\x00\x00\x00\x00\x00\x00"sv)) return "tbi";

    // firebird and interbase database files
    if (header_is("\x01\x00\x39\x30"sv)) return "fdb";

    // webex advanced recording format
    if (header_is("\x01\x00\x02\x00"sv)) return "arf";

    // powerpoint presentation subheader_1
    if (header_is("\x00\x6E\x1E\xF0"sv, 512)) return "ppt";

    // paessler prtg monitoring system
    if (header_is("\x00\x3B\x05\x00\x01\x00\x00\x00"sv)) return "db";

    // netscape communicator (v4) mail folder
    if (header_is("\x00\x1E\x84\x90\x00\x00\x00\x00"sv)) return "snm";

    // bios details in ram
    if (header_is("\x00\x14\x00\x00\x01\x02"sv)) return "data";

    // flic animation
    if (header_is("\x00\x11"sv)) return "fli";

    // mbox table of contents file
    if (header_is("\x00\x0D\xBB\xA0"sv)) return "data";

    // netscape navigator (v4) database
    if (header_is("\x00\x06\x15\x61\x00\x00\x00\x02\x00\x00\x04\xD2\x00\x00\x10\x00"sv)) return "db";

    // palm datebook archive
    if (header_is("\x00\x01\x42\x44"sv)) return "dba";

    // palm address book archive
    if (header_is("\x00\x01\x42\x41"sv)) return "aba";

    // microsoft access
    if (header_is("\x00\x01\x00\x00\x53\x74\x61\x6E\x64\x61\x72\x64\x20\x4A\x65\x74\x20\x44\x42"sv)) return "mdb";

    // microsoft access 2007
    if (header_is("\x00\x01\x00\x00\x53\x74\x61\x6E\x64\x61\x72\x64\x20\x41\x43\x45\x20\x44\x42"sv)) return "accdb";

    // microsoft money file
    if (header_is("\x00\x01\x00\x00\x4D\x53\x49\x53\x41\x4D\x20\x44\x61\x74\x61\x62\x61\x73\x65"sv)) return "mny";

    // truetype font file
    if (header_is("\x00\x01\x00\x00\x00"sv)) return "ttf";

    // windows help file_1
    if (header_is("\x00\x00\xFF\xFF\xFF\xFF"sv, 6)) return "hlp";

    // quark express (motorola)
    if (header_is("\x00\x00\x4D\x4D\x58\x50\x52"sv)) return "qxd";

    // quark express (intel)
    if (header_is("\x00\x00\x49\x49\x58\x50\x52"sv)) return "qxd";

    // lotus 1-2-3 (v9)
    if (header_is("\x00\x00\x1A\x00\x05\x10\x04"sv)) return "123";

    // lotus 1-2-3 (v4-v5)
    if (header_is("\x00\x00\x1A\x00\x02\x10\x04\x00"sv)) return "wk4";

    // lotus 1-2-3 (v3)
    if (header_is("\x00\x00\x1A\x00\x00\x10\x04\x00"sv)) return "wk3";

    // lotus 1-2-3 (v1)
    if (header_is("\x00\x00\x02\x00\x06\x04\x06\x00"sv)) return "wk1";

    // wii images container
    if (header_is("\x00\x20\xAF\x30"sv)) return "tpl";

    // amiga hunk executable
    if (header_is("\x00\x00\x03\xF3"sv)) return "data";

    // quattropro spreadsheet
    if (header_is("\x00\x00\x02\x00"sv)) return "wb2";

    // compucon-singer embroidery design file
    if (header_is("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"sv)) return "xxx";

    // windows cursor
    if (header_is("\x00\x00\x02\x00"sv)) return "cur";

    // dvd video file
    if (header_is("\x00\x00\x01\xBA"sv)) return "mpg";

    // mpeg video file
    if (header_is("\x00\x00\x01\xB3"sv)) return "mpg";

    // windows icon|printer spool file
    if (header_is("\x00\x00\x01\x00"sv)) return "ico";

    // 3rd generation partnership project 3gpp2
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70"sv)) return "3gg";

    // apple audio and video
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70\x4D\x34\x41"sv)) return "m4a";

    // 3gpp2 multimedia files
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70"sv)) return "3gp";

    // mpeg-4 video_2
    if (header_is("\x00\x00\x00\x1C\x66\x74\x79\x70"sv)) return "mp4";

    // mpeg-4 video_1
    if (header_is("\x00\x00\x00\x18\x66\x74\x79\x70"sv)) return "3gp5";

    // bitcoin core wallet.dat file
    if (header_is("\x00\x00\x00\x00\x62\x31\x05\x00\x09\x00\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x00\x00\x00"sv, 8)) return "dat";

    // windows disk image
    if (header_is("\x00\x00\x00\x00\x14\x00\x00\x00"sv)) return "tbi";

    // 3rd generation partnership project 3gpp
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70"sv)) return "3gg";

    // mpeg-4 v1
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70\x69\x73\x6F\x6D"sv)) return "mp4";

    // 3gpp multimedia files
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70"sv)) return "3gp";

    // jpeg2000 image files
    if (header_is("\x00\x00\x00\x0C\x6A\x50\x20\x20"sv)) return "jp2";

    // high efficiency image container (heic)_2
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70\x68\x65\x69\x63"sv)) return "heic";

    // high efficiency image container (heic)_1
    if (header_is("\x00\x00\x00"sv)) return "avif";

    return std::nullopt;
}