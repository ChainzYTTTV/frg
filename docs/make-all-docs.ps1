# Combines every docs page into one file, docs/FORGE_ALL_DOCS.md, to upload to a chat.
#
#   .\docs\make-all-docs.ps1          rebuild the file
#   .\docs\make-all-docs.ps1 -Check   only check it's up to date (exit code 1 if not); the tests run this
#
# Each page's headings move down one level, under a heading for each part. Links between pages become plain
# text like "the 2D camera (see "<graphics2d>" > "Camera: scrolling and zooming")", because the pages are no
# longer separate files. Pages that aren't in the list below are still added at the end, so none get left out.
param([switch]$Check)

$ErrorActionPreference = "Stop"
$docs = $PSScriptRoot
$out = Join-Path $docs "FORGE_ALL_DOCS.md"

$parts = [ordered]@{
    "Part A: Start here" = @("README.md", "getting-started.md")
    "Part B: The language" = @(
        "language/01-basics.md", "language/02-variables-and-types.md", "language/03-text.md", "language/04-operators.md",
        "language/05-control-flow.md", "language/06-functions.md", "language/07-files-and-folders.md",
        "language/08-lists-and-maps.md", "language/09-structs-and-classes.md", "language/10-enums-and-optionals.md",
        "language/11-tasks.md", "language/12-errors.md", "language/13-reserved-words.md",
        "language/14-coming-from-other-languages.md")
    "Part C: The libraries" = @(
        "libraries/README.md", "libraries/core-types.md", "libraries/stdio.md", "libraries/math.md", "libraries/random.md",
        "libraries/window.md", "libraries/input.md", "libraries/graphics2d.md", "libraries/graphics3d.md",
        "libraries/assets.md", "libraries/audio.md", "libraries/physics.md", "libraries/physics3d.md", "libraries/time.md", "libraries/file.md",
        "libraries/save.md", "libraries/tween.md", "libraries/particles.md", "libraries/tilemap.md", "libraries/scenes.md", "libraries/ui.md", "libraries/ecs.md", "libraries/tasks.md", "libraries/testing.md",
        "libraries/toolkits.md", "libraries/opengl.md", "libraries/directx.md", "libraries/vulkan.md")
    "Part D: Reference (exact rules, every command, every error message)" = @(
        "reference/syntax.md", "reference/rules.md", "reference/commands.md", "reference/error-messages.md")
    "Part E: Guides" = @("guides/game-loop.md", "guides/sprites-and-animation.md", "guides/platformer-walkthrough.md",
        "guides/practice-projects.md")
    "Part F: Under the hood" = @("how-forge-works.md")
    "Part G: The cheat sheet (everything on one page)" = @("LANGUAGE.md")
}
$skipped = @()

