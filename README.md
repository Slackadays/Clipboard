# 📋 Clipboard 
Cut, copy, and paste absolutely anything anywhere you want, all from the comfort of your terminal! This is the missing clipboard system for the command line.

- **Zero-bloat:** Clipboard rings in at mere kilobytes on most platforms.
- **Zero-config:** Clipboard infers everything out of the box.
- **Zero-effort:** Clipboard is carefully designed to be friendly to newbies and power users alike.
- **Zero-dependency:** Clipboard works on any up-to-date Windows, Linux, Android, macOS, FreeBSD, OpenBSD, NetBSD, DragonFlyBSD, or OpenIndiana system, or anything that supports C++20, all with ZERO dependencies. Yes, really!
- **Zero-legacy:** Clipboard carries zero (0) legacy baggage from the software of yore.
- **Universal:** Clipboard supports English, Spanish, Portuguese, and Turkish locales.
- **A time-saver:** Clipboard frees you from ugly, temporary directories and memorizing file locations!

![Clipboard Demo Image](CBDemo.png)

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

## Actions Install

You can also install Clipboard by downloading the latest build from GitHub Actions.

## AUR Install

Arch-Linux users can install the [clipboard](https://aur.archlinux.org/packages/clipboard), [clipboard-bin](https://aur.archlinux.org/packages/clipboard-bin), or [clipboard-git](https://aur.archlinux.org/packages/clipboard-git) AUR package.

# (Really) Simple Configuration

## `CI`

Set this environment variable to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

## `FORCE_COLOR`

Set this environment variable to make Clipboard always show color regardless of what you set `NO_COLOR` to.

## `TMPDIR`

Set this environment variable to the directory that Clipboard will use to hold the items you cut or copy.

## `NO_COLOR`

Set this environment variable to make Clipboard not show any colors.

# Painless Documentation 

[Click here](https://github.com/Slackadays/Clipboard/wiki) to go the Clipboard Wiki.

# Fast Support

[Click here](https://discord.gg/J6asnc3pEG) to go to our Discord group.
