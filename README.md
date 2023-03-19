### ![Clipboard by Jackson Huff](documentation/readme-assets/CBBanner.png)

<p align="center">
    <a href="https://github.com/Slackadays/Clipboard/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/build-clipboard.yml?branch=main&label=BUILDS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/slackadays/Clipboard/lint-clipboard.yml?branch=main&label=CHECKS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test-clipboard.yml?branch=main&label=TESTS&style=for-the-badge">
    </a>
    <a href="https://app.codecov.io/gh/Slackadays/Clipboard">
        <img src="https://img.shields.io/codecov/c/github/slackadays/Clipboard/main?style=for-the-badge&label=COVERAGE&token=RO7KDOZ6Q2">
    </a>
</p>

The Clipboard project is a _seriously_ easy-to-use clipboard manager with tons of useful features and _gorgeous_ eye candy.

That's nice, but we did even better.

Clipboard is **_<ins>your</ins>_** time and effort saver to use **anytime and anywhere**. Have a psychic with a perfect memory always by your side. Cut, copy, paste, add, remove, and make note of **anything** that dares lay in your terminal at the mere press of a button. You'll be reclined back on a sunny beach in the Caribbean sipping a succulent papaya smoothie with the love of your life in no time. Ok, so maybe that last part won't actually happen, but you get the idea:

<p align="center"> 
    <img src="documentation/readme-assets/ClipboardMakesYourLifeEasy.png" alt="Clipboard makes your life easy." />
</p>

![Clipboard Demo Video](documentation/readme-assets/ClipboardDemo.gif)

<details><summary><b>Feast your eyes on our exquisite features.</b></summary>

