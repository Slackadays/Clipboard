echo "=> Clipboard For Windows"
echo "=> Made by gentoo-btw"

echo "=> You are about to install Clipboard. Press ENTER to confirm and install clipboard."
pause

echo "=> cmake Clipboard/src"
cmake Clipboard/src
echo "=> cmake --build ."
cmake --build .
echo "=> cmake --install ."
cmake --install .
echo "=> Thanks for installing Clipboard."
