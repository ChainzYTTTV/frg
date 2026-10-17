#!/bin/bash
# Runs the window tests (tests/cases/window_*.test) that run_tests.ps1 built and collected with
# FRG_COLLECT_WINDOW_TESTS=<folder>. Used by CI on Linux, under Wine with a virtual screen, because GitHub's
# Windows machines can't open a window.
#
#   tests/run_window_tests.sh <the collected folder>
#
# Each test folder has game.exe, frames.txt, input.txt, exit-code.txt, and expected-output.txt and/or
# expected-errors.txt. A test passes if the game ends with that exit code, prints exactly the expected output,
# and shows every line of the expected errors.
tests=$1
export WINEDEBUG=-all
failed=()
for dir in "$tests"/*/; do
    name=$(basename "$dir")
    frames=$(cat "$dir/frames.txt")
    input=$(cat "$dir/input.txt")
    want=$(cat "$dir/exit-code.txt")
    out=$(cd "$dir" && FRG_AUTOTEST_FRAMES=$frames FRG_AUTOTEST_INPUT="$input" timeout 120 wine game.exe 2>stderr.txt < /dev/null)
    code=$?
    out=$(printf '%s' "$out" | tr -d '\r')
    errs=$(tr -d '\r' < "$dir/stderr.txt")
    problem=""
    if [ "$code" != "$want" ]; then
        problem="expected exit code $want, but it was $code"
    elif [ -f "$dir/expected-output.txt" ]; then
        expected=$(sed 's/[[:space:]]*$//' "$dir/expected-output.txt")
        got=$(printf '%s\n' "$out" | sed 's/[[:space:]]*$//')
        if [ "$got" != "$expected" ]; then
            problem=$(printf -- "--- expected:\n%s\n--- got:\n%s" "$expected" "$got")
        fi
    fi
    if [ -z "$problem" ] && [ -f "$dir/expected-errors.txt" ]; then
        while IFS= read -r line; do
            line=$(printf '%s' "$line" | tr -d '\r')
            [ -z "$line" ] && continue
            if ! printf '%s' "$errs" | grep -qF -- "$line"; then
                problem="the errors don't contain: $line"
                break
            fi
        done < "$dir/expected-errors.txt"
    fi
    if [ -n "$problem" ]; then
        echo "FAIL  $name"
        printf '%s\n%s\n' "$problem" "$errs" | sed 's/^/      /'
        failed+=("$name")
    else
        echo "pass  $name"
    fi
done
if [ ${#failed[@]} -gt 0 ]; then
    echo "Window tests that failed: ${failed[*]}"
    exit 1
fi
echo "All window tests passed."
