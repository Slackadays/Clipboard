#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

# Color codes for better readability
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

print_success() { printf "%b\n" "${GREEN}$1${RESET}"; }
print_error() { printf "%b\n" "${RED}$1${RESET}"; }


unsupported() {
    print_error "Sorry, but this installer script doesn't support $1."
    printf "%b\n" "${GREEN}💡 However, you can still install CB using the other methods in the readme!${RESET}"
}

verify_flatpak() {
    if flatpak list | grep -q "$flatpak_package"
    then
        print_success "Clipboard installed successfully!"
        printf "%b\n" "${RESET}Add this alias to your terminal profile (like .bashrc) to make it work every time:${RESET}"
        printf "%b\n" "${YELLOW}alias cb=\"flatpak run $flatpak_package\"${RESET}"
        exit 0
    fi
    print_error "Unable to install CB with Flatpak"
    exit 1
}

verify() {
    if command -v cb >/dev/null 2>&1
    then
        if ! cb >/dev/null 2>&1
        then
            unsupported "this system"
            exit 1
        fi
        print_success "Clipboard installed successfully!"
        exit 0
    fi
    exit 1
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv >/dev/null 2>&1
    then
        return 0    # No password needed
    fi
    if echo "$prompt" | grep -q '^sudo:'
    then
        return 0    # Password needed but sudo available
    fi
    return 1       # Sudo not available
}

has_alsa() {
    if command -v aplay >/dev/null 2>&1
    then
      return 0 # aplay executed successfully 
    else
      return 1 # aplay failed to execute
    fi
}

compile() {
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build || exit 1
    
    if has_alsa
    then
        cmake ..
    else
        cmake -DNO_ALSA=TRUE ..
    fi
    
    cmake --build .

    if [ "$(uname)" = "OpenBSD" ]
    then
        doas cmake --install .
    else
        if can_use_sudo
        then
            sudo cmake --install .
        else
            mkdir -p "$HOME/.local"
            cmake --install . --install-prefix="$HOME/.local"
        fi
    fi
}

download_and_install() {
    # Temporarily disable exit on error
    set +e
    
    zip_file="$1"
    if ! curl -SL "$download_link" -o "$zip_file"
    then
        false
        return
    fi
    
    if ! unzip "$zip_file"
    then
        false
        return
    fi
    
    rm "$zip_file"

    if [ "$requires_sudo" = true ]
    then
        if ! sudo mv bin/cb "$install_path/bin/cb"
        then
            false
            return
        fi
        
        if [ -f "lib/libcbx11.so" ]
        then
            if ! sudo mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
            then
                false
                return
            fi
        fi

        if [ -f "lib/libcbwayland.so" ]
        then
            if ! sudo mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
            then
                false
                return
            fi
        fi
    else
        if ! mv bin/cb "$install_path/bin/cb"
        then
            false
            return
        fi

        if [ -f "lib/libcbx11.so" ]
        then
            if ! mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
            then
                false
                return
            fi
        fi

        if [ -f "lib/libcbwayland.so" ]
        then
            if ! mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
            then
                false
                return
            fi
        fi
    fi
    
    if ! chmod +x "$install_path/bin/cb"
    then
        false
        return
    fi
    
    # Re-enable exit on error
    set -e
    true
    return
}

is_package_missing() {
    dpkg-query -W -f='${Status}' "$1" 2>/dev/null | grep -q "ok installed"
    return
}

install_debian_deps(){
   deps=""
    for pkg in openssl libssl3 libssl-dev
    do
        if is_package_missing "$pkg"
        then
            deps="$deps $pkg"
        fi
    done
    
    if [ -n "$deps" ]
    then
        sudo apt-get update
        sudo apt-get install -y $deps
    fi
}
# Start installation process
print_success "Searching for a package manager..."

# Try package managers first
if command -v apk >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo apk add clipboard
        verify
    fi
fi

if command -v yay >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo yay -S clipboard
        verify
    fi
fi

if command -v emerge >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo emerge -av app-misc/clipboard
        verify
    fi
fi

if command -v brew >/dev/null 2>&1
then
    brew install clipboard
    verify
fi

if command -v flatpak >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo flatpak install flathub "$flatpak_package" -y
    else
        flatpak install flathub "$flatpak_package" -y
    fi
    verify_flatpak
fi

# if command -v snap >/dev/null 2>&1
# then
#     if can_use_sudo
#     then
#         sudo snap install clipboard
#         verify
#     fi
# fi

if command -v nix-env >/dev/null 2>&1
then
    nix-env -iA nixpkgs.clipboard-jh
    verify
fi

if command -v pacstall >/dev/null 2>&1
then
    pacstall -I clipboard-bin
    verify
fi

if command -v scoop >/dev/null 2>&1
then
    scoop install clipboard
    verify
fi

if command -v xbps-install >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo xbps-install -S clipboard
        verify
    fi
fi

if command -v apt-get >/dev/null 2>&1 && command -v dpkg-query >/dev/null 2>&1; then
    if can_use_sudo; then
        install_debian_deps
    fi
fi

print_error "No supported package manager found."

print_success "Attempting to download release zip file for architecture..."

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir" || exit 1

if can_use_sudo
then
    requires_sudo=true
    install_path="/usr/local"
    sudo mkdir -p "$install_path/bin" "$install_path/lib"
else
    requires_sudo=false
    install_path="$HOME/.local"
    mkdir -p "$install_path/bin" "$install_path/lib"
fi

download_name="skip"

case "$(uname)" in
    "Linux")
        case "$(uname -m)" in
            "x86_64")  download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-amd64.zip" ;;
            "aarch64") download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-arm64.zip" ;;
            "riscv64") download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-riscv64.zip" ;;
            "i386")    download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-i386.zip" ;;
            "ppc64le") download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-ppc64le.zip" ;;
            "s390x")   download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-linux-s390x.zip" ;;
            *)         download_link="skip" ;;
        esac
        if [ "$download_link" != "skip" ]
        then
          download_name="clipboard-linux.zip"
        fi
        ;; 
    "Darwin")
        case "$(uname -m)" in
            "x86_64") download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-amd64.zip" ;;
            "arm64")  download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-arm64.zip" ;;
        esac
          download_name="clipboard-macos.zip"        
        ;;
    "FreeBSD"|"OpenBSD"|"NetBSD")
        unsupported "$(uname)"
        exit 0
        ;;
    *)
        print_error "No supported release download available."
        print_success "Attempting compile with CMake..."
        ;;
esac
  
if [ "$download_name" != "skip" ]
then
  print_success "Release download found for this platform!"
  print_success "Attempting download and install..."
  if download_and_install $download_name
  then
    print_success "Download and installed complete with no errors!"
  else
    print_error "Something went wrong with the download, attempting to compile..."
    compile
  fi
else
  compile
fi

cd ..
rm -rf "$tmp_dir"

verify
