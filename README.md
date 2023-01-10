![Clipboard Banner](documentation/readme-banners/CBBanner.png)

**Clipboard is a software powertool that saves you time and effort.** Now, you can have a clipboard to use anywhere in the command line, as if you were using a GUI.

- **Tiny.** Mere tens of kilobytes on most platforms.
- **Simple.** Requires zero configuration to use.
- **Easy.** Friendly to newbies and power users alike.
- **Compatible.** Works with up-to-date Windows, Linux, Android, macOS, FreeBSD, OpenBSD, NetBSD, DragonFlyBSD, or OpenIndiana systems, or anything that supports C++20. Really!
- **Universal.** Supports English, Spanish, Portuguese, and Turkish.
- **Integrated.** Works with many native GUI clipboards.

![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/clipboard?style=for-the-badge)
![Clipboard Demo Image](documentation/readme-banners/CBDemo.png)
![Quick Installation](documentation/readme-banners/CBQuickInstallation.png)
### **All Except Windows** 
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/install.sh | sh
```
### **Windows** 
```powershell
Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/install.ps1 | powershell
```

---

### **Install Manually**
Get the latest release instead by adding `--branch 0.2.1r2` right after `git clone...`. Change the installation prefix by adding `-D CMAKE_INSTALL_PREFIX=/CUSTOM/PREFIX` and enable Debug Mode by adding `-D TEST=1` to `cmake .`.
```bash
git clone https://github.com/slackadays/Clipboard 
cd Clipboard/build
cmake ..
cmake --build .
cmake --install .
```

---

### **Uninstall**
```bash
xargs rm < install_manifest.txt
```

Add `sudo` to the beginning for Linux, macOS, all BSDs except OpenBSD, and OpenIndiana, and add `doas` for OpenBSD.

For Windows, individually remove all the files in install_manifest.txt.

---

### **Premade Builds**

<a>
    <img src="https://repology.org/badge/vertical-allrepos/clipboard.svg" alt="Packaging status">
</a>

You can also download Clipboard directly from GitHub Actions.

---

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

---

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

---

[<img src="documentation/readme-banners/TheCBWiki.png" width="46%" alt="The Clipboard Wiki"/>](https://github.com/Slackadays/Clipboard/wiki)
[<img src="documentation/readme-banners/DiscordSupport.png" width="46%" alt="Discord Support" align="right"/>](https://discord.gg/J6asnc3pEG)
