#!/bin/sh
set -eu

unsupported() {
    printf "\033[31mSorry, but this installer script doesn't support %s.\n\033[0m" "$1"
    printf '\033[32m💡 However, you can still install CB using the other methods in the readme!\n\033[0m'
}

verify() {
    if command -v cb 2>&1
    then
        if ! cb 2>&1
        then
            unsupported "this system"
            exit 1
        else
            printf "\033[32mClipboard installed successfully!\n\033[0m"
            exit 0
        fi
    else
        printf "\033[31mCouldn't install CB\n\033[0m"
        exit 1
    fi
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv > /dev/null 2>&1; then
      # exit code of sudo-command is 0
      return 0
    elif echo "$prompt" | grep -q '^sudo:'; then
      return 0
    else
      return 1
    fi
}

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir"

if can_use_sudo
then
    requires_sudo=true
    install_path="/usr/local"
    sudo mkdir -p "$install_path/bin"
    sudo mkdir -p "$install_path/lib"
else
    requires_sudo=false
    install_path="$HOME/.local"
    mkdir -p "$install_path/bin"
    mkdir -p "$install_path/lib"
fi

if [ "$(uname)" = "Linux" ]
then
    if [ "$(uname -m)" = "x86_64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-amd64.zip
    elif [ "$(uname -m)" = "aarch64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-arm64.zip
    elif [ "$(uname -m)" = "riscv64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-riscv64.zip
    elif [ "$(uname -m)" = "i386" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-i386.zip
    elif [ "$(uname -m)" = "ppc64le" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-ppc64le.zip
    elif [ "$(uname -m)" = "s390x" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-s390x.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SL $download_link -o clipboard-linux.zip
        unzip clipboard-linux.zip
        rm clipboard-linux.zip
        if [ "$requires_sudo" = true ]
        then
            sudo mv bin/cb "$install_path/bin/cb"
        else
            mv bin/cb "$install_path/bin/cb"
        fi
        chmod +x "$install_path/bin/cb"
        if [ -f "lib/libcbx11.so" ]
        then
            if [ "$requires_sudo" = true ]
            then
                sudo mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
            else
                mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
            fi
        fi
        if [ -f "lib/libcbwayland.so" ]
        then
            if [ "$requires_sudo" = true ]
            then
                sudo mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
            else
                mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
            fi
        fi
    fi
elif [ "$(uname)" = "Darwin" ]
then
    if [ "$(uname -m)" = "x86_64" ]
    then
        curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-amd64.zip -o clipboard-macos.zip
    elif [ "$(uname -m)" = "arm64" ]
    then
        curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-arm64.zip -o clipboard-macos.zip
    fi
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/cb "$install_path/bin/cb"
    chmod +x "$install_path/bin/cb"
elif [ "$(uname)" = "FreeBSD" ]
then
    unsupported "FreeBSD"
    exit 0
elif [ "$(uname)" = "OpenBSD" ]
then
    unsupported "OpenBSD"
    exit 0
elif [ "$(uname)" = "NetBSD" ]
then
    unsupported "NetBSD"
    exit 0
else
    unsupported "$(uname)"
    exit 0
fi

cd ..

rm -rf "$tmp_dir"

verify
