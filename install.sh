#!/bin/sh
set -eu

unsupported() {
    printf "\033[31mSorry, but this installer script doesn't support %s.\n\033[0m" "$1"
    printf '\033[32m💡 However, you can still install CB using the other methods in the readme!\n\033[0m'
}

verify() {
    if command -v cb >/dev/null 2>&1
    then
        if ! cb >/dev/null 2>&1
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

compile() {
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    cmake ..
    cmake --build .
    cmake --install .

    if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
    then
        doas cmake --install .
    else
        sudo cmake --install .
    fi
}

if command -v apk > /dev/null 2>&1
then
    sudo apk add clipboard
    verify
elif command -v yay > /dev/null 2>&1
then
    sudo yay -S clipboard
    verify
elif command -v emerge > /dev/null 2>&1
then
    sudo emerge -av app-misc/clipboard
    verify
elif command -v brew > /dev/null 2>&1
then
    brew install clipboard
    verify
elif command -v nix-env > /dev/null 2>&1
then
    nix-env -iA nixpkgs.clipboard-jh
    verify
elif command -v pacstall > /dev/null 2>&1
then
    pacstall -I clipboard-bin
    verify
elif command -v scoop > /dev/null 2>&1
then
    scoop install clipboard
    verify
elif command -v xbps-install > /dev/null 2>&1
then
    sudo xbps-install -S clipboard
    verify
fi

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir"

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
        sudo mv bin/cb /usr/local/bin/cb
        chmod +x /usr/local/bin/cb
        if [ -f "lib/libcbx11.so" ]
        then
            sudo mv lib/libcbx11.so /usr/local/lib/libcbx11.so
        fi
        if [ -f "lib/libcbwayland.so" ]
        then
            sudo mv lib/libcbwayland.so /usr/local/lib/libcbwayland.so
        fi
    fi
elif [ "$(uname)" = "Darwin" ]
then
    curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-arm64-amd64.zip -o clipboard-macos.zip
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/cb /usr/local/bin/cb
    chmod +x /usr/local/bin/cb
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
