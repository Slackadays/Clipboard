#!/bin/sh
set -eux

check_installation() {
    #check if we can run the clipboard command
    if [ ! -x "$(command -v clipboard)" ]
    then
        printf "\e[1;31mCouldn't install Clipboard\e[0m"
        exit 1
    else
        printf "\e[1;32mClipboard installed successfully!\e[0m"
        exit 0
    fi
}

compile_section() {
    if [ "$(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt)" = "NixOS" ]
    then
        printf "\e[1;32mCompiling Clipboard for NixOS\e[0m"
    fi

    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    cmake ..
    cmake --build .

    if [ "$(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt)" = "NixOS" ]
    then
        mkdir -p ~/.local/bin
        sudo cp clipboard ~/.local/bin
        sudo ln -s ~/.local/bin/clipboard ~/.local/bin/cb
        export PATH="$HOME/.local/bin:$PATH"
        printf "\e[1;33mMake sure to add Clipboard to your PATH!\e[0m"
    else
        cmake --install .
    fi

    if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
    then
        doas cmake --install .
    else
        sudo cmake --install .
    fi
}

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir"

if [ "$(nix-info --host-os > info.txt && grep -ow "NixOS" info.txt)" = "NixOS" ]
then
    compile_section
    nix=true
fi

if [ "$(uname)" = "Linux" ] && [ "$nix" != "true" ]
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
    fi
fi

if [ "$(uname)" = "Darwin" ]
then
    curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-macos-arm64-amd64.zip -o clipboard-macos.zip
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/clipboard /usr/local/bin/clipboard
    chmod +x /usr/local/bin/clipboard
    sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
fi

if [ "$(uname)" = "FreeBSD" ]
then
    if [ "$(uname -m)" = "amd64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-freebsd-amd64.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SsLl $download_link -o clipboard-freebsd.zip
        unzip clipboard-freebsd.zip
        rm clipboard-freebsd.zip
        sudo mv bin/clipboard /usr/local/bin/clipboard
        if [ -f "lib/libclipboardx11.so" ]
        then
            sudo mv lib/libclipboardx11.so /usr/local/lib/libclipboardx11.so
        fi
        if [ -f "lib/libclipboardwayland.so" ]
        then
            sudo mv lib/libclipboardwayland.so /usr/local/lib/libclipboardwayland.so
        fi
        chmod +x /usr/local/bin/clipboard
        sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
    fi
fi

if [ "$(uname)" = "OpenBSD" ]
then
    if [ "$(uname -m)" = "amd64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-openbsd-amd64.zip
    elif [ "$(uname -m)" = "arm64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-openbsd-arm64.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SsLl $download_link -o clipboard-openbsd.zip
        unzip clipboard-openbsd.zip
        rm clipboard-openbsd.zip
        doas mv bin/clipboard /usr/local/bin/clipboard
        if [ -f "lib/libclipboardx11.so" ]
        then
            doas mv lib/libclipboardx11.so /usr/local/lib/libclipboardx11.so
        fi
        if [ -f "lib/libclipboardwayland.so" ]
        then
            doas mv lib/libclipboardwayland.so /usr/local/lib/libclipboardwayland.so
        fi
        chmod +x /usr/local/bin/clipboard
        doas ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
    fi
fi

if [ "$(uname)" = "NetBSD" ]
then
    if [ "$(uname -m)" = "amd64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-netbsd-amd64.zip
    else
        download_link="skip"
    fi
    if [ "$download_link" != "skip" ]
    then
        curl -SsLl $download_link -o clipboard-netbsd.zip
        unzip clipboard-netbsd.zip
        rm clipboard-netbsd.zip
        sudo mv bin/clipboard /usr/pkg/bin/clipboard
        if [ -f "lib/libclipboardx11.so" ]
        then
            sudo mv lib/libclipboardx11.so /usr/pkg/lib/libclipboardx11.so
        fi
        if [ -f "lib/libclipboardwayland.so" ]
        then
            sudo mv lib/libclipboardwayland.so /usr/pkg/lib/libclipboardwayland.so
        fi
        chmod +x /usr/pkg/bin/clipboard
        sudo ln -sf /usr/pkg/bin/clipboard /usr/pkg/bin/cb
    fi
fi

compile_section

rm -rf "$tmp_dir"
check_installation

cd ..
