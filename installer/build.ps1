# Builds the Forge installer: installer\out\Forge-<version>-setup.exe.
#
#   .\installer\build.ps1 -W64devkit C:\w64devkit
#
# Build frg first (cmake --build build). -W64devkit is the w64devkit folder to put inside (the one with
# bin\g++.exe): the installed frg uses that compiler by itself. Needs Inno Setup 6 (ISCC.exe on the PATH or in its
# usual folder; -Iscc gives its path). Everything that's installed is gathered in installer\stage first.
param(
    [Parameter(Mandatory = $true)][string]$W64devkit,
    [string]$Iscc = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$stage = Join-Path $PSScriptRoot "stage"
$frg = Join-Path $root "bin\frg.exe"

if (-not (Test-Path $frg)) { throw "Build Forge first: $frg doesn't exist" }
if (-not (Test-Path (Join-Path $W64devkit "bin\g++.exe"))) { throw "$W64devkit isn't a w64devkit folder (there's no bin\g++.exe in it)" }
$version = ((& $frg version) -replace '^Forge ', '').Trim()
Write-Host "Forge $version"

if (-not $Iscc) {
    $found = Get-Command iscc -ErrorAction SilentlyContinue
    $Iscc = if ($found) { $found.Source } else {
        @("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe", "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") |
            Where-Object { Test-Path $_ } | Select-Object -First 1
    }
}
if (-not $Iscc -or -not (Test-Path $Iscc)) { throw "Can't find Inno Setup's ISCC.exe: install Inno Setup 6, or pass -Iscc" }

# ---- gather what's installed
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item $stage -ItemType Directory | Out-Null

function Copy-Folder([string]$from, [string]$to, [string[]]$skipDirs = @()) {
    # robocopy: copies a folder, skipping some folders by name (exit codes below 8 mean success)
    $args = @($from, $to, "/E", "/NFL", "/NDL", "/NJH", "/NJS", "/NP")
    if ($skipDirs.Count) { $args += "/XD"; $args += $skipDirs }
    & robocopy @args | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "Copying $from failed (robocopy $LASTEXITCODE)" }
    $global:LASTEXITCODE = 0
}

New-Item (Join-Path $stage "bin") -ItemType Directory | Out-Null
Copy-Item $frg (Join-Path $stage "bin")
Copy-Item (Join-Path $root "bin\forge-edit.exe") (Join-Path $stage "bin")   # Forge Edit, the light editor
Copy-Folder (Join-Path $root "bin\fonts") (Join-Path $stage "bin\fonts")
Copy-Folder (Join-Path $root "bin\runtime") (Join-Path $stage "bin\runtime")
Copy-Folder $W64devkit (Join-Path $stage "w64devkit")
Copy-Folder (Join-Path $root "docs") (Join-Path $stage "docs")
Copy-Folder (Join-Path $root "examples") (Join-Path $stage "examples") @("build", "dist")
New-Item (Join-Path $stage "art") -ItemType Directory | Out-Null
Copy-Item (Join-Path $root "art\frg-file.ico") (Join-Path $stage "art")
foreach ($f in "LICENSE", "README.md", "CHANGELOG.md") { Copy-Item (Join-Path $root $f) $stage }

# the VS Code extension
& (Join-Path $root "editors\vscode\package.ps1")
$vsix = Get-ChildItem (Join-Path $root "editors\vscode\dist") -Filter "forge-$version.vsix" | Select-Object -First 1
if (-not $vsix) { throw "The VS Code extension wasn't built (editors\vscode\dist\forge-$version.vsix)" }
New-Item (Join-Path $stage "editors") -ItemType Directory | Out-Null
Copy-Item $vsix.FullName (Join-Path $stage "editors\forge.vsix")

# what else is inside, and their licenses
@"
Forge $version - the parts that come from other projects

w64devkit (the w64devkit folder): the C++ compiler Forge uses to make .exe files - GCC, binutils and mingw-w64,
put together by Chris Wellons. Each part keeps its own license (GCC and binutils: GPL, with the GCC runtime
library exception, so games made with it can use any license). Their sources: https://github.com/skeeto/w64devkit

raylib (bin\runtime\raylib): the library Forge's engine draws, plays sound and reads input with. zlib license:
bin\runtime\raylib\LICENSE. https://www.raylib.com

DejaVu fonts (bin\fonts): the fonts Forge Edit draws its text with. Free license: bin\fonts\LICENSE-DejaVu.txt.
https://dejavu-fonts.github.io

Forge itself: MIT license, see LICENSE.
"@ | Out-File -Encoding utf8 (Join-Path $stage "THIRD-PARTY.txt")

# ---- build it
& $Iscc "/DVersion=$version" "/Q" (Join-Path $PSScriptRoot "forge.iss")
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed ($LASTEXITCODE)" }
$out = Join-Path $PSScriptRoot "out\Forge-$version-setup.exe"
Write-Host ("Made {0} ({1:N0} MB)" -f $out, ((Get-Item $out).Length / 1MB))
