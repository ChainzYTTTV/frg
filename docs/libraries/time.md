# `<time>`: Clock, Date and Timers

```cpp
#include <time>
```

## Functions

| Function | Gives back | What it is |
|---|---|---|
| `time.now()` | `double` | seconds since the game started (real time) |
| `time.sleep(float seconds)` | — | stops **everything** for that long |
| `time.clock()` | `string` | the time of day, like `"14:05:09"` |
| `time.date()` | `string` | today's date, like `"2026-10-14"` |
| `time.year()` | `int` | for example `2026` |
| `time.month()` | `int` | 1 (January) to 12 |
| `time.day()` | `int` | day of the month, 1–31 |
| `time.hour()` | `int` | 0–23 |
| `time.minute()` | `int` | 0–59 |
| `time.second()` | `int` | 0–59 |

The clock and date are the computer's local time.

**Don't use `time.sleep` in a windowed game.** It freezes the whole game: nothing draws and input stops. To wait inside game logic, use a [task](../language/11-tasks.md) with `wait(seconds)`, or a `Timer`.

## Timer

A countdown, for cooldowns, spawn intervals, power-up durations and so on.

```cpp
Timer cooldown = Timer(0.5);        // half a second

void update(float dt){
    if (input.keyDown(Key.SPACE) and cooldown.isDone()) {
        shoot();
        cooldown.restart();
    }
}
```

| Making one | |
|---|---|
| `Timer(float seconds)` | starts counting straight away |

| Method | Gives back | What it does |
|---|---|---|
| `isDone()` | `bool` | true once the time has run out |
| `left()` | `float` | seconds remaining (0 when done) |
| `elapsed()` | `float` | seconds since it started or was restarted |
| `restart()` | — | starts counting again from now |

A Timer measures **real** time, so it keeps counting while the game is paused. For game-time effects that should pause with the game, use a [task](../language/11-tasks.md) or count `dt` yourself.

Showing a countdown:

```cpp
Timer roundTimer = Timer(60);
graphics2d.drawText("{int(roundTimer.left())}", 20, 20, 32, color.WHITE);
```
