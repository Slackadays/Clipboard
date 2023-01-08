Set-PSDebug -Trace 1
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

git clone --depth 1 --branch 0.2.1r2 https://github.com/slackadays/Clipboard

Push-Location Clipboard\build

cmake ..
cmake --build . --config Release
cmake --install . --config Release

Pop-Location
Pop-Location
Set-PSDebug -Trace 0
