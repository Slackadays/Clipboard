# 📋 Clipboard 
Cut, copy, and paste absolutely anything anywhere you want, all from the comfort of your terminal! This is the ultimate clipboard system for the command line.

**Clipboard is...**
- **zero-bloat**. Clipboard rings in at mere kilobytes on most platforms.
- **zero-config**. Clipboard infers everything out of the box.
- **zero-effort**. Clipboard is carefully designed to be friendly to newbies and power users alike.
- **zero-dependency**. Clipboard works on any Windows, Linux, macOS, FreeBSD, OpenBSD, or OpenIndiana system, or anything that supports C++20, all with ZERO dependencies.
- **universal**. Clipboard supports English, Spanish, Portuguese, and Turkish locales.
- **pretty**. Clipboard has colorful text baked in everywhere.** 🌈 Say adiós to boring black & white!
- **a time-saver**. Clipboard frees you from ugly, temporary directories and memorizing file locations!

**Disable colors with the NO_COLOR environment variable.

# How To Use

In all commands, you can substitute `cb` for `clipboard`. 
Add a number to the end of the action to choose which clipboard you want to use (the default is 0). 

## Copy
`clipboard copy (file) [files]`

Examples:

```
cb copy foo.txt launchcodes.doc
clipboard copy1 MyDirectory
cb copy800 bar.conf AnotherDirectory baz.txt
```
## Cut
`clipboard cut (file) [files]`

Examples:

```
cb cut bar.txt baz.docx
clipboard cut5 MyDirectory
cb cut69 bar.conf AnotherDirectory baz.txt
```
## Paste
`clipboard paste`

## Pipe In

`(something) | clipboard [copy]`

## Pipe Out

`clipboard [paste] | (something)`

or

`clipboard [paste] > (some file)`

## Show Contents
`clipboard show`

## Clear
`clipboard clear`

# Quick Installation
## Clone, Configure, & Compile 
```
git clone https://github.com/slackadays/Clipboard
cmake Clipboard
cmake --build .
```
## Install
Platforms where you have `sudo` to install software (Linux, macOS, *BSD, OI):
```
sudo cmake --install .
```
OpenBSD:
```
doas cmake --install .
```
Windows:
```
cmake --install .
```

## Uninstall
Platforms where you have `sudo` to uninstall software (Linux, macOS, FreeBSD, OI):
```
sudo xargs rm < install_manifest.txt
```
OpenBSD:
```
doas xargs rm < install_manifest.txt
```
Windows:

This is currently WIP

## AUR Install

Arch-Linux users can install the [clipboard](https://aur.archlinux.org/packages/clipboard), [clipboard-bin](https://aur.archlinux.org/packages/clipboard-bin), or [clipboard-git](https://aur.archlinux.org/packages/clipboard-git) AUR package.

# Painless Documentation 

[Click here](https://github.com/Slackadays/Clipboard/wiki) to go the Clipboard Wiki.

# Fast Support

[Click here](https://discord.gg/J6asnc3pEG) to go to our Discord group.
