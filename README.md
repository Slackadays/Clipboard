# Clipboard
The user-friendly cut-and-paste solution for the command line.

## How To Use
### Copy
To copy a file into the clipboard, just do `copy (file) [files]`. 

Examples:

```
copy foo.txt
copy MyDirectory
copy bar.conf AnotherDirectory baz.txt
```
### Cut
Cutting is identical to copying, but you use `cut` instead of `paste`. Remember, cutting removes the files from where they currently are.
### Paste
To paste all files in the clipboard, just do `paste`.

## How It Works
Clipboard is simply a convenient version of `cp` and `mv`. Copying works by copying the files with `cp` into a temporary directory. Cutting does the same, but with `mv` instead. Pasting simply copies whatever is in the temporary directory.

