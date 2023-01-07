function Show-Exec {
    param (
        $Command
    )
    Write-Host $Command
    Invoke-Expression $Command
}

Show-Exec -Command "git clone --depth 1 --branch 0.2.1 https://github.com/slackadays/Clipboard"

Set-Location Clipboard

Show-Exec -Command "cmake ."
Show-Exec -Command "cmake --build . --config Release"
Show-Exec -Command "cmake --install . --config Release"