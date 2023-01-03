![Clipboard Banner](readme_assets/CBBanner.webp)

Cut, copy, and paste absolutely anything anywhere you want, all from the comfort of your terminal! This is the clipboard powertool for the command line.

- **Zero-bloat.** Clipboard rings in at mere kilobytes on most platforms.
- **Zero-config.** Clipboard doesn't require any configuration to use.
- **Zero-effort.** Clipboard is friendly to newbies and power users alike.
- **Zero-dependency.** Clipboard works on any up-to-date Windows, Linux, Android, macOS, FreeBSD, OpenBSD, NetBSD, DragonFlyBSD, or OpenIndiana system, or anything that supports C++20, all with ZERO dependencies. Really!
- **Universal.** Clipboard supports English, Spanish, Portuguese, and Turkish.
- **A time-saver.** Clipboard frees you from ugly temporary directories and memorizing file locations!

![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/clipboard?style=for-the-badge)
![Clipboard Demo Image](readme_assets/CBDemo.png)

![Quick Installation](readme_assets/CBQuickInstallation.webp)
### Linux, macOS, all BSDs, and OI:
```bash
curl -sSL https://github.com/Slackadays/Clipboard/raw/main/install.sh | sh
```
### Windows:
```powershell
Invoke-WebRequest -UseBasicParsing https://github.com/Slackadays/Clipboard/raw/main/install.ps1 | powershell
```

---

### Clone, Configure, Compile, and Install Manually
Replace `git clone` with `git clone --branch 0.2.0` to get the latest release (0.2.0). Otherwise, you will get the latest commit. Change the installation prefix by adding `-DCMAKE_INSTALL_PREFIX=/YOUR/CUSTOM/PREFIX/HERE` to `cmake .`.
```bash
git clone https://github.com/slackadays/Clipboard 
cd Clipboard
cmake .
cmake --build .
cmake --install .
```

---

### Uninstall
```
xargs rm < install_manifest.txt
```
Add `sudo` to the beginning for Linux, macOS, all BSDs except OpenBSD, and OpenIndiana, and add `doas` for OpenBSD.

For Windows, you may need to individually remove all the files in install_manifest.txt.

---

### Premade Builds

You can download Clipboard directly from GitHub Actions.

Arch-Linux users can install the [clipboard](https://aur.archlinux.org/packages/clipboard), [clipboard-bin](https://aur.archlinux.org/packages/clipboard-bin), or [clipboard-git](https://aur.archlinux.org/packages/clipboard-git) AUR package.

---

![How To Use](readme_assets/CBHowToUse.webp)

In all commands, you can substitute `cb` for `clipboard`. 
Add a number to the end of the action to choose which clipboard you want to use (the default is 0). 

---

**Copy** &emsp; `clipboard ([--]copy|[-]cp) (file) [files]`

---

**Cut** &emsp; `clipboard ([--]cut|[-]ct) (file) [files]`

---

**Paste** &emsp; `clipboard ([--]paste|[-]p)`

---

**Pipe In** &emsp; `(something) | clipboard [([--]copy|[-]cp)]`

---

**Pipe Out** &emsp; `clipboard [([--]paste|[-]p] | (something)` or `clipboard [([--]paste|[-]p)] > (some file)`

---

**Show Contents** &emsp; `clipboard ([--]show|[-]sh)`

---

**Clear Contents** &emsp; `clipboard ([--]clear|[-]clr)`

---

**Examples**

```
cb copy foo.txt launchcodes.doc
clipboard cut1 MyDirectory
cb cp800 bar.conf AnotherDirectory baz.txt
```

---

![Simple Configuration](readme_assets/CBSimpleConfiguration.webp)

---

**`CI`** &emsp; Set this environment variable to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

**`FORCE_COLOR`** &emsp; Set this environment variable to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

**`TMPDIR`** &emsp; Set this environment variable to the directory that Clipboard will use to hold the items you cut or copy.

---

**`NO_COLOR`** &emsp; Set this environment variable to make Clipboard not show any colors.

---

**`--fast-copy`, `-fc`** &emsp; Add this flag to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

---

![Painless Documentation](readme_assets/CBPainlessDocumentation.webp)

[Click here](https://github.com/Slackadays/Clipboard/wiki) to go the Clipboard Wiki.

![Fast Support](readme_assets/CBFastSupport.webp)

[Click here](https://discord.gg/J6asnc3pEG) to go to our Discord group.
