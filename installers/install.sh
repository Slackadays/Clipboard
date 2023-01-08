#!/bin/bash
set -euxo pipefail

git clone --depth 1 --branch 0.2.1r1 https://github.com/slackadays/Clipboard

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
popd
