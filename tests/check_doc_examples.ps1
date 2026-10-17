# Checks the examples in the docs against the real compiler, so what the docs show is what Forge does.
#
# In a docs page, a line like
#
#     <!-- check: error -->
#
# says that the code blocks right after it (```cpp or ~~~cpp, only blank lines between them) are a game, and the ```text block
# after those shows what happens. The kinds of check:
#
#   error           the game must fail to build; every line of the text block must be in the errors
#   warning         the game must build; every line of the text block must be in the warnings
#   output          the game must build and run; the text block is exactly what it prints
#   runtime-error   the game must build, then stop with an error; every line of the text block must be in what it shows
#   exit N          (after output or runtime-error) the game must end with exit code N, like: <!-- check: output exit 3 -->
#   builds          the game must build without errors (no text block)
#   command ARGS    runs "frg ARGS" ({game} is the game's folder, if there's a code block); every line of the text
#                   block must be in what it prints, and with "fails" after it, it must fail: <!-- check: command fails new -->
#
# A code block whose first line is "// file: enemies/goblin.frg" is that file; otherwise it's main.frg.
# The game's folder is called mygame, so its .exe is mygame.exe.
# A game that opens a window can be run for a number of frames with scripted input:
#     <!-- check: output frames=20 input=CLOSE:9-9 -->
# (FRG_AUTOTEST_FRAMES and FRG_AUTOTEST_INPUT). Set FRG_NO_WINDOW=1 to skip those, on a computer without a screen.
#
# Usage:  .\tests\check_doc_examples.ps1            check every page in docs/
#         .\tests\check_doc_examples.ps1 -Update    fill each text block that only says FILL with what Forge really
#                                                    prints (then read what it wrote!)
#         .\tests\check_doc_examples.ps1 -Page docs/reference/error-messages.md
param([switch]$Update, [string]$Page = "")

$root = Split-Path -Parent $PSScriptRoot
$frg = Join-Path $root "bin\frg.exe"
$work = Join-Path ([IO.Path]::GetTempPath()) "forge-doc-checks"
[Console]::OutputEncoding = [Text.Encoding]::UTF8

# Reads every check in a page: its line numbers, kind, files and the text block.
function Get-Checks([string]$path) {
    $lines = [IO.File]::ReadAllLines($path)
    $checks = @()
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -notmatch '^<!-- check: (.+?) -->\s*$') { continue }
        $spec = $Matches[1]
        $check = [ordered]@{ Spec = $spec; Line = $i + 1; Files = [ordered]@{}; Text = $null; TextStart = -1; TextEnd = -1 }
        $j = $i + 1
        while ($j -lt $lines.Count) {
            if ($lines[$j].Trim() -eq '') { $j++; continue }
            if ($lines[$j] -match '^(```|~~~)(cpp|frg)\s*$') {
                # ~~~ fences are for code that has ``` in it (a comment).
                $fence = $Matches[1]
                $body = [Collections.Generic.List[string]]::new()
                $j++
                while ($j -lt $lines.Count -and $lines[$j] -notmatch ('^' + $fence + '\s*$')) { $body.Add($lines[$j]); $j++ }
                $j++
                $file = "main.frg"
                if ($body.Count -and $body[0] -match '^// file: (\S+)') { $file = $Matches[1] }
                $check.Files[$file] = ($body -join "`n") + "`n"
                continue
            }
            if ($lines[$j] -match '^```text\s*$') {
                $check.TextStart = $j + 1
                $body = [Collections.Generic.List[string]]::new()
                $j++
                while ($j -lt $lines.Count -and $lines[$j] -notmatch '^```\s*$') { $body.Add($lines[$j]); $j++ }
                $check.TextEnd = $j - 1
                $check.Text = @($body)
            }
            break
        }
        $checks += [pscustomobject]$check
    }
    return $checks
}

