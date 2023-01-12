Invoke-WebRequest https://nightly.link/Slackadays/Clipboard/workflows/main/main/clipboard-windows-amd64.zip -OutFile clipboard-windows-amd64.zip
Expand-Archive clipboard-windows-amd64.zip -DestinationPath .\clipboard-windows-amd64

New-Item -ItemType Directory -Force -Path "C:\Program Files\Clipboard"

Copy-Item .\clipboard-windows-amd64\bin\clipboard.exe -Force -Destination "C:\Program Files\Clipboard\clipboard.exe"

New-Item -ItemType SymbolicLink -Force -Path "C:\Program Files\Clipboard\cb.exe" -Value "C:\Program Files\Clipboard\clipboard.exe"

$Old = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name path).path
$New = "$Old;C:\Program Files\Clipboard"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name path -Value $New

Remove-Item .\clipboard-windows-amd64.zip -Force
Remove-Item .\clipboard-windows-amd64 -Force -Recurse

Write-Host "Please restart to use Clipboard"