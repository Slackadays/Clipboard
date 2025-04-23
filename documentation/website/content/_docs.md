+++
title = "Docs"
sort_by = "weight"
weight = 0
+++

# Welcome to the Clipboard Project documentation!

This page is mostly a copy of [the readme on GitHub](https://github.com/Slackadays/Clipboard#-7), but available right here for your convenience.

# Actions

<h3><b>Copy</b> &emsp; <code>cb [--](copy|cp)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [[--](copy|cp)][(num)|_(id)]</code></h3>

Copy a file.
```sh
$ cb copy FooFile
$ cb --copy FooFile
$ cb cp FooFile 
$ cb --cp FooFile
# These are the same!
```

Copy a file and a directory.
```sh
$ cb copy FooFile BarDir
# These are also the same!
```

Copy piped in data.
```sh
$ echo "Foobar" | cb
$ echo "Foobar" | cb copy 
# The "copy" action is optional here since the only possible action here in the first place is "copy"
```

Copy text directly.
```sh
$ cb copy "Aventura was the best bachata band"
```
Note: This happens instead of copying a file/directory if there is only one item present and that item doesn't exist as a file/directory.

Copy a file to the clipboard named "4"
```sh
$ cb copy4 FooFile
```

Copy piped in data to the persistent clipboard named "hello"
```sh
$ echo "Foobar" | cb copy_hello
```

Copy text to the clipboard named "hey"
```sh
$ cb --clipboard hey copy "Aventura was the best bachata band"
$ cb -c hey copy "Aventura was the best bachata band" 
# These are the same!
```

Copy a file with spaces and many directories to clipboard "50" using the abbreviated action name.
```sh
$ cb cp50 "Aventura/God's Project/04 Un Chi Chi.flac" BarDir BazDir
```

<br>

<h3><b>Cut</b> &emsp; <code>cb [--](cut|ct)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [[--](cut|ct)][(num)|_(id)]</code></h3>

Cut a file.
```sh
$ cb cut FooFile
$ cb --cut FooFile
$ cb ct FooFile 
$ cb --ct FooFile
# These are the same!
```

Cut a file and a directory.
```sh
$ cb cut FooFile BarDir
# These are also the same!
```

Cut piped in data.
```sh
$ echo "Foobar" | cb cut
```
Note: Cutting piped in data is the same as copying, except that CB will delete all content after you paste it somewhere.

Cut text directly.
```sh
$ cb cut "Hunter2"
```
Note: This happens instead of cutting a file/directory if there is only one item present and that item doesn't exist as a file/directory.

Cut a file to the clipboard named "4"
```sh
$ cb cut4 FooFile
```

Cut piped in data to the persistent clipboard named "hello"
```sh
$ echo "Foobar" | cb cut_hello
```

Cut text to the clipboard named "hey"
```sh
$ cb --clipboard hey cut "Aventura was the best bachata band"
$ cb -c hey cut "Aventura was the best bachata band"
# These are the same!
```

Cut a file with spaces and many directories to clipboard "50" using the abbreviated action name.
```sh
$ cb ct50 "Aventura/God's Project/04 Un Chi Chi.flac" BarDir BazDir
```

<br>

<h3><b>Paste</b> &emsp; <code>cb [--](paste|p)[(num)|_(id)] [regex] [regexes]</code> or <code>cb [[--](paste|p][(num)|_(id)] | (something)</code> or <code>cb [[--](paste|p)][(num)|_(id)] > (some file)</code></h3>

Start by copying or cutting something.
```sh
$ cb copy FooFile WhyAventuraIsTheBest.pdf
```

Paste in the current working directory.
```sh
$ cb paste
$ cb --paste
$ cb p
$ cb --p
# These are the same!
```
Note: If you paste after cutting, then CB will delete the original files that you cut.

Paste anything containing "Aventura."

```sh
$ cb p ".*Aventura.*"
```

Now, let's copy some raw data.
```sh
$ echo "Bananas!" | cb
```

Paste the raw data file in the current working directory.
```sh
$ cb paste
# Also the same
```

Pipe everything out to some file.
```sh
$ cb paste > SomeFile
```

Pipe everything from clipboard "42" out to some file.
```sh
$ cb paste42 > SomeFile
$ cb p42 > SomeFile
$ cb -c 42 > SomeFile 
# These three versions all work great!
```

Pipe everything out to some program.
```sh
$ cb | cat
# These three versions also all work great.
$ cb | Write-Output 
# The version for PowerShell
```

Pipe everything from persistent clipboard "2" out to some program.
```sh
$ cb paste_2 | cat
$ cb p_2 | cat
$ cb -c _2 | cat 
# These three versions also all work great.
$ cb -c _2 | Write-Output 
# The version for PowerShell
```

Note: If you paste after cutting, then CB will delete the raw data afterwards, effectively only letting you paste once.

<br>

<h3><b>Add Contents</b> &emsp; <code>cb [--](add|ad)[(num)|_(id)] (file|text) [files]</code> or <code>(something) | cb [[--](add|ad)][(num)|_(id)]</code></h3>

Start by copying something.
```sh
$ cb copy FooFile
```

Add a file.
```sh
$ cb add SomeOtherFile
$ cb --add SomeOtherFile
$ cb ad SomeOtherFile 
$ cb --ad SomeOtherFile
# CB now holds FooFile and SomeOtherFile
```

Add a directory.
```sh
$ cb add "We Broke The Rules"
```

Now let's copy some raw data.
```sh
$ cb copy "'Let me find that'"
```

Add raw data to the end of what's stored.
```sh
$ cb add " is one of Romeo Santos' catchphrases."
# The content is now: 'Let me find that' is one of Romeo Santos' catchphrases.
```

Add raw data by piping it in.
```sh
$ echo " What's yours?" | cb add 
# The content is now: 'Let me find that' is one of Romeo Santos' catchphrases. What's yours?
```

<br>

<h3><b>Remove Contents</b> &emsp; <code>cb [--](remove|rm)[(num)|_(id)] (regex) [regexes]</code> or <code>(some regex) | cb [[--](remove|rm)][(num)|_(id)]</code></h3>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

Remove everything starting with "B"
```sh
$ cb remove "B.*"
$ cb --remove "B.*"
$ cb rm "B.*"
$ cb --rm "B.*"
# CB will match this against "BarDir" and "BazDir" and remove them
```

Remove everything matching a specific name
```sh
$ cb remove "BarDir"
# CB will match this against "BarDir" only and remove it
```

Now let's copy some raw data.
```sh
$ cb copy "A bachatero is someone who makes bachata music."
```

Remove anything with a space beforehand and that ends with "-ero"
```sh
$ cb remove "(?<= ).*ero"
# The content is now: A  is someone who makes bachata music.
```

Remove anything matching "music" by piping the pattern in.
```sh
$ echo "music" | cb remove
# The content is now: A  is someone who makes bachata .
```

<br>

<h3><b>Show Contents</b> &emsp; <code>cb [--](show|sh)[(num)|_(id)] [regex] [regexes]</code> or <code>cb [--](show|sh)[(num)|_(id)] [regex] [regexes] | (something)</code></h3>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

List all the items in the clipboard.
```sh
$ cb show
$ cb --show
$ cb sh 
$ cb --sh
# These all work great!
```

Now let's copy some raw data.
```sh
$ cb copy "Those who are tired of bachata are tired of life"
```

Show the contents of the clipboard.
```sh
$ cb show
```

Show the raw filepaths of everything in the clipboard.
```sh
$ cb sh | cat
```

Show raw filepaths to a program.
```sh
$ cb copy "02 I Believe.flac"
$ eval vlc $(cb sh)
# Use the eval command here to process the raw filepath (which in this case looks like "/tmp/Clipboard/0/02 I Believe.flac") as if it were entered in a shell
# Otherwise, you'll likely get errors complaining about being unable to process quote characters.
```

<br>

<h3><b>Clear Clipboard</b> &emsp; <code>cb [--](clear|clr)[(num)|_(id)]</code></h3>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

Clear the clipboard of all data.
```sh
$ cb clear
$ cb --clear
$ cb clr 
$ cb --clr
# These all work great!
```

Clear a certain range of history entries.
```sh
$ cb clear 69-420
# Clears entries 69 through 420 inclusive
$ cb clr 0-100
# Clears entries 0 through 100 inclusive
# Note: Entry 0 is always the most recent one
```

Just clear everything.
```sh
$ cb clear -a
# Note: This will really clear everything in all clipboards!
```

<br>

<h3><b>Edit Clipboard Content</b> &emsp; <code>cb [--](edit|ed)[(num)|_(id)] [editor]</code></h3>

Start by copying some text.
```sh
$ cb copy "Hello Clipboard! This is just some example content."
```

Edit the clipboard content.
```sh
$ cb edit
$ cb --edit
$ cb ed
$ cb --ed
# These all work great!
```

Use a custom editor to edit with.
```sh
cb edit nano
cb ed vim
cb ed code
```

<br>

<h3><b>Add Script to Clipboard</b> &emsp; <code>cb [--](script|sc)[(num)|_(id)] [script path|content]</code></h3>

Start simple.
```sh
$ cb script pwd # Note: The exact output of "pwd" will vary depending on your system.
$ cb
/run/user/1000/Clipboard/0/data/0
# The output of CB will be here
/run/user/1000/Clipboard/0/data/0
```

Use a script file instead.
```sh
$ cat myscript.sh
echo "Hello Clipboard!"
echo "Here's what's in the directory:"
ls
$ cb script myscript.sh
$ cb
Hello Clipboard!
Here's what's in the directory:
rawdata.clipboard
# The output of CB will be here
Hello Clipboard!
Here's what's in the directory:
rawdata.clipboard
```

Customize during what action the script runs.
```sh
$ cb script ls --actions search,history
$ cb
$ cb history
rawdata.clipboard
# The output of CB will be here
rawdata.clipboard
$ cb search
rawdata.clipboard
# The output of CB will be here
rawdata.clipboard
$ cb copy
$
```

Customize if the script runs before, after, or both.
```sh
$ cb script ls --timings before
$ cb
rawdata.clipboard
# The output of CB will be here
$ cb script ls --timings after
$ cb
# The output of CB will be here
rawdata.clipboard
$ cb script ls --timings before,after
rawdata.clipboard
# The output of CB will be here
rawdata.clipboard
```

Customize both during what action the script runs and if the script runs before, after, or both.
```sh
$ cb script ls --actions copy --timings before
$ cb
$ cb copy
rawdata.clipboard
# The output of CB will be here
```

View the current script.
```sh
$ cb script
# Script content shows here
```

<br>

<h3><b>Load Contents</b> &emsp; <code>cb [--](load|ld)[(num)|_(id)] [clipboard] [clipboards]</code></h3>

Start by copying something.
```sh
$ cb copy "Yo dawg! I heard you liked bachata music."
```

Load the contents of the clipboard into other clipboards.

```sh
$ cb load 1 2 3 _foo
$ cb --load 1 2 3 _foo
$ cb ld 1 2 3 _foo
$ cb --ld 1 2 3 _foo
# All work great!
```

Note: If you don't provide a destination clipboard, then the Load action will load the contents into the default clipboard.

Load the contents of some clipboard into the default.

```sh
$ cb load_foo
```

Note: This is useful if you want to load content into GUI clipboard systems, as they only connect to the default clipboard.

<br>

<h3><b>Swap Contents</b> &emsp; <code>cb [--](swap|sw)[(num)|_(id)] [clipboard]</code></h3>

Start by copying something to two clipboards.
```sh
$ cb copy "After breaking up from Aventura, Romeo Santos' music just wasn't up to snuff."
$ cb copy2 "I'd just like to interject for a moment. What you're referring to as Linux, is in fact, GNU/Linux, or as I've recently taken to calling it, GNU plus Linux."
```

Swap the contents of two clipboards.

```sh
$ cb swap 2
$ cb --swap 2
$ cb sw 2
$ cb --sw 2
# All work great!

$ cb swap2
$ cb --swap2
$ cb sw2
$ cb --sw2
# Since swapping is commutative, the target can be freely swapped with the destination.
```

Note: If you don't provide a destination clipboard, then the Swap action will swap the contents into the default clipboard.

<br>

<h3><b>Import Clipboards</b> &emsp; <code>cb [--](import|imp) [source folder]</code></h3>

Start by exporting a clipboard.
```sh
$ cb copy "The reason Aventura was so popular was because the music was so well-written as well as the luscious guitar chorus effect."
$ cb export
```

Import all clipboards from a folder.
```sh
$ cb import
$ cb --import
$ cb imp
$ cb --imp
# These all work great!
```

Note: Currently, CB imports from a folder called `Exported_Clipboards`.

Choose what folder to import from.
```sh
$ cb import MySavedClipboards
```

<br>

<h3><b>Export Clipboards</b> &emsp; <code>cb [--](export|ex) [clipboard] [clipboards]</code></h3>

Start by copying something.
```sh
$ cb copy "Aventura's music is some of the most erotic you'll find anywhere."
```

Export all clipboards to a folder.
```sh
$ cb export
$ cb --export
$ cb ex
$ cb --ex
# These all work great!
```

Note: Currently, CB exports to a folder called `Exported_Clipboards`.

Choose what clipboards to export.
```sh
$ cb export 1 2 3
```

<br>

<h3><b>Queue Clipboard History</b> &emsp; <code>cb [--](history|hs)[(num)|_(id)]</code></h3>

Start by copying several things.
```sh
$ cb copy "There are at least two \"Anthony Santos\" who are known for bachata music: the \"regular\" Anthony Santos and Anthony \"Romeo\" Santos."
$ cb copy "blah blah blah"
$ cb copy "Clipboard Project is the best clipboard manager around"
```

Show the history.
```sh
$ cb history
$ cb --history
$ cb hs
$ cb --hs
# These all work great!
```

<br>

<h3><b>Get Older Clipboard Entries</b> &emsp; <code>cb [--](history|hs)[(num)|_(id)] (clipboard) [clipboards]</code></h3>

Start by copying several things.
```sh
$ cb copy "There are at least two \"Anthony Santos\" who are known for bachata music: the \"regular\" Anthony Santos and Anthony \"Romeo\" Santos."
$ cb copy "blah blah blah"
$ cb copy "Clipboard Project is the best clipboard manager around"
```

Bring an older entry (or entries) to the front.
```sh
$ cb history 1
$ cb --history 2
$ cb hs 1
$ cb --hs 1 2
# These all work great!
```

<br>

<h3><b>Set Note</b> &emsp; <code>cb [--](note|nt)[(num)|_(id)] (text)</code> or <code>(something) | cb [[--](note|nt)][(num)|_(id)]</code></h3>

Add a personal note to a clipboard.
```sh
$ cb note "For my Aventura music collection"
$ cb --note "For my Aventura music collection"
$ cb nt "For my Aventura music collection"
$ cb --nt "For my Aventura music collection"
# All work great!
```

Add a personal note to a clipboard by piping it in.
```sh
$ echo "For my Aventura music collection" | cb note
```

Remove a note from a clipboard.
```sh
$ cb note ""
```

<br>

<h3><b>Show Note</b> &emsp; <code>cb [--](note|nt)[(num)|_(id)]</code></h3>

Start by adding a note to a clipboard.
```sh
$ cb note "For my Aventura music collection"
```

Show the note you added.
```sh
$ cb note
$ cb --note
$ cb nt
$ cb --nt
```

<br>

<h3><b>Set Ignore Rules</b> &emsp; <code>cb [--](ignore|ig)[(num)|_(id)] (regex) [regexes]</code> or <code>(regex) | cb [[--](ignore|ig)][(num)|_(id)]</code></h3>

Set some kinds of content to always ignore.
```sh
$ cb ignore "(?<![A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])"
$ cb --ignore "(?<![A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])"
$ cb ig "(?<![A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])"
$ cb --ig "(?<![A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])"
# All work great!
# "(?<![A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])" is the regex for an AWS SK secret.
```

Set an ignore regex rule by piping it in.
```sh
$ echo "[abc]{10}" | cb ignore
```

Remove all ignore regex rules from a clipboard.
```sh
$ cb ignore ""
```

<br>

<h3><b>Show Ignore Rules</b> &emsp; <code>cb [--](ignore|ig)[(num)|_(id)]</code></h3>

Start by adding some ignore regex rules to a clipboard.
```sh
$ cb ignore "Foo" "Bar" "Baz"
```

Show the rules you just added.
```sh
$ cb ignore
$ cb --ignore
$ cb ig
$ cb --ig
```

<br>

<h3><b>Show Detailed Info</b> &emsp; <code>cb [--](info|in)[(num)|_(id)]</code> or <code>cb [--](info|in)[(num)|_(id)] | (something)</code></h3>

Show helpful details for a clipboard.
```sh
$ cb info
$ cb --info
$ cb in
$ cb --in
# All are the same!
```

Output these helpful details in JSON format.
```sh
$ cb info | cat
$ cb info | jq
```

<br>

<h3><b>Search Clipboard Contents</b> &emsp; <code>cb [--](search|sr)[(num)|_(id)] (query) [queries]</code></h3>

Start by copying several things.
```sh
$ cb copy Foo Bar Baz
$ cb copy "Some example content"
$ cb copy2 "Blah bleh bluh bloh"
```

Search a clipboard's contents.
```sh
$ cb search Foo
$ cb --search Blah
$ cb sr Bar
$ cb --sr Baz
# All are the same!
```

<br>

<h3><b>Show Help Message</b> &emsp; <code>cb (-h|[--]help)</code></h3>

Show the help message.
```sh
$ cb help
$ cb --help
$ cb -h 
# These three versions all work great!
```

<br>

<h3><b>Check All Clipboards' Status</b> &emsp; <code>cb [[--]status|st]</code> or <code>cb [--](status|st) | (something)</code></h3>

Check the status of all clipboards that have content.
```sh
$ cb status
$ cb --status
$ cb st
$ cb --st
$ cb 
# These all work great!
```

Get the status of all clipboards in JSON format.
```sh
$ cb status | cat
```

<br>

<h3><b>Check Your Configuration</b> &emsp; <code>cb [[--]config|cfg]</code></h3>

Check your configuration of CB.
```sh
$ cb config
$ cb --config
$ cb cfg
$ cb --cfg
# These all work great!
```

<br>

<br>

# Action Tips and Tricks

Need to paste a funky symbol somewhere a lot? Copy it to a persistent clipboard.
```sh
$ cb cp_theta θ
$ cb -c amog cp ඞ
```

Paste whatever's in the clipboard straight into your favorite text editor.
```sh
# Vim
:r !cb
# Nano
[Ctrl-T] cb
# VSCode
cb | code -
```

Copy a password securely by deleting it once you've pasted it.
```sh
$ cb cut "AVeryStrongPassword!"
$ cb | some-program
# Now gone
```

On a slow system? Cache certain things so you don't have to do them again.
```sh
$ neofetch | cb cp_neo
$ cb | cat
```
    
Yank anything sitting in your terminal without ever touching the mouse.
```sh
$ env | yank -d = -- cb
```
    
Choose a text clipboard entry to instantly copy to the main clipboard using dmenu.
```sh
cb st | jq -r '.[]' | dmenu | cb
```

Need to share or pore over log files? Copy them in one step!
```sh
$ journalctl | cb
# For systemd
$ sudo dmesg | cb
# For Linux; note that you're not running "sudo cb" here
$ cb copy logs/latest.log
# For Minecraft servers
```

Want CB to look different? Change up the color scheme.
```sh
# This one looks like The Matrix
$ export CLIPBOARD_THEME=green
$ cb
# This one is for light backgrounds
$ export CLIPBOARD_THEME=light
$ cb
# Check out the other themes too!
```

Make your own scripts that can fully automate your workflows.
```sh
#!/bin/sh
# This script does nothing except serve as an example of automating CB.
link="https://SomeWebsiteWithLotsOfContent"
wget link
cb copy *.jpg *.png
cb remove "AZ.*\.png"
cb | tar -cf foobar.tar
cb -c footar < foobar.tar
cb note "Latest files from website ABCXYZ"
```

<br>
    
<br>

# Configuration

## Flags

<h3><b><code>--all</code>, <code>-a</code> &emsp; Add this when clearing to clear all clipboards at once.</b></h3>

Start from a blank slate.
```sh
$ cb --all clear
```
WARNING! This will get rid of everything you've stored with CB, so be very careful when clearing with this option.

<br>

<h3><b><code>--clipboard (clipboard)</code>, <code>-c (clipboard)</code> &emsp; Add this to choose which clipboard you want to use. </b></h3>

Choose a non-default clipboard.
```sh
$ cb -c 5 copy Foobar
$ cb --clipboard 10 copy BarBaz
```

Copy to a temporary clipboard that doesn't start with a number.
```sh
$ cb -c SomeCB copy "A really really long sentence, and I mean really really super DUPER long!"
```

Note: Although copying to a temporary clipboard that doesn't start with a number is impossible using the conventional method of adding it to the end of the action, this alternative method is completely supported and works great.

Choose a persistent clipboard.
```sh
$ cb -c _ copy "Generation Next"/*
```

<br>

<h3><b><code>--entry (entry)</code>, <code>-e (entry)</code> &emsp; Add this to choose which history entry you want to use. </b></h3>

Choose a non-default history entry.
```sh
$ cb -e 5 copy Foobar
$ cb --entry 10 copy BarBaz
```

Note: To copy to a non-default entry, that entry must exist already.

<br>

<h3><b><code>--fast-copy</code>, <code>-fc</code> &emsp; Add this to use links when copying, cutting, pasting, or loading. If you modify the items that you used with this flag, then the items you paste will have the same changes.</b></h3>

Copy a lot of files in much less time than before.
```sh
$ cb --fast-copy copy /usr/bin/*
```

<br>

<h3><b><code>--mime</code>, <code>-m</code> &emsp; Add this to request a specific content MIME type from GUI clipboard systems.</b></h3>

Save GUI clipboard content of a specific MIME type to the main clipboard.
```sh
$ cb --mime text/html | cb
```

<br>

<h3><b><code>--no-confirmation</code>, <code>-nc</code> &emsp; Add this to disable confirmation messages from CB. </b></h3>

Reduce distractions after showing some text content.
```sh
$ cb -nc sh | cat
```

<br>

<h3><b><code>--no-progress</code>, <code>-np</code> &emsp; Add this to disable progress messages from CB. </b></h3>

Reduce distractions while doing a search that takes a while.
```sh
$ fzf | cb -np
```

<br>

<h3><b><code>--secret</code> &emsp; Add this when ignoring content to ignore a secret (or secrets) instead. </b></h3>

Ignore a password.
```sh
$ cb ignore --secret MyVerySecurePassword
$ cb ignore --secret # This will show the SHA512 hash of MyVerySecurePassword
$ cb ignore --secret MyVerySecurePassword2 "someOTHER secret1 banana"
$ cb ignore --secret # Now it will show the SHA512 hashes of MyVerySecurePassword2 and someOTHER secret1 banana
```

Clear all your passwords.
```sh
$ cb ignore --secret ""
# WARNING: Once you clear all secrets, there is no going back.
```

<br>

<h3><b><code>--bachata</code> &emsp; Add this for something special! </b></h3>

Make your life less boring.
```sh
$ cb --bachata
```


    
<br>
<br>

## Environment Variables

<h3><b><code>CI</code> &emsp; Set this to "true" or "1" to make CB overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.</b></h3>

Use CB in a CI script.
```sh
...
cb cp Temp/*
...
cb p
...
```

<br>

<h3><b><code>CLIPBOARD_ACTION</code> &emsp; CB will set this to the current action when running a script. </b></h3>

Start with a script.
```sh
$ cb script ls
$ cb history
# Now, whatever script CB runs will have access to the CLIPBOARD_ACTION environment variable, here with value "history"
```

<br>

<h3><b><code>CLIPBOARD_EDITOR</code> &emsp; Set this to the editor you want to use for the Edit action. </b></h3>

Set a custom editor to use.
```sh
$ export CLIPBOARD_EDITOR=nano
```

Note: The environment variables `EDITOR` and `VISUAL` by default take precedence if `CLIPBOARD_EDITOR` isn't set.

<br>

<h3><b><code>CLIPBOARD_HISTORY</code> &emsp; Set this to the maximum history size you want to keep, like <code>10000</code> or <code>50gb</code>. </b></h3>

`CLIPBOARD_HISTORY` supports up to 2^32 - 1 as a value. So, go ahead. Crank that b*tch up to a billion.
```sh
$ export CLIPBOARD_HISTORY=1000000000
$ cb copy "Oh yeah!"
```

Only keep a certain amount of data.
```sh
$ export CLIPBOARD_HISTORY=100tb
$ cb copy Yo_Mama.tar.gz
```

Note: You can choose between `tb`, `gb`, `mb`, `kb`, and `b` to specify amounts for terabytes, gigabytes, megabytes, kilobytes, and bytes respectively.

Only keep a certain time length.
```sh
$ export CLIPBOARD_HISTORY=52w
```

Note: You can choose between `y`, `m`, `w`, `d`, and `h` to specify amounts for years, months, weeks, days, and hours respectively.

<br>

<h3><b><code>CLIPBOARD_LOCALE</code> &emsp; Set this to the locale that only CB will use for its commands and output, like <code>en_US.UTF-8</code> or <code>es_DO.UTF-8</code>. </b></h3>

Change the locale to match what you're more comfortable with.
```sh
$ export CLIPBOARD_LOCALE=es_DO.UTF-8
$ cb cp "Amo a Aventura"

> $Env:CLIPBOARD_LOCALE=es_DO.UTF-8
# Powershell version
```

Override the locale case-by-case.
```sh
$ export CLIPBOARD_LOCALE=fr_CA.UTF-8
...
$ CLIPBOARD_LOCALE="" cb cp Foobar
```

<br>

<h3><b><code>CLIPBOARD_SCRIPT_TIMING</code> &emsp; CB will set this to the timing of the script that it runs.</b></h3>

Start with a script.
```sh
$ cb script ls
$ cb history
# Now, whatever script CB runs will have access to the CLIPBOARD_SCRIPT_TIMING environment variable, here with value "before"
# (output of "cb history")
# Now, CLIPBOARD_SCRIPT_TIMING will have value "after"
```

<br>

<h3><b><code>CLIPBOARD_TMPDIR</code> &emsp; Set this to the directory that only CB will use to hold the items you cut or copy into a temporary directory.</b></h3>

Choose a special place to put your temporary clipboards this one time.
```sh
$ CLIPBOARD_TMPDIR=/home/jackson/SomeDirectory cb copy *
```

Choose a special place to put your temporary clipboards every time.

```sh
$ export CLIPBOARD_TMPDIR=/home/jackson/SomeDirectory
$ cb copy *
> $Env:CLIPBOARD_TMPDIR = /home/jackson/SomeDirectory
# Powershell
```

Note: By default, CB uses the C++ filesystem library function `fs::temp_directory_path()` to generate the temporary directory, prioritizing `CLIPBOARD_TMPDIR` and then `XDG_RUNTIME_DIR` respectively first if CB can get a value from them.

<br>

<h3><b><code>CLIPBOARD_PERSISTDIR</code> &emsp; Set this to the directory that only CB will use to hold the items you cut or copy into a persistent directory.</b></h3>

Choose a special place to put your persistent clipboards this one time.
```sh
$ CLIPBOARD_PERSISTDIR=/home/jackson/SomeDirectory cb copy *
```

Choose a special place to put your persistent clipboards every time.

```sh
$ export CLIPBOARD_PERSISTDIR=/home/jackson/SomeDirectory
$ cb copy *
> $Env:CLIPBOARD_PERSISTDIR = /home/jackson/SomeDirectory
# Powershell
```

Note: By default, CB uses the user's home directory to generate the persistent directory, prioritizing `CLIPBOARD_PERSISTDIR` and then `XDG_STATE_HOME` first respectively if CB can get a value from them.

<br>

<h3><b><code>CLIPBOARD_CUSTOMPERSIST</code> &emsp; Set this to the clipboards you want to make persistent, using regex.</b></h3>

Make everything you copy persistent.

```sh
$ export CLIPBOARD_CUSTOMPERSIST=".*"
$ cb copy Foo Bar Baz
# This puts everything in the persistent directory but still with the clipboard name "0"
```

Make some clipboards persistent.

```sh
$ export CLIPBOARD_CUSTOMPERSIST=5
$ cb copy5 Foo Bar Baz
```

<br>

<h3><b><code>CLIPBOARD_NOAUDIO</code> &emsp; Set this to "true" or "1" to disable audio coming from CB.</b></h3>

Turn off those sound effects.
```sh
$ export CLIPBOARD_NOAUDIO=1
$ cb ffksdjfdj 
# No more error sounds after doing a nonexistent command
```

<br>

<h3><b><code>CLIPBOARD_NOGUI</code> &emsp; Set this to "true" or "1" to disable integration with GUI clipboards.</b></h3>

Debug a flaky GUI system by disabling its integration with CB.

```sh
$ CLIPBOARD_NOGUI=1 cb show
$ export CLIPBOARD_NOGUI=1
$ cb show
```

<br>

<h3><b><code>CLIPBOARD_NOPROGRESS</code> &emsp; Set this to "true" or "1" to disable only progress messages from CB.</b></h3>

Reduce distractions while doing a search that takes a while.
```sh
$ fzf | CLIPBOARD_NOPROGRESS=1 cb
```

Disable progress messages from CB entirely.
```sh
$ export CLIPBOARD_NOPROGRESS=1
$ fzf | cb
```

<br>

<h3><b><code>CLIPBOARD_NOREMOTE</code> &emsp; Set this to "true" or "1" to disable remote clipboard sharing.</b></h3>

Disable all clipboard content transfers through the terminal.
```sh
$ export CLIPBOARD_NOREMOTE=1
```

<br>

<h3><b><code>CLIPBOARD_SILENT</code> &emsp; Set this to "true" or "1" to disable progress and confirmation messages from CB.</b></h3>

Rest in peace by seeing nothing that isn't an error.
```sh
$ export CLIPBOARD_SILENT=1
$ cb cp "I'm running out of Aventura references"
```

<br>

<h3><b><code>CLIPBOARD_THEME</code> &emsp; Set this to the color theme that CB will use. Choose between <code>light</code>, <code>darkhighcontrast</code>, <code>lighthighcontrast</code>, <code>amber</code>, <code>green</code>, and <code>ansi</code> (the default is <code>dark</code>).</b></h3>

Remind yourself of the terminals of the past.
```sh
$ export CLIPBOARD_THEME=green
$ cb cp "I'm in the Matrix now"
$ export CLIPBOARD_THEME=amber
$ cb cp "Yellow terminals feel just like sitting in front of a nice campfire"
```

Make CB more accessible.
```sh
$ export CLIPBOARD_THEME=darkhighcontrast
$ cb show
```

<br>

<h3><b><code>FORCE_COLOR</code> &emsp; Set this to "true" or "1" to make CB always show color regardless of what you set <code>NO_COLOR</code> to.</b></h3>

Override somebody else's choice to disable colors.
```sh
$ export NO_COLOR=1
...
$ FORCE_COLOR=1 cb copy "There are almost no bachateros where I live right now"
```

Note: CB also supports `CLICOLOR_FORCE`.

Override somebody else's choice to disable colors, but in a different way.
```sh
$ export CLICOLOR=0
...
$ CLICOLOR_FORCE=1 cb copy "There are almost no bachateros where I live right now"
```

<br>

<h3><b><code>NO_COLOR</code> &emsp; Set this to anything to make CB not show any colors.</b></h3>

Make CB look boring.
```sh
$ export NO_COLOR=1
$ cb cp "From the 1960s until the 1990s, bachata was perceived as boring music for poor Dominicans."
```

Note: CB also supports `CLICOLOR`.

Make CB look boring, but in a different way.
```sh
$ export CLICOLOR=0
$ cb cp "In the 1990s, though, several innovative musicians reinvigorated bachata by using electric guitars with fancy effects."
```


 
<br>