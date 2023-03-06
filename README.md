![Clipboard Banner](documentation/readme-assets/CBBanner.png)

<p align="center">
    <a href="https://github.com/Slackadays/Clipboard/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/build-clipboard.yml?branch=main&label=BUILDS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test-clipboard.yml?branch=main&label=TESTS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/slackadays/Clipboard/lint-clipboard.yml?branch=main&label=CHECKS&style=for-the-badge">
    </a>
</p>

<h1 align="center" style="border-bottom: none;"> 
:tropical_drink::turtle: Make your life easier with Clipboard. 🏖️🌴
</h1>

Our mission is to make your life easier and more productive by saving you time and effort.

Before, you've always had to think about where exactly you want to move things, which multiplied your mental workload and made many scenarios totally impractical. 

Now, have a real _power tool_ to use **anywhere in the terminal** to cut, copy, and paste things with, just like a GUI!

<details><summary><h3>Why use Clipboard</h3></summary>

- **`Easy.`** Friendly to newbies and power users. &emsp; **`Breezy.`** Zero configuration needed to use.
- **`Beautiful.`** Breaks the rules to look pretty. &emsp; **`Versatile.`** Filled with useful features.
- **`Unified.`** Functions exactly the same everywhere. &emsp; **`Compatible.`** Works on anything that supports C++20.
- **`Scriptable.`** Embed it right into your own programs. &emsp; **`Universal.`** Supports English, español, português, and Türkçe.
- **`Tiny.`** Mere tens of kilobytes in size. &emsp; **`FOSS.`** 100% free and open-source.
- **`Integrated.`** Connects with [many native GUI clipboards.](https://github.com/Slackadays/Clipboard/wiki/GUI-Clipboard-Compat)

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
Get the latest release instead by adding `--branch 0.4.0` right after `git clone...`.
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

**Show Contents** &emsp; `clipboard ([--]show|[-]sh)[(num)|_(id)]`

---

**Clear Contents** &emsp; `clipboard ([--]clear|[-]clr)[(num)|_(id)]`

---

**Show Help Message** &emsp; `clipboard (-h|[--]help)`

---

**Check Clipboard Status** &emsp; `clipboard`

![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### Environment Variables

**`CI`** &emsp; Set this to anything to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

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

Go to [the Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, ask your questions [in Clipboard Discussions](https://github.com/Slackadays/Clipboard/discussions), or join [our Discord group](https://discord.gg/J6asnc3pEG)! [![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)

![Thank You!](documentation/readme-assets/ThankYou.png)

Thank you to all the contributors who have helped make Clipboard great[.](https://www.youtube.com/watch?v=jnD8Av4Dl4o)
