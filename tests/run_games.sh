#!/bin/bash
# Runs every built game for a few frames with scripted keys, and saves a screenshot of the last frame.
# Used by CI on Linux (the games are built on Windows, then run here under Wine with a virtual screen
# and software OpenGL), so drawing, input and game logic are checked on every push, not only building.
#
#   tests/run_games.sh <folder with one game per subfolder> <folder for the screenshots> [frames]
#
# A game fails if it crashes, stops with a runtime error, or hangs. Needs wine, and a DISPLAY (xvfb-run).
games=$1
shots=$2
frames=${3:-180}
mkdir -p "$shots"
export WINEDEBUG=-all
failed=()
for dir in "$games"/*/; do
    name=$(basename "$dir")
    exe=$(ls "$dir"build/*.exe 2>/dev/null | head -1)
    if [ -z "$exe" ]; then
        echo "skip  $name (no .exe)"
        continue
    fi
    # Hold right and tap jump/space and click now and then: enough to move, jump and press buttons in most games.
    start=$(date +%s.%N)
    out=$(cd "$dir" && FRG_AUTOTEST_FRAMES=$frames FRG_AUTOTEST_SHOT=autotest.png \
          FRG_AUTOTEST_INPUT="RIGHT:10-150,SPACE:30-32,SPACE:90-92,MOUSE_LEFT:60-61" \
          timeout 180 wine "build/$(basename "$exe")" < /dev/null 2>&1)
    code=$?
    secs=$(printf "%.1f" "$(echo "$(date +%s.%N) - $start" | bc)")
    if [ -f "$dir/autotest.png" ]; then mv "$dir/autotest.png" "$shots/$name.png"; fi
    if [ $code -ne 0 ] || echo "$out" | grep -q "Runtime error"; then
        echo "FAIL  $name (exit code $code, ${secs}s)"
        echo "$out" | tail -15 | sed 's/^/      /'
        failed+=("$name")
    else
        echo "pass  $name (${secs}s)"
    fi
done
if [ ${#failed[@]} -gt 0 ]; then
    echo "Games that failed: ${failed[*]}"
    exit 1
fi
echo "All games ran."
