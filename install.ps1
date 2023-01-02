$clone_command = "git clone https://github.com/slackadays/Clipboard"
Write-Host $clone_command
Invoke-Expression $clone_command

cd Clipboard

$configure_command = "cmake ."
Write-Host $configure_command
Invoke-Expression $configure_command

$build_command = "cmake --build . --config Release"
Write-Host $build_command
Invoke-Expression $build_command

$install_command = "cmake --install . --config Release"
Write-Host $install_command
Invoke-Expression $install_command