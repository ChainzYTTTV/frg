# Builds the Forge VS Code extension into dist/forge-<version>.vsix, and installs it with -Install.
#
#   .\editors\vscode\package.ps1 -Install
#
# A .vsix is a zip file: the extension's files go under extension/, next to two small manifest files.
# The package also records where frg.exe is, so the extension finds it without any setup.
param([switch]$Install)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$here = $PSScriptRoot
$pkg = Get-Content (Join-Path $here "package.json") -Raw | ConvertFrom-Json
$frg = [IO.Path]::GetFullPath((Join-Path $here "..\..\bin\frg.exe"))
if (-not (Test-Path $frg)) { throw "Build Forge first: $frg doesn't exist" }

$files = @("package.json", "extension.js", "language-configuration.json", "README.md",
           "syntaxes/forge.tmLanguage.json", "snippets/forge.json", "icons/frg.svg", "icons/forge-128.png")

$contentTypes = @'
<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".json" ContentType="application/json"/>
  <Default Extension=".js" ContentType="application/javascript"/>
  <Default Extension=".md" ContentType="text/markdown"/>
  <Default Extension=".svg" ContentType="image/svg+xml"/>
  <Default Extension=".png" ContentType="image/png"/>
  <Default Extension=".txt" ContentType="text/plain"/>
  <Default Extension=".vsixmanifest" ContentType="text/xml"/>
</Types>
'@

$manifest = @"
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="$($pkg.name)" Version="$($pkg.version)" Publisher="$($pkg.publisher)" />
    <DisplayName>$($pkg.displayName)</DisplayName>
    <Description xml:space="preserve">$($pkg.description)</Description>
    <Tags>forge,frg,game</Tags>
    <Categories>Programming Languages,Snippets</Categories>
    <GalleryFlags>Public</GalleryFlags>
    <Icon>extension/icons/forge-128.png</Icon>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="$($pkg.engines.vscode)" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace" />
      <Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value="" />
    </Properties>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code" />
  </Installation>
  <Dependencies />
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true" />
    <Asset Type="Microsoft.VisualStudio.Services.Icons.Default" Path="extension/icons/forge-128.png" Addressable="true" />
  </Assets>
</PackageManifest>
"@

$dist = Join-Path $here "dist"
New-Item -ItemType Directory -Force $dist | Out-Null
$vsix = Join-Path $dist "$($pkg.name)-$($pkg.version).vsix"
if (Test-Path $vsix) { Remove-Item $vsix }

$zip = [IO.Compression.ZipFile]::Open($vsix, [IO.Compression.ZipArchiveMode]::Create)
try {
    function Add-Text([string]$name, [string]$text) {
        $entry = $zip.CreateEntry($name)
        $w = [IO.StreamWriter]::new($entry.Open(), [Text.UTF8Encoding]::new($false))
        $w.Write($text)
        $w.Dispose()
    }
    Add-Text "[Content_Types].xml" $contentTypes
    Add-Text "extension.vsixmanifest" $manifest
    foreach ($f in $files) {
        [void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, (Join-Path $here $f), "extension/$f")
    }
    Add-Text "extension/frg-path.txt" $frg
} finally {
    $zip.Dispose()
}
Write-Host "Built $vsix"

if ($Install) {
    & code --install-extension $vsix --force
    Write-Host "Installed. Reload VS Code (Ctrl+Shift+P > 'Developer: Reload Window') to start using it."
}
