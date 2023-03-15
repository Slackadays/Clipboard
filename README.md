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

The Clipboard project is an easy-to-use terminal clipboard manager with many useful features.

Yuck, ugh, and puke! We can do better.

Clipboard is _<ins>your</ins>_ time and effort saver to use **anytime and anywhere.** Cut, copy, paste, add, remove, and make note of **anything** in your terminal just how you would on a desktop! Have a helper with a perfect memory by your side. You'll be sitting on a sunny beach sipping a papaya smoothie with the love of your life in no time. Ok, so maybe that last part won't actually happen, but you get the idea:

<p align="center"> 
    <img src="documentation/readme-assets/ClipboardMakesYourLifeEasy.png" alt="Clipboard makes your life easy." />
</p>

![Clipboard Demo Video](documentation/readme-assets/ClipboardDemo.gif)

<details><summary><b>Click here to see our exquisite features.</b></summary>

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

### <img src="documentation/readme-assets/InstallManually.png" alt="Install Manually" height=25px /> (you'll need CMake and C++20 support)
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

**Copy** &emsp; `cb ([--]copy|[-]cp)[(num)|_(id)] (file) [files]` or `(something) | cb [([--]copy|[-]cp)][(num)|_(id)]`

---


**Cut** &emsp; `cb ([--]cut|[-]ct)[(num)|_(id)] (file) [files]`

---

**Paste** &emsp; `cb ([--]paste|[-]p)[(num)|_(id)]` or `cb [([--]paste|[-]p][(num)|_(id)] | (something)` or `cb [([--]paste|[-]p)][(num)|_(id)] > (some file)`

---

**Add Contents** &emsp; `cb ([--]add|[-]ad)[(num)|_(id)] (file|text) [files]` or `(something) | cb [([--]add|[-]ad)][(num)|_(id)]`

---

**Remove Contents** &emsp; `cb ([--]remove|[-]rm)[(num)|_(id)] (regex) [regexes]` or `(some regex) | cb [([--]remove|[-]rm)][(num)|_(id)]`

---

**Show Contents** &emsp; `cb ([--]show|[-]sh)[(num)|_(id)]`

---

**Clear Contents** &emsp; `cb ([--]clear|[-]clr)[(num)|_(id)]`

---

**Set Note** &emsp; `cb ([--]note|[-]nt)[(num)|_(id)] (text)` or `(something) | cb [([--]note|[-]nt)][(num)|_(id)]`

---

**Show Note** &emsp; `cb ([--]note|[-]nt)[(num)|_(id)]`

---

**Show Help Message** &emsp; `cb (-h|[--]help)`

---

**Check Clipboard Status** &emsp; `cb [[--]status|[-]st]`

### ![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### <img src="documentation/readme-assets/Flags.png" alt="Flags" height=25px />

**`--clipboard (clipboard)`, `-c (clipboard)`** &emsp; Add this to choose which clipboard you want to use.

---

**`--fast-copy`, `-fc`** &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

### <img src="documentation/readme-assets/EnvironmentVariables.png" alt="Environment Variables" height=25px />

**`CI`** &emsp; Set this to anything to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

**`CLIPBOARD_LOCALE`** &emsp; Set this to the locale that only Clipboard will use for its commands and output, like `en_US.UTF-8` or `es_DO.UTF-8`.

---

**`CLIPBOARD_TMPDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a temporary directory.

---

**`CLIPBOARD_PERSISTDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a persistent directory.

---

**`CLIPBOARD_ALWAYS_PERSIST`** &emsp; Set this to anything to make Clipboard always use persistent clipboards.

---

**`CLIPBOARD_NOGUI`** &emsp; Set this to anything to disable integration with GUI clipboards.

---

**`CLIPBOARD_SILENT`** &emsp; Set this to anything to disable progress and confirmation messages from Clipboard.

---

**`CLIPBOARD_THEME`** &emsp; Set this to the color theme that Clipboard will use. Choose between `light`, `darkhighcontrast`, `lighthighcontrast`, `amber`, and `green` (the default is `dark`).

---

**`FORCE_COLOR`** &emsp; Set this to anything to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

**`NO_COLOR`** &emsp; Set this to anything to make Clipboard not show any colors.

### ![Need Help?](documentation/readme-assets/NeedHelp.png)

Go to [the Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, ask your questions in [Clipboard Discussions](https://github.com/Slackadays/Clipboard/discussions), or join [our Discord group](https://discord.gg/J6asnc3pEG)! 

[![Clipboard Wiki](https://img.shields.io/badge/Docs-Wiki-green?style=for-the-badge)](https://github.com/Slackadays/Clipboard/wiki)
[![Questions? Just ask](https://img.shields.io/badge/Questions%3F-Just%20Ask-red?style=for-the-badge)](https://github.com/Slackadays/Clipboard/discussions)
[![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)

### ![Thank You!](documentation/readme-assets/ThankYou.png)

Say thank you to all the contributors who have helped make Clipboard great[.](https://www.youtube.com/watch?v=yjdHGmRKz08)

If you're feeling generous, feel free to give us a :star:!

<p align="right"><sup><sub><code>cb copy haters && cb > /dev/null</code></sub></sup></p>
