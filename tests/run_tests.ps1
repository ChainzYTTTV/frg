# Runs every tests/cases/*.test file against bin/frg.exe.
#
# A .test file is split into sections by lines like "=== name":
#   === main.frg        a source file (any path, e.g. "=== enemies/goblin.frg")
#   === output          the exact text the program must print
#   === error           text that must appear in the compile errors (one check per line)
#
# Usage:  .\tests\run_tests.ps1            run everything
#         .\tests\run_tests.ps1 strings    run only strings.test
param([string]$Filter = "*")

$root = Split-Path -Parent $PSScriptRoot
$frg = Join-Path $root "bin\frg.exe"
$work = Join-Path ([IO.Path]::GetTempPath()) "forge-tests"
[Console]::OutputEncoding = [Text.Encoding]::UTF8

function Invoke-Frg([string]$command, [string]$dir) {
    $psi = [Diagnostics.ProcessStartInfo]::new($frg)
    $psi.ArgumentList.Add($command)
    $psi.ArgumentList.Add($dir)
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
        if ($key -in 'output', 'error') { continue }
        $path = Join-Path $dir $key
        New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
        [IO.File]::WriteAllText($path, ($lines -join "`n") + "`n")
    }

    $problem = $null
    if ($sections.Contains('error')) {
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
