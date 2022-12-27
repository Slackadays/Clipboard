clipboard(1) -- cut, copy, and paste in the terminal
=====

## SYNOPSIS

**clipboard** [copy|cp][(id)] [cut|ct][id] [paste|p][(id)] (files)

**clipboard** [show|sh][(id)] [clear|clr][(id)]

(stdout/stderr) | **clipboard** [copy|cp][(id)]

**clipboard** [paste|p][(id)] | (stdin)

**clipboard** [paste|p][(id)] > (file)

**clipboard**

## DESCRIPTION

**clipboard** lets you cut, copy, and paste files and piped data in the terminal. It lacks dependencies on any GUI clipboard system, although they can be enabled if you want. If available, you can substitute **cb** for **clipboard** as a shortcut.

Input a number into **(id)** to select which clipboard you want to use. 

## FILES

**clipboard** stores its temporary data in the **Clipboard** subdirectory in a system-provided temporary folder or in the **.clipboard** subdirectory in the user's home folder. There may be a symlink **cb** in the same directory where you installed **clipboard**.

## ENVIRONMENT VARIABLES

### **CI**

Set this environment variable to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

### **FORCE_COLOR**

Set this environment variable to make Clipboard always show color regardless of what you set **NO_COLOR** to.

### **TMPDIR**

Set this environment variable to the directory that Clipboard will use to hold the items you cut or copy.

### **NO_COLOR**

Set this environment variable to make Clipboard not show any colors.

## EXAMPLES

```
clipboard copy SomeFile.iso foobarDirectory
clipboard cut5 MyDirectory
cb cut69 bar.conf AnotherDirectory baz.txt
```

## FULL DOCUMENTATION

Full documentation is available at __https://github.com/Slackadays/Clipboard__.

## SUPPORT

Our Discord group is at __https://discord.gg/J6asnc3pEG__.

## BUGS

Report all bugs to __https://github.com/Slackadays/Clipboard__ or __https://discord.gg/J6asnc3pEG__.

## COPYRIGHT

Copyright (c) 2022 Jackson Huff.