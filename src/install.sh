#!/bin/sh
read -p "Do you want to compile clipboard and install it to ~/.local/bin? (Y/N)" COMPILE_CLIPBOARD

set -eux

compile_section() {
    if [ $(nix-info --host-os > info.txt && cat info.txt | grep -ow "NixOS" info.txt) = "NixOS" ]
    then
    echo -e "\e[1;32mInstalling Clipboard for NixOS..\e[0m"
    else
    echo -e "\e[1;32mInstalling Clipboard for Linux..\e[0m"
    fi
    git clone --depth 1 https://github.com/slackadays/Clipboard
    pushd Clipboard/build
    cmake ..
    cmake --build .
    mkdir -p ~/.local/bin
    sudo cp clipboard ~/.local/bin
    sudo ln -s ~/.local/bin/clipboard ~/.local/bin/cb
    export PATH="$HOME/.local/bin:$PATH"
    echo -e "\e[1;33mMake sure to add Clipboard to your PATH! (If you installed clipboard on NixOS)\e[0m"
}

if [ $(nix-info --host-os > info.txt && cat info.txt | grep -ow "NixOS" info.txt) = "NixOS" ]
then
    compile_section
fi

if [ "$COMPILE_CLIPBOARD" = "Y" ]
then
    compile_section
fi

if [ "$(uname)" = "Linux" ]
then
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
    if [ "$(uname -m)" = "x86_64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc11-amd64.zip
    elif [ "$(uname -m)" = "aarch64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc11-arm64.zip
    elif [ "$(uname -m)" = "riscv64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc11-riscv64.zip
    elif [ "$(uname -m)" = "i386" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc11-i386.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SsLl $download_link -o clipboard-linux.zip
        unzip clipboard-linux.zip
        rm clipboard-linux.zip
        sudo mv bin/clipboard /usr/bin/clipboard
        chmod +x /usr/bin/clipboard
        sudo ln -sf /usr/bin/clipboard /usr/bin/cb
        if [ -f "lib/libclipboardx11.so" ]
        then
            sudo mv lib/libclipboardx11.so /usr/lib/libclipboardx11.so
        fi
        if [ -f "lib/libclipboardwayland.so" ]
        then
            sudo mv lib/libclipboardwayland.so /usr/lib/libclipboardwayland.so
        fi
        rm -rf $tmp_dir
        echo "Installed Clipboard"
        exit 0
    fi
fi

if [ "$(uname)" = "Darwin" ]
then
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
    curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-macos-arm64-amd64.zip -o clipboard-macos.zip
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/clipboard /usr/local/bin/clipboard
    chmod +x /usr/local/bin/clipboard
    sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
    rm -rf $tmp_dir
    echo "Installed Clipboard"
    exit 0
fi

compile_section


if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
then
    doas cmake --install .
else
    sudo cmake --install .
fi

popd
