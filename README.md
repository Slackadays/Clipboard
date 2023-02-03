![Clipboard Banner](documentation/readme-assets/CBBanner.png)

<p align="center">
    <a href="https://github.com/Slackadays/Clipboard/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/main.yml?branch=main&label=BUILDS&style=for-the-badge">
        <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test.yml?branch=main&label=TESTS&style=for-the-badge">
    </a>
    <a href="https://github.com/Slackadays/Clipboard/stargazers">
        <img src="https://img.shields.io/github/stars/slackadays/clipboard?style=for-the-badge&color=brightgreen">
    </a>
</p>

:sparkles::rocket: **Clipboard is the power tool that saves you time and effort.** Before, you've always had to think about where exactly you want to move things, which multiplied your mental workload and made many scenarios totally impractical. 
Now, have a _powerful_ clipboard to use anywhere in the terminal, just like a GUI!

- **`Easy.`** Friendly to newbies and power users.
- **`Breezy.`** Zero configuration needed to use.
- **`Beautiful.`** Breaks the rules to look pretty.
- **`Unified.`** Functions exactly the same everywhere.
- **`Compatible.`** Works on anything that supports C++20.
- **`Universal.`** Supports English, Spanish, Portuguese, and Turkish.
- **`Tiny.`** Mere tens of kilobytes in size.
- **`Integrated.`** Connects with [many](https://github.com/Slackadays/Clipboard/wiki/GUI-Clipboard-Compat) native GUI clipboards.

![Clipboard Demo](documentation/readme-assets/ClipboardDemo.gif)
![Quick Installation](documentation/readme-assets/CBQuickInstallation.png)
### **All Except Windows** 
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/src/install.sh | bash
```
### **Windows (run as Administrator)** 
```powershell
(Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/src/install.ps1).Content | powershell
```

---

### **Install Manually**
Get the latest release instead by adding `--branch 0.3.0` right after `git clone...`. Change the installation prefix by adding `-DINSTALL_PREFIX=/CUSTOM/PREFIX` to `cmake ..`.
```bash
git clone https://github.com/slackadays/Clipboard 
cd Clipboard/build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build .
cmake --install .
```

### **Uninstall**

Remove all the files in `install_manifest.txt`. If you're not using Windows, you can also do `xargs rm < install_manifest.txt`.

---

### **Premade Builds**

<a href="https://repology.org/project/clipboard/versions"><img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status"></a>

You can also download Clipboard [directly from GitHub Actions.](https://nightly.link/Slackadays/Clipboard/workflows/main/main)


![How To Use](documentation/readme-assets/CBHowToUse.png)

You can substitute `cb` for `clipboard` in all commands, and add a number to the end of the action to choose which clipboard you want to use (the default is 0), or `_` to use a persistent clipboard. 

---

**Copy** &emsp; `clipboard ([--]copy|[-]cp)[(num)|_(id)] (file) [files]`

---


**Cut** &emsp; `clipboard ([--]cut|[-]ct)[(num)|_(id)] (file) [files]`

---

**Paste** &emsp; `clipboard ([--]paste|[-]p)[(num)|_(id)]`

---

**Pipe In** &emsp; `(something) | clipboard [([--]copy|[-]cp)][(num)|_(id)]`

---

**Pipe Out** &emsp; `clipboard [([--]paste|[-]p][(num)|_(id)] | (something)` or `clipboard [([--]paste|[-]p)][(num)|_(id)] > (some file)`

---

**Show Contents** &emsp; `clipboard ([--]show|[-]sh)[(num)|_(id)]`

---

**Clear Contents** &emsp; `clipboard ([--]clear|[-]clr)[(num)|_(id)]`

---

**Show Help Message** &emsp; `clipboard (-h|[--]help)`

---

**Examples**

```
cb copy foo.txt launchcodes.doc
clipboard cut1 MyDirectory
cb cp800 bar.conf AnotherDirectory baz.txt
```

![Simple Configuration](documentation/readme-assets/CBSimpleConfiguration.png)

### **Environment Variables**

**`CI`** &emsp; Set this to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

**`CLIPBOARD_TMPDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a temporary directory.

---

**`CLIPBOARD_PERSISTDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a persistent directory.

---

**`CLIPBOARD_ALWAYS_PERSIST`** &emsp; Set this to make Clipboard always use persistent clipboards.

---

**`CLIPBOARD_NOGUI`** &emsp; Set this to disable integration with GUI clipboards.

---

**`FORCE_COLOR`** &emsp; Set this to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

**`NO_COLOR`** &emsp; Set this to make Clipboard not show any colors.

</details>

---

### **Flags**

**`--clipboard (clipboard)`, `-c (clipboard)`** &emsp; Add this to choose which clipboard you want to use.

---

**`--fast-copy`, `-fc`** &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

![Need Help?](documentation/readme-assets/NeedHelp.png)

Go to the [Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, ask your questions in [Clipboard Discussions](https://github.com/Slackadays/Clipboard/discussions), or join our Discord group!  

[![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)

![Thank You!](documentation/readme-assets/ThankYou.png)

Thank you to all the contributors who have helped make Clipboard great.
