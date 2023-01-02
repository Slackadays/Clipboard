clone_command="git clone https://github.com/slackadays/Clipboard"
echo $clone_command
$clone_command

cd Clipboard

configure_command="cmake ."
echo $configure_command
$configure_command

build_command="cmake --build ."
echo $build_command
$build_command

#check if OpenBSD
if [ "$(uname)" = "OpenBSD" ]
then
    install_command="doas cmake --install ."
else
    install_command="sudo cmake --install ."
fi

echo $install_command
$install_command