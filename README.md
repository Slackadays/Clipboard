![Clipboard Banner](documentation/readme-assets/CBBanner.png)

<p align="center">
    <a href="https://github.com/Slackadays/Clipboard/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/build-clipboard.yml?branch=main&label=BUILDS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/slackadays/Clipboard/lint-clipboard.yml?branch=main&label=CHECKS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test-clipboard.yml?branch=main&label=TESTS&style=for-the-badge">
    </a>
    <a href="https://app.codecov.io/gh/Slackadays/Clipboard">
        <img src="https://img.shields.io/codecov/c/github/slackadays/Clipboard/main?style=for-the-badge&label=TEST COVERAGE&token=RO7KDOZ6Q2">
    </a>
</p>

The Clipboard project is an easy-to-use terminal clipboard manager with many useful features.

Yuck, ugh, and puke! We can do better.

Clipboard is _<ins>your</ins>_ time and effort saver to use **anytime, anywhere in the terminal.** Cut, copy, paste, add, remove, and make note of anything in your path, just how you would on a desktop! It's like having a helper with a perfect memory by your side. You'll be sitting on a sunny beach sipping a papaya smoothie with the love of your life in no time. Ok, so maybe that last part won't actually happen, but you get the idea:

<h1 align="center"> 
🍹😎 Clipboard makes your life <ins>easy</ins>. 🏖️🌴
</h1>

<details><summary>Click here to see our exquisite features.</summary>

- **Cut, copy, or paste files, directories, text, data, or any other kind of information.**
- **Add, remove, or make note of whatever you hold with Clipboard.**
- **Store everything in an _infinite_ number of different containers at your disposal.**
- **Choose if your containers are temporary or totally persistent.**
- **Connect right with your regular desktop [which Clipboard probably supports.](https://github.com/Slackadays/Clipboard/wiki/GUI-Clipboard-Compat)**
- **Do all of this in style with Clipboard's beautiful design.**
- **Speak español, português, or Türkçe? You're in luck because Clipboard's in these languages too.**
- **Love freedom? We've got your back because Clipboard's 100% free and open source under the GPLv3.**

</details>

![Clipboard Demo](documentation/readme-assets/ClipboardDemo.gif)

![Quick Installation](documentation/readme-assets/CBQuickInstallation.png)
### [**All Except Windows**](https://github.com/Slackadays/Clipboard/blob/main/src/install.sh) 
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/src/install.sh | sh
```
### [**Windows (run as Administrator)** ](https://github.com/Slackadays/Clipboard/blob/main/src/install.ps1)
```powershell
(Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/src/install.ps1).Content | powershell
```

---

### Premade Builds

<a href="https://repology.org/project/clipboard/versions"><img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status"></a>

You can also download Clipboard [directly from GitHub Actions.](https://nightly.link/Slackadays/Clipboard/workflows/main/main)

---

### Install Manually (you'll need CMake and C++20 support)
Get the latest release instead by adding `--branch 0.5.0` right after `git clone...`.
Change the system installation prefix by adding `-DCMAKE_INSTALL_PREFIX=/custom/prefix` to `cmake ..`.
```bash
git clone https://github.com/Slackadays/Clipboard 
cd Clipboard/build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build .
cmake --install .
```

### Uninstall

Remove all the files in `install_manifest.txt`. If you're not using Windows, you can also do `xargs rm < install_manifest.txt`.

![How To Use](documentation/readme-assets/CBHowToUse.png)

You can substitute `cb` for `clipboard` in all commands. Add a number to the end of the action to choose which clipboard you want to use (the default is 0), or `_` to use a persistent clipboard. 

### Examples

```
clipboard copy contacts/JohnSmith NuclearLaunchCodes.pdf
clipboard cut69 MyDirectory
cb cp420 bar.conf NotAVirus.pdf.mp3.exe baz.txt
cb show420
cb add69 SomeFile
clipboard clr
```

---

**Copy** &emsp; `clipboard ([--]copy|[-]cp)[(num)|_(id)] (file) [files]` or `(something) | clipboard [([--]copy|[-]cp)][(num)|_(id)]`

---


**Cut** &emsp; `clipboard ([--]cut|[-]ct)[(num)|_(id)] (file) [files]`

---

**Paste** &emsp; `clipboard ([--]paste|[-]p)[(num)|_(id)]` or `clipboard [([--]paste|[-]p][(num)|_(id)] | (something)` or `clipboard [([--]paste|[-]p)][(num)|_(id)] > (some file)`

---

**Add Contents** &emsp; `clipboard ([--]add|[-]ad)[(num)|_(id)] (file|text) [files]` or `(something) | clipboard [([--]add|[-]ad)][(num)|_(id)]`

---

**Remove Contents** &emsp; `clipboard ([--]remove|[-]rm)[(num)|_(id)] (regex) [regexes]` or `(some regex) | clipboard [([--]remove|[-]rm)][(num)|_(id)]`

---

**Show Contents** &emsp; `clipboard ([--]show|[-]sh)[(num)|_(id)]`

---

**Clear Contents** &emsp; `clipboard ([--]clear|[-]clr)[(num)|_(id)]`

---

**Set Note** &emsp; `clipboard ([--]note|[-]nt)[(num)|_(id)] (text)` or `(something) | clipboard [([--]note|[-]nt)][(num)|_(id)]`

---

**Show Note** &emsp; `clipboard ([--]note|[-]nt)[(num)|_(id)]`

---

**Show Help Message** &emsp; `clipboard (-h|[--]help)`

---

**Check Clipboard Status** &emsp; `clipboard`

![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### Environment Variables

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

</details>

---

### Flags

**`--clipboard (clipboard)`, `-c (clipboard)`** &emsp; Add this to choose which clipboard you want to use.

---

**`--fast-copy`, `-fc`** &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

![Need Help?](documentation/readme-assets/NeedHelp.png)

Go to [the Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, ask your questions in [Clipboard Discussions](https://github.com/Slackadays/Clipboard/discussions), or join [our Discord group](https://discord.gg/J6asnc3pEG)! [![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)

![Thank You!](documentation/readme-assets/ThankYou.png)

Thank you to all the contributors who have helped make Clipboard great[.](https://www.youtube.com/watch?v=yjdHGmRKz08)