- **Cut, copy, or paste files, directories, text, data, or any other kind of information.**
- **Add, remove, or make note of whatever you hold with Clipboard.**
- **Store everything in an _infinite_ number of different containers at your disposal.**
- **Choose if your containers are temporary or totally persistent.**
- **Connect right with your regular desktop [which Clipboard probably supports.](https://github.com/Slackadays/Clipboard/wiki/GUI-Clipboard-Compat)**
- **Do all of this in style with Clipboard's beautiful design.**
- **Works great on anything that supports C++20, which is Linux, Windows, macOS, FreeBSD, OpenBSD, NetBSD, OpenIndiana, DragonFlyBSD, Haiku, and probably more.**
- **Speak español, português, or Türkçe? You're in luck because Clipboard's in these languages too.**
- **Love freedom? We've got your back because Clipboard's 100% free and open source under the GPLv3.**
- **Fan of creativity? Say no more as you can choose any of several color themes to make Clipboard look exactly how you want.**
- **Addicted to technical details? Have we got something real good for you, as Clipboard is currently the only program (as of this writing) to implement a filesystem-based clipboard storage system and fully support it.**

</details>

### ![Quick Installation](documentation/readme-assets/CBQuickInstallation.png)
### <a href="https://github.com/Slackadays/Clipboard/blob/main/src/install.sh"><img src="documentation/readme-assets/AllExceptWindows.png" alt="All Except Windows" height=25px /></a>
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/src/install.sh | sh
```
### <a href="https://github.com/Slackadays/Clipboard/blob/main/src/install.ps1"><img src="documentation/readme-assets/WindowsRunAsAdmin.png" alt="Windows (run as Administrator)" height=30px /></a>
```powershell
(Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/src/install.ps1).Content | powershell
```

---

### <img src="documentation/readme-assets/PremadeBuilds.png" alt="Premade Builds" height=25px />

<a href="https://repology.org/project/clipboard/versions"><img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status"></a>

You can also download Clipboard [directly from GitHub Actions.](https://nightly.link/Slackadays/Clipboard/workflows/main/main)

---

### <img src="documentation/readme-assets/InstallManually.png" alt="Install Manually" height=25px />
You'll need CMake and C++20 support, and if you want X11 and/or Wayland compatibility, libx11 and/or libwayland.

Get the latest release instead by adding `--branch 0.5.0` right after `git clone...`.

Change the system installation prefix by adding `-DCMAKE_INSTALL_PREFIX=/custom/prefix` to `cmake ..`.
```bash
$ git clone https://github.com/Slackadays/Clipboard 
$ cd Clipboard/build
$ cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
$ cmake --build .
$ cmake --install .
```

### <img src="documentation/readme-assets/Uninstall.png" alt="Uninstall" height=25px />

Remove all the files in `install_manifest.txt`. If you're not using Windows, you can also do `xargs rm < install_manifest.txt`.

### ![How To Use](documentation/readme-assets/CBHowToUse.png)

Add a number to the end of your action to choose which clipboard you want to use (the default is 0), or also add `_` to use a persistent clipboard. 

### <img src="documentation/readme-assets/Examples.png" alt="Examples" height=25px />

```
$ cb copy NuclearLaunchCodes.pdf
$ cb note "Keep this a secret"
$ cb cut69 MyDirectory
$ cb add69 SomeFile
$ cb cp_420 foo.conf NotAVirus.bar.mp3.exe baz.txt
$ cb remove_420 "*.mp3\.exe"
$ cb show_420
$ cb note
$ cb clr
$ cb status
$ cb
```

---

<details><summary> &ensp; <b>Copy</b> &emsp; <code>cb ([--]copy|[-]cp)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [([--]copy|[-]cp)][(num)|_(id)]</code></summary>

---

Copy a file.
```
$ cb copy FooFile
$ cb cp FooFile # These are the same!
```

Copy a file and a directory.
```
$ cb copy FooFile BarDir
$ cb cp FooFile BarDir # These are also the same!
```

Copy piped in data.
```
$ echo "Foobar" | cb
$ echo "Foobar" | cb copy # These are the same!
```

Copy text directly.
```
$ cb copy "Aventura was the best bachata band"
```
Note: This happens instead of copying a file/directory if there is only one item present and that item doesn't exist as a file/directory.

Copy a file to the clipboard named "4"
```
$ cb copy4 FooFile
```

Copy piped in data to the persistent clipboard named "hello"
```
$ echo "Foobar" | cb copy_hello
```

Copy text to the clipboard named "hey"
```
$ cb --clipboard hey copy "Aventura was the best bachata band"
$ cb -c hey copy "Aventura was the best bachata band" # These are the same!
```

Copy a file with spaces and many directories to clipboard "50" using the abbreviated action name.
```
$ cb cp50 "Aventura/God's Project/04 Un Chi Chi.flac" BarDir BazDir
```

</details>

---

<details><summary> &ensp; <b>Cut</b> &emsp; <code>cb ([--]cut|[-]ct)[(num)|_(id)] (file) [files]</code> or <code>(something) | cb [([--]cut|[-]ct)][(num)|_(id)]</code></summary>

---

Cut a file.
```
$ cb cut FooFile
$ cb ct FooFile # These are the same!
```

Cut a file and a directory.
```
$ cb cut FooFile BarDir
$ cb ct FooFile BarDir # These are also the same!
```

Cut piped in data.
```
$ echo "Foobar" | cb cut
```
Note: Cutting piped in data is the same as copying, except that Clipboard will delete all content after you paste it somewhere.

Cut text directly.
```
$ cb cut "Hunter2"
```
Note: This happens instead of cutting a file/directory if there is only one item present and that item doesn't exist as a file/directory.

Cut a file to the clipboard named "4"
```
$ cb cut4 FooFile
```

Cut piped in data to the persistent clipboard named "hello"
```
$ echo "Foobar" | cb cut_hello
```

Cut text to the clipboard named "hey"
```
$ cb --clipboard hey cut "Aventura was the best bachata band"
$ cb -c hey cut "Aventura was the best bachata band" # These are the same!
```

Cut a file with spaces and many directories to clipboard "50" using the abbreviated action name.
```
$ cb ct50 "Aventura/God's Project/04 Un Chi Chi.flac" BarDir BazDir
```

</details>

---

<details><summary> &ensp; <b>Paste</b> &emsp; <code>cb ([--]paste|[-]p)[(num)|_(id)]</code> or <code>cb [([--]paste|[-]p][(num)|_(id)] | (something)</code> or <code>cb [([--]paste|[-]p)][(num)|_(id)] > (some file)</code></summary>

---

Start by copying or cutting something.
```
$ cb copy FooFile WhyAventuraIsTheBest.pdf
```

Paste in the current working directory.
```
$ cb paste
$ cb p # These are the same!
```
Note: If you paste after cutting, then Clipboard will delete the original files that you cut.

Now, let's copy some raw data.
```
$ echo "Bananas!" | cb
```

Paste the raw data file in the current working directory.
```
$ cb paste
$ cb p # Also the same
```

Pipe everything out to some file.
```
$ cb paste > SomeFile
$ cb p > SomeFile
$ cb > SomeFile # These three versions all work great!
```

Pipe everything from clipboard "42" out to some file.
```
$ cb paste42 > SomeFile
$ cb p42 > SomeFile
$ cb -c 42 > SomeFile # These three versions all work great!
```

Pipe everything out to some program.
```
$ cb paste | cat
$ cb p | cat
$ cb | cat # These three versions also all work great.
$ cb | Write-Output # The version for PowerShell
```

Pipe everything from persistent clipboard "2" out to some program.
```
$ cb paste_2 | cat
$ cb p_2 | cat
$ cb -c _2 | cat # These three versions also all work great.
$ cb -c _2 | Write-Output # The version for PowerShell
```

Note: If you paste after cutting, then Clipboard will delete the raw data afterwards, effectively only letting you paste once.

</details>

---

<details><summary> &ensp; <b>Add Contents</b> &emsp; <code>cb ([--]add|[-]ad)[(num)|_(id)] (file|text) [files]</code> or <code>(something) | cb [([--]add|[-]ad)][(num)|_(id)]</code></summary>

---

Start by copying something.
```
$ cb copy FooFile
```

Add a file.
```
$ cb add SomeOtherFile
$ cb ad SomeOtherFile # Abbreviated
# Clipboard now holds FooFile and SomeOtherFile
```

Add a directory.
```
$ cb add BarDir
$ cb ad BarDir # Abbreviated
```

Now let's copy some raw data.
```
$ cb copy "'Let me find that'"
```

Add raw data to the end of what's stored.
```
$ cb add " is one of Romeo Santos' catchphrases."
# The content is now: 'Let me find that' is one of Romeo Santos' catchphrases.
```

Add raw data by piping it in.
```
$ echo " What's yours?" | cb add 
# The content is now: 'Let me find that' is one of Romeo Santos' catchphrases. What's yours?
```

</details>

---

<details><summary> &ensp; <b>Remove Contents</b> &emsp; <code>cb ([--]remove|[-]rm)[(num)|_(id)] (regex) [regexes]</code> or <code>(some regex) | cb [([--]remove|[-]rm)][(num)|_(id)]</code></summary>

---

Start by copying something.
```
$ cb copy FooFile BarDir BazDir
```

Remove everything starting with "B"
```
$ cb remove "B.*"
# Clipboard will match this against "BarDir" and "BazDir" and remove them
```

Remove everything matching a specific name
```
$ cb remove "BarDir"
# Clipboard will match this against "BarDir" only and remove it
```

Now let's copy some raw data.
```
$ cb copy "A bachatero is someone who makes bachata music."
```

Remove anything with a space beforehand and that ends with "-ero"
```
$ cb remove "(?<= ).*ero"
# The content is now: A  is someone who makes bachata music.
```

Remove anything matching "music" by piping the pattern in.
```
$ echo "music" | cb remove
# The content is now: A  is someone who makes bachata .
```

</details>

---

<details><summary> &ensp; <b>Show Contents</b> &emsp; <code>cb ([--]show|[-]sh)[(num)|_(id)]</code></summary>

---

Start by copying something.
```
$ cb copy FooFile BarDir BazDir
```

List all the items in the clipboard.
```
$ cb show
$ cb sh # These both work great!
```

Now let's copy some raw data.
```
$ cb copy "Those who are tired of bachata are tired of life"
```

Show the contents of the clipboard.
```
$ cb show
```
Note: If there is a lot of data, Clipboard may only show enough to fill the terminal screen.

</details>

---

<details><summary> &ensp; <b>Clear Contents</b> &emsp; <code>cb ([--]clear|[-]clr)[(num)|_(id)]</code></summary>

---

Start by copying something.
```
$ cb copy FooFile BarDir BazDir
```

Clear the clipboard of all data.
```
$ cb clear
$ cb clr # These both work great!
```

</details>

---

<details><summary> &ensp; <b>Set Note</b> &emsp; <code>cb ([--]note|[-]nt)[(num)|_(id)] (text)</code> or <code>(something) | cb [([--]note|[-]nt)][(num)|_(id)]</code></summary>

---

Add a personal note to a clipboard.
```
$ cb note "For my Aventura music collection"
$ cb nt "For my Aventura music collection" # This also works great!
```

Add a personal note to a clipboard by piping it in.
```
$ echo "For my Aventura music collection" | cb note
$ echo "For my Aventura music collection" | cb nt # This also works great!
```

Remove a note from a clipboard.
```
$ cb note ""
```

</details>

---

<details><summary> &ensp; <b>Show Note</b> &emsp; <code>cb ([--]note|[-]nt)[(num)|_(id)]</code></summary>

---

Start by adding a note to a clipboard.
```
$ cb note "For my Aventura music collection"
```

Show the note you added.
```
$ cb note
```

</details>

---

<details><summary> &ensp; <b>Show Help Message</b> &emsp; <code>cb (-h|[--]help)</code></summary>

---

Show the help message.
```
$ cb help
$ cb --help
$ cb -h # These three versions all work great!
```

</details>

---

<details><summary> &ensp; <b>Check Clipboard Status</b> &emsp; <code>cb [[--]status|[-]st]</code></summary>

---

Check the status of all clipboards that have content.
```
$ cb status
$ cb st
$ cb # These all work great!
```

</details>

### ![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### <img src="documentation/readme-assets/Flags.png" alt="Flags" height=25px />

 &emsp; **`--clipboard (clipboard)`, `-c (clipboard)`** &emsp; Add this to choose which clipboard you want to use.

---

 &emsp; **`--fast-copy`, `-fc`** &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

---

 &emsp; **`--no-progress`, `-np`** &emsp; Add this to disable progress messages from Clipboard.

### <img src="documentation/readme-assets/EnvironmentVariables.png" alt="Environment Variables" height=25px />

 &emsp; **`CI`** &emsp; Set this to anything to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

 &emsp; **`CLIPBOARD_LOCALE`** &emsp; Set this to the locale that only Clipboard will use for its commands and output, like `en_US.UTF-8` or `es_DO.UTF-8`.

---

 &emsp; **`CLIPBOARD_TMPDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a temporary directory.

---

 &emsp; **`CLIPBOARD_PERSISTDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a persistent directory.

---

 &emsp; **`CLIPBOARD_ALWAYS_PERSIST`** &emsp; Set this to anything to make Clipboard always use persistent clipboards.

---

 &emsp; **`CLIPBOARD_NOGUI`** &emsp; Set this to anything to disable integration with GUI clipboards.

---

 &emsp; **`CLIPBOARD_NOPROGRESS`** &emsp; Set this to anything to disable only progress messages from Clipboard.

---

 &emsp; **`CLIPBOARD_SILENT`** &emsp; Set this to anything to disable progress and confirmation messages from Clipboard.

---

 &emsp; **`CLIPBOARD_THEME`** &emsp; Set this to the color theme that Clipboard will use. Choose between `light`, `darkhighcontrast`, `lighthighcontrast`, `amber`, and `green` (the default is `dark`).

---

 &emsp; **`FORCE_COLOR`** &emsp; Set this to anything to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

 &emsp; **`NO_COLOR`** &emsp; Set this to anything to make Clipboard not show any colors.

### ![Need Help?](documentation/readme-assets/NeedHelp.png)

Go to [the Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, ask your questions in [Clipboard Discussions](https://github.com/Slackadays/Clipboard/discussions), or join [our Discord group](https://discord.gg/J6asnc3pEG)! 

[![Clipboard Wiki](https://img.shields.io/badge/Docs-Wiki-green?style=for-the-badge)](https://github.com/Slackadays/Clipboard/wiki)
[![Questions? Just ask](https://img.shields.io/badge/Questions%3F-Just%20Ask-red?style=for-the-badge)](https://github.com/Slackadays/Clipboard/discussions)
[![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)

### ![Thank You!](documentation/readme-assets/ThankYou.png)

Say thank you to all the contributors who have helped make Clipboard great[.](https://www.youtube.com/watch?v=yjdHGmRKz08)

If you're feeling generous, feel free to give us a :star:!

<p align="right"><sup><sub><code>cb copy haters && cb > /dev/null</code></sub></sup></p>
