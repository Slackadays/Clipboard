#!/bin/bash
set -uxo pipefail
set +e

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
