#!/bin/sh

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
printf "=> Clipboard for Linux Installer\n"
printf "=> Made by gentoo-btw\n"
printf " \n"
read -p "=> You are about to install Clipboard for linux. Press ENTER to install Clipboard.."

printf "=> cmake Clipboard/src\n"
exec cmake Clipboard/src
printf "=> cmake --build .\n"
exec cmake --build .
printf "=> sudo cmake --install .\n"
exec sudo cmake --install .
fi

if [[ "$OSTYPE" == "freebsd"* ]]; then
printf "=> Clipboard for FreeBSD Installer\n"
printf "=> Made by gentoo-btw\n"
printf " \n"
read -p "=> You are about to install Clipboard for FreeBSD. Press ENTER to install Clipboard.."
printf "=> cmake Clipboard/src\n"
exec cmake Clipboard/src
printf "=> cmake --build .\n"
exec cmake --build .
printf "=> sudo cmake --install .\n"
exec sudo cmake --install .
fi

if [[ "$OSTYPE" == "openbsd"* ]]; then
printf "=> Clipboard for OpenBSD Installer\n"
printf "=> Made by gentoo-btw\n"
printf " \n"
read -p "=> You are about to install Clipboard for OpenBSD. Press ENTER to install Clipboard.."
printf "=> cmake Clipboard/src\n"
exec cmake Clipboard/src
printf "=> cmake --build .\n"
exec cmake --build .
printf "=> doas cmake --install .\n"
exec sudo cmake --install .
fi

if [[ "$OSTYPE" == "solaris"* ]]; then
printf "=> Clipboard for Solaris Installer\n"
printf "=> Made by gentoo-btw\n"
read -p "=> You are about to install Clipboard for Solaris. Press ENTER to install Clipboard.."
printf "=> NOTE: sudo is required. if you don't have sudo. cmake will fail to install Clipboard."
printf "=> cmake Clipboard/src\n"
exec cmake Clipboard/src
printf "=> cmake --build .\n"
exec cmake --build .
printf "=> sudo cmake --install .\n"
exec sudo cmake --install .
fi
