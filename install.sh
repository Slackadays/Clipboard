#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

unsupported() {
    printf "\033[31mSorry, but this installer script doesn't support %s.\n\033[0m" "$1"
    printf '\033[32m💡 However, you can still install CB using the other methods in the readme!\n\033[0m'
}

verify_flatpak(){
    if flatpak list | grep -q "$flatpak_package"; then
        printf "\033[32mClipboard installed successfully!\n\033[0m"
        printf "\033[0mAdd this alias to your terminal profile (like .bashrc) to make it work every time:\n\033[0m"
        printf '\033[33malias cb="flatpak run %s"\n\033[0m' "$flatpak_package"
        exit 0
    else
        printf "\033[31mCouldn't install CB\n\033[0m"
        exit 1
    fi
}

verify() {
    if command -v cb 2>&1
    then
        if ! cb 2>&1
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

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv > /dev/null 2>&1; then
      # exit code of sudo-command is 0
      return 0
    elif echo "$prompt" | grep -q '^sudo:'; then
      return 0
    else
      return 1
    fi
}

has_alsa(){
    if command -v aplay >/dev/null 2>&1 && [ -d "/dev/snd" ]; then
      return 1 
    else
      return 0
   fi 
}

compile() {
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build
    
    if has_alsa
    then
      cmake ..
    else
      cmake -DNO_ALSA=true ..
    fi

    cmake --build . 

    if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
    then
        doas cmake --install .
    elif can_use_sudo
    then
        sudo cmake --install .
    else
        cmake --install .
    fi
}


printf "\033[32mSearching for a package manager...\n\033[0m"

if command -v apk > /dev/null 2>&1
then
    if can_use_sudo
    then
        sudo apk add clipboard
        verify
    fi
elif command -v yay > /dev/null 2>&1
then
    if can_use_sudo
    then
        sudo yay -S clipboard
        verify
    fi
elif command -v emerge > /dev/null 2>&1
then
    if can_use_sudo
    then
        sudo emerge -av app-misc/clipboard
        verify
    fi
elif command -v flatpak > /dev/null 2>&1
then
    if can_use_sudo
    then
      sudo flatpak install flathub "$flatpak_package" -y
    else
      flatpak install flathub "$flatpak_package" -y
    fi

    verify_flatpak
elif command -v snap > /dev/null 2>&1
then
    if can_use_sudo
    then
        sudo snap install clipboard
        verify
    fi
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
    if can_use_sudo
    then
        sudo xbps-install -S clipboard
        verify
    fi
else
##TODO: If the download links are updated, I left the rest of the script in ./download.sh and this should work##
# cd ..
# sh ./download.sh
printf "\033[32mNot found in package manager, compiling with cmake...\n\033[0m"
    compile
fi


verify
