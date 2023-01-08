#!/bin/bash
set -euxo pipefail

git clone --depth 1 --branch 0.2.1 https://github.com/slackadays/Clipboard
pushd Clipboard

mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
then
    doas cmake --install .
else
    sudo cmake --install .
fi

popd
popd
