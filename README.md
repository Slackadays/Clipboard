# Clipboard
The cut/copy and paste system for the command line is here. Now you can copy and paste files wherever you want, all from the comfort of the terminal. What a work saver!

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
cb copy foo.txt
clipboard copy MyDirectory
cb copy bar.conf AnotherDirectory baz.txt
```
## Cut
`clipboard cut (file) [files]`

or

`cb cut (file) [files]`

Examples:

```
cb cut foo.txt
clipboard cut MyDirectory
cb cut bar.conf AnotherDirectory baz.txt
```
## Paste
`clipboard paste`

or

`cb paste`

# Compile Clipboard
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
Right now, you can install Clipboard manually on Linux systems with this:
```
sudo cp clipboard /usr/bin
sudo ln -s /usr/bin/clipboard /usr/bin/cb
```