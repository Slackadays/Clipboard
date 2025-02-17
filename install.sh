#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

user_prefix="$HOME/.local"
sudo_prefix="/usr/local"

requires_sudo=""

# Color codes for better readability
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

print_success() { printf "%b\n" "${GREEN}$1${RESET}"; }
print_warning() { printf "%b\n" "${YELLOW}$1${RESET}"; }
print_error()   { printf "%b\n" "${RED}$1${RESET}"; }


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
        print_success "cb installed successfully!"
        exit 0
    fi

    if [ -f "$install_path/bin/cb" ]
    then
        print_error "cb is not able to be executed, check that $install_path/bin is accessible by PATH."
    else
        print_error "cb was not able to be installed in $install_path/bin, check your file permissions."
    fi

    exit 1
}

check_install_prefix(){
    if [ "$requires_sudo" = "true" ]
    then
        current_dir="$(pwd)"
        test_dir="$(mktemp -d -t cb-test-XXXXXXXXXX)"
        cd "$test_dir"
        touch test
        if ! sudo mv test "$sudo_prefix" >/dev/null 2>&1
        then 
            print_warning "User has sudo permissions, but no access to $sudo_prefix."
            sudo_prefix="$user_prefix"
        fi
        cd "$current_dir"
        rm -rf "$test_dir"
    fi
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv >/dev/null 2>&1
    then
      requires_sudo="true"
      check_install_prefix
      return 0    # No password needed
    fi
    if echo "$prompt" | grep -q '^sudo:'
    then
      requires_sudo="true"
      check_install_prefix
      print_warning "Using sudo will require your password, possibly more than once."  
      return 0     # Password needed but sudo available
    fi
    requires_sudo="false"
    print_error "User is not able to use sudo, local installation."
    return 1       # Sudo not available
}



has_apt(){
 command -v apt-get >/dev/null 2>&1
 return 
}

install_debian_deps(){
  if [ "$requires_sudo" = "true" ]
  then
    sudo apt-get install -qq openssl libssl3 libssl-dev git cmake
  else
    print_warn "Was not able to install dependencies, build may fail."
    print_warn "Ensure installed: openssl libssl3 libssl-dev git cmake"
  fi
}

compile() {
    can_use_sudo

    if has_apt
    then
      install_debian_deps
    fi
    
    [ -d "Clipboard" ] && rm -rf Clipboard

    git clone --depth 1 https://github.com/Slackadays/Clipboard
    cd Clipboard/build
   
    cmake ..
    cmake --build .

    if [ "$(uname)" = "OpenBSD" ]
    then
        #It will get here but doas is not configured on testing
        doas cmake --install .
    else
        if [ $requires_sudo = true ] 
        then
            print_success "Installing at "
            sudo cmake --install .
        else
            print_success "Installing locally, sudo is not available."
            mkdir -p "$user_prefix"
            cmake --install . --prefix="$user_prefix"
        fi
    fi
    verify
}

package="find"
download_release="find"
compile_source="try"

fallback_compile() {
    if [ "$compile_source" = "try" ]
    then 
        print_error "Error: download was not successful."
        print_warning "Attempting compile with CMake.."
        compile
    fi
}

download_and_install() {
  download_link="$1"
  os_type="$2"

  if can_use_sudo
  then
    requires_sudo=true
    install_path="$sudo_prefix" 
  else
    requires_sudo=false
    install_path="$user_prefix"
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

  [ -f "$(pwd)/clipboard.zip" ] || fallback_compile

  unzip "clipboard.zip"
  print_warning "Unzipping release file.."


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
      [ -f "bin/cb" ] && mv bin/cb "$install_path/bin/cb"
      [ -f "lib/libgui.a" ] && mv "lib/libgui.a" "$install_path/lib/libgui.a"
      [ -f "lib/libcbx11.so" ] && mv "lib/libcbx11.so" "$install_path/lib/libcbx11.so"
      [ -f "lib/libcbwayland.so" ] && mv "lib/libcbwayland.so" "$install_path/lib/libcbwayland.so"

 if [ "$requires_sudo" = true ]
    then
      sudo chmod +x "$install_path/bin/cb"
    else
      chmod +x "$install_path/bin/cb"
    fi  
  fi

  verify
}

print_usage(){
    echo "Usage: ./install.sh [-c] [-p] [-d]"
    echo ""
    echo "Passing no flag attempts to install in the order of:"
    echo "    Package Manager"
    echo "    Release Download"
    echo "    Build from source"
    echo ""
    echo "-c|--compile          Skip package and release checks, compile from source."
    echo "-p|--package          Only attempt to find cb in your package manager."
    echo "-d|--download         Only search for a release binary."
    echo "<No Flags>            Package Manager => Release Download => Attempt Compile."
    exit 0
}


if  ! [ "$#" -eq "0" ]
then
    case "$1" in
        "-c" | "--compile")  download_release="skip" && package="skip" ;;
        "-p" | "--package")  compile_source="skip" && download_release="skip" ;;
        "-d" | "--download") compile_source="skip" && package="skip" ;;
        *) print_usage
    esac
fi

if [ "$package" = "find" ]
then
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

    # Snap package no longer works 

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
    if [ $download_release = "skip" ]
    then
        exit 0
    fi
fi #Package Manager

if [ $download_release = "find" ]
then
    print_success "Attempting to download release binary for your system and architecture..."

    tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
    cd "$tmp_dir" || eval "$(print_error "Temp Directory not acccessible."; if [ "$compile_source" = "try" ]; then compile; fi)"
    

    download_link="skip"
    case "$(uname)" in
      "Linux")
        case "$(uname -m)" in
          "x86_64"  | "amd64" )  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-amd64.zip" ;;
          "aarch64" | "arm64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-arm64.zip" ;;
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
          "aarch64" | "arm64")  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-macos-arm64.zip" ;;
        esac
        ;;
       "FreeBSD")
          case "$(uname -m)" in
            "x86_64" | "amd64" ) download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-freebsd-amd64.zip" ;;
                   *) print_error "No supported release download available for $(uname):$(uname -m)"
                      print_success "Attempting compile with CMake..."
                      ;;
          esac
          ;;
        "NetBSD")
           case "$(uname -m)" in
            "x86_64" | "amd64" ) download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-netbsd-amd64.zip" ;;
                   *) print_error "No supported release download available for $(uname):$(uname -m)"
                      ;; 
           esac
           ;;
      *)
        print_error "No supported release download available for $(uname):$(uname -m)"
        ;;
    esac

    if [ "$download_link" != "skip" ]
    then
      download_and_install "$download_link" "$(uname)" 
    else
        print_error "No supported release was found for your system ($(uname):$(uname -m))."
    fi

    cd ..
    rm -rf "$tmp_dir"
fi #Release Download

if [ "$compile_source" = "try" ]
then
  print_success "Attempting to compile with CMake.."
  compile
fi #Compile

