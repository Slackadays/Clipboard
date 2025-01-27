#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

# Color codes for better readability
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

print_success() { printf "%s\n" "${GREEN}$1${RESET}"; }
print_error() { printf "%s\n" "${RED}$1${RESET}"; }

unsupported() {
    print_error "Sorry, but this installer script doesn't support $1."
    printf "%s\n" "${GREEN}💡 However, you can still install CB using the other methods in the readme!${RESET}"
}

verify_flatpak() {
    if flatpak list | grep -q "$flatpak_package"; then
        print_success "Clipboard installed successfully!"
        printf "%s\n" "${RESET}Add this alias to your terminal profile (like .bashrc) to make it work every time:${RESET}"
        printf "%s\n" "${YELLOW}alias cb=\"flatpak run $flatpak_package\"${RESET}"
        exit 0
    fi
    print_error "Couldn't install CB"
    exit 1
}

verify() {
    if command -v cb >/dev/null 2>&1; then
        if ! cb >/dev/null 2>&1; then
            unsupported "this system"
            exit 1
        fi
        print_success "Clipboard installed successfully!"
        exit 0
    fi
    print_error "Couldn't install CB"
    exit 1
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv >/dev/null 2>&1; then
        return 0    # No password needed
    elif echo "$prompt" | grep -q '^sudo:'; then
        return 0    # Password needed but sudo available
    fi
    return 1       # Sudo not available
}

has_alsa() {
    command -v aplay >/dev/null 2>&1
}

compile() {
    git clone --depth 1 https://github.com/slackadays/Clipboard
    cd Clipboard/build || exit 1
    
    if has_alsa; then
        cmake ..
    else
        cmake -DNO_ALSA=ON ..
    fi
    
    cmake --build .

    if [ "$(uname)" = "OpenBSD" ]; then
        doas cmake --install .
    elif can_use_sudo; then
        sudo cmake --install .
    else
        mkdir -p "$HOME/.local"
        cmake --install . --install-prefix="$HOME/.local"
    fi
}

download_and_install() {
    zip_file="$1"
    curl -SL "$download_link" -o "$zip_file"
    unzip "$zip_file"
    rm "$zip_file"

    if [ "$requires_sudo" = true ]; then
        sudo mv bin/cb "$install_path/bin/cb"
        [ -f "lib/libcbx11.so" ] && sudo mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
        [ -f "lib/libcbwayland.so" ] && sudo mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
    else
        mv bin/cb "$install_path/bin/cb"
        [ -f "lib/libcbx11.so" ] && mv lib/libcbx11.so "$install_path/lib/libcbx11.so"
        [ -f "lib/libcbwayland.so" ] && mv lib/libcbwayland.so "$install_path/lib/libcbwayland.so"
    fi
    chmod +x "$install_path/bin/cb"
}

# Start installation process
print_success "Searching for a package manager..."

# Try package managers first
if command -v apk >/dev/null 2>&1 && can_use_sudo; then
    sudo apk add clipboard
    verify
elif command -v yay >/dev/null 2>&1 && can_use_sudo; then
    sudo yay -S clipboard
    verify
elif command -v emerge >/dev/null 2>&1 && can_use_sudo; then
    sudo emerge -av app-misc/clipboard
    verify
elif command -v brew >/dev/null 2>&1; then
    brew install clipboard
    verify
elif command -v flatpak >/dev/null 2>&1; then
    if can_use_sudo; then
        sudo flatpak install flathub "$flatpak_package" -y
    else
        flatpak install flathub "$flatpak_package" -y
    fi
    verify_flatpak
elif command -v snap >/dev/null 2>&1 && can_use_sudo; then
    sudo snap install clipboard
    verify
elif command -v nix-env >/dev/null 2>&1; then
    nix-env -iA nixpkgs.clipboard-jh
    verify
elif command -v pacstall >/dev/null 2>&1; then
    pacstall -I clipboard-bin
    verify
elif command -v scoop >/dev/null 2>&1; then
    scoop install clipboard
    verify
elif command -v xbps-install >/dev/null 2>&1 && can_use_sudo; then
    sudo xbps-install -S clipboard
    verify
fi

# If no package manager worked, try direct download
tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir" || exit 1

if can_use_sudo; then
    requires_sudo=true
    install_path="/usr/local"
    sudo mkdir -p "$install_path/bin" "$install_path/lib"
else
    requires_sudo=false
    install_path="$HOME/.local"
    mkdir -p "$install_path/bin" "$install_path/lib"
fi

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
        [ "$download_link" != "skip" ] && download_and_install "clipboard-linux.zip"
        ;;
    "Darwin")
        case "$(uname -m)" in
            "x86_64") download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-amd64.zip" ;;
            "arm64")  download_link="https://nightly.link/Slackadays/Clipboard/workflows/build-clipboard/main/clipboard-macos-arm64.zip" ;;
        esac
        download_and_install "clipboard-macos.zip"
        ;;
    "FreeBSD"|"OpenBSD"|"NetBSD")
        unsupported "$(uname)"
        exit 0
        ;;
    *)
        compile
        ;;
esac

cd ..
rm -rf "$tmp_dir"

verify
