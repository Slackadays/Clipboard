#!/bin/bash
set -uxo pipefail
set +e

if [ "$(uname)" = "Linux" ]
then
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
    if [ "$(uname -m)" = "x86_64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc10-amd64.zip
    elif [ "$(uname -m)" = "aarch64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc10-arm64.zip
    elif [ "$(uname -m)" = "riscv64" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc10-riscv64.zip
    elif [ "$(uname -m)" = "i386" ]
    then
        download_link=https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc10-i386.zip
    else
        download_link="skip"
    fi
    #check if download_link is NOT equal to skip
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

if [ "$(uname)" = "Darwin" ] && [ "$(uname -m)" = "x86_64" ]
then
    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd $tmp_dir
    curl -SsLl https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-macos-amd64.zip -o clipboard-macos-amd64.zip
    unzip clipboard-macos-amd64.zip
    rm clipboard-macos-amd64.zip
    sudo mv bin/clipboard /usr/local/bin/clipboard
    chmod +x /usr/local/bin/clipboard
    sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
    rm -rf $tmp_dir
    echo "Installed Clipboard"
    exit 0
fi

git clone --depth 1 https://github.com/slackadays/Clipboard

pushd Clipboard/build
cmake ..
cmake --build .

if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
then
    doas cmake --install .
else
    sudo cmake --install .
fi

popd
