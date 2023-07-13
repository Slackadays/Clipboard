cb(1) -- cut, copy, and paste anything in the terminal
=====

## SYNOPSIS

**cb** \[\-\-](add|copy|cut)[(num)|_(id)] (file) [files]

(stdout|stderr) | **cb** \[\-\-](add|copy|cut)[(num)|_(id)]

**cb** \[\-\-](remove|ignore|paste)[(num)|_(id)] (regex) [regexes]

(regex) | **cb** \[\-\-](remove|ignore|paste)[(num)|_(id)]

**cb** \[\-\-](paste)[(num)|_(id)] (regex) [regexes] | (stdin)

**cb** \[\-\-](clear|edit|export|history|help|status|show|info)[(num)|_(id)]

**cb** \[\-\-](load|swap)[(num)|_(id)] (clipboard) [clipboards]

**cb** \[\-\-](note|search)[(num)|_(id)] (text)

## DESCRIPTION

**cb** is the ultimate clipboard manager for the terminal. You can cut, copy, and paste anything, anytime, anywhere with unlimited capacity, clipboards, and history. **cb** doesn't require a GUI clipboard system, but it does work with many, including X11, Wayland, Windows, macOS, Haiku, and OSC 52.

## EXAMPLES

```
cb copy FooFile
cb cut5 FooFile MyDirectory
cb info
cb remove "Foo.*"
cb paste
cb copy "Some example text"
cb | cat
cat FooFile | cb
cb ignore "My[a-z]+"
cb edit
```

## FILES

**cb** stores its temporary data in the **Clipboard** subdirectory in a system-provided temporary folder or in the **.clipboard** subdirectory in the user's home folder. **cb** is also XDG-compliant, prioritizing the relevant XDG directories over the defaults if available.

## ENVIRONMENT VARIABLES

### **CI**

Set this environment variable to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

### **CLIPBOARD_CUSTOMPERSIST**

Set this to the clipboards you want to make persistent, using regex.

### **CLIPBOARD_EDITOR**

Set this to the editor you want to use for the Edit action.

### **CLIPBOARD_HISTORY**

Set this to the maximum history size you want to keep, like 1000, 50.67gb, or 100w.

### **CLIPBOARD_LOCALE**

Set this to the locale that only CB will use for its commands and output, like en_US.UTF-8 or es_DO.UTF-8.

### **CLIPBOARD_TMPDIR**

Set this to the directory that only CB will use to hold the items you cut or copy into a temporary directory.

### **CLIPBOARD_PERSISTDIR** 

Set this to the directory that only CB will use to hold the items you cut or copy into a persistent directory.

### **CLIPBOARD_NOAUDIO**

Set this to "true" or "1" to disable audio coming from CB.

### **CLIPBOARD_NOGUI**

Set this environment variable to disable GUI clipboard integration.

### **CLIPBOARD_NOPROGRESS**

Set this to "true" or "1" to disable only progress messages from CB.

### **CLIPBOARD_NOREMOTE**

Set this to "true" or "1" to disable remote clipboard sharing.

### **CLIPBOARD_SILENT**

Set this to "true" or "1" to disable progress and confirmation messages from CB.

### **CLIPBOARD_THEME**

Set this to the color theme that CB will use. Choose between light, darkhighcontrast, lighthighcontrast, amber, and green (the default is dark).

### **FORCE_COLOR**

Set this environment variable to make Clipboard always show color regardless of what you set **NO_COLOR** to.

### **NO_COLOR**

Set this environment variable to make Clipboard not show any colors.

## FLAGS

### **\-\-all**, **-a**

Add this when clearing to clear all clipboards at once, or when searching to search all clipboards.

### **\-\-clipboard (clipboard)**, **-c (clipboard)**

Add this to choose which clipboard you want to use.

### **\-\-entry (entry)**, **-e**

Add this to choose which history entry you want to use.

### **\-\-fast-copy**,**-fc**

Add this to use links when copying, cutting, pasting, or loading. If you modify the items that you used with this flag, then the items you paste will have the same changes.

### **\-\-mime**, **-m**

Add this to request a specific content MIME type from GUI clipboard systems.

## **\-\-no-confirmation**, **-nc**

Add this to disable confirmation messages from CB.

## **\-\-no-progress**, **-np**

Add this to disable progress messages from CB.

## **\-\-bachata**

Add this for something special!

## FULL DOCUMENTATION

Full documentation is available at __https://github.com/Slackadays/Clipboard__.

## SUPPORT

Our Discord group is at __https://discord.gg/J6asnc3pEG__.

## BUGS

Report all bugs to __https://github.com/Slackadays/Clipboard__ or __https://discord.gg/J6asnc3pEG__.

## COPYRIGHT

Copyright (c) 2023 Jackson Huff.