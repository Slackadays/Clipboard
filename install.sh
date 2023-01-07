print_exec() {
    echo "$1"
    $1
}

print_exec "git clone --depth 1 --branch 0.2.1 https://github.com/slackadays/Clipboard"

cd Clipboard

print_exec "cmake ."
print_exec "cmake --build ."

if [ "$(uname)" = "OpenBSD" ] #check if OpenBSD
then
    print_exec "doas cmake --install ."
else
    print_exec "sudo cmake --install ."
fi