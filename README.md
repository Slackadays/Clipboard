### ![The Clipboard Project](documentation/readme-assets/CBBanner.png)

<br>

Clipboard (or CB) is one of the most advanced clipboard managers available for your terminal, and maybe even in general.

It supports:

- copying files and directories in addition to text, 
- adding or removing contents (with regex), 
- an infinite number of compartmentalized clipboards, 
- interfacing with [GUI clipboard systems](https://github.com/Slackadays/Clipboard/wiki/GUI-Clipboard-Compat),
- multiple languages like español, português, and Türkçe,
- any POSIX/Windows platform with C++20 compat,
- flexible scripting to automate workflows,
- customizable <img src="documentation/readme-assets/eyecandy.svg" height="16px" alt="eye candy"/>,
- a unique filesystem-based implementation,
- your freedom by being 100% free and open source,
- and more!

When we first started writing CB, we needed something more versatile than other tools like `xclip`, `wl-clipboard`, or `pbcopy`, and something really easy to use by anyone out there. That's why our mantra is this:

<p align="center"> 
    <img src="documentation/readme-assets/ClipboardMakesYourLifeEasy.png" alt="CB makes your life easy." />
</p>

In short, we're here to help you save time and effort.

<br>

![CB Demo Video](documentation/readme-assets/ClipboardDemo.gif)

<p align="center" id="dummy">
    <a href="https://discord.gg/J6asnc3pEG">
        <img src="https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge&logo=discord&labelColor=rgb(55,55,55)" alt="Discord Support">
    </a>
    <a href="https://github.com/Slackadays/Clipboard/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/build-clipboard.yml?branch=main&label=BUILDS&style=for-the-badge&logo=githubactions&labelColor=rgb(55,55,55)">
        <img src="https://img.shields.io/github/actions/workflow/status/slackadays/Clipboard/lint-clipboard.yml?branch=main&label=CHECKS&style=for-the-badge&labelColor=rgb(55,55,55)">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test-clipboard.yml?branch=main&label=TESTS&style=for-the-badge&labelColor=rgb(55,55,55)">
    </a>
    <a href="https://app.codecov.io/gh/Slackadays/Clipboard">
        <img src="https://img.shields.io/codecov/c/github/slackadays/Clipboard/main?style=for-the-badge&label=COVERAGE&token=RO7KDOZ6Q2&logo=codecov&labelColor=rgb(55,55,55)">
    </a>
</p>

<br>

<p align="center">
    <img src="documentation/readme-assets/GetStartedBelow.png" alt="Get started with CB below." height="40px" />
</p>

<br>

### ![Quick Installation](documentation/readme-assets/CBQuickInstallation.png)

### <a href="https://github.com/Slackadays/Clipboard/blob/main/src/install.sh"><img src="documentation/readme-assets/AllExceptWindows.png" alt="All Except Windows" height=25px /></a>
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/src/install.sh | sh
```
### <a href="https://github.com/Slackadays/Clipboard/blob/main/src/install.ps1"><img src="documentation/readme-assets/WindowsRunAsAdmin.png" alt="Windows (run as Administrator)" height=30px /></a>
```powershell
(Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/src/install.ps1).Content | powershell
```

<br>
<br>

### <img src="documentation/readme-assets/PremadeBuilds.png" alt="Premade Builds" height=25px />

<a href="https://repology.org/project/clipboard/versions"><img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status" align="right"></a>

**Alpine Linux** (you'll need to enable the Community packages first)
```sh
apk add clipboard
```

**AUR** (Use your favorite AUR helper such as `yay`. You can also get `clipboard-bin` and `clipboard-git`)
```sh
yay -S clipboard
```

**Homebrew**
```sh
brew install clipboard
```

**Nix**
```sh
nix-env -iA nixpkgs.clipboard-jh
```

**Scoop**
```powershell
scoop install clipboard
```

**Void Linux**
```sh
sudo xbps-install -S clipboard
```

You can also get the latest revision of the Clipboard Project [from GitHub Actions](https://nightly.link/Slackadays/Clipboard/workflows/main/main) or the latest release [from GitHub Releases.](https://github.com/Slackadays/Clipboard/releases/latest)

<br>
<br>

### <img src="documentation/readme-assets/InstallManually.png" alt="Install Manually" height=25px />
You'll need CMake and C++20 support, and if you want X11 and/or Wayland compatibility, you'll also need libx11 and/or libwayland plus Wayland Protocols.

Get the latest release instead of the latest commit by adding `--branch 0.6.0` right after `git clone...`.

Change the system installation prefix by adding `-DCMAKE_INSTALL_PREFIX=/custom/prefix` to `cmake ..`, or the library install location by adding `-DCMAKE_INSTALL_LIBDIR=/custom/dir`.
```bash
$ git clone https://github.com/Slackadays/Clipboard 
$ cd Clipboard/build
$ cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
$ cmake --build .
$ cmake --install .
```

### <img src="documentation/readme-assets/Uninstall.png" alt="Uninstall" height=25px />

Remove everything listed in `install_manifest.txt`. If you're not using Windows, you can also do `xargs rm < install_manifest.txt`.

<br>
<br>

### ![How To Use](documentation/readme-assets/CBHowToUse.png)

### <img src="documentation/readme-assets/Examples.png" alt="Examples" height=25px />

```sh
$ cb copy NuclearLaunchCodes.pdf
$ cb note "Keep this a secret"
$ echo "Keep this a secret OR ELSE" | cb note
$ cb paste

$ cb cut69 MyDirectory
$ cb add69 SomeFile
$ cb remove69 SomeFile
$ cb paste69

$ cb cp_420 foo.bar NotAVirus.bar.mp3.exe
$ cb remove_420 "*.mp3\.exe"
$ cb note_420 "Some kosher content here"
$ cb sh_420

$ cb note
$ cb clr
$ cb note_420
$ cb
```

<br>

### <img src="documentation/readme-assets/Actions.png" alt="Actions" height=25px />

Add a number to the end of your action to choose which clipboard you want to use (the default is 0), or also add `_` to use a persistent clipboard. 

<details><summary> &ensp; <b>Copy</b> &emsp; <code>cb ([--]copy|[-]cp)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [([--]copy|[-]cp)][(num)|_(id)]</code></summary>

<br>

Copy a file.
```sh
$ cb copy FooFile
$ cb cp FooFile 
# These are the same!
```

Copy a file and a directory.
```sh
$ cb copy FooFile BarDir
$ cb cp FooFile BarDir 
# These are also the same!
```

Copy piped in data.
```sh
$ echo "Foobar" | cb
$ echo "Foobar" | cb copy 
# Also the same again
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

</details>

<br>

<details><summary> &ensp; <b>Cut</b> &emsp; <code>cb ([--]cut|[-]ct)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [([--]cut|[-]ct)][(num)|_(id)]</code></summary>

<br>

Cut a file.
```sh
$ cb cut FooFile
$ cb ct FooFile 
# These are the same!
```

Cut a file and a directory.
```sh
$ cb cut FooFile BarDir
$ cb ct FooFile BarDir 
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
$ cb -c hey cut "Aventura was the best bachata band" # These are the same!
```

Cut a file with spaces and many directories to clipboard "50" using the abbreviated action name.
```sh
$ cb ct50 "Aventura/God's Project/04 Un Chi Chi.flac" BarDir BazDir
```

</details>

<br>

<details><summary> &ensp; <b>Paste</b> &emsp; <code>cb ([--]paste|[-]p)[(num)|_(id)] [regex] [regexes]</code> or <code>cb [([--]paste|[-]p][(num)|_(id)] | (something)</code> or <code>cb [([--]paste|[-]p)][(num)|_(id)] > (some file)</code></summary>

<br>

Start by copying or cutting something.
```sh
$ cb copy FooFile WhyAventuraIsTheBest.pdf
```

Paste in the current working directory.
```sh
$ cb paste
$ cb p 
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
$ cb p 
# Also the same
```

Pipe everything out to some file.
```sh
$ cb paste > SomeFile
$ cb p > SomeFile
$ cb > SomeFile 
# These three versions all work great!
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
$ cb paste | cat
$ cb p | cat
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

</details>

<br>

<details><summary> &ensp; <b>Add Contents</b> &emsp; <code>cb ([--]add|[-]ad)[(num)|_(id)] (file|text) [files]</code> or <code>(something) | cb [([--]add|[-]ad)][(num)|_(id)]</code></summary>

<br>

Start by copying something.
```sh
$ cb copy FooFile
```

Add a file.
```sh
$ cb add SomeOtherFile
$ cb ad SomeOtherFile 
# Abbreviated
# CB now holds FooFile and SomeOtherFile
```

Add a directory.
```sh
$ cb add "We Broke The Rules"
$ cb ad "We Broke The Rules"
# Abbreviated
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

</details>

<br>

<details><summary> &ensp; <b>Remove Contents</b> &emsp; <code>cb ([--]remove|[-]rm)[(num)|_(id)] (regex) [regexes]</code> or <code>(some regex) | cb [([--]remove|[-]rm)][(num)|_(id)]</code></summary>

<br>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

Remove everything starting with "B"
```sh
$ cb remove "B.*"
# CB will match this against "BarDir" and "BazDir" and remove them
```

Remove everything matching a specific name
```sh
$ cb remove "BarDir"
# CB will match this against "BarDir" only and remove it
```

Now let's shcopy some raw data.
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

</details>

<br>

<details><summary> &ensp; <b>Show Contents</b> &emsp; <code>cb ([--]show|[-]sh)[(num)|_(id)] [regex] [regexes]</code> or <code>cb ([--]show|[-]sh)[(num)|_(id)] [regex] [regexes] | (something)</code></summary>

<br>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

List all the items in the clipboard.
```sh
$ cb show
$ cb sh 
# These both work great!
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

</details>

<br>

<details><summary> &ensp; <b>Clear Contents</b> &emsp; <code>cb ([--]clear|[-]clr)[(num)|_(id)]</code></summary>

<br>

Start by copying something.
```sh
$ cb copy FooFile BarDir BazDir
```

Clear the clipboard of all data.
```sh
$ cb clear
$ cb clr 
# These both work great!
```

</details>

<br>

<details><summary> &ensp; <b>Load Contents</b> &emsp; <code>cb ([--]load|[-]ld)[(num)|_(id)] [clipboard] [clipboards]</code></summary>

<br>

Start by copying something.
```sh
$ cb copy "Yo dawg! I heard you liked bachata music."
```

Load the contents of the clipboard into other clipboards.

```sh
$ cb load 1 2 3 _foo
$ cb ld 1 2 3 _foo
# Both work great!
```

Note: If you don't provide a destination clipboard, then the Load action will load the contents into the default clipboard.

Load the contents of some clipboard into the default.

```sh
$ cb load_foo
$ cb ld_foo
# Both also work great!
```

Note: This is useful if you want to load content into GUI clipboard systems, as they only connect to the default clipboard.

</details>

<br>

<details><summary> &ensp; <b>Set Note</b> &emsp; <code>cb ([--]note|[-]nt)[(num)|_(id)] (text)</code> or <code>(something) | cb [([--]note|[-]nt)][(num)|_(id)]</code></summary>

<br>

Add a personal note to a clipboard.
```sh
$ cb note "For my Aventura music collection"
$ cb nt "For my Aventura music collection"
# This also works great!
```

Add a personal note to a clipboard by piping it in.
```sh
$ echo "For my Aventura music collection" | cb note
$ echo "For my Aventura music collection" | cb nt 
# This also works great!
```

Remove a note from a clipboard.
```sh
$ cb note ""
```

</details>

<br>

<details><summary> &ensp; <b>Show Note</b> &emsp; <code>cb ([--]note|[-]nt)[(num)|_(id)]</code></summary>

<br>

Start by adding a note to a clipboard.
```sh
$ cb note "For my Aventura music collection"
```

Show the note you added.
```sh
$ cb note
```

</details>

<br>

<details><summary> &ensp; <b>Show Detailed Info</b> &emsp; <code>cb ([--]info|[-]in)[(num)|_(id)]</code></summary>

<br>

Show helpful details for a clipboard.
```sh
$ cb info
$ cb in
# Both are the same!
```

</details>

<br>

<details><summary> &ensp; <b>Show Help Message</b> &emsp; <code>cb (-h|[--]help)</code></summary>

<br>

Show the help message.
```sh
$ cb help
$ cb --help
$ cb -h 
# These three versions all work great!
```

</details>

<br>

<details><summary> &ensp; <b>Check All Clipboards' Status</b> &emsp; <code>cb [[--]status|[-]st]</code></summary>

<br>

Check the status of all clipboards that have content.
```sh
$ cb status
$ cb st
$ cb 
# These all work great!
```

</details>

<br>
<br>

### <img src="documentation/readme-assets/TipsAndTricks.png" alt="Tips And Tricks" height=25px />

Need to paste a funky symbol somewhere a lot? Just copy it to a persistent clipboard.

```sh
$ cb cp_theta θ
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

### ![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### <img src="documentation/readme-assets/Flags.png" alt="Flags" height=25px />

<details><summary> &ensp; <b><code>--all</code>, <code>-a</code></b> &emsp; Add this when clearing to clear all clipboards at once.</summary>

<br>

Start from a blank slate.
```sh
$ cb --all clear
```
WARNING! This will get rid of everything you've stored with CB, so be very careful when clearing with this option.

</details>

<br>

<details><summary> &ensp; <b><code>--clipboard (clipboard)</code>, <code>-c (clipboard)</code></b> &emsp; Add this to choose which clipboard you want to use. </summary>

<br>

Choose a non-default clipboard.
```sh
$ cb -c 5 copy Foobar
$ cb --clipboard 10 copy BarBaz
```

Copy to a temporary clipboard that doesn't start with a number.
```sh
$ cb -c SomeCB copy "A really really long sentence, and I mean really realy super DUPER long!"
```

Note: Although copying to a temporary clipboard that doesn't start with a number is impossible using the conventional method of adding it to the end of the action, this alternative method is completely supported and works great.

Choose a persistent clipboard.
```sh
$ cb -c _ copy "Generation Next"/*
```

</details>

<br>

<details><summary> &ensp; <b><code>--fast-copy</code>, <code>-fc</code></b> &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.</summary>

<br>

Copy a lot of files in much less time than before.
```sh
$ cb --fast-copy copy /usr/bin/*
```

</details>

<br>

<details><summary> &ensp; <b><code>--mime</code>, <code>-m</code></b> &emsp; Add this to request a specific content MIME type from GUI clipboard systems.</summary>

<br>

Save GUI clipboard content of a specific MIME type to the main clipboard.
```sh
$ cb --mime text/html | cb
```

</details>

<br>

<details><summary> &ensp; <b><code>--no-confirmation</code>, <code>-nc</code></b> &emsp; Add this to disable confirmation messages from CB. </summary>

<br>

Reduce distractions after showing some text content.
```sh
$ cb -nc sh | cat
```

</details>

<br>

<details><summary> &ensp; <b><code>--no-progress</code>, <code>-np</code></b> &emsp; Add this to disable progress messages from CB. </summary>

<br>

Reduce distractions while doing a search that takes a while.
```sh
$ fzf | cb -np
```

</details>
    
<br>
<br>

### <img src="documentation/readme-assets/EnvironmentVariables.png" alt="Environment Variables" height=25px />

<details><summary> &ensp; <b><code>CI</code></b> &emsp; Set this to anything to make CB overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.</summary>

<br>

Use CB in a CI script.
```sh
...
cb cp Temp/*
...
cb p
...
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_LOCALE</code></b> &emsp; Set this to the locale that only CB will use for its commands and output, like <code>en_US.UTF-8</code> or <code>es_DO.UTF-8</code>. </summary>

<br>

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

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_TMPDIR</code></b> &emsp; Set this to the directory that only CB will use to hold the items you cut or copy into a temporary directory.</summary>

<br>

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

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_PERSISTDIR</code></b> &emsp; Set this to the directory that only CB will use to hold the items you cut or copy into a persistent directory.</summary>

<br>

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

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_ALWAYS_PERSIST</code></b> &emsp; Set this to anything to make CB always use persistent clipboards.</summary>

<br>

Make everything you copy persistent.

```sh
$ export CLIPBOARD_ALWAYS_PERSIST=1
$ cb copy Foo Bar Baz
# This puts everything in the persistent directory but still with the clipboard name "0"
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_NOEMOJI</code></b> &emsp; Set this to anything to strip emojis from all messages.</summary>

<br>

Get rid of those ugly emojis.
```sh
$ export CLIPBOARD_NOEMOJI=1
$ cb cp "You won't be annoyed 😒 by these things ever again! 😏😋🥵"
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_NOGUI</code></b> &emsp; Set this to anything to disable integration with GUI clipboards.</summary>

<br>

Debug a flaky GUI system by disabling its integration with CB.

```sh
$ CLIPBOARD_NOGUI=1 cb show
$ export CLIPBOARD_NOGUI=1
$ cb show
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_NOPROGRESS</code></b> &emsp; Set this to anything to disable only progress messages from CB.</summary>

<br>

Reduce distractions while doing a search that takes a while.
```sh
$ fzf | CLIPBOARD_NOPROGRESS=1 cb
```

Disable progress messages from CB entirely.
```sh
$ export CLIPBOARD_NOPROGRESS=1
$ fzf | cb
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_SILENT</code></b> &emsp; Set this to anything to disable progress and confirmation messages from CB.</summary>

<br>

Rest in peace by seeing nothing that isn't an error.
```sh
$ export CLIPBOARD_SILENT=1
$ cb cp "I'm running out of Aventura references"
```

</details>

<br>

<details><summary> &ensp; <b><code>CLIPBOARD_THEME</code></b> &emsp; Set this to the color theme that CB will use. Choose between <code>light</code>, <code>darkhighcontrast</code>, <code>lighthighcontrast</code>, <code>amber</code>, and <code>green</code> (the default is <code>dark</code>).</summary>

<br>

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

</details>

<br>

<details><summary> &ensp; <b><code>FORCE_COLOR</code></b> &emsp; Set this to anything to make CB always show color regardless of what you set <code>NO_COLOR</code> to.</summary>

<br>

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

</details>

<br>

<details><summary> &ensp; <b><code>NO_COLOR</code></b> &emsp; Set this to anything to make CB not show any colors.</summary>

<br>

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

</details>
 
<br>
<br>

### ![Need Help?](documentation/readme-assets/NeedHelp.png)

We're here for you! Check out [the Clipboard Project Wiki](https://github.com/Slackadays/Clipboard/wiki) for even more information, ask all your burning questions in [GitHub Discussions](https://github.com/Slackadays/Clipboard/discussions), or join [the awesome Discord group](https://discord.gg/J6asnc3pEG)!

<p align="center">
    <a href="https://github.com/Slackadays/Clipboard/wiki"><img src="https://img.shields.io/badge/Docs-Wiki-green?style=for-the-badge&labelColor=rgb(55,55,55)" alt="CB Wiki">
    <a href="https://github.com/Slackadays/Clipboard/discussions"><img src="https://img.shields.io/badge/Questions%3F-Ask%20Away-red?style=for-the-badge&labelColor=rgb(55,55,55)" alt="Questions? Ask Away!"></a>
    <a href="https://discord.gg/J6asnc3pEG"><img src="https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge&logo=discord&labelColor=rgb(55,55,55)" alt="Discord Chat"></a>
    <a href="https://twitter.com/GetClipboard"><img src="https://img.shields.io/badge/FOLLOW-%40GETCLIPBOARD-blue?logo=twitter&style=for-the-badge&labelColor=rgb(55,55,55)" alt="Follow us on Twitter"></a>
</p>

<br>

### ![Thank You!](documentation/readme-assets/ThankYou.png)

Say thank you to all our beautiful contributors who have helped make the Clipboard Project incredible[.](https://www.youtube.com/watch?v=yjdHGmRKz08)
 Want to join the club? We're always accepting [new contributions](https://github.com/Slackadays/Clipboard/blob/main/.github/CONTRIBUTING.md) too.

And if you're feeling generous, feel free to give us a :star:! We appreciate every single one - including yours.
    
<br>
<br>

<p align="right"><sub><code>cb copy haters && cb > /dev/null</code></sub></p>
