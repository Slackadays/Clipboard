# Clipboard
Cut, copy, and paste files wherever you want, all from the comfort of your terminal! This is the ultimate clipboard system for the command line.

Clipboard...
- is tiny. Clipboard rings in at mere tens of kilobytes for most platforms.
- is simple. Clipboard takes no time to learn because there's nothing to learn.
- just works everywhere. All you need is a C++17 compiler.
- saves time. Stop making temporary directories or memorizing file locations!

## Copy
`clipboard copy (file) [files]`

or

`cb copy (file) [files]`

Examples:

```
cb copy foo.txt launchcodes.doc
clipboard copy MyDirectory
cb copy bar.conf AnotherDirectory baz.txt
```
## Cut
`clipboard cut (file) [files]`

or

`cb cut (file) [files]`

Examples:

```
cb cut bar.txt baz.docx
clipboard cut MyDirectory
cb cut bar.conf AnotherDirectory baz.txt
```
## Paste
`clipboard paste`

or

`cb paste`

# Installation
## Clone
```
git clone https://github.com/slackadays/Clipboard
```
## Compile
```
cmake Clipboard/src
cmake --build .
```
## Install
```
sudo cmake --install .
```
