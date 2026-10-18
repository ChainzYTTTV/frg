# Runs every tests/cases/*.test file against bin/frg.exe (bin/frg on Linux).
#
# A .test file is split into sections by lines like "=== name":
#   === main.frg        a source file (any path, e.g. "=== enemies/goblin.frg")
#   === icon.png (base64)   a binary file, written from the base64 text below it
#   === output          the exact text the program must print
#   === error           text that must appear in the compile errors (one check per line)
#   === build           (empty) the program must compile; it isn't run (for games that open a window)
#   === command         (one line) runs frg with these words instead of building, like "cache" ({game} is this
#                       test's game folder, as in "build {game} --debug"). Each line of
#                       === prints must appear in what it prints; with === error, it must fail with those messages.
#   === query           (one line: "<question> <file> <line> <column> [new name]") runs frg query on this game;
#                       each line of === prints must appear in the JSON it prints (or, starting with !, must not)
#   === new             (one line: a template name, and optionally the game's name) makes a game with
#                       frg new --template, which must build without warnings. With === error, frg new must fail.
#   === package         runs frg package; each line is a path (from the game's folder) that must exist afterwards,
#                       or, starting with !, must not
#   === runtime error   text that must appear when the program stops with a runtime error (one check per line)
#   === warning         text that must appear in the warnings (one check per line); add it to any of the others
#   === no warnings     (empty) there must be no warnings at all
#   === windows only    (empty) only run on Windows. Tests of the low-level toolkits (<OpenGL>, <DirectX>, <Vulkan>)
#                       are Windows-only without it.
#   === window          runs a game that opens a window, with these lines (both optional):
#                           frames: 60              how many frames it runs (FRG_AUTOTEST_FRAMES; 60 if not given)
#                           input: SPACE:1-2,CLOSE:5-5   scripted keys (FRG_AUTOTEST_INPUT)
#                           cwd: build              start it from that folder inside the game's folder
#                       Then === output is exactly what it prints, === runtime error lines must be in its errors,
#                       and === exit code (one line) is the code it must end with (0 if not given).
#                       FRG_NO_WINDOW=1 skips these (a computer without a screen). FRG_COLLECT_WINDOW_TESTS=<folder>
#                       builds them and copies each game there instead of running it, for tests/run_window_tests.sh.
#
# Needs PowerShell 7 (pwsh), not the older Windows PowerShell 5. On Linux, the docs' examples aren't checked (they
# show Windows file names, like mygame.exe).
#
# Usage:  .\tests\run_tests.ps1            run everything
#         .\tests\run_tests.ps1 strings    run only strings.test
param([string]$Filter = "*")

$root = Split-Path -Parent $PSScriptRoot
$exe = if ($IsWindows) { ".exe" } else { "" }   # what a program's file name ends in
$frg = Join-Path $root "bin" "frg$exe"
$work = Join-Path ([IO.Path]::GetTempPath()) "forge-tests"
[Console]::OutputEncoding = [Text.Encoding]::UTF8
# the tests never look for a newer Forge (no network, and no update notes in the output)
$env:FRG_NO_UPDATE_CHECK = "1"

