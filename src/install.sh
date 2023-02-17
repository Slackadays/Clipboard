#!/bin/sh
# shellcheck disable=SC2046
set -eu

unsupported() {
    printf "\033[31mSorry, but this installer script doesn't support %s.\n\033[0m" "$1"
    printf '\033[32m👉 However, you can still install Clipboard using the other methods in the readme!\n\033[0m'
}

verify() {
    if command -v clipboard >/dev/null 2>&1
    then
        if ! clipboard >/dev/null 2>&1
        then
            unsupported "this system"
            exit 1
        else
            printf "\033[32mClipboard installed successfully!\n\033[0m"
            exit 0
        fi
    else
        printf "\033[31mCouldn't install Clipboard\n\033[0m"
        exit 1
    fi
}

compile_nixos() {
    printf "%s" "\e[1;32mInstalling Clipboard for NixOS...\e[0m"
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
    cmake --build .
    mkdir -p ~/.local/bin
    sudo cp clipboard ~/.local/bin
    sudo ln -s ~/.local/bin/clipboard ~/.local/bin/cb
    printf "%s" "\e[1;33mMake sure to add Clipboard to your PATH!\e[0m"
    printf "%s" "\e[1;32mClipboard installed successfully!\e[0m"
    rm -rfv "$tmp_dir"
    exit 0
}

compile() {
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-amd64.zip
    curl -SL $link -o clipboard.zip

    if [ ! "$(7z)" ]
    then
        ZIP_EXTRACTER="unzip"
    else
        ZIP_EXTRACTER="7z"
    fi

    $ZIP_EXTRACTER clipboard.zip
    cmake ..
    cmake --build .
    cmake --install .

    if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
    then
        doas cmake --install .
    else
        sudo cmake --install .
    fi

     if [ -f "lib/libclipboardx11.so" ]
     then
            sudo mv lib/libclipboardx11.so /usr/lib/libclipboardx11.so
     fi
     if [ -f "lib/libclipboardwayland.so" ]
     then
            sudo mv lib/libclipboardwayland.so /usr/lib/libclipboardwayland.so
     fi
     rm -rfv "$tmp_dir"
     exit 0
}


tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir"

if [ "$(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt)" = "NixOS" ]; then
    compile_nixos
fi

if [ "$(uname)" = "Linux" ]
then
    if [ "$(uname -m)" = "x86_64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-amd64.zip
    elif [ "$(uname -m)" = "aarch64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-arm64.zip
    elif [ "$(uname -m)" = "riscv64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-riscv64.zip
    elif [ "$(uname -m)" = "i386" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-i386.zip
    elif [ "$(uname -m)" = "ppc64le" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-ppc64le.zip
    elif [ "$(uname -m)" = "s390x" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-s390x.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SL $download_link -o clipboard-linux.zip
        unzip clipboard-linux.zip
        rm clipboard-linux.zip
        sudo mv bin/clipboard /usr/local/bin/clipboard
        chmod +x /usr/local/bin/clipboard
        sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
        if [ -f "lib/libclipboardx11.so" ]
        then
            sudo mv lib/libclipboardx11.so /usr/lib/libclipboardx11.so
        fi
        if [ -f "lib/libclipboardwayland.so" ]
        then
            sudo mv lib/libclipboardwayland.so /usr/lib/libclipboardwayland.so
        fi
    fi
elif [ "$(uname)" = "Darwin" ]
then
    curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-macos-arm64-amd64.zip -o clipboard-macos.zip
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/clipboard /usr/local/bin/clipboard
    chmod +x /usr/local/bin/clipboard
    sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
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
compile
fi

cd ..

rm -rf "$tmp_dir"

verify
