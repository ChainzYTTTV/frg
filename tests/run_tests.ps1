# Runs every tests/cases/*.test file against bin/frg.exe.
#
# A .test file is split into sections by lines like "=== name":
#   === main.frg        a source file (any path, e.g. "=== enemies/goblin.frg")
#   === icon.png (base64)   a binary file, written from the base64 text below it
#   === output          the exact text the program must print
#   === error           text that must appear in the compile errors (one check per line)
#   === build           (empty) the program must compile; it isn't run (for games that open a window)
#   === new             (one line: a template name, and optionally the game's name) makes a game with
#                       frg new --template, which must build without warnings. With === error, frg new must fail.
#   === package         runs frg package; each line is a path (from the game's folder) that must exist afterwards,
#                       or, starting with !, must not
#   === runtime error   text that must appear when the program stops with a runtime error (one check per line)
#   === warning         text that must appear in the warnings (one check per line); add it to any of the others
#   === no warnings     (empty) there must be no warnings at all
#
# Needs PowerShell 7 (pwsh), not the older Windows PowerShell 5.
#
# Usage:  .\tests\run_tests.ps1            run everything
#         .\tests\run_tests.ps1 strings    run only strings.test
param([string]$Filter = "*")

$root = Split-Path -Parent $PSScriptRoot
$frg = Join-Path $root "bin\frg.exe"
$work = Join-Path ([IO.Path]::GetTempPath()) "forge-tests"
[Console]::OutputEncoding = [Text.Encoding]::UTF8

function Invoke-Frg([string]$command, [string]$dir, [string[]]$extra = @()) {
    $psi = [Diagnostics.ProcessStartInfo]::new($frg)
    $psi.ArgumentList.Add($command)
    $psi.ArgumentList.Add($dir)
    foreach ($a in $extra) { $psi.ArgumentList.Add($a) }
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = [Text.Encoding]::UTF8
    $psi.StandardErrorEncoding = [Text.Encoding]::UTF8
    $p = [Diagnostics.Process]::Start($psi)
    $errTask = $p.StandardError.ReadToEndAsync()
    $out = $p.StandardOutput.ReadToEnd()
    $p.WaitForExit()
    [pscustomobject]@{
        Code = $p.ExitCode
        Out  = $out -replace "`r`n", "`n"
        Err  = $errTask.Result -replace "`r`n", "`n"
    }
}

