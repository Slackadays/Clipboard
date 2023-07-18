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
cpp_code = ""
for signature in file_sigs:
    cpp_code += "    // " + signature["File description"].lower() + "\n"
    cpp_code += '    if (header_is("' + "".join([f"\\x{c}" for c in signature["Header (hex)"].split()]) + '"sv'
    if signature["Header offset"] != '0':
        cpp_code += ', ' + signature["Header offset"] + ')) return "";\n'
    else:
        cpp_code += ')) return "";\n'
    cpp_code += "\n"

print(cpp_code)

now, fill in the right MIME type in "return", and you're done */

std::optional<std::string_view> inferMIMEType(const std::string_view& content) {
    auto header_is = [&](const std::string_view& pattern, const size_t offset = 0) {
        if (content.size() < (pattern.size() + offset)) return false;
        for (size_t i = 0; i < pattern.size(); i++) // DON'T use a substr comparison because the stdlib uses a 3-way comparison (super slow)
            if (content[i + offset] != pattern[i]) [[likely]]
                return false; // more likely to be false than true because there can only be one match out of hundreds
            else [[unlikely]]
                continue;
        return true;
    };

    // jpeg xl
    if (header_is("\x00\x00\x00\x0C\x4A\x58\x4C\x20\x0D\x0A\x87\x0A"sv)) return "image/jxl";

    // tbi
    if (header_is("\x00\x00\x00\x14\x00\x00\x00"sv)) return "application/x-tbi";

    // xml
    if (header_is("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"sv)) return "text/xml";

    // svg
    if (header_is("<svg"sv)) return "image/svg+xml";

    // mpeg 1
    if (header_is("\x00\x00\x01\xB3"sv)) return "video/mpeg";

    // mpeg 2
    if (header_is("\x00\x00\x01\xBA"sv)) return "video/mpeg";

    // utf-32be
    if (header_is("\x00\x00\xFE\xFF"sv)) return "text/plain";

    // ttf
    if (header_is("\x00\x01\x00\x00\x00"sv)) return "font/ttf";

    // otf
    if (header_is("\x4F\x54\x54\x4F"sv)) return "font/otf";

    // xml
    if (header_is("\x00\x3C\x00\x3F\x00\x78\x00\x6D\x00\x6C\x00\x20"sv)) return "text/xml";

    // wasm
    if (header_is("\x00\x61\x73\x6D"sv)) return "application/wasm";

    // jpeg 2000
    if (header_is("\x00\x00\x00\x0C\x6A\x50\x20\x20\x0D\x0A\x87\x0A"sv)) return "image/jp2";

    // jpeg 2000
    if (header_is("\xFF\x4F\xFF\x51"sv)) return "image/jp2";

    // lz4
    if (header_is("\x04\x22\x4D\x18"sv)) return "application/x-lz4";

    // pcap
    if (header_is("\x0A\x0D\x0D\x0A"sv)) return "application/vnd.tcpdump.pcap";

    // winbox
    if (header_is("\x0A\xF0\x1D\xC0"sv)) return "application/x-winbox";

    // lua
    if (header_is("\x1B\x4C\x75\x61"sv)) return "text/x-lua";

    // lzw compression
    if (header_is("\x1F\x9D"sv)) return "application/x-lzw";

    // lzh compression
    if (header_is("\x1F\xA0"sv)) return "application/x-lzh";

    // dss
    if (header_is("\x02\x64\x73\x73"sv)) return "audio/dss";

    // deb
    if (header_is("\x21\x3C\x61\x72\x63\x68\x3E\x0A"sv)) return "application/vnd.debian.binary-package";

    // u-boot
    if (header_is("\x27\x05\x19\x56"sv)) return "application/x-uboot";

    // zstd
    if (header_is("\x28\xB5\x2F\xFD"sv)) return "application/zstd";

    // x.509
    if (header_is("-----BEGIN CERTIFICATE-----"sv) || header_is("-----BEGIN CERTIFICATE REQUEST-----") || header_is("-----BEGIN PRIVATE KEY-----") || header_is("-----BEGIN DSA PRIVATE KEY-----")
        || header_is("-----BEGIN RSA PRIVATE KEY-----"))
        return "application/x-x509-user-cert";

    // lzh file
    if (header_is("-lh0-"sv, 2) || header_is("-lh1"sv, 2) || header_is("-lh2-"sv, 2) || header_is("-lh3-"sv, 2) || header_is("-lh4-"sv, 2) || header_is("-lh5-"sv, 2) || header_is("-lhd-"sv, 2))
        return "application/x-lzh";

    // ace
    if (header_is("**ACE**"sv, 7)) return "application/x-ace";

    // utf-7
    if (header_is("+/v8-"sv) || header_is("+/v9-"sv) || header_is("+/v+"sv) || header_is("+/v-"sv)) return "text/plain";

    // html
    if (header_is("<!DOCTYPE html>"sv)) return "text/html";

    // gif
    if (header_is("GIF87a"sv) || header_is("GIF89a"sv)) return "image/gif";

    // webp
    if (header_is("RIFF\x00\x00\x00\x00WEBPVP8 "sv)) return "image/webp";

    // webm
    if (header_is("\x1A\x45\xDF\xA3"sv)) return "video/webm";

    // bmp
    if (header_is("BM"sv)) return "image/bmp";

    // tiff
    if (header_is("II\x2A\x00"sv) || header_is("MM\x00\x2A"sv)) return "image/tiff";

    // zip
    if (header_is("PK\x03\x04"sv) || header_is("PK\x05\x06"sv) || header_is("PK\x07\x08"sv)) return "application/zip";

    // 7z
    if (header_is("7z\xBC\xAF\x27\x1C"sv)) return "application/x-7z-compressed";

    // mp4
    if (header_is("ftypmp42"sv) || header_is("ftypisom"sv) || header_is("ftypM4V "sv) || header_is("ftypM4A "sv)) return "video/mp4";

    // ogg
    if (header_is("OggS"sv)) return "audio/ogg";

    // flac
    if (header_is("fLaC"sv)) return "audio/flac";

    // tar
    if (header_is("ustar"sv)) return "application/x-tar";

    // bzip2
    if (header_is("BZh"sv)) return "application/x-bzip2";

    // macos file alias
    if (header_is("\x62\x6F\x6F\x6B\x00\x00\x00\x00\x6D\x61\x72\x6B\x00\x00\x00\x00"sv)) return "application/x-apple-file-alias";

    // xcf
    if (header_is("gimp xcf"sv)) return "image/x-xcf";

    // qcow
    if (header_is("QFI"sv)) return "application/x-qcow";

    // icc profile
    if (header_is("KCMS"sv)) return "application/vnd.iccprofile";

    // odt
    if (header_is("\x50\x4B\x03\x04\x14"sv)) return "application/vnd.oasis.opendocument.text";

    // java jar
    if (header_is("\x50\x4B\x03\x04\x14\x00\x08\x00\x08\x00"sv)) return "application/java-archive";

    // silk
    if (header_is("#!SILK"sv)) return "audio/silk";

    // fuzzy bitmap
    if (header_is("\x25\x62\x69\x74\x6D\x61\x70"sv)) return "image/vnd.microsoft.icon";

    // high efficiency image container (heic)_1
    if (header_is("\x00\x00\x00"sv)) return "image/heic";

    // high efficiency image container (heic)_2
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70\x68\x65\x69\x63"sv)) return "image/heic";

    // jpeg2000 image files
    if (header_is("\x00\x00\x00\x0C\x6A\x50\x20\x20"sv)) return "image/jp2";

    // 3gpp multimedia files
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70"sv)) return "video/3gpp";

    // mpeg-4 v1
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70\x69\x73\x6F\x6D"sv)) return "video/mp4";

    // 3rd generation partnership project 3gpp
    if (header_is("\x00\x00\x00\x14\x66\x74\x79\x70"sv)) return "video/3gpp";

    // windows disk image
    if (header_is("\x00\x00\x00\x00\x14\x00\x00\x00"sv)) return "application/x-ms-dos-executable";

    // bitcoin core wallet.dat file
    if (header_is("\x00\x00\x00\x00\x62\x31\x05\x00\x09\x00\x00\x00\x00\x20\x00\x00\x00\x09\x00\x00\x00\x00\x00\x00"sv, 8)) return "application/x-bitcoin-blockchain";

    // mpeg-4 video_1
    if (header_is("\x00\x00\x00\x18\x66\x74\x79\x70"sv)) return "video/mp4";

    // mpeg-4 video_2
    if (header_is("\x00\x00\x00\x1C\x66\x74\x79\x70"sv)) return "video/mp4";

    // 3gpp2 multimedia files
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70"sv)) return "video/3gpp2";

    // apple audio and video
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70\x4D\x34\x41"sv)) return "video/quicktime";

    // 3rd generation partnership project 3gpp2
    if (header_is("\x00\x00\x00\x20\x66\x74\x79\x70"sv)) return "video/3gpp2";

    // windows icon|printer spool file
    if (header_is("\x00\x00\x01\x00"sv)) return "image/x-icon";

    // mpeg video file
    if (header_is("\x00\x00\x01\xB3"sv)) return "video/mpeg";

    // dvd video file
    if (header_is("\x00\x00\x01\xBA"sv)) return "video/mpeg";

    // windows cursor
    if (header_is("\x00\x00\x02\x00"sv)) return "image/x-icon";

    // compucon-singer embroidery design file
    if (header_is("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"sv)) return "application/octet-stream";

    // quattropro spreadsheet
    if (header_is("\x00\x00\x02\x00"sv)) return "application/vnd.lotus-1-2-3";

    // amiga hunk executable
    if (header_is("\x00\x00\x03\xF3"sv)) return "application/x-executable";

    // wii images container
    if (header_is("\x00\x20\xAF\x30"sv)) return "application/octet-stream";

    // lotus 1-2-3 (v1)
    if (header_is("\x00\x00\x02\x00\x06\x04\x06\x00"sv)) return "application/vnd.lotus-1-2-3";

    // lotus 1-2-3 (v3)
    if (header_is("\x00\x00\x1A\x00\x00\x10\x04\x00"sv)) return "application/vnd.lotus-1-2-3";

    // lotus 1-2-3 (v4-v5)
    if (header_is("\x00\x00\x1A\x00\x02\x10\x04\x00"sv)) return "application/vnd.lotus-1-2-3";

    // lotus 1-2-3 (v9)
    if (header_is("\x00\x00\x1A\x00\x05\x10\x04"sv)) return "application/vnd.lotus-1-2-3";

    // quark express (intel)
    if (header_is("\x00\x00\x49\x49\x58\x50\x52"sv)) return "application/octet-stream";

    // quark express (motorola)
    if (header_is("\x00\x00\x4D\x4D\x58\x50\x52"sv)) return "application/octet-stream";

    // windows help file_1
    if (header_is("\x00\x00\xFF\xFF\xFF\xFF"sv, 6)) return "application/x-msdownload";

    // truetype font file
    if (header_is("\x00\x01\x00\x00\x00"sv)) return "application/x-font-ttf";

    // microsoft money file
    if (header_is("\x00\x01\x00\x00\x4D\x53\x49\x53\x41\x4D\x20\x44\x61\x74\x61\x62\x61\x73\x65"sv)) return "application/x-msmoney";

    // microsoft access 2007
    if (header_is("\x00\x01\x00\x00\x53\x74\x61\x6E\x64\x61\x72\x64\x20\x41\x43\x45\x20\x44\x42"sv)) return "application/x-msaccess";

    // microsoft access
    if (header_is("\x00\x01\x00\x00\x53\x74\x61\x6E\x64\x61\x72\x64\x20\x4A\x65\x74\x20\x44\x42"sv)) return "application/x-msaccess";

    // palm address book archive
    if (header_is("\x00\x01\x42\x41"sv)) return "application/octet-stream";

    // palm datebook archive
    if (header_is("\x00\x01\x42\x44"sv)) return "application/octet-stream";

    // netscape navigator (v4) database
    if (header_is("\x00\x06\x15\x61\x00\x00\x00\x02\x00\x00\x04\xD2\x00\x00\x10\x00"sv)) return "application/x-mozilla-bookmarks";

    // mbox table of contents file
    if (header_is("\x00\x0D\xBB\xA0"sv)) return "application/mbox";

    // flic animation
    if (header_is("\x00\x11"sv)) return "video/x-flic";

    // bios details in ram
    if (header_is("\x00\x14\x00\x00\x01\x02"sv)) return "application/octet-stream";

    // netscape communicator (v4) mail folder
    if (header_is("\x00\x1E\x84\x90\x00\x00\x00\x00"sv)) return "application/x-mozilla-bookmarks";

    // paessler prtg monitoring system
    if (header_is("\x00\x3B\x05\x00\x01\x00\x00\x00"sv)) return "application/octet-stream";

    // powerpoint presentation subheader_1
    if (header_is("\x00\x6E\x1E\xF0"sv, 512)) return "application/vnd.ms-powerpoint";

    // webex advanced recording format
    if (header_is("\x01\x00\x02\x00"sv)) return "application/octet-stream";

    // firebird and interbase database files
    if (header_is("\x01\x00\x39\x30"sv)) return "application/octet-stream";

    // the bat! message base index
    if (header_is("\x01\x01\x47\x19\xA4\x00\x00\x00\x00\x00\x00\x00"sv)) return "application/octet-stream";

    // sql data base
    if (header_is("\x01\x0F\x00\x00"sv)) return "application/sql";

    // novell lanalyzer capture file
    if (header_is("\x01\x10"sv)) return "application/octet-stream";

    // silicon graphics rgb bitmap
    if (header_is("\x01\xDA\x01\x01\x00\x03"sv)) return "image/x-rgb";

    // micrografx vector graphic file
    if (header_is("\x01\xFF\x02\x04\x03\x02"sv)) return "application/octet-stream";

    // digital speech standard file
    if (header_is("\x02\x64\x73\x73"sv)) return "audio/dss";

    // mapinfo native data format
    if (header_is("\x03"sv)) return "application/octet-stream";

    // dbase iii file
    if (header_is("\x03"sv)) return "application/octet-stream";

    // quicken price history
    if (header_is("\x03\x00\x00\x00"sv)) return "application/octet-stream";

    // nokia pc suite content copier file
    if (header_is("\x03\x00\x00\x00"sv)) return "application/octet-stream";

    // approach index file
    if (header_is("\x03\x00\x00\x00\x41\x50\x50\x52"sv)) return "application/octet-stream";

    // digital speech standard (v3)
    if (header_is("\x03\x64\x73\x73"sv)) return "audio/dss";

    // dbase iv file
    if (header_is("\x04"sv)) return "application/octet-stream";

    // info2 windows recycle bin_1
    if (header_is("\x04\x00\x00\x00"sv)) return "application/octet-stream";

    // info2 windows recycle bin_2
    if (header_is("\x05\x00\x00\x00"sv)) return "application/octet-stream";

    // adobe indesign
    if (header_is("\x06\x06\xED\xF5\xD8\x1D\x46\xE5\xBD\x31\xEF\xE7\xFE\x74\xB7\x1D"sv)) return "application/x-indesign";

    // material exchange format
    if (header_is("\x06\x0E\x2B\x34\x02\x05\x01\x01\x0D\x01\x02\x01\x01\x02"sv)) return "application/mxf";

    // generic drawing programs
    if (header_is("\x07"sv)) return "application/octet-stream";

    // skincrafter skin
    if (header_is("\x07\x53\x4B\x46"sv)) return "application/skincrafter";

    // designtools 2d design file
    if (header_is("\x07\x64\x74\x32\x64\x64\x74\x64"sv)) return "application/designtools";

    // dbase iv or dbfast configuration file
    if (header_is("\x08"sv)) return "application/octet-stream";

    // excel spreadsheet subheader_1
    if (header_is("\x09\x08\x10\x00\x00\x06\x05\x00"sv, 512)) return "application/vnd.ms-excel";

    // zsoft paintbrush file_1
    if (header_is("\x0A\x02\x01\x01"sv)) return "image/x-pcx";

    // zsoft paintbrush file_2
    if (header_is("\x0A\x03\x01\x01"sv)) return "image/x-pcx";

    // zsoft paintbrush file_3
    if (header_is("\x0A\x05\x01\x01"sv)) return "image/x-pcx";

    // multibit bitcoin wallet file
    if (header_is("\x0A\x16\x6F\x72\x67\x2E\x62\x69\x74\x63\x6F\x69\x6E\x2E\x70\x72"sv)) return "application/octet-stream";

    // monochrome picture tiff bitmap
    if (header_is("\x0C\xED"sv)) return "image/tiff";

    // deskmate document
    if (header_is("\x0D\x44\x4F\x43"sv)) return "application/octet-stream";

    // nero cd compilation
    if (header_is("\x0E\x4E\x65\x72\x6F\x49\x53\x4F"sv)) return "application/octet-stream";

    // deskmate worksheet
    if (header_is("\x0E\x57\x4B\x53"sv)) return "application/octet-stream";

    // powerpoint presentation subheader_2
    if (header_is("\x0F\x00\xE8\x03"sv, 512)) return "application/vnd.ms-powerpoint";

    // sibelius music - score
    if (header_is("\x0F\x53\x49\x42\x45\x4C\x49\x55\x53"sv)) return "application/vnd.sibelius.sib";

    // easy cd creator 5 layout file
    if (header_is("\x10\x00\x00\x00"sv)) return "application/octet-stream";

    // windows prefetch file
    if (header_is("\x11\x00\x00\x00\x53\x43\x43\x41"sv)) return "application/octet-stream";

    // lotus notes database template
    if (header_is("\x1A\x00\x00"sv)) return "application/vnd.lotus-notes";

    // lotus notes database
    if (header_is("\x1A\x00\x00\x04\x00\x00"sv)) return "application/vnd.lotus-notes";

    // lh archive (old vers.-type 1)
    if (header_is("\x1A\x02"sv)) return "application/octet-stream";

    // lh archive (old vers.-type 2)
    if (header_is("\x1A\x03"sv)) return "application/octet-stream";

    // lh archive (old vers.-type 3)
    if (header_is("\x1A\x04"sv)) return "application/octet-stream";

    // lh archive (old vers.-type 4)
    if (header_is("\x1A\x08"sv)) return "application/octet-stream";

    // lh archive (old vers.-type 5)
    if (header_is("\x1A\x09"sv)) return "application/octet-stream";

    // compressed archive file
    if (header_is("\x1A\x0B"sv)) return "application/octet-stream";

    // winpharoah capture file
    if (header_is("\x1A\x35\x01\x00"sv)) return "application/winpharoah";

    // webm video file
    if (header_is("\x1A\x45\xDF\xA3"sv)) return "video/webm";

    // matroska stream file_1
    if (header_is("\x1A\x45\xDF\xA3"sv)) return "video/x-matroska";

    // matroska stream file_2
    if (header_is("\x1A\x45\xDF\xA3\x93\x42\x82\x88"sv)) return "video/x-matroska";

    // runtime software disk image
    if (header_is("\x1A\x52\x54\x53\x20\x43\x4F\x4D"sv)) return "application/octet-stream";

    // wordstar version 5.0-6.0 document
    if (header_is("\x1D\x7D"sv)) return "application/octet-stream";

    // gzip archive file
    if (header_is("\x1F\x8B\x08"sv)) return "application/gzip";

    // vlc player skin file
    if (header_is("\x1F\x8B\x08"sv)) return "application/octet-stream";

    // synology router configuration backup file
    if (header_is("\x1F\x8B\x08\x00"sv)) return "application/octet-stream";

    // compressed tape archive_1
    if (header_is("\x1F\x9D\x90"sv)) return "application/octet-stream";

    // compressed tape archive_2
    if (header_is("\x1F\xA0"sv)) return "application/octet-stream";

    // mapinfo sea chart
    if (header_is("\x21"sv)) return "application/octet-stream";

    // noaa raster navigation chart (rnc) file
    if (header_is("\x21\x0D\x0A\x43\x52\x52\x2F\x54\x68\x69\x73\x20\x65\x6C\x65\x63"sv)) return "application/octet-stream";

    // ain compressed archive
    if (header_is("\x21\x12"sv)) return "application/octet-stream";

    // unix archiver (ar)-ms program library common object file format (coff)
    if (header_is("\x21\x3C\x61\x72\x63\x68\x3E\x0A"sv)) return "application/octet-stream";

    // microsoft outlook exchange offline storage folder
    if (header_is("\x21\x42\x44\x4E"sv)) return "application/octet-stream";

    // cerius2 file
    if (header_is("\x23\x20"sv)) return "application/octet-stream";

    // vmware 4 virtual disk description
    if (header_is("\x23\x20\x44\x69\x73\x6B\x20\x44"sv)) return "application/octet-stream";

    // ms developer studio project file
    if (header_is("\x23\x20\x4D\x69\x63\x72\x6F\x73"sv)) return "application/octet-stream";

    // google earth keyhole placemark file
    if (header_is("\x23\x20\x54\x68\x69\x73\x20\x69\x73\x20\x61\x6E\x20\x4B\x65\x79"sv)) return "application/octet-stream";

    // adaptive multi-rate acelp codec (gsm)
    if (header_is("\x23\x21\x41\x4D\x52"sv)) return "audio/amr";

    // skype audio compression
    if (header_is("\x23\x21\x53\x49\x4C\x4B\x0A"sv)) return "audio/x-speex";

    // radiance high dynamic range image file
    if (header_is("\x23\x3F\x52\x41\x44\x49\x41\x4E"sv)) return "image/vnd.radiance";

    // vbscript encoded script
    if (header_is("\x23\x40\x7E\x5E"sv)) return "application/octet-stream";

    // nvidia scene graph binary file
    if (header_is("\x23\x4E\x42\x46"sv)) return "application/octet-stream";

    // brother-babylock-bernina home embroidery
    if (header_is("\x23\x50\x45\x43\x30\x30\x30\x31"sv)) return "application/octet-stream";

    // brother-babylock-bernina home embroidery
    if (header_is("\x23\x50\x45\x53\x30"sv)) return "application/octet-stream";

    // spss data file
    if (header_is("\x24\x46\x4C\x32\x40\x28\x23\x29"sv)) return "application/octet-stream";

    // encapsulated postscript file
    if (header_is("\x25\x21\x50\x53\x2D\x41\x64\x6F"sv)) return "application/postscript";

    // postscript file
    if (header_is("\x25\x21\x50\x53\x2D\x41\x64\x6F\x62\x65\x2D"sv)) return "application/postscript";

    // pdf file
    if (header_is("\x25\x50\x44\x46"sv)) return "application/pdf";

    // fuzzy bitmap (fbm) file
    if (header_is("\x25\x62\x69\x74\x6D\x61\x70"sv)) return "image/vnd.fpx";

    // binhex 4 compressed archive
    if (header_is("\x28\x54\x68\x69\x73\x20\x66\x69"sv)) return "application/mac-binhex40";

    // symantec wise installer log
    if (header_is("\x2A\x2A\x2A\x20\x20\x49\x6E\x73"sv)) return "application/octet-stream";

    // compressed archive
    if (header_is("\x2D\x6C\x68"sv, 2)) return "application/octet-stream";

    // realplayer video file (v11+)
    if (header_is("\x2E\x52\x45\x43"sv)) return "application/vnd.rn-realmedia";

    // realmedia streaming media
    if (header_is("\x2E\x52\x4D\x46"sv)) return "application/vnd.rn-realmedia";

    // realaudio file
    if (header_is("\x2E\x52\x4D\x46\x00\x00\x00\x12"sv)) return "audio/vnd.rn-realaudio";

    // realaudio streaming media
    if (header_is("\x2E\x72\x61\xFD\x00"sv)) return "audio/vnd.rn-realaudio";

    // next-sun microsystems audio file
    if (header_is("\x2E\x73\x6E\x64"sv)) return "audio/basic";

    // thunderbird-mozilla mail summary file
    if (header_is("\x2F\x2F\x20\x3C\x21\x2D\x2D\x20\x3C\x6D\x64\x62\x3A\x6D\x6F\x72\x6B\x3A\x7A"sv)) return "application/octet-stream";

    // windows event viewer file
    if (header_is("\x30\x00\x00\x00\x4C\x66\x4C\x65"sv)) return "application/octet-stream";

    // genealogical data communication (gedcom) file
    if (header_is("\x30\x20\x48\x45\x41\x44"sv)) return "application/octet-stream";

    // windows media audio-video file
    if (header_is("\x30\x26\xB2\x75\x8E\x66\xCF\x11"sv)) return "video/x-ms-wmv";

    // national transfer format map
    if (header_is("\x30\x31\x4F\x52\x44\x4E\x41\x4E"sv)) return "application/octet-stream";

    // cpio archive
    if (header_is("\x30\x37\x30\x37\x30"sv)) return "application/octet-stream";

    // ms security catalog file
    if (header_is("\x30"sv)) return "application/octet-stream";

    // ms write file_1
    if (header_is("\x31\xBE"sv)) return "application/octet-stream";

    // ms write file_2
    if (header_is("\x32\xBE"sv)) return "application/octet-stream";

    // pfaff home embroidery
    if (header_is("\x32\x03\x10\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\x00"sv)) return "application/octet-stream";

    // tcpdump capture file
    if (header_is("\x34\xCD\xB2\xA1"sv)) return "application/octet-stream";

    // 7-zip compressed file
    if (header_is("\x37\x7A\xBC\xAF\x27\x1C"sv)) return "application/x-7z-compressed";

    // zisofs compressed file
    if (header_is("\x37\xE4\x53\x96\xC9\xDB\xD6\x07"sv)) return "application/octet-stream";

    // photoshop image
    if (header_is("\x38\x42\x50\x53"sv)) return "image/vnd.adobe.photoshop";

    // surfplan kite project file
    if (header_is("\x3A\x56\x45\x52\x53\x49\x4F\x4E"sv)) return "application/octet-stream";

    // aol html mail
    if (header_is("\x3C\x21\x64\x6F\x63\x74\x79\x70"sv)) return "application/octet-stream";

    // windows script component
    if (header_is("\x3C\x3F"sv)) return "application/octet-stream";

    // advanced stream redirector
    if (header_is("\x3C"sv)) return "application/vnd.ms-asf";

    // biztalk xml-data reduced schema
    if (header_is("\x3C"sv)) return "application/octet-stream";

    // windows visual stylesheet
    if (header_is("\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D"sv)) return "application/octet-stream";

    // user interface language
    if (header_is("\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E\x30\x22\x3F\x3E"sv)) return "application/octet-stream";

    // mmc snap-in control file
    if (header_is(
                "\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E\x30\x22\x3F\x3E\x0D\x0A\x3C\x4D\x4D\x43\x5F\x43\x6F\x6E\x73\x6F\x6C\x65\x46\x69\x6C\x65\x20\x43\x6F\x6E\x73\x6F\x6C\x65\x56\x65\x72\x73\x69\x6F\x6E\x3D\x22"sv
        ))
        return "application/octet-stream";

    // picasa movie project file
    if (header_is("\x3C\x43\x54\x72\x61\x6E\x73\x54\x69\x6D\x65\x6C\x69\x6E\x65\x3E"sv)) return "application/octet-stream";

    // csound music
    if (header_is("\x3C\x43\x73\x6F\x75\x6E\x64\x53\x79\x6E\x74\x68\x65\x73\x69\x7A"sv)) return "application/octet-stream";

    // google earth keyhole overlay file
    if (header_is("\x3C\x4B\x65\x79\x68\x6F\x6C\x65\x3E"sv)) return "application/octet-stream";

    // adobe framemaker
    if (header_is("\x3C\x4D\x61\x6B\x65\x72\x46\x69"sv)) return "application/octet-stream";

    // gps exchange (v1.1)
    if (header_is("\x3C\x67\x70\x78\x20\x76\x65\x72\x73\x69\x6F\x6E\x3D\x22\x31\x2E"sv)) return "application/octet-stream";

    // base85 file
    if (header_is("\x3C\x7E\x36\x3C\x5C\x25\x5F\x30\x67\x53\x71\x68\x3B"sv)) return "application/octet-stream";

    // quatro pro for windows 7.0
    if (header_is("\x3E\x00\x03\x00\xFE\xFF\x09\x00\x06"sv, 24)) return "application/octet-stream";

    // windows help file_2
    if (header_is("\x3F\x5F\x03\x00"sv)) return "application/octet-stream";

    // endnote library file
    if (header_is("\x40\x40\x40\x20\x00\x00\x40\x40\x40\x40"sv, 32)) return "application/octet-stream";

    // analog box (abox) circuit files
    if (header_is("\x41\x42\x6F\x78"sv)) return "application/octet-stream";

    // generic autocad drawing
    if (header_is("\x41\x43\x31\x30"sv)) return "application/octet-stream";

    // steganos virtual secure drive
    if (header_is("\x41\x43\x76"sv)) return "application/octet-stream";

    // aol parameter-info files
    if (header_is("\x41\x43\x53\x44"sv)) return "application/octet-stream";

    // harvard graphics symbol graphic
    if (header_is("\x41\x4D\x59\x4F"sv)) return "application/octet-stream";

    // aol config files
    if (header_is("\x41\x4F\x4C"sv)) return "application/octet-stream";

    // aol and aim buddy list
    if (header_is("\x41\x4F\x4C\x20\x46\x65\x65\x64"sv)) return "application/octet-stream";

    // aol address book
    if (header_is("\x41\x4F\x4C\x44\x42"sv)) return "application/octet-stream";

    // aol user configuration
    if (header_is("\x41\x4F\x4C\x44\x42"sv)) return "application/octet-stream";

    // aol client preferences-settings file
    if (header_is("\x41\x4F\x4C\x49\x44\x58"sv)) return "application/octet-stream";

    // aol address book index
    if (header_is("\x41\x4F\x4C\x49\x4E\x44\x45\x58"sv)) return "application/octet-stream";

    // aol personal file cabinet
    if (header_is("\x41\x4F\x4C\x56\x4D\x31\x30\x30"sv)) return "application/octet-stream";

    // avg6 integrity database
    if (header_is("\x41\x56\x47\x36\x5F\x49\x6E\x74"sv)) return "application/octet-stream";

    // riff windows audio
    if (header_is("\x41\x56\x49\x20\x4C\x49\x53\x54"sv, 8)) return "application/octet-stream";

    // freearc compressed file
    if (header_is("\x41\x72\x43\x01"sv)) return "application/octet-stream";

    // ntfs mft (baad)
    if (header_is("\x42\x41\x41\x44"sv)) return "application/octet-stream";

    // google chrome dictionary file
    if (header_is("\x42\x44\x69\x63"sv)) return "application/octet-stream";

    // vcard
    if (header_is("\x42\x45\x47\x49\x4E\x3A\x56\x43"sv)) return "application/octet-stream";

    // speedtouch router firmware
    if (header_is("\x42\x4C\x49\x32\x32\x33"sv)) return "application/octet-stream";

    // bitmap image
    if (header_is("\x42\x4D"sv)) return "image/bmp";

    // palmpilot resource file
    if (header_is("\x42\x4F\x4F\x4B\x4D\x4F\x42\x49"sv)) return "application/octet-stream";

    // better portable graphics
    if (header_is("\x42\x50\x47\xFB"sv)) return "image/bpg";

    // bzip2 compressed archive
    if (header_is("\x42\x5A\x68"sv)) return "application/bzip2";

    // mac disk image (bz2 compressed)
    if (header_is("\x42\x5A\x68"sv)) return "application/x-apple-diskimage";

    // puffer ascii encrypted archive
    if (header_is("\x42\x65\x67\x69\x6E\x20\x50\x75\x66\x66\x65\x72"sv)) return "application/octet-stream";

    // blink compressed archive
    if (header_is("\x42\x6C\x69\x6E\x6B"sv)) return "application/octet-stream";

    // ragtime document
    if (header_is("\x43\x23\x2B\x44\xA4\x43\x4D\xA5"sv)) return "application/octet-stream";

    // ea interchange format file (iff)_3
    if (header_is("\x43\x41\x54\x20"sv)) return "application/octet-stream";

    // wordperfect dictionary
    if (header_is("\x43\x42\x46\x49\x4C\x45"sv)) return "application/octet-stream";

    // iso-9660 cd disc image
    if (header_is("\x43\x44\x30\x30\x31"sv)) return "application/x-iso9660-image";

    // riff cd audio
    if (header_is("\x43\x44\x44\x41\x66\x6D\x74\x20"sv, 8)) return "application/octet-stream";

    // compressed iso cd image
    if (header_is("\x43\x49\x53\x4F"sv)) return "application/x-iso9660-image";

    // windows 7 thumbnail
    if (header_is("\x43\x4D\x4D\x4D\x15\x00\x00\x00"sv)) return "application/octet-stream";

    // corel binary metafile
    if (header_is("\x43\x4D\x58\x31"sv)) return "application/octet-stream";

    // com+ catalog
    if (header_is("\x43\x4F\x4D\x2B"sv)) return "application/octet-stream";

    // vmware 3 virtual disk
    if (header_is("\x43\x4F\x57\x44"sv)) return "application/octet-stream";

    // corel photopaint file_1
    if (header_is("\x43\x50\x54\x37\x46\x49\x4C\x45"sv)) return "application/octet-stream";

    // corel photopaint file_2
    if (header_is("\x43\x50\x54\x46\x49\x4C\x45"sv)) return "application/octet-stream";

    // win9x registry hive
    if (header_is("\x43\x52\x45\x47"sv)) return "application/octet-stream";

    // crush compressed archive
    if (header_is("\x43\x52\x55\x53\x48\x20\x76"sv)) return "application/octet-stream";

    // shockwave flash file
    if (header_is("\x43\x57\x53"sv)) return "application/x-shockwave-flash";

    // calculux indoor lighting project file
    if (header_is("\x43\x61\x6C\x63\x75\x6C\x75\x78\x20\x49\x6E\x64\x6F\x6F\x72\x20"sv)) return "application/octet-stream";

    // whereisit catalog
    if (header_is("\x43\x61\x74\x61\x6C\x6F\x67\x20"sv)) return "application/octet-stream";

    // ie history file
    if (header_is("\x43\x6C\x69\x65\x6E\x74\x20\x55"sv)) return "application/octet-stream";

    // google chrome extension
    if (header_is("\x43\x72\x32\x34"sv)) return "application/x-chrome-extension";

    // google chromium patch update
    if (header_is("\x43\x72\x4F\x44"sv)) return "application/octet-stream";

    // creative voice
    if (header_is("\x43\x72\x65\x61\x74\x69\x76\x65\x20\x56\x6F\x69\x63\x65\x20\x46"sv)) return "application/octet-stream";

    // poweriso direct-access-archive image
    if (header_is("\x44\x41\x41\x00\x00\x00\x00\x00"sv)) return "application/x-iso9660-image";

    // dax compressed cd image
    if (header_is("\x44\x41\x58\x00"sv)) return "application/x-iso9660-image";

    // palm zire photo database
    if (header_is("\x44\x42\x46\x48"sv)) return "application/octet-stream";

    // amiga diskmasher compressed archive
    if (header_is("\x44\x4D\x53\x21"sv)) return "application/octet-stream";

    // amiga disk file
    if (header_is("\x44\x4F\x53"sv)) return "application/octet-stream";

    // dst compression
    if (header_is("\x44\x53\x54\x62"sv)) return "application/octet-stream";

    // dvr-studio stream file
    if (header_is("\x44\x56\x44"sv)) return "application/octet-stream";

    // dvd info file
    if (header_is("\x44\x56\x44"sv)) return "application/octet-stream";

    // elite plus commander game file
    if (header_is("\x45\x4C\x49\x54\x45\x20\x43\x6F"sv)) return "application/octet-stream";

    // videovcd-vcdimager file
    if (header_is("\x45\x4E\x54\x52\x59\x56\x43\x44"sv)) return "application/octet-stream";

    // apple iso 9660-hfs hybrid cd image
    if (header_is("\x45\x52\x02\x00\x00"sv)) return "application/x-iso9660-image";

    // easyrecovery saved state file
    if (header_is("\x45\x52\x46\x53\x53\x41\x56\x45"sv)) return "application/octet-stream";

    // dsd storage facility audio file
    if (header_is("\x44\x53\x44\x20"sv)) return "application/octet-stream";

    // ms document imaging file
    if (header_is("\x45\x50"sv)) return "application/octet-stream";

    // expert witness compression format
    if (header_is("\x45\x56\x46\x09\x0D\x0A\xFF\x00"sv)) return "application/octet-stream";

    // encase evidence file format v2
    if (header_is("\x45\x56\x46\x32\x0D\x0A\x81"sv)) return "application/octet-stream";

    // windows vista event log
    if (header_is("\x45\x6C\x66\x46\x69\x6C\x65\x00"sv)) return "application/octet-stream";

    // quickbooks backup
    if (header_is("\x45\x86\x00\x00\x06\x00"sv)) return "application/octet-stream";

    // ms fax cover sheet
    if (header_is("\x46\x41\x58\x43\x4F\x56\x45\x52"sv)) return "application/octet-stream";

    // fiasco database definition file
    if (header_is("\x46\x44\x42\x48\x00"sv)) return "application/octet-stream";

    // ntfs mft (file)
    if (header_is("\x46\x49\x4C\x45"sv)) return "application/octet-stream";

    // flash video file
    if (header_is("\x46\x4C\x56"sv)) return "application/octet-stream";

    // iff anim file
    if (header_is("\x46\x4F\x52\x4D"sv)) return "application/octet-stream";

    // ea interchange format file (iff)_1
    if (header_is("\x46\x4F\x52\x4D"sv)) return "application/octet-stream";

    // audio interchange file
    if (header_is("\x46\x4F\x52\x4D\x00"sv)) return "application/octet-stream";

    // dakx compressed audio
    if (header_is("\x46\x4F\x52\x4D\x00"sv)) return "application/octet-stream";

    // shockwave flash player
    if (header_is("\x46\x57\x53"sv)) return "application/x-shockwave-flash";

    // generic e-mail_2
    if (header_is("\x46\x72\x6F\x6D"sv)) return "message/rfc822";

    // gif file
    if (header_is("\x47\x49\x46\x38"sv)) return "image/gif";

    // gimp pattern file
    if (header_is("\x47\x50\x41\x54"sv)) return "application/octet-stream";

    // general regularly-distributed information (gridded) binary
    if (header_is("\x47\x52\x49\x42"sv)) return "application/octet-stream";

    // show partner graphics file
    if (header_is("\x47\x58\x32"sv)) return "application/octet-stream";

    // genetec video archive
    if (header_is("\x47\x65\x6E\x65\x74\x65\x63\x20\x4F\x6D\x6E\x69\x63\x61\x73\x74"sv)) return "application/octet-stream";

    // sap powerbuilder integrated development environment file
    if (header_is("\x48\x44\x52\x2A\x50\x6F\x77\x65\x72\x42\x75\x69\x6C\x64\x65\x72"sv)) return "application/octet-stream";

    // sas transport dataset
    if (header_is("\x48\x45\x41\x44\x45\x52\x20\x52\x45\x43\x4F\x52\x44\x2A\x2A\x2A"sv)) return "application/octet-stream";

    // harvard graphics presentation file
    if (header_is("\x48\x48\x47\x42\x31"sv)) return "application/octet-stream";

    // tiff file_1
    if (header_is("\x49\x20\x49"sv)) return "image/tiff";

    // mp3 audio file
    if (header_is("\x49\x44\x33"sv)) return "audio/mpeg";

    // sprint music store audio
    if (header_is("\x49\x44\x33\x03\x00\x00\x00"sv)) return "audio/mpeg";

    // canon raw file
    if (header_is("\x49\x49\x1A\x00\x00\x00\x48\x45"sv)) return "application/octet-stream";

    // tiff file_2
    if (header_is("\x49\x49\x2A\x00"sv)) return "image/tiff";

    // windows 7 thumbnail_2
    if (header_is("\x49\x4D\x4D\x4D\x15\x00\x00\x00"sv)) return "application/octet-stream";

    // install shield compressed file
    if (header_is("\x49\x53\x63\x28"sv)) return "application/octet-stream";

    // ms reader ebook
    if (header_is("\x49\x54\x4F\x4C\x49\x54\x4C\x53"sv)) return "application/octet-stream";

    // ms compiled html help file
    if (header_is("\x49\x54\x53\x46"sv)) return "application/octet-stream";

    // inno setup uninstall log
    if (header_is("\x49\x6E\x6E\x6F\x20\x53\x65\x74"sv)) return "application/octet-stream";

    // inter@ctive pager backup (blackberry file
    if (header_is("\x49\x6E\x74\x65\x72\x40\x63\x74\x69\x76\x65\x20\x50\x61\x67\x65"sv)) return "application/octet-stream";

    // jarcs compressed archive
    if (header_is("\x4A\x41\x52\x43\x53\x00"sv)) return "application/octet-stream";

    // aol art file_1
    if (header_is("\x4A\x47\x03\x0E"sv)) return "application/octet-stream";

    // aol art file_2
    if (header_is("\x4A\x47\x04\x0E"sv)) return "application/octet-stream";

    // vmware 4 virtual disk
    if (header_is("\x4B\x44\x4D"sv)) return "application/octet-stream";

    // kgb archive
    if (header_is("\x4B\x47\x42\x5F\x61\x72\x63\x68"sv)) return "application/octet-stream";

    // win9x printer spool file
    if (header_is("\x4B\x49\x00\x00"sv)) return "application/octet-stream";

    // kwaj (compressed) file
    if (header_is("\x4B\x57\x41\x4A\x88\xF0\x27\xD1"sv)) return "application/octet-stream";

    // windows shortcut file
    if (header_is("\x4C\x00\x00\x00\x01\x14\x02\x00"sv)) return "application/octet-stream";

    // ms coff relocatable object code
    if (header_is("\x4C\x01"sv)) return "application/octet-stream";

    // tajima emboridery
    if (header_is("\x4C\x41\x3A"sv)) return "application/octet-stream";

    // windows help file_3
    if (header_is("\x4C\x4E\x02\x00"sv)) return "application/octet-stream";

    // ea interchange format file (iff)_2
    if (header_is("\x4C\x49\x53\x54"sv)) return "application/octet-stream";

    // deluxepaint animation
    if (header_is("\x4C\x50\x46\x20\x00\x01"sv)) return "application/octet-stream";

    // logical file evidence format
    if (header_is("\x4C\x56\x46\x09\x0D\x0A\xFF\x00"sv)) return "application/octet-stream";

    // merriam-webster pocket dictionary
    if (header_is("\x4D\x2D\x57\x20\x50\x6F\x63\x6B"sv)) return "application/octet-stream";

    // mozilla archive
    if (header_is("\x4D\x41\x52\x31\x00"sv)) return "application/octet-stream";

    // microsoft-msn marc archive
    if (header_is("\x4D\x41\x52\x43"sv)) return "application/octet-stream";

    // matlab v5 workspace
    if (header_is("\x4D\x41\x54\x4C\x41\x42\x20\x35\x2E\x30\x20\x4D\x41\x54\x2D\x66\x69\x6C\x65"sv)) return "application/octet-stream";

    // mar compressed archive
    if (header_is("\x4D\x41\x72\x30\x00"sv)) return "application/octet-stream";

    // targetexpress target file
    if (header_is("\x4D\x43\x57\x20\x54\x65\x63\x68\x6E\x6F\x67\x6F\x6C\x69\x65\x73"sv)) return "application/octet-stream";

    // windows dump file
    if (header_is("\x4D\x44\x4D\x50\x93\xA7"sv)) return "application/octet-stream";

    // milestones project management file
    if (header_is("\x4D\x49\x4C\x45\x53"sv)) return "application/octet-stream";

    // skype localization data file
    if (header_is("\x4D\x4C\x53\x57"sv)) return "application/octet-stream";

    // tiff file_3
    if (header_is("\x4D\x4D\x00\x2A"sv)) return "image/tiff";

    // tiff file_4
    if (header_is("\x4D\x4D\x00\x2B"sv)) return "image/tiff";

    // yamaha synthetic music mobile application format
    if (header_is("\x4D\x4D\x4D\x44\x00\x00"sv)) return "application/octet-stream";

    // vmware bios state file
    if (header_is("\x4D\x52\x56\x4E"sv)) return "application/octet-stream";

    // microsoft cabinet file
    if (header_is("\x4D\x53\x43\x46"sv)) return "application/octet-stream";

    // onenote package
    if (header_is("\x4D\x53\x43\x46"sv)) return "application/octet-stream";

    // powerpoint packaged presentation
    if (header_is("\x4D\x53\x43\x46"sv)) return "application/octet-stream";

    // ms access snapshot viewer file
    if (header_is("\x4D\x53\x43\x46"sv)) return "application/octet-stream";

    // ole-spss-visual c++ library file
    if (header_is("\x4D\x53\x46\x54\x02\x00\x01\x00"sv)) return "application/octet-stream";

    // health level-7 data (pipe delimited) file
    if (header_is("\xD\x53\x48\x7C\x5E\x7E\x5C\x26\x7C"sv)) return "application/octet-stream";

    // microsoft windows imaging format
    if (header_is("\x4D\x53\x57\x49\x4D"sv)) return "application/octet-stream";

    // sony compressed voice file
    if (header_is("\x4D\x53\x5F\x56\x4F\x49\x43\x45"sv)) return "application/octet-stream";

    // midi sound file
    if (header_is("\x4D\x54\x68\x64"sv)) return "audio/midi";

    // yamaha piano
    if (header_is("\x4D\x54\x68\x64"sv)) return "audio/midi";

    // cd stomper pro label file
    if (header_is("\x4D\x56"sv)) return "application/octet-stream";

    // milestones project management file_1
    if (header_is("\x4D\x56\x32\x31\x34"sv)) return "application/octet-stream";

    // milestones project management file_2
    if (header_is("\x4D\x56\x32\x43"sv)) return "application/octet-stream";

    // windows-dos executable file
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // ms audio compression manager driver
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // library cache file
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // control panel application
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // font file
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // activex-ole custom control
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // ole object library
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // screen saver
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // visualbasic application
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // windows virtual device drivers
    if (header_is("\x4D\x5A"sv)) return "application/octet-stream";

    // acrobat plug-in
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "application/octet-stream";

    // directshow filter
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "application/octet-stream";

    // audition graphic filter
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00"sv)) return "application/octet-stream";

    // zonealam data file
    if (header_is("\x4D\x5A\x90\x00\x03\x00\x00\x00\x04\x00\x00\x00\xFF\xFF"sv)) return "application/octet-stream";

    // ms c++ debugging symbols file
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x43\x2F\x43\x2B\x2B\x20"sv)) return "application/octet-stream";

    // visual studio .net file
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x56\x69\x73\x75\x61\x6C"sv)) return "application/octet-stream";

    // windows media player playlist
    if (header_is("\x4D\x69\x63\x72\x6F\x73\x6F\x66\x74\x20\x57\x69\x6E\x64\x6F\x77\x73\x20\x4D\x65\x64\x69\x61\x20\x50\x6C\x61\x79\x65\x72\x20\x2D\x2D\x20"sv, 84)) return "application/octet-stream";

    // vmapsource gps waypoint database
    if (header_is("\x4D\x73\x52\x63\x66"sv)) return "application/octet-stream";

    // tomtom traffic data
    if (header_is("\x4E\x41\x56\x54\x52\x41\x46\x46"sv)) return "application/octet-stream";

    // ms windows journal
    if (header_is("\x4E\x42\x2A\x00"sv)) return "application/octet-stream";

    // nes sound file
    if (header_is("\x4E\x45\x53\x4D\x1A\x01"sv)) return "audio/x-nsf";

    // national imagery transmission format file
    if (header_is("\x4E\x49\x54\x46\x30"sv)) return "application/octet-stream";

    // agent newsreader character map
    if (header_is("\x4E\x61\x6D\x65\x3A\x20"sv)) return "application/octet-stream";

    // 1password 4 cloud keychain
    if (header_is("\x4F\x50\x43\x4C\x44\x41\x54"sv)) return "application/octet-stream";

    // psion series 3 database
    if (header_is("\x4F\x50\x4C\x44\x61\x74\x61\x62"sv)) return "application/octet-stream";

    // opentype font
    if (header_is("\x4F\x54\x54\x4F\x00"sv)) return "font/otf";

    // ogg vorbis codec compressed file
    if (header_is("\x4F\x67\x67\x53\x00\x02\x00\x00"sv)) return "audio/ogg";

    // visio-displaywrite 4 text file
    if (header_is("\x4F\x7B"sv)) return "application/octet-stream";

    // quicken quickfinder information file
    if (header_is("\x50\x00\x00\x00\x20\x00\x00\x00"sv)) return "application/octet-stream";

    // portable graymap graphic
    if (header_is("\x50\x35\x0A"sv)) return "image/x-portable-graymap";

    // quake archive file
    if (header_is("\x50\x41\x43\x4B"sv)) return "application/octet-stream";

    // windows memory dump
    if (header_is("\x50\x41\x47\x45\x44\x55"sv)) return "application/octet-stream";

    // pax password protected bitmap
    if (header_is("\x50\x41\x58"sv)) return "application/octet-stream";

    // pestpatrol data-scan strings
    if (header_is("\x50\x45\x53\x54"sv)) return "application/octet-stream";

    // pgp disk image
    if (header_is("\x50\x47\x50\x64\x4D\x41\x49\x4E"sv)) return "application/octet-stream";

    // chromagraph graphics card bitmap
    if (header_is("\x50\x49\x43\x54\x00\x08"sv)) return "application/octet-stream";

    // pkzip archive_1
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/zip";

    // android package
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/vnd.android.package-archive";

    // macos x dashboard widget
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // ms office open xml format document
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // java archive_1
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/java-archive";

    // google earth session file
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // kword document
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // opendocument template
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/vnd.oasis.opendocument.text-template";

    // microsoft open xml paper specification
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // openoffice documents
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // staroffice spreadsheet
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // windows media compressed skin file
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // mozilla browser archive
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // xml paper specification file
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // exact packager models
    if (header_is("\x50\x4B\x03\x04"sv)) return "application/octet-stream";

    // open publication structure ebook
    if (header_is("\x50\x4B\x03\x04\x0A\x00\x02\x00"sv)) return "application/epub+zip";

    // zlock pro encrypted zip
    if (header_is("\x50\x4B\x03\x04\x14\x00\x01\x00"sv)) return "application/octet-stream";

    // ms office 2007 documents
    if (header_is("\x50\x4B\x03\x04\x14\x00\x06\x00"sv)) return "application/octet-stream";

    // java archive_2
    if (header_is("\x50\x4B\x03\x04\x14\x00\x08\x00"sv)) return "application/java-archive";

    // pkzip archive_2
    if (header_is("\x50\x4B\x05\x06"sv)) return "application/zip";

    // pkzip archive_3
    if (header_is("\x50\x4B\x07\x08"sv)) return "application/zip";

    // pklite archive
    if (header_is("\x50\x4B\x4C\x49\x54\x45"sv, 30)) return "application/octet-stream";

    // pksfx self-extracting archive
    if (header_is("\x50\x4B\x53\x70\x58"sv, 526)) return "application/octet-stream";

    // windows program manager group file
    if (header_is("\x50\x4D\x43\x43"sv)) return "application/octet-stream";

    // norton disk doctor undo file
    if (header_is("\x50\x4E\x43\x49\x55\x4E\x44\x4F"sv)) return "application/octet-stream";

    // microsoft windows user state migration tool
    if (header_is("\x50\x4D\x4F\x43\x43\x4D\x4F\x43"sv)) return "application/octet-stream";

    // dreamcast sound format
    if (header_is("\x50\x53\x46\x12"sv)) return "application/octet-stream";

    // puffer encrypted archive
    if (header_is("\x50\x55\x46\x58"sv)) return "application/octet-stream";

    // parrot video encapsulation
    if (header_is("\x50\x61\x56\x45"sv)) return "application/octet-stream";

    // quicken data
    if (header_is("\x51\x45\x4C\x20"sv, 92)) return "application/octet-stream";

    // qcow disk image
    if (header_is("\x51\x46\x49"sv)) return "application/octet-stream";

    // riff qualcomm purevoice
    if (header_is("\x51\x4C\x43\x4D\x66\x6D\x74\x20"sv, 8)) return "audio/vnd.qcelp";

    // quicken data file
    if (header_is("\x51\x57\x20\x56\x65\x72\x2E\x20"sv)) return "application/octet-stream";

    // outlook-exchange message subheader
    if (header_is("\x52\x00\x6F\x00\x6F\x00\x74\x00\x20\x00\x45\x00\x6E\x00\x74\x00\x72\x00\x79\x00"sv, 512)) return "application/octet-stream";

    // shareaza (p2p) thumbnail
    if (header_is("\x52\x41\x5A\x41\x54\x44\x42\x31"sv)) return "application/octet-stream";

    // r saved work space
    if (header_is("\x52\x44\x58\x32\x0A"sv)) return "application/octet-stream";

    // winnt registry-registry undo files
    if (header_is("\x52\x45\x47\x45\x44\x49\x54"sv)) return "application/octet-stream";

    // antenna data file
    if (header_is("\x52\x45\x56\x4E\x55\x4D\x3A\x2C"sv)) return "application/octet-stream";

    // windows animated cursor
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // corel presentation exchange metadata
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // coreldraw document
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // video cd mpeg movie
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // micrografx designer graphic
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // 4x movie video
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // resource interchange file format
    if (header_is("\x52\x49\x46\x46"sv)) return "application/octet-stream";

    // riff windows midi
    if (header_is("\x52\x4D\x49\x44\x64\x61\x74\x61"sv, 8)) return "audio/midi";

    // winnt netmon capture file
    if (header_is("\x52\x54\x53\x53"sv)) return "application/octet-stream";

    // winrar compressed archive
    if (header_is("\x52\x61\x72\x21\x1A\x07\x00"sv)) return "application/octet-stream";

    // generic e-mail_1
    if (header_is("\x52\x65\x74\x75\x72\x6E\x2D\x50"sv)) return "message/rfc822";

    // windows prefetch
    if (header_is("\x53\x43\x43\x41"sv, 4)) return "application/octet-stream";

    // underground audio
    if (header_is("\x53\x43\x48\x6C"sv)) return "application/octet-stream";

    // img software bitmap
    if (header_is("\x53\x43\x4D\x49"sv)) return "application/octet-stream";

    // smpte dpx (big endian)
    if (header_is("\x53\x44\x50\x58"sv)) return "application/octet-stream";

    // harvard graphics presentation
    if (header_is("\x53\x48\x4F\x57"sv)) return "application/octet-stream";

    // sietronics cpi xrd document
    if (header_is("\x53\x49\x45\x54\x52\x4F\x4E\x49"sv)) return "application/octet-stream";

    // flexible image transport system (fits) file
    if (header_is("\x53\x49\x4D\x50\x4C\x45\x20\x20\x3D\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x54"sv)) return "application/octet-stream";

    // stuffit archive
    if (header_is("\x53\x49\x54\x21\x00"sv)) return "application/octet-stream";

    // smartdraw drawing file
    if (header_is("\x53\x4D\x41\x52\x54\x44\x52\x57"sv)) return "application/octet-stream";

    // storagecraft shadownprotect backup file
    if (header_is("\x53\x50\x46\x49\x00"sv)) return "application/octet-stream";

    // multibit bitcoin blockchain file
    if (header_is("\x53\x50\x56\x42"sv)) return "application/octet-stream";

    // sqlite database file
    if (header_is("\x53\x51\x4C\x69\x74\x65\x20\x66\x6F\x72\x6D\x61\x74\x20\x33\x00"sv)) return "application/octet-stream";

    // db2 conversion file
    if (header_is("\x53\x51\x4C\x4F\x43\x4F\x4E\x56"sv)) return "application/octet-stream";

    // qbasic szdd file
    if (header_is("\x53\x5A\x20\x88\xF0\x27\x33\xD1"sv)) return "application/octet-stream";

    // szdd file format
    if (header_is("\x53\x5A\x44\x44\x88\xF0\x27\x33"sv)) return "application/octet-stream";

    // stuffit compressed archive
    if (header_is("\x53\x74\x75\x66\x66\x49\x74\x20"sv)) return "application/octet-stream";

    // supercalc worksheet
    if (header_is("\x53\x75\x70\x65\x72\x43\x61\x6C"sv)) return "application/octet-stream";

    // wii-gamecube
    if (header_is("\x54\x48\x50\x00"sv)) return "application/octet-stream";

    // gnu info reader file
    if (header_is("\x54\x68\x69\x73\x20\x69\x73\x20"sv)) return "application/octet-stream";

    // unicode extensions
    if (header_is("\x55\x43\x45\x58"sv)) return "application/octet-stream";

    // ufa compressed archive
    if (header_is("\x55\x46\x41\xC6\xD2\xC1"sv)) return "application/octet-stream";

    // ufo capture map file
    if (header_is("\x55\x46\x4F\x4F\x72\x62\x69\x74"sv)) return "application/octet-stream";

    // measurement data format file
    if (header_is("\x55\x6E\x46\x69\x6E\x4D\x46"sv)) return "application/octet-stream";

    // visual c precompiled header
    if (header_is("\x56\x43\x50\x43\x48\x30"sv)) return "application/octet-stream";

    // visual basic user-defined control file
    if (header_is("\x56\x45\x52\x53\x49\x4F\x4E\x20"sv)) return "application/octet-stream";

    // mapinfo interchange format file
    if (header_is("\x56\x65\x72\x73\x69\x6F\x6E\x20"sv)) return "application/octet-stream";

    // spss template
    if (header_is("\x57\x04\x00\x00\x53\x50\x53\x53\x20\x74\x65\x6D\x70\x6C\x61\x74"sv)) return "application/octet-stream";

    // riff windows audio
    if (header_is("\x57\x41\x56\x45\x66\x6D\x74\x20"sv, 8)) return "audio/x-wav";

    // riff webp
    if (header_is("\x57\x45\x42\x50"sv, 8)) return "image/webp";

    // walkman mp3 file
    if (header_is("\x57\x4D\x4D\x50"sv)) return "audio/mpeg";

    // wordstar for windows file
    if (header_is("\x57\x53\x32\x30\x30\x30"sv)) return "application/octet-stream";

    // winzip compressed archive
    if (header_is("\x57\x69\x6E\x5A\x69\x70"sv, 29152)) return "application/octet-stream";

    // lotus wordpro file
    if (header_is("\x57\x6F\x72\x64\x50\x72\x6F"sv)) return "application/octet-stream";

    // exchange e-mail
    if (header_is("\x58\x2D"sv)) return "application/octet-stream";

    // packet sniffer files
    if (header_is("\x58\x43\x50\x00"sv)) return "application/octet-stream";

    // xpcom libraries
    if (header_is("\x58\x50\x43\x4F\x4D\x0A\x54\x79"sv)) return "application/octet-stream";

    // smpte dpx file (little endian)
    if (header_is("\x58\x50\x44\x53"sv)) return "application/octet-stream";

    // ms publisher
    if (header_is("\x58\x54"sv)) return "application/octet-stream";

    // zoo compressed archive
    if (header_is("\x5A\x4F\x4F\x20"sv)) return "application/octet-stream";

    // macromedia shockwave flash
    if (header_is("\x5A\x57\x53"sv)) return "application/octet-stream";

    // ms exchange configuration file
    if (header_is("\x5B\x47\x65\x6E\x65\x72\x61\x6C"sv)) return "application/octet-stream";

    // visual c++ workbench info file
    if (header_is("\x5B\x4D\x53\x56\x43"sv)) return "application/octet-stream";

    // dial-up networking file
    if (header_is("\x5B\x50\x68\x6F\x6E\x65\x5D"sv)) return "application/octet-stream";

    // lotus ami pro document_1
    if (header_is("\x5B\x56\x45\x52\x5D"sv)) return "application/octet-stream";

    // vocaltec voip media file
    if (header_is("\x5B\x56\x4D\x44\x5D"sv)) return "application/octet-stream";

    // microsoft code page translation file
    if (header_is("\x5B\x57\x69\x6E\x64\x6F\x77\x73"sv)) return "application/octet-stream";

    // flight simulator aircraft configuration
    if (header_is("\x5B\x66\x6C\x74\x73\x69\x6D\x2E"sv)) return "application/octet-stream";

    // winamp playlist
    if (header_is("\x5B\x70\x6C\x61\x79\x6C\x69\x73\x74\x5D"sv)) return "application/octet-stream";

    // lotus ami pro document_2
    if (header_is("\x5B\x76\x65\x72\x5D"sv)) return "application/octet-stream";

    // husqvarna designer
    if (header_is("\x5D\xFC\xC8\x00"sv)) return "application/octet-stream";

    // jar archive
    if (header_is("\x5F\x27\xA8\x89"sv)) return "application/java-archive";

    // encase case file
    if (header_is("\x5F\x43\x41\x53\x45\x5F"sv)) return "application/octet-stream";

    // compressed archive file
    if (header_is("\x60\xEA"sv)) return "application/octet-stream";

    // uuencoded file
    if (header_is("\x62\x65\x67\x69\x6E"sv)) return "application/octet-stream";

    // uuencoded base64 file
    if (header_is("\x62\x65\x67\x69\x6E\x2D\x62\x61\x73\x65\x36\x34"sv)) return "application/octet-stream";

    // binary property list (plist)
    if (header_is("\x62\x70\x6C\x69\x73\x74"sv)) return "application/octet-stream";

    // apple core audio file
    if (header_is("\x63\x61\x66\x66"sv)) return "audio/x-caf";

    // macintosh encrypted disk image (v1)
    if (header_is("\x63\x64\x73\x61\x65\x6E\x63\x72"sv)) return "application/octet-stream";

    // virtual pc hd image
    if (header_is("\x63\x6F\x6E\x65\x63\x74\x69\x78"sv)) return "application/octet-stream";

    // photoshop custom shape
    if (header_is("\x63\x75\x73\x68\x00\x00\x00\x02"sv)) return "application/octet-stream";

    // intel proset-wireless profile
    if (header_is("\x64\x00\x00\x00"sv)) return "application/octet-stream";

    // torrent file
    if (header_is("\x64\x38\x3A\x61\x6E\x6E\x6F\x75\x6E\x63\x65"sv)) return "application/octet-stream";

    // dalvik (android) executable file
    if (header_is("\x64\x65\x78\x0A"sv)) return "application/octet-stream";

    // audacity audio file
    if (header_is("\x64\x6E\x73\x2E"sv)) return "application/octet-stream";

    // ms visual studio workspace file
    if (header_is("\x64\x73\x77\x66\x69\x6C\x65"sv)) return "application/octet-stream";

    // macintosh encrypted disk image (v2)
    if (header_is("\x65\x6E\x63\x72\x63\x64\x73\x61"sv)) return "application/octet-stream";

    // winnt printer spool file
    if (header_is("\x66\x49\x00\x00"sv)) return "application/octet-stream";

    // free lossless audio codec file
    if (header_is("\x66\x4C\x61\x43\x00\x00\x00\x22"sv)) return "audio/flac";

    // mpeg-4 video file_1
    if (header_is("\x66\x74\x79\x70\x33\x67\x70\x35"sv, 4)) return "video/mp4";

    // apple lossless audio codec file
    if (header_is("\x66\x74\x79\x70\x4D\x34\x41\x20"sv, 4)) return "audio/x-m4a";

    // iso media-mpeg v4-itunes avc-lc
    if (header_is("\x66\x74\x79\x70\x4D\x34\x56\x20"sv, 4)) return "video/mp4";

    // mpeg-4 video file_2
    if (header_is("\x66\x74\x79\x70\x4D\x53\x4E\x56"sv, 4)) return "video/mp4";

    // iso base media file (mpeg-4) v1
    if (header_is("\x66\x74\x79\x70\x69\x73\x6F\x6D"sv, 4)) return "video/mp4";

    // mpeg-4 video-quicktime file
    if (header_is("\x66\x74\x79\x70\x6D\x70\x34\x32"sv, 4)) return "video/mp4";

    // quicktime movie_7
    if (header_is("\x66\x74\x79\x70\x71\x74\x20\x20"sv, 4)) return "video/quicktime";

    // win2000-xp printer spool file
    if (header_is("\x67\x49\x00\x00"sv)) return "application/octet-stream";

    // gimp file
    if (header_is("\x67\x69\x6d\x70\x20\x78\x63\x66"sv)) return "application/octet-stream";

    // win server 2003 printer spool file
    if (header_is("\x68\x49\x00\x00"sv)) return "application/octet-stream";

    // macos icon file
    if (header_is("\x69\x63\x6E\x73"sv)) return "image/x-icon";

    // skype user data file
    if (header_is("\x6C\x33\x33\x6C"sv)) return "application/octet-stream";

    // quicktime movie_1
    if (header_is("\x6D\x6F\x6F\x76"sv, 4)) return "video/quicktime";

    // quicktime movie_2
    if (header_is("\x66\x72\x65\x65"sv, 4)) return "video/quicktime";

    // quicktime movie_3
    if (header_is("\x6D\x64\x61\x74"sv, 4)) return "video/quicktime";

    // quicktime movie_4
    if (header_is("\x77\x69\x64\x65"sv, 4)) return "video/quicktime";

    // quicktime movie_5
    if (header_is("\x70\x6E\x6F\x74"sv, 4)) return "video/quicktime";

    // quicktime movie_6
    if (header_is("\x73\x6B\x69\x70"sv, 4)) return "video/quicktime";

    // internet explorer v11 tracking protection list
    if (header_is("\x6D\x73\x46\x69\x6C\x74\x65\x72\x4C\x69\x73\x74"sv)) return "application/octet-stream";

    // multibit bitcoin wallet information
    if (header_is("\x6D\x75\x6C\x74\x69\x42\x69\x74\x2E\x69\x6E\x66\x6F"sv)) return "application/octet-stream";

    // sms text (sim)
    if (header_is("\x6F\x3C"sv)) return "application/octet-stream";

    // 1password 4 cloud keychain encrypted data
    if (header_is("\x6F\x70\x64\x61\x74\x61\x30\x31"sv)) return "application/octet-stream";

    // winnt registry file
    if (header_is("\x72\x65\x67\x66"sv)) return "application/octet-stream";

    // sonic foundry acid music file
    if (header_is("\x72\x69\x66\x66"sv)) return "application/octet-stream";

    // realmedia metafile
    if (header_is("\x72\x74\x73\x70\x3A\x2F\x2F"sv)) return "application/octet-stream";

    // allegro generic packfile (compressed)
    if (header_is("\x73\x6C\x68\x21"sv)) return "application/octet-stream";

    // allegro generic packfile (uncompressed)
    if (header_is("\x73\x6C\x68\x2E"sv)) return "application/octet-stream";

    // palmos supermemo
    if (header_is("\x73\x6D\x5F"sv)) return "application/octet-stream";

    // stl (stereolithography) file
    if (header_is("\x73\x6F\x6C\x69\x64"sv)) return "application/octet-stream";

    // cals raster bitmap
    if (header_is("\x73\x72\x63\x64\x6F\x63\x69\x64"sv)) return "application/octet-stream";

    // powerbasic debugger symbols
    if (header_is("\x73\x7A\x65\x7A"sv)) return "application/octet-stream";

    // pathway map file
    if (header_is("\x74\x42\x4D\x50\x4B\x6E\x57\x72"sv, 60)) return "application/octet-stream";

    // truetype font
    if (header_is("\x74\x72\x75\x65\x00"sv)) return "application/octet-stream";

    // tape archive
    if (header_is("\x75\x73\x74\x61\x72"sv, 257)) return "application/octet-stream";

    // openexr bitmap image
    if (header_is("\x76\x2F\x31\x01"sv)) return "application/octet-stream";

    // qimage filter
    if (header_is("\x76\x32\x30\x30\x33\x2E\x31\x30"sv)) return "application/octet-stream";

    // web open font format 2
    if (header_is("\x77\x4F\x46\x32"sv)) return "application/octet-stream";

    // web open font format
    if (header_is("\x77\x4F\x46\x46"sv)) return "application/octet-stream";

    // macos x image file
    if (header_is("\x78\x01\x73\x0D\x62\x62\x60"sv)) return "application/octet-stream";

    // extensible archive file
    if (header_is("\x78\x61\x72\x21"sv)) return "application/octet-stream";

    // zoombrowser image index
    if (header_is("\x7A\x62\x65\x78"sv)) return "application/octet-stream";

    // windows application log
    if (header_is("\x7B\x0D\x0A\x6F\x20"sv)) return "application/octet-stream";

    // google drive drawing link
    if (header_is("\x7B\x22\x75\x72\x6C\x22\x3A\x20\x22\x68\x74\x74\x70\x73\x3A\x2F"sv)) return "application/octet-stream";

    // ms winmobile personal note
    if (header_is("\x7B\x5C\x70\x77\x69"sv)) return "application/octet-stream";

    // rich text format
    if (header_is("\x7B\x5C\x72\x74\x66\x31"sv)) return "text/rtf";

    // huskygram poem or singer embroidery
    if (header_is("\x7C\x4B\xC3\x74\xE1\xC8\x53\xA4\x79\xB9\x01\x1D\xFC\x4F\xDD\x13"sv)) return "application/octet-stream";

    // corel paint shop pro image
    if (header_is("\x7E\x42\x4B\x00"sv)) return "application/octet-stream";

    // easy street draw diagram file
    if (header_is("\x7E\x45\x53\x44\x77\xF6\x85\x3E\xBF\x6A\xD2\x11\x45\x61\x73\x79\x20\x53\x74\x72\x65\x65\x74\x20\x44\x72\x61\x77"sv)) return "application/octet-stream";

    // digital watchdog dw-tp-500g audio
    if (header_is("\x7E\x74\x2C\x01\x50\x70\x02\x4D\x52"sv)) return "application/octet-stream";

    // elf executable
    if (header_is("\x7F\x45\x4C\x46"sv)) return "application/x-executable";

    // relocatable object code
    if (header_is("\x80"sv)) return "application/x-object";

    // dreamcast audio
    if (header_is("\x80\x00\x00\x20\x03\x12\x04"sv)) return "application/octet-stream";

    // kodak cineon image
    if (header_is("\x80\x2A\x5F\xD7"sv)) return "application/octet-stream";

    // outlook express address book (win95)
    if (header_is("\x81\x32\x84\xC1\x85\x05\xD0\x11"sv)) return "application/octet-stream";

    // wordperfect text
    if (header_is("\x81\xCD\xAB"sv)) return "application/octet-stream";

    // png image
    if (header_is("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"sv)) return "image/png";

    // ms answer wizard
    if (header_is("\x8A\x01\x09\x00\x00\x00\xE1\x08"sv)) return "application/octet-stream";

    // hamarsoft compressed archive
    if (header_is("\x91\x33\x48\x46"sv)) return "application/octet-stream";

    // pgp secret keyring_1
    if (header_is("\x95\x00"sv)) return "application/octet-stream";

    // pgp secret keyring_2
    if (header_is("\x95\x01"sv)) return "application/octet-stream";

    // jbog2 image file
    if (header_is("\x97\x4A\x42\x32\x0D\x0A\x1A\x0A"sv)) return "image/x-jbig2";

    // gpg public keyring
    if (header_is("\x99"sv)) return "application/octet-stream";

    // pgp public keyring
    if (header_is("\x99\x01"sv)) return "application/octet-stream";

    // outlook address file
    if (header_is("\x9C\xCB\xCB\x8D\x13\x75\xD2\x11"sv)) return "application/octet-stream";

    // tcpdump (libpcap) capture file
    if (header_is("\xA1\xB2\xC3\xD4"sv)) return "application/octet-stream";

    // extended tcpdump (libpcap) capture file
    if (header_is("\xA1\xB2\xCD\x34"sv)) return "application/octet-stream";

    // access data ftk evidence
    if (header_is("\xA9\x0D\x00\x00\x00\x00\x00\x00"sv)) return "application/octet-stream";

    // khronos texture file
    if (header_is("\xAB\x4B\x54\x58\x20\x31\x31\xBB\x0D\x0A\x1A\x0A"sv)) return "image/ktx";

    // quicken data
    if (header_is("\xAC\x9E\xBD\x8F\x00\x00"sv)) return "application/octet-stream";

    // powerpoint presentation subheader_3
    if (header_is("\xA0\x46\x1D\xF0"sv, 512)) return "application/vnd.ms-powerpoint";

    // java serialization data
    if (header_is("\xAC\xED"sv)) return "application/octet-stream";

    // bgblitz position database file
    if (header_is("\xAC\xED\x00\x05\x73\x72\x00\x12"sv)) return "application/octet-stream";

    // win95 password file
    if (header_is("\xB0\x4D\x46\x43"sv)) return "application/octet-stream";

    // pcx bitmap
    if (header_is("\xB1\x68\xDE\x3A"sv)) return "image/x-pcx";

    // acronis true image_1
    if (header_is("\xB4\x6E\x68\x44"sv)) return "application/octet-stream";

    // windows calendar
    if (header_is("\xB5\xA2\xB0\xB3\xB3\xB0\xA5\xB5"sv)) return "application/octet-stream";

    // installshield script
    if (header_is("\xB8\xC9\x0C\x00"sv)) return "application/octet-stream";

    // ms write file_3
    if (header_is("\xBE\x00\x00\x00\xAB"sv)) return "application/octet-stream";

    // palm desktop datebook
    if (header_is("\xBE\xBA\xFE\xCA\x0F\x50\x61\x6C\x6D\x53\x47\x20\x44\x61\x74\x61"sv)) return "application/octet-stream";

    // ms agent character file
    if (header_is("\xC3\xAB\xCD\xAB"sv)) return "application/octet-stream";

    // adobe encapsulated postscript
    if (header_is("\xC5\xD0\xD3\xC6"sv)) return "application/postscript";

    // jeppesen flitelog file
    if (header_is("\xC8\x00\x79\x00"sv)) return "application/octet-stream";

    // java bytecode
    if (header_is("\xCA\xFE\xBA\xBE"sv)) return "application/octet-stream";

    // nokia phone backup file
    if (header_is("\xCC\x52\x33\xFC\xE9\x2C\x18\x48\xAF\xE3\x36\x30\x1A\x39\x40\x06"sv)) return "application/octet-stream";

    // nav quarantined virus file
    if (header_is("\xCD\x20\xAA\xAA\x02\x00\x00\x00"sv)) return "application/octet-stream";

    // acronis true image_2
    if (header_is("\xCE\x24\xB9\xA2\x20\x00\x00\x00"sv)) return "application/octet-stream";

    // java cryptography extension keystore
    if (header_is("\xCE\xCE\xCE\xCE"sv)) return "application/octet-stream";

    // os x abi mach-o binary (32-bit reverse)
    if (header_is("\xCE\xFA\xED\xFE"sv)) return "application/x-mach-binary";

    // perfect office document
    if (header_is("\xCF\x11\xE0\xA1\xB1\x1A\xE1\x00"sv)) return "application/octet-stream";

    // outlook express e-mail folder
    if (header_is("\xCF\xAD\x12\xFE"sv)) return "application/octet-stream";

    // os x abi mach-o binary (64-bit reverse)
    if (header_is("\xCF\xFA\xED\xFE"sv)) return "application/x-mach-binary";

    // microsoft office document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/msword";

    // caseware working papers
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // access project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // lotus-ibm approach 97 file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // msworks database file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // microsoft common console document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // microsoft installer package
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/x-msi";

    // microsoft installer patch
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/x-msi";

    // minitab data file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // arcmap gis project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // developer studio file options file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // ms publisher file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/x-mspublisher";

    // revit project file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // visual studio solution user options file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/octet-stream";

    // spss output file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/x-spss-sav";

    // visio file
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/vnd.visio";

    // msworks text document
    if (header_is("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"sv)) return "application/x-msworks";

    // winpharoah filter file
    if (header_is("\xD2\x0A\x00\x00"sv)) return "application/octet-stream";

    // aol history|typed url files
    if (header_is("\xD4\x2A"sv)) return "application/octet-stream";

    // windump (winpcap) capture file
    if (header_is("\xD4\xC3\xB2\xA1"sv)) return "application/octet-stream";

    // windows graphics metafile
    if (header_is("\xD7\xCD\xC6\x9A"sv)) return "application/octet-stream";

    // word 2.0 file
    if (header_is("\xDB\xA5\x2D\x00"sv)) return "application/msword";

    // corel color palette
    if (header_is("\xDC\xDC"sv)) return "application/octet-stream";

    // efax file
    if (header_is("\xDC\xFE"sv)) return "application/octet-stream";

    // amiga icon
    if (header_is("\xE3\x10\x00\x01\x00\x00\x00\x00"sv)) return "image/x-icon";

    // win98 password file
    if (header_is("\xE3\x82\x85\x96"sv)) return "application/octet-stream";

    // ms onenote note
    if (header_is("\xE4\x52\x5C\x7B\x8C\xD8\xA7\x4D"sv)) return "application/octet-stream";

    // windows executable file_1
    if (header_is("\xE8"sv)) return "application/x-msdownload";

    // windows executable file_2
    if (header_is("\xE9"sv)) return "application/x-msdownload";

    // windows executable file_3
    if (header_is("\xEB"sv)) return "application/x-msdownload";

    // gem raster file
    if (header_is("\xEB\x3C\x90\x2A"sv)) return "application/octet-stream";

    // bitlocker boot sector (vista)
    if (header_is("\xEB\x52\x90\x2D\x46\x56\x45\x2D"sv)) return "application/octet-stream";

    // bitlocker boot sector (win7)
    if (header_is("\xEB\x58\x90\x2D\x46\x56\x45\x2D"sv)) return "application/octet-stream";

    // word document subheader
    if (header_is("\xEC\xA5\xC1\x00"sv, 512)) return "application/msword";

    // redhat package manager
    if (header_is("\xED\xAB\xEE\xDB"sv)) return "application/x-rpm";

    // utf-8 file
    if (header_is("\xEF\xBB\xBF"sv)) return "text/plain";

    // windows script component (utf-8)_1
    if (header_is("\xEF\xBB\xBF\x3C"sv)) return "application/octet-stream";

    // windows script component (utf-8)_2
    if (header_is("\xEF\xBB\xBF\x3C\x3F"sv)) return "application/octet-stream";

    // youtube timed text (subtitle) file
    if (header_is("\xEF\xBB\xBF\x3C\x3F\x78\x6D\x6C\x20\x76\x65\x72\x73\x69\x6F\x6E"sv)) return "application/octet-stream";

    // fat12 file allocation table
    if (header_is("\xF0\xFF\xFF"sv)) return "application/octet-stream";

    // fat16 file allocation table
    if (header_is("\xF8\xFF\xFF\xFF"sv)) return "application/octet-stream";

    // fat32 file allocation table_1
    if (header_is("\xF8\xFF\xFF\x0F\xFF\xFF\xFF\x0F"sv)) return "application/octet-stream";

    // fat32 file allocation table_2
    if (header_is("\xF8\xFF\xFF\x0F\xFF\xFF\xFF\xFF"sv)) return "application/octet-stream";

    // bitcoin-qt blockchain block file
    if (header_is("\xF9\xBE\xB4\xD9"sv)) return "application/octet-stream";

    // xz archive
    if (header_is("\xFD\x37\x7A\x58\x5A\x00"sv)) return "application/x-xz";

    // ms publisher subheader
    if (header_is("\xFD\x37\x7A\x58\x5A\x00"sv, 512)) return "application/octet-stream";

    // thumbs.db subheader
    if (header_is("\xFD\xFF\xFF\xFF"sv, 512)) return "application/octet-stream";

    // ms publisher file subheader
    if (header_is("\xFD\xFF\xFF\xFF\x02"sv, 512)) return "application/octet-stream";

    // microsoft outlook-exchange message
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "application/octet-stream";

    // quickbooks portable company file
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "application/octet-stream";

    // visual studio solution subheader
    if (header_is("\xFD\xFF\xFF\xFF\x04"sv, 512)) return "application/octet-stream";

    // powerpoint presentation subheader_4
    if (header_is("\xFD\xFF\xFF\xFF\x0E\x00\x00\x00"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_2
    if (header_is("\xFD\xFF\xFF\xFF\x10"sv, 512)) return "application/octet-stream";

    // powerpoint presentation subheader_5
    if (header_is("\xFD\xFF\xFF\xFF\x1C\x00\x00\x00"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_3
    if (header_is("\xFD\xFF\xFF\xFF\x1F"sv, 512)) return "application/octet-stream";

    // developer studio subheader
    if (header_is("\xFD\xFF\xFF\xFF\x20"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_4
    if (header_is("\xFD\xFF\xFF\xFF\x22"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_5
    if (header_is("\xFD\xFF\xFF\xFF\x23"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_6
    if (header_is("\xFD\xFF\xFF\xFF\x28"sv, 512)) return "application/octet-stream";

    // excel spreadsheet subheader_7
    if (header_is("\xFD\xFF\xFF\xFF\x29"sv, 512)) return "application/octet-stream";

    // powerpoint presentation subheader_6
    if (header_is("\xFD\xFF\xFF\xFF\x43\x00\x00\x00"sv, 512)) return "application/octet-stream";

    // os x abi mach-o binary (32-bit)
    if (header_is("\xFE\xED\xFA\xCE"sv)) return "application/x-mach-binary";

    // os x abi mach-o binary (64-bit)
    if (header_is("\xFE\xED\xFA\xCF"sv)) return "application/x-mach-binary";

    // javakeystore
    if (header_is("\xFE\xED\xFE\xED"sv)) return "application/octet-stream";

    // symantex ghost image file
    if (header_is("\xFE\xEF"sv)) return "application/octet-stream";

    // utf-16-ucs-2 file
    if (header_is("\xFE\xFF"sv)) return "application/octet-stream";

    // windows executable
    if (header_is("\xFF"sv)) return "application/octet-stream";

    // works for windows spreadsheet
    if (header_is("\xFF\x00\x02\x00\x04\x04\x05\x54"sv)) return "application/octet-stream";

    // quickreport report
    if (header_is("\xFF\x0A\x00"sv)) return "application/octet-stream";

    // windows international code page
    if (header_is("\xFF\x46\x4F\x4E\x54"sv)) return "application/octet-stream";

    // keyboard driver file
    if (header_is("\xFF\x4B\x45\x59\x42\x20\x20\x20"sv)) return "";

    // wordperfect text and graphics
    if (header_is("\xFF\x57\x50\x43"sv)) return "application/octet-stream";

    // generic jpeg image file
    if (header_is("\xFF\xD8"sv)) return "image/jpeg";

    // jpeg-exif-spiff images
    if (header_is("\xFF\xD8\xFF"sv)) return "image/jpeg";

    // mpeg-4 aac audio
    if (header_is("\xFF\xF1"sv)) return "audio/aac";

    // mpeg-2 aac audio
    if (header_is("\xFF\xF9"sv)) return "audio/aac";

    // windows registry file
    if (header_is("\xFF\xFE"sv)) return "application/octet-stream";

    // utf-32-ucs-2 file
    if (header_is("\xFF\xFE"sv)) return "text/plain";

    // utf-32-ucs-4 file
    if (header_is("\xFF\xFE\x00\x00"sv)) return "text/plain";

    // msinfo file
    if (header_is("\xFF\xFE\x23\x00\x6C\x00\x69\x00"sv)) return "application/octet-stream";

    // dos system driver
    if (header_is("\xFF\xFF\xFF\xFF"sv)) return "application/octet-stream";

    return std::nullopt;
}