function Invoke-Process([string]$exe, [string[]]$arguments, [string]$dir, [hashtable]$env = @{}) {
    $psi = [Diagnostics.ProcessStartInfo]::new($exe)
    foreach ($a in $arguments) { $psi.ArgumentList.Add($a) }
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
    if (-not $p.WaitForExit(120000)) { $p.Kill(); return [pscustomobject]@{ Code = -999; Out = $out; Err = "(it was still running after 2 minutes)" } }
    [pscustomobject]@{ Code = $p.ExitCode; Out = ($out -replace "`r`n", "`n"); Err = ($errTask.Result -replace "`r`n", "`n") }
}

# The lines of build output that are about the compiler's progress, not the game.
function Remove-Progress([string]$text) {
    (($text -split "`n") | Where-Object { $_ -notmatch '^\s+(Compiling|Finished|Preparing|Running|Testing)\s' -and $_ -notmatch '^Build failed' }) -join "`n"
}

# Runs one check. Gives back $null if it's right, or what went wrong; with -Update, also what the text block should be.
function Test-Check($check, [string]$dir) {
    $words = @($check.Spec -split '\s+')
    $kind = $words[0]
    $settings = @{}
    $exit = $null
    $fails = $false
    $commandArgs = @()
    for ($w = 1; $w -lt $words.Count; $w++) {
        if ($words[$w] -match '^(frames|input)=(.+)$') { $settings[$Matches[1]] = $Matches[2] }
        elseif ($kind -ne 'command' -and $words[$w] -eq 'exit') { $exit = [int]$words[++$w] }
        elseif ($kind -eq 'command' -and $w -eq 1 -and $words[$w] -eq 'fails') { $fails = $true }
        elseif ($kind -eq 'command') { $commandArgs += $words[$w] }
    }
    if ($settings.Count -and $env:FRG_NO_WINDOW) { return @{ Skipped = $true } }
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
    New-Item -ItemType Directory -Force $dir | Out-Null
    foreach ($f in $check.Files.Keys) {
        $path = Join-Path $dir $f
        New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
        [IO.File]::WriteAllText($path, $check.Files[$f])
    }
    $want = @($check.Text | Where-Object { $_ -and $_.Trim() -ne '' } | ForEach-Object { $_.Trim() })
    $shown = $null   # what Forge really printed, for -Update
    $problem = $null

    if ($kind -eq 'command') {
        $cmdArgs = @($commandArgs | ForEach-Object { $_ -replace '\{game\}', $dir })
        $r = Invoke-Process $frg $cmdArgs $dir
        $shown = ($r.Out + $r.Err).TrimEnd()
        if ($fails -and $r.Code -eq 0) { $problem = "expected frg $($commandArgs -join ' ') to fail, but it worked" }
        if (-not $fails -and $r.Code -ne 0) { $problem = "frg $($commandArgs -join ' ') failed:`n$shown" }
    } else {
        $r = Invoke-Process $frg @('build', $dir) $dir
        $buildText = (Remove-Progress $r.Err).Trim()
        if ($kind -eq 'error') {
            $shown = $buildText
            if ($r.Code -eq 0) { $problem = "expected a compile error, but it built" }
        } elseif ($kind -eq 'warning') {
            $shown = $buildText
            if ($r.Code -ne 0) { $problem = "expected it to build, but:`n$($r.Err)" }
            elseif (-not $r.Err.Contains('warning:')) { $problem = "expected a warning, but there was none" }
        } elseif ($r.Code -ne 0) {
            $problem = "expected it to build, but:`n$($r.Err)"
        } elseif ($kind -eq 'builds') {
            if ($r.Err.Contains('warning:')) { $problem = "it built, but with warnings:`n$buildText" }
        } elseif ($kind -in 'output', 'runtime-error') {
            $exe = Get-ChildItem (Join-Path $dir "build") -Filter *.exe | Select-Object -First 1
            $env2 = @{ FRG_AUTOTEST_HIDDEN = "1" }   # no windows popping up while checking
            if ($settings.frames) { $env2["FRG_AUTOTEST_FRAMES"] = $settings.frames }
            if ($settings.input) { $env2["FRG_AUTOTEST_INPUT"] = $settings.input }
            $run = Invoke-Process $exe.FullName @() $dir $env2
            if ($kind -eq 'output') {
                $shown = (($run.Out -split "`n") | ForEach-Object { $_.TrimEnd() }) -join "`n"
                $shown = $shown.TrimEnd()
                $expected = (($check.Text | ForEach-Object { $_.TrimEnd() }) -join "`n").TrimEnd()
                if ($null -eq $exit -and $run.Code -ne 0) { $problem = "it ended with exit code $($run.Code):`n$($run.Err)" }
                elseif ($shown -ne $expected) { $problem = "--- the page shows:`n$expected`n--- it printed:`n$shown" }
            } else {
                $shown = $run.Err.Trim()
                if ($run.Code -eq 0) { $problem = "expected a runtime error, but it ran without one`n$($run.Out)" }
            }
            if (-not $problem -and $null -ne $exit -and $run.Code -ne $exit) { $problem = "expected exit code $exit, but it was $($run.Code)" }
        } else {
            $problem = "unknown kind of check '$kind'"
        }
    }
    # The game's real folder is shown as D:\games\mygame in the docs.
    if ($null -ne $shown) { $shown = $shown.Replace($dir, 'D:\games\mygame') }
    if (-not $problem -and $kind -ne 'output' -and $kind -ne 'builds') {
        foreach ($line in $want) {
            if (-not $shown.Contains($line)) { $problem = "this line isn't in what Forge printed: $line`n--- it printed:`n$shown"; break }
        }
    }
    return @{ Problem = $problem; Shown = $shown }
}

