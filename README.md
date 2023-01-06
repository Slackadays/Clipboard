![Clipboard Banner](readme_assets/CBBanner.png)

Cut, copy, and paste absolutely anything anywhere you want, all from the comfort of your terminal! The ultimate file powertool for the command line.

- **Zero-bloat.** Clipboard rings in at mere kilobytes on most platforms.
- **Zero-config.** Clipboard doesn't require any configuration to use.
- **Zero-effort.** Clipboard is friendly to newbies and power users alike.
- **Zero-dependency.** Clipboard works on any up-to-date Windows, Linux, Android, macOS, FreeBSD, OpenBSD, NetBSD, DragonFlyBSD, or OpenIndiana system, or anything that supports C++20, all with ZERO dependencies. Really!
- **Universal.** Clipboard supports English, Spanish, Portuguese, and Turkish.
- **A time-saver.** Clipboard frees you from ugly temporary directories and memorizing file locations!

![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/clipboard?style=for-the-badge)
![Clipboard Demo Image](readme_assets/CBDemo.png)

![Quick Installation](readme_assets/CBQuickInstallation.png)
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
Get the latest commit by removing `--branch 0.2.0` from `git clone...`. Change the installation prefix by adding `-D CMAKE_INSTALL_PREFIX=/CUSTOM/PREFIX`, and enable Debug Mode by adding `-D TEST=1` to `cmake .`.
```bash
git clone --branch 0.2.0 https://github.com/slackadays/Clipboard 
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

![How To Use](readme_assets/CBHowToUse.png)

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

![Simple Configuration](readme_assets/CBSimpleConfiguration.png)

---

**`CI`** &emsp; Set this environment variable to make Clipboard overwrite existing items without a user prompt when pasting. This variable is intended for Continuous Integration scripts where a live human is not present to make decisions.

---

**`FORCE_COLOR`** &emsp; Set this environment variable to make Clipboard always show color regardless of what you set `NO_COLOR` to.

---

**`TMPDIR`** &emsp; Set this environment variable to the directory that Clipboard will use to hold the items you cut or copy into a temporary directory. Other programs use `TMPDIR` as well, so be careful about changing this.

---

**`CLIPBOARD_TMPDIR`** &emsp; Set this environment variable to the directory that only Clipboard will use to hold the items you cut or copy into a temporary directory.

---

**`CLIPBOARD_PERSISTDIR`** &emsp; Set this environment variable to the directory that only Clipboard will use to hold the items you cut or copy into a persistent directory.

---

**`CLIPBOARD_ALWAYS_PERSIST`** &emsp; Set this environment variable to make Clipboard always use persistent clipboards.

---

**`NO_COLOR`** &emsp; Set this environment variable to make Clipboard not show any colors.

---

**`--fast-copy`, `-fc`** &emsp; Add this flag to use links when copying, cutting, or pasting. If you modify the items that you used with this flag, then the items you paste will have the same changes.

---

![Painless Documentation](readme_assets/CBPainlessDocumentation.png)

[Click here](https://github.com/Slackadays/Clipboard/wiki) to go the Clipboard Wiki.

![Fast Support](readme_assets/CBFastSupport.png)

[Click here](https://discord.gg/J6asnc3pEG) to go to our Discord group.
