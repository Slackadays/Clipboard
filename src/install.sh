#!/bin/bash
set -uxo pipefail
set +e

if [ "$(uname)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]
then
    wget https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-linux-gcc10-amd64.zip -O clipboard-linux-amd64.zip
    unzip clipboard-linux-amd64.zip
    rm clipboard-linux-amd64.zip
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
    rm -r bin lib
    echo "Installed Clipboard"
    exit 0
fi

if [ "$(uname)" = "Darwin" ] && [ "$(uname -m)" = "x86_64" ]
then
    wget https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-macos-amd64.zip -O clipboard-macos-amd64.zip
    unzip clipboard-macos-amd64.zip
    rm clipboard-macos-amd64.zip
    sudo mv bin/clipboard /usr/local/bin/clipboard
    chmod +x /usr/local/bin/clipboard
    sudo ln -sf /usr/local/bin/clipboard /usr/local/bin/cb
    rm -r bin lib
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