$pass = 0
$failed = @()
foreach ($file in Get-ChildItem (Join-Path $PSScriptRoot "cases") -Filter "$Filter.test" | Sort-Object Name) {
    $name = $file.BaseName
    $dir = Join-Path $work $name
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
    New-Item -ItemType Directory -Force $dir | Out-Null

    $sections = [ordered]@{}
    $current = $null
    foreach ($line in [IO.File]::ReadAllLines($file.FullName)) {
        if ($line -match '^=== (.+)$') {
            $current = $Matches[1].Trim()
            $sections[$current] = [Collections.Generic.List[string]]::new()
        } elseif ($current) {
            $sections[$current].Add($line)
        }
    }
    foreach ($key in @($sections.Keys)) {
        $lines = $sections[$key]
        while ($lines.Count -gt 0 -and $lines[$lines.Count - 1] -eq '') { $lines.RemoveAt($lines.Count - 1) }
        if ($key -in 'output', 'error', 'build', 'warning', 'no warnings', 'runtime error', 'package', 'new') { continue }
        if ($key -match '^(.+) \(base64\)$') {
            $path = Join-Path $dir $Matches[1]
            New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
            [IO.File]::WriteAllBytes($path, [Convert]::FromBase64String(($lines -join '')))
            continue
        }
        $path = Join-Path $dir $key
        New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
        [IO.File]::WriteAllText($path, ($lines -join "`n") + "`n")
    }

    $problem = $null
    if ($sections.Contains('new')) {
        $words = $sections['new'][0] -split ' ', 2
        $game = Join-Path $dir ($(if ($words.Count -gt 1) { $words[1] } else { 'game' }))
        $r = Invoke-Frg new $game @('--template', $words[0])
        if ($sections.Contains('error')) {
            if ($r.Code -eq 0) { $problem = "expected frg new to fail, but it worked" }
            foreach ($want in $sections['error']) {
                if ($want -and -not $r.Err.Contains($want)) { $problem = "errors don't contain: $want`n--- errors were:`n$($r.Err)" }
            }
        }
        elseif ($r.Code -ne 0) { $problem = "frg new failed:`n$($r.Err)" }
        else {
            $r = Invoke-Frg build $game
            if ($r.Code -ne 0) { $problem = "the new game doesn't build:`n$($r.Err)" }
            elseif ($r.Err.Contains('warning:')) { $problem = "the new game has warnings:`n$($r.Err)" }
        }
    } elseif ($sections.Contains('package')) {
        $r = Invoke-Frg package $dir
        if ($r.Code -ne 0) { $problem = "expected it to package, but:`n$($r.Err)" }
        else {
            foreach ($want in $sections['package']) {
                if (-not $want) { continue }
                $absent = $want.StartsWith('!')
                $path = Join-Path $dir $want.TrimStart('!')
                if ($absent -and (Test-Path $path)) { $problem = "$($want.TrimStart('!')) shouldn't be there after frg package" }
                if (-not $absent -and -not (Test-Path $path)) { $problem = "$want is missing after frg package`n$($r.Err)" }
            }
        }
    } elseif ($sections.Contains('runtime error')) {
        $r = Invoke-Frg run $dir
        if ($r.Code -eq 0) { $problem = "expected a runtime error, but it ran without one`n$($r.Out)" }
        foreach ($want in $sections['runtime error']) {
            if ($want -and -not $r.Err.Contains($want)) { $problem = "the runtime error doesn't contain: $want`n--- it printed:`n$($r.Err)" }
        }
    } elseif ($sections.Contains('build') -or (-not $sections.Contains('error') -and -not $sections.Contains('output'))) {
        $r = Invoke-Frg build $dir
        if ($r.Code -ne 0) { $problem = "expected it to build, but:`n$($r.Err)" }
    } elseif ($sections.Contains('error')) {
        $r = Invoke-Frg build $dir
        if ($r.Code -eq 0) { $problem = "expected a compile error, but it built" }
        foreach ($want in $sections['error']) {
            if ($want -and -not $r.Err.Contains($want)) { $problem = "errors don't contain: $want`n--- errors were:`n$($r.Err)" }
        }
    } else {
        $r = Invoke-Frg run $dir
        # Trailing spaces at the ends of lines are ignored (editors tend to strip them from .test files).
        $expected = (($sections['output'] | ForEach-Object { $_.TrimEnd() }) -join "`n") + "`n"
        $got = (($r.Out -split "`n") | ForEach-Object { $_.TrimEnd() }) -join "`n"
        if ($r.Code -ne 0) { $problem = "exit code $($r.Code)`n$($r.Err)" }
        elseif ($got -ne $expected) { $problem = "--- expected:`n$expected--- got:`n$($r.Out)" }
    }
    if (-not $problem -and $sections.Contains('warning')) {
        if (-not $r.Err.Contains('warning:')) { $problem = "expected warnings, but there were none`n$($r.Err)" }
        foreach ($want in $sections['warning']) {
            if ($want -and -not $r.Err.Contains($want)) { $problem = "warnings don't contain: $want`n--- warnings were:`n$($r.Err)" }
        }
    }
    if (-not $problem -and $sections.Contains('no warnings') -and $r.Err.Contains('warning:')) {
        $problem = "expected no warnings, but got:`n$($r.Err)"
    }

    if ($problem) {
        Write-Host "FAIL  $name" -ForegroundColor Red
        Write-Host ($problem -replace '(?m)^', '      ')
        $failed += $name
    } else {
        Write-Host "pass  $name" -ForegroundColor Green
        $pass++
    }
}

Write-Host ""
if ($failed.Count) {
    Write-Host "$pass passed, $($failed.Count) failed: $($failed -join ', ')" -ForegroundColor Red
    exit 1
}
Write-Host "All $pass tests passed." -ForegroundColor Green
