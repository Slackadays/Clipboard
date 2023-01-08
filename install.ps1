Set-PSDebug -Trace 1

git clone --depth 1 --branch 0.2.1r2 https://github.com/slackadays/Clipboard

Push-Location Clipboard\build

cmake ..
cmake --build . --config Release
cmake --install . --config Release

Pop-Location
Set-PSDebug -Trace 0
