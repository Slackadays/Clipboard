# Clipboard
The cut and paste system for the command line.

## Copy
`clipboard copy (file) [files]`

or

`cb copy (file) [files]`

Examples:

```
cb foo.txt
clipboard copy MyDirectory
cb copy bar.conf AnotherDirectory baz.txt
```
## Cut
`clipboard cut (file) [files]`

or

`cb cut (file) [files]`

Examples:

```
cb foo.txt
clipboard copy MyDirectory
cb copy bar.conf AnotherDirectory baz.txt
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
This part isn't ready yet.