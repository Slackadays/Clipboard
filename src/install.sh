#!/bin/sh
set -eux

check_installation() {
    #check if we can run the clipboard command
    if [ ! -x "$(command -v clipboard)" ]
    then
        echo "\e[1;31mCouldn't install Clipboard\e[0m"
        exit 1
    else
        echo "\e[1;32mClipboard installed successfully!\e[0m"
        exit 0
    fi
}

compile_section() {
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
    if [ $(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt) = "NixOS" ]
    then
        echo -e "\e[1;32mInstalling Clipboard for NixOS..\e[0m"
    fi

    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    cmake ..
    cmake --build .

    if [ $(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt) = "NixOS" ]
    then
        mkdir -p ~/.local/bin
        sudo cp clipboard ~/.local/bin
        sudo ln -s ~/.local/bin/clipboard ~/.local/bin/cb
        export PATH="$HOME/.local/bin:$PATH"
        echo -e "\e[1;33mMake sure to add Clipboard to your PATH!\e[0m"
    else
        cmake --install .
    fi

    if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
    then
        doas cmake --install .
    else
        sudo cmake --install .
    fi

    check_installation
}

if [ $(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt) = "NixOS" ]
then
    compile_section
fi

if [ "$(uname)" = "Linux" ]
then
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
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
        check_installation
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
    check_installation
fi

compile_section

cd ..
