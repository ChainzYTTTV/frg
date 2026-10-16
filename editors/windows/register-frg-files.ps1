# Gives .frg files the Forge icon in File Explorer, and makes double-clicking one open it in VS Code.
#
#   .\editors\windows\register-frg-files.ps1           set it up
#   .\editors\windows\register-frg-files.ps1 -Remove   undo it
#
# It only changes settings for your own Windows account (HKEY_CURRENT_USER), so it doesn't need admin rights.
# The icon is art\frg-file.ico in this folder's Forge copy, so keep the Forge folder where it is (or run this again
# after moving it).
param([switch]$Remove)

$ErrorActionPreference = "Stop"
$classes = "HKCU:\Software\Classes"
$progId = "Forge.SourceFile"

function Refresh-Icons {
    # Tells Explorer that file types changed, so it redraws the icons straight away.
    Add-Type -Namespace Win32 -Name Shell -MemberDefinition '[DllImport("shell32.dll")] public static extern void SHChangeNotify(int eventId, uint flags, System.IntPtr item1, System.IntPtr item2);'
    [Win32.Shell]::SHChangeNotify(0x08000000, 0, [IntPtr]::Zero, [IntPtr]::Zero)
}

if ($Remove) {
    foreach ($key in "$classes\.frg", "$classes\$progId") {
        if (Test-Path $key) { Remove-Item $key -Recurse -Force }
    }
    Refresh-Icons
    Write-Host "Removed: .frg files are back to a plain icon."
    exit 0
}

$icon = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\art\frg-file.ico"))
if (-not (Test-Path $icon)) { throw "Can't find the icon: $icon" }

New-Item "$classes\.frg" -Force | Out-Null
Set-ItemProperty "$classes\.frg" -Name "(default)" -Value $progId
Set-ItemProperty "$classes\.frg" -Name "Content Type" -Value "text/plain"
Set-ItemProperty "$classes\.frg" -Name "PerceivedType" -Value "text"

New-Item "$classes\$progId\DefaultIcon" -Force | Out-Null
Set-ItemProperty "$classes\$progId" -Name "(default)" -Value "Forge source file"
Set-ItemProperty "$classes\$progId\DefaultIcon" -Name "(default)" -Value "`"$icon`""

# Double-click opens it in VS Code, if VS Code is installed.
$code = @("$env:LOCALAPPDATA\Programs\Microsoft VS Code\Code.exe", "$env:ProgramFiles\Microsoft VS Code\Code.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1
if ($code) {
    New-Item "$classes\$progId\shell\open\command" -Force | Out-Null
    Set-ItemProperty "$classes\$progId\shell\open\command" -Name "(default)" -Value "`"$code`" `"%1`""
}

Refresh-Icons
Write-Host "Done: .frg files now show the Forge icon$(if ($code) { ' and open in VS Code' })."
Write-Host "If File Explorer still shows the old icon, close and reopen its window."