$pages = if ($Page) { @(Resolve-Path $Page) } else { Get-ChildItem (Join-Path $root "docs") -Recurse -Filter *.md | Where-Object { $_.Name -ne 'FORGE_ALL_DOCS.md' } }
$pass = 0
$skipped = 0
$failed = @()
foreach ($pagePath in $pages) {
    $path = "$pagePath"
    $checks = @(Get-Checks $path)
    if (-not $checks.Count) { continue }
    $rel = [IO.Path]::GetRelativePath($root, $path) -replace '\\', '/'
    $lines = [Collections.Generic.List[string]]([IO.File]::ReadAllLines($path))
    $edits = @()
    for ($c = 0; $c -lt $checks.Count; $c++) {
        $check = $checks[$c]
        $name = "$rel line $($check.Line)"
        # Every game is in a folder called mygame, so the docs can show "mygame.exe" like in the examples.
        $result = Test-Check $check (Join-Path (Join-Path $work ("{0}-{1}" -f ($rel -replace '[/.]', '_'), $check.Line)) "mygame")
        if ($result.Skipped) { $skipped++; continue }
        $fill = $check.TextStart -ge 0 -and ($check.Text -join "`n").Trim() -eq 'FILL'
        if ($Update -and $fill -and $null -ne $result.Shown) { $edits += @{ Check = $check; Shown = $result.Shown } }
        if ($result.Problem -and -not ($Update -and $fill -and $result.Problem -match '^(this line isn''t in|--- the page shows)')) {
            Write-Host "FAIL  $name ($($check.Spec))" -ForegroundColor Red
            Write-Host ($result.Problem -replace '(?m)^', '      ')
            $failed += $name
        } else {
            $pass++
        }
    }
    if ($Update -and $edits.Count) {
        # From the bottom up, so earlier line numbers stay right.
        foreach ($e in ($edits | Sort-Object { $_.Check.TextStart } -Descending)) {
            $count = $e.Check.TextEnd - $e.Check.TextStart + 1
            if ($count -gt 0) { $lines.RemoveRange($e.Check.TextStart, $count) }
            $lines.InsertRange($e.Check.TextStart, [string[]]@($e.Shown -split "`n"))
        }
        [IO.File]::WriteAllText($path, ($lines -join "`n") + "`n")
        Write-Host "updated $rel"
    }
}
Write-Host ""
$note = if ($skipped) { " ($skipped that open a window were skipped)" } else { "" }
if ($failed.Count) {
    Write-Host "Doc examples: $pass passed, $($failed.Count) failed$note" -ForegroundColor Red
    exit 1
}
Write-Host "Doc examples: all $pass passed$note" -ForegroundColor Green
