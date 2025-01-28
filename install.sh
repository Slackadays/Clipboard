#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

# Color codes for better readability
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

print_success() { printf "%b\n" "${GREEN}$1${RESET}"; }
print_warning() { printf "%b\n" "${YELLOW}$1${RESET}"; }
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
            print_error "Error with the runtime of cb, but able to execute." 
            exit 1
        fi
        print_success "Clipboard installed successfully!"
        exit 0
    fi
    print_error "Clipboard is not able to be called, check that /usr/local/bin is accessible by PATH."
    exit 1
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv >/dev/null 2>&1
    then
      print_success "Sudo ist nicht verboten!"
      return 0    # No password needed
    fi
    if echo "$prompt" | grep -q '^sudo:'
    then
      print_warning "Sudo ist normalerweise verboten."
      return 0    # Password needed but sudo available
    fi
    print_error "Sudo ist verboten!"
    return 1       # Sudo not available
}


# has_header() {
#     header="$1"
#     # See if pre-processor exists 
#     if command -v cpp >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | cpp -H -o /dev/null >/dev/null 2>&1
#         return
#     fi
#     # Try gcc if available
#     if command -v gcc >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | gcc -E - >/dev/null 2>&1
#         return
#     fi
#     # Try clang if available
#     if command -v clang >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | clang -E - >/dev/null 2>&1
#         return
#     fi
#     # No known compiler found
#     false
#     return
# }

has_apt(){
 command -v apt-get >/dev/null 2>&1
 return 
}

install_debian_deps(){
  if can_use_sudo
  then
    sudo apt-get install -qq openssl
    sudo apt-get install -qq libssl3
    sudo apt-get install -qq libssl-dev
  fi
  }

compile() {
    if has_apt
    then
      install_debian_deps
    fi

    git clone --depth 1 https://github.com/map588/Clipboard
    cd Clipboard/build
   
    cmake ..
    cmake --build .

    if [ "$(uname)" = "OpenBSD" ]
    then
        #It will get here but doas is not configured on testing
        doas cmake --install .
    else
        if can_use_sudo
        then
            print_success "Sudo used for cmake."
            sudo cmake --install .
        else
            print_success "Installing locally, sudo is verboten."
            mkdir -p "$HOME/.local"
            cmake --install . --prefix="$HOME/.local"
        fi
    fi
}

download_and_install() {
  download_link="$1"
  os_type="$2"

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
  print_success "Install path: $install_path"
  print_success "Current directory: $(pwd)"
  print_success "Platform: $(uname):$(uname -m)"
  case "$os_type" in
    "Linux")
      curl -SsLl "$download_link" -o "clipboard.zip" 
      ;;
    "NetBSD")
      if command -v ftp >/dev/null 2>&1
      then
        ftp -o "clipboard.zip" "$download_link" 
      elif command -v curl >/dev/null 2>&1
      then
        curl -SsLl "$download_link" -o "clipboard.zip" 
      else
        unsupported "download $(uname):$(uname -m)"
      fi
      ;;
  "Darwin" | "FreeBSD")
      curl -SsLl "$download_link" -o "clipboard.zip" 
      ;;
    *) unsupported "$(uname):$(uname -m)"; exit 1 ;;
  esac

  unzip "clipboard.zip"

  print_warning "Libraries in $install_path/lib:"

  if [ "$os_type" = "Darwin" ]
  then
    if [ "$requires_sudo" = true ] 
    then
      sudo mv bin/cb "$install_path/bin/cb"
      sudo mv lib/libgui.a "$install_path/lib/libgui.a"
      sudo chmod +x "$install_path/bin/cb"
    else
      mv bin/cb "$install_path/bin/cb"
      mv lib/libgui.a "$install_path/lib/libgui.a"
      chmod +x "$install_path/bin/cb"
    fi

  else

 if [ "$requires_sudo" = true ]
    then
      [ -f "bin/cb" ] && sudo mv bin/cb "$install_path/bin/cb" 
      [ -f "lib/libgui.a" ] && sudo mv "lib/libgui.a" "$install_path/lib/libgui.a"
      [ -f "lib/libcbx11.so" ] && sudo mv "lib/libcbx11.so" "$install_path/lib/libcbx11.so"
      [ -f "lib/libcbwayland.so" ] && sudo mv "lib/libcbwayland.so" "$install_path/lib/libcbwayland.so"
      sudo chmod +x "$install_path/bin/cb"
    else
      [ -f "bin/cb" ] && mv bin/cb "$install_path/bin/cb"
      [ -f "lib/libgui.a" ] && mv "lib/libgui.a" "$install_path/lib/libgui.a"
      [ -f "lib/libcbx11.so" ] && mv "lib/libcbx11.so" "$install_path/lib/libcbx11.so"
      [ -f "lib/libcbwayland.so" ] && mv "lib/libcbwayland.so" "$install_path/lib/libcbwayland.so"
      chmod +x "$install_path/bin/cb"
    fi  
  fi
  
     if [ -f "$install_path/lib/libgui.a" ]
     then
       print_warning "libgui.a"
     fi
     if [ -f "$install_path/lib/libcbx11.so" ]
     then
       print_warning "libcbx11.so"
     fi
     if [ -f "$install_path/lib/libcbwayland.so" ]
     then
       print_warning "libcbwayland.so"
     fi

     if [ -f "$install_path/bin/cb" ]
     then
       print_success "cb is in $install_path/bin!"
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


print_error "No supported package manager found."
print_success "Attempting to download release zip file for architecture..."

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir" || exit 1

download_link="skip"

case "$(uname)" in
  "Linux")
    case "$(uname -m)" in
      "amd64" )  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-amd64.zip" ;;
      "aarch64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-arm64.zip" ;;
      "riscv64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-riscv64.zip" ;;
      "i386")    download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-i386.zip" ;;
      "ppc64le") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-ppc64le.zip" ;;
      "s390x")   download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-s390x.zip" ;;
      *)         download_link="skip" ;;
    esac
    ;; 
  "Darwin")
    case "$(uname -m)" in
      "x86_64" | "amd64" ) download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-macos-amd64.zip" ;;
      "arm64")  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-macos-arm64.zip" ;;
    esac
    ;;
   "FreeBSD")
      case "$(uname -m)" in
        "x86_64" | "amd64" ) download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-freebsd-amd64.zip" ;;
               *) print_error "No supported release download available for $(uname):$(uname -m)"
                  print_success "Attempting compile with CMake..."
                  compile
                  ;;
      esac
      ;;
    "NetBSD")
       case "$(uname -m)" in
        "x86_64" | "amd64" ) download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-netbsd-amd64.zip" ;;
               *) print_error "No supported release download available for $(uname):$(uname -m)"
                  print_success "Attempting compile with CMake..."
                  compile
                  ;; 
       esac
       ;;
  *)
    print_error "No supported release download available for $(uname):$(uname -m)"
    print_success "Attempting compile with CMake..."
    compile
    ;;
esac

if [ "$download_link" != "skip" ]
then
  download_and_install "$download_link" "$(uname)" 
else
  compile
fi

cd ..
rm -rf "$tmp_dir"

verify
