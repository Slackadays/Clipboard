![Clipboard Banner](documentation/readme-banners/CBBanner.png)

<p align="center">
    <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/main.yml?branch=main&label=BUILDS&style=for-the-badge">
    <img src="https://img.shields.io/github/actions/workflow/status/Slackadays/Clipboard/test.yml?branch=main&label=TESTS&style=for-the-badge">
    <img src="https://img.shields.io/github/stars/slackadays/clipboard?style=for-the-badge&color=brightgreen">
</p>

**Clipboard is a power tool that saves you time and effort.** Previously, you've always had to think about where exactly you want to move text and files. This increases your mental workload and makes some scenarios simply impractical. 
Now, you can have a _unified_ clipboard to use anywhere in the command line, just as if you were using a GUI.

- **Quick.** Zero configuration needed to use.
- **Easy.** Friendly to newbies and power users alike.
- **Compatible.** Works on any system that supports C++20. Really!
- **Unified.** Functions exactly the same everywhere.
- **Universal.** Supports English, Spanish, Portuguese, and Turkish.
- **Integrated.** Connects with many native GUI clipboards.
- **Tiny.** Mere tens of kilobytes in size.

![Clipboard Demo Image](documentation/readme-banners/CBDemo.png)
![Quick Installation](documentation/readme-banners/CBQuickInstallation.png)
### **All Except Windows** 
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/src/install.sh | bash
```
### **Windows** 
```powershell
Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/src/install.ps1 | powershell
```

---

### **Install Manually**
Get the latest release instead by adding `--branch 0.2.1r2` right after `git clone...`. Change the installation prefix by adding `-DINSTALL_PREFIX=/CUSTOM/PREFIX` to `cmake ..`.
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

<a>
    <img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status">
</a>

You can also download Clipboard [directly from GitHub Actions.](https://nightly.link/Slackadays/Clipboard/workflows/main/main)


![How To Use](documentation/readme-banners/CBHowToUse.png)

In all commands, you can substitute `cb` for `clipboard`. 
Add a number to the end of the action to choose which clipboard you want to use (the default is 0) or `_` to use a persistent clipboard. 

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

**Examples**

```
cb copy foo.txt launchcodes.doc
clipboard cut1 MyDirectory
cb cp800 bar.conf AnotherDirectory baz.txt
```

![Simple Configuration](documentation/readme-banners/CBSimpleConfiguration.png)

### **Environment Variables**

**`CI`** &emsp; Set this to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

**`FORCE_COLOR`** &emsp; Set this to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

**`TMPDIR`** &emsp; Set this to the directory that Clipboard will use to hold the items you cut or copy into a temporary directory. Other programs use `TMPDIR` as well, so be careful about changing this.

---

**`CLIPBOARD_TMPDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a temporary directory.

---

**`CLIPBOARD_PERSISTDIR`** &emsp; Set this to the directory that only Clipboard will use to hold the items you cut or copy into a persistent directory.

---

**`CLIPBOARD_ALWAYS_PERSIST`** &emsp; Set this to make Clipboard always use persistent clipboards.

---

**`CLIPBOARD_NOGUI`** &emsp; Set this to disable integration with GUI clipboards.

---

**`NO_COLOR`** &emsp; Set this to make Clipboard not show any colors.

</details>

---

### **Flags**

**`--fast-copy`, `-fc`** &emsp; Add this to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

## **Need More Help?**

Go to the [Clipboard Wiki](https://github.com/Slackadays/Clipboard/wiki) for more information, or join our Discord group!  [![Discord Support](https://img.shields.io/badge/CHAT-DISCORD-blue?style=for-the-badge)](https://discord.gg/J6asnc3pEG)