function RelPath([string]$full) { [IO.Path]::GetRelativePath($docs, $full).Replace('\', '/') }
function Read([string]$rel) { [IO.File]::ReadAllText((Join-Path $docs $rel)).Replace("`r`n", "`n") }

# Every page, so nothing is forgotten when a new page is added.
$listed = @($parts.Values | ForEach-Object { $_ }) + $skipped
$extra = Get-ChildItem $docs -Recurse -Filter *.md | ForEach-Object { RelPath $_.FullName } |
    Where-Object { $_ -ne "FORGE_ALL_DOCS.md" -and $_ -ne "TEACH_ME_FORGE.md" -and $listed -notcontains $_ } | Sort-Object
if ($extra) { $parts["Part H: Other pages"] = @($extra) }

# Lines inside ``` code blocks are left alone. A block opened with ```` only closes with ```` or more.
function ForEachProseLine([string]$text, [scriptblock]$fn) {
    $fence = 0
    $lines = $text -split "`n"
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        if ($line -match '^\s*(`{3,})(.*)$') {
            $ticks = $Matches[1].Length
            if ($fence -eq 0) { $fence = $ticks; continue }
            if ($ticks -ge $fence -and $Matches[2].Trim() -eq "") { $fence = 0; continue }
        }
        if ($fence -eq 0) { $lines[$i] = & $fn $line }
    }
    $lines -join "`n"
}

function Slug([string]$heading) {
    $h = $heading.Trim().ToLowerInvariant()
    $h = [regex]::Replace($h, '[^\w\- ]', '')
    $h.Replace(' ', '-')
}

# Each page's title (its first heading) and its headings by anchor, for turning links into text.
$titles = @{}
$headings = @{}
foreach ($rel in ($parts.Values | ForEach-Object { $_ })) {
    $map = @{}
    $title = $null
    ForEachProseLine (Read $rel) {
        param($line)
        if ($line -match '^(#{1,6}) (.+)$') {
            $text = $Matches[2].Trim()
            if (-not $title) { $script:title = $text }
            $slug = Slug $text
            if (-not $map.ContainsKey($slug)) { $map[$slug] = $text }
        }
        $line
    } | Out-Null
    $titles[$rel] = $title
    $headings[$rel] = $map
}

$lq = [char]0x201C; $rq = [char]0x201D; $arrow = [char]0x203A

function LinkToText([string]$rel, [string]$text, [string]$url) {
    if ($url -match '^(https?:|mailto:)') { return "[$text]($url)" }
    $path, $anchor = $url -split '#', 2
    if (-not $path) {
        $target = $rel
    } else {
        $full = [IO.Path]::GetFullPath((Join-Path (Split-Path (Join-Path $docs $rel)) $path))
        $target = RelPath $full
    }
    if ($titles.ContainsKey($target)) {
        $where = if ($target -eq $rel -and $anchor) { "" } else { "$lq$($titles[$target])$rq" }
        if ($anchor -and $headings[$target].ContainsKey($anchor)) {
            $section = "$lq$($headings[$target][$anchor])$rq"
            $where = if ($where) { "$where $arrow $section" } else { $section }
        }
        if (-not $where) { return $text }
        return "$text (see $where)"
    }
    # A file outside the docs, like an example game: name it by its path in the Forge folder.
    $repoPath = $target -replace '^\.\./', ''
    return "$text (the file ``$repoPath`` in the Forge folder)"
}

$version = ([regex]::Match((Get-Content (Join-Path $docs "..\src\main.cpp") -Raw), 'kVersion = "([^"]+)"')).Groups[1].Value
$sb = [Text.StringBuilder]::new()
[void]$sb.Append(@"
# Forge ${version}: the complete documentation

This one file holds everything written about the Forge programming language, so it can be uploaded in one go:

- **Parts A to F** are the full documentation, page by page: getting started, every part of the language, every library, the reference (every form of the syntax, exactly how Forge behaves, every `frg` command and every error message), the guides (practice projects too), and how Forge works inside.
- **Part G** is the one-page cheat sheet.

Where the parts disagree, the detailed pages (A to F) are more precise, and the compiler is always right. The examples in Part D are run by Forge's tests, so their output is exactly what Forge prints. This file is made from the separate pages in Forge's docs folder by docs/make-all-docs.ps1, so don't edit it by hand.

## Contents


"@.Replace("`r`n", "`n"))
foreach ($part in $parts.Keys) {
    [void]$sb.Append("- **$part**: ")
    [void]$sb.Append((($parts[$part] | ForEach-Object { $titles[$_] -replace '`', '' }) -join " - "))
    [void]$sb.Append("`n")
}

foreach ($part in $parts.Keys) {
    [void]$sb.Append("`n---`n`n# $part`n")
    foreach ($rel in $parts[$part]) {
        $text = Read $rel
        $text = ForEachProseLine $text {
            param($line)
            if ($line -match '^#{1,5} ') { $line = "#" + $line }
            [regex]::Replace($line, '(?<!!)\[([^\]]+)\]\(([^)\s]+)\)', { param($m) LinkToText $rel $m.Groups[1].Value $m.Groups[2].Value })
        }
        [void]$sb.Append("`n<!-- docs/$rel -->`n`n")
        [void]$sb.Append($text.Trim("`n"))
        [void]$sb.Append("`n")
    }
}
$result = $sb.ToString()

if ($Check) {
    $current = if (Test-Path $out) { [IO.File]::ReadAllText($out).Replace("`r`n", "`n") } else { "" }   # git may have changed the line endings
    if ($current -ne $result) {
        Write-Host "docs/FORGE_ALL_DOCS.md is out of date: run .\docs\make-all-docs.ps1" -ForegroundColor Red
        exit 1
    }
    Write-Host "docs/FORGE_ALL_DOCS.md is up to date." -ForegroundColor Green
    exit 0
}

[IO.File]::WriteAllText($out, $result, [Text.UTF8Encoding]::new($false))
$kb = [math]::Round((Get-Item $out).Length / 1KB)
Write-Host "Wrote docs/FORGE_ALL_DOCS.md ($kb KB, $($listed.Count - $skipped.Count + @($extra).Count) pages)."
if ($extra) { Write-Host "Pages not in the list, added at the end: $($extra -join ', ')" -ForegroundColor Yellow }