function Invoke-Frg([string]$command, [string]$dir, [string[]]$extra = @()) {
    $psi = [Diagnostics.ProcessStartInfo]::new($frg)
    $psi.ArgumentList.Add($command)
    if ($dir) { $psi.ArgumentList.Add($dir) }
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

# Runs a built game with scripted frames and input; gives back its exit code, output and errors.
function Invoke-Game([string]$exe, [string]$dir, [hashtable]$env) {
    $psi = [Diagnostics.ProcessStartInfo]::new($exe)
    $psi.WorkingDirectory = $dir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = [Text.Encoding]::UTF8
    $psi.StandardErrorEncoding = [Text.Encoding]::UTF8
    foreach ($k in $env.Keys) { $psi.Environment[$k] = $env[$k] }
    $p = [Diagnostics.Process]::Start($psi)
    $errTask = $p.StandardError.ReadToEndAsync()
    $out = $p.StandardOutput.ReadToEnd()
    if (-not $p.WaitForExit(60000)) { $p.Kill(); return [pscustomobject]@{ Code = -999; Out = $out; Err = "it was still running after a minute" } }
    [pscustomobject]@{ Code = $p.ExitCode; Out = ($out -replace "`r`n", "`n"); Err = ($errTask.Result -replace "`r`n", "`n") }
}

$pass = 0
$skipped = 0
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
        if ($key -in 'output', 'error', 'build', 'warning', 'no warnings', 'runtime error', 'package', 'new', 'command', 'prints', 'query', 'window', 'exit code', 'windows only') { continue }
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

    if (-not $IsWindows) {
        $toolkit = @($sections.Values | Where-Object { ($_ -join "`n") -match '#include\s*<(OpenGL|DirectX|Vulkan)>' }).Count
        if ($sections.Contains('windows only') -or $toolkit) { $skipped++; continue }
    }

    $problem = $null
    if ($sections.Contains('query')) {
        # "=== query" is one line, "<question> <file> <line> <column> [new name]": frg query on this test's game.
        # Every line of "=== prints" must be in the JSON it prints.
        $words = @($sections['query'][0] -split ' ')
        $extra = @($words[0], $dir, (Join-Path $dir $words[1])) + @($words | Select-Object -Skip 2)
        $r = Invoke-Frg query '' $extra
        foreach ($want in $sections['prints']) {
            if ($want.StartsWith('!')) {
                if ($r.Out.Contains($want.Substring(1))) { $problem = "the query's answer shouldn't contain: $($want.Substring(1))`n--- it printed:`n$($r.Out)" }
            } elseif ($want -and -not $r.Out.Contains($want)) { $problem = "the query's answer doesn't contain: $want`n--- it printed:`n$($r.Out)$($r.Err)" }
        }
    } elseif ($sections.Contains('command')) {
        $words = @(($sections['command'][0] -replace '\{game\}', $dir) -split ' ')
        $r = Invoke-Frg $words[0] '' @($words | Select-Object -Skip 1)
        $all = $r.Out + $r.Err
        if ($sections.Contains('error')) {
            if ($r.Code -eq 0) { $problem = "expected frg $($sections['command'][0]) to fail, but it worked" }
            foreach ($want in $sections['error']) {
                if ($want -and -not $all.Contains($want)) { $problem = "errors don't contain: $want`n--- it printed:`n$all" }
            }
        } else {
            if ($r.Code -ne 0) { $problem = "frg $($sections['command'][0]) failed:`n$all" }
            foreach ($want in $sections['prints']) {
                if ($want -and -not $all.Contains($want)) { $problem = "it didn't print: $want`n--- it printed:`n$all" }
            }
        }
    } elseif ($sections.Contains('new')) {
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
    } elseif ($sections.Contains('window')) {
        $settings = @{ frames = "60"; input = ""; cwd = "" }
        foreach ($l in $sections['window']) { if ($l -match '^\s*(frames|input|cwd):\s*(.*)$') { $settings[$Matches[1]] = $Matches[2].Trim() } }
        $wantCode = if ($sections.Contains('exit code')) { [int]$sections['exit code'][0] } else { 0 }
        if ($env:FRG_NO_WINDOW -and -not $env:FRG_COLLECT_WINDOW_TESTS) { $skipped++; continue }
        $r = Invoke-Frg build $dir
        if ($r.Code -ne 0) { $problem = "expected it to build, but:`n$($r.Err)" }
        elseif ($env:FRG_COLLECT_WINDOW_TESTS) {
            # Kept for tests/run_window_tests.sh: the game, and what it should do.
            $to = Join-Path $env:FRG_COLLECT_WINDOW_TESTS $name
            New-Item -ItemType Directory -Force $to | Out-Null
            Get-ChildItem $dir -Exclude build | Copy-Item -Destination $to -Recurse -Force
            Copy-Item (Join-Path $dir "build" "$name$exe") (Join-Path $to "game.exe")
            [IO.File]::WriteAllText((Join-Path $to "frames.txt"), $settings.frames)
            [IO.File]::WriteAllText((Join-Path $to "input.txt"), $settings.input)
            [IO.File]::WriteAllText((Join-Path $to "exit-code.txt"), "$wantCode")
            if ($sections.Contains('output')) { [IO.File]::WriteAllText((Join-Path $to "expected-output.txt"), (($sections['output'] | ForEach-Object { $_.TrimEnd() }) -join "`n") + "`n") }
            if ($sections.Contains('runtime error')) { [IO.File]::WriteAllText((Join-Path $to "expected-errors.txt"), ($sections['runtime error'] -join "`n") + "`n") }
            $skipped++
            continue
        } else {
            $env2 = @{ FRG_AUTOTEST_FRAMES = $settings.frames; FRG_AUTOTEST_HIDDEN = "1" }
            if ($settings.input) { $env2["FRG_AUTOTEST_INPUT"] = $settings.input }
            $from = if ($settings.cwd) { Join-Path $dir $settings.cwd } else { $dir }   # cwd: build starts it from another folder
            $g = Invoke-Game (Join-Path $dir "build" "$name$exe") $from $env2
            if ($g.Code -ne $wantCode) { $problem = "expected exit code $wantCode, but it was $($g.Code)`n$($g.Out)$($g.Err)" }
            if (-not $problem -and $sections.Contains('output')) {
                $expected = (($sections['output'] | ForEach-Object { $_.TrimEnd() }) -join "`n") + "`n"
                $got = (($g.Out -split "`n") | ForEach-Object { $_.TrimEnd() }) -join "`n"
                if ($got -ne $expected) { $problem = "--- expected:`n$expected--- got:`n$($g.Out)$($g.Err)" }
            }
            if (-not $problem -and $sections.Contains('runtime error')) {
                foreach ($want in $sections['runtime error']) {
                    if ($want -and -not $g.Err.Contains($want)) { $problem = "the runtime error doesn't contain: $want`n--- it printed:`n$($g.Err)" }
                }
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
        if ($env:GITHUB_ACTIONS) {
            # An annotation on the run's page: readable by anyone, unlike the log.
            $text = ($problem -split "`n" | Select-Object -First 12) -join '%0A'
            Write-Host "::error file=tests/cases/$name.test,title=$name::$text"
        }
        $failed += $name
    } else {
        Write-Host "pass  $name" -ForegroundColor Green
        $pass++
    }
}

# The examples in the docs (<!-- check: ... --> blocks) have to do what the docs say. (Windows only: they show
# Windows' file names.)
if ($IsWindows) {
    $docExamples = & pwsh -NoProfile -File (Join-Path $PSScriptRoot "check_doc_examples.ps1") 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL  doc examples" -ForegroundColor Red
        Write-Host (($docExamples | Where-Object { $_ -notmatch '^\s*$' }) -join "`n" -replace '(?m)^', '      ')
        $failed += "doc examples"
    } else {
        Write-Host "pass  doc examples ($(($docExamples | Select-Object -Last 1) -replace '^Doc examples: ', ''))" -ForegroundColor Green
        $pass++
    }

    # The combined docs file (docs/FORGE_ALL_DOCS.md) has to match the docs pages it's made from.
    $docsCheck = & pwsh -NoProfile -File (Join-Path $PSScriptRoot "..\docs\make-all-docs.ps1") -Check 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL  combined docs" -ForegroundColor Red
        Write-Host ($docsCheck -join "`n" -replace '(?m)^', '      ')
        $failed += "combined docs"
    } else {
        Write-Host "pass  combined docs" -ForegroundColor Green
        $pass++
    }
}

Write-Host ""
if ($failed.Count) {
    Write-Host "$pass passed, $($failed.Count) failed: $($failed -join ', ')" -ForegroundColor Red
    exit 1
}
$why = if ($IsWindows) { "that open a window" } else { "that open a window or only work on Windows" }
$note = if ($skipped) { " ($skipped $why were skipped)" } else { "" }
Write-Host "All $pass tests passed$note." -ForegroundColor Green
