# 11. Tasks

Games are full of things that happen **over time**: flash for half a second, wait 3 seconds and then spawn a wave, count down 3, 2, 1. Writing these with timers and frame counters gets messy fast. **Tasks** let you write them top to bottom, as if the game paused, without actually pausing the game.

## A first task

```cpp
task flash(){
    tint = color.WHITE;
    wait(0.2);              // this task pauses for 0.2 seconds...
    tint = color.ORANGE;    // ...then carries on from here
}
```

A function whose return type is **`task`** can call **`wait(seconds)`**. Only that task pauses; every other part of the game (drawing, input, other tasks) keeps running.

**Calling a task starts it.** It runs straight away until its first `wait`, then the call returns and your code carries on. The task picks up where it left off once the time has passed:

```cpp
void respawn(){
    position = START;
    flash();                // starts flashing, and returns immediately
    print("respawned");     // runs right away, while the flash is still going
}
```

## wait

| Call | Pauses the task until |
|---|---|
| `wait(seconds)` | that much game time has passed (`wait(0.5)` is half a second) |
| `wait()` | the next frame |
| `wait(otherTask)` | `otherTask` has finished |

`wait` can only be used inside a `task` function; anywhere else it's an error that suggests changing the function's return type to `task`.

## Keeping hold of a task

Calling a task gives back a `task` value. Keep it if you want to check on it or stop it:

```cpp
task t = countdown();

if (t.isDone()) { ... }     // has it finished?
t.stop();                   // stop it early: it never continues
```

| Method | What it does |
|---|---|
| `isDone()` | `true` once the task has finished (or was stopped) |
| `stop()` | stops the task; it won't continue from its `wait` |

A `task` variable made without a value counts as already done. `print(t)` shows `task (running)` or `task (done)`.

## Examples

### Countdown

```cpp
task countdown(){
    for (int i = 3; i > 0; i--) {
        message = "{i}";
        wait(1);
    }
    message = "GO!";
    wait(1);
    message = "";
}
```

### Spawning waves forever

```cpp
task spawnWaves(){
    int wave = 1;
    while (true) {
        for (int i = 0; i < wave * 3; i++) {
            spawnEnemy();
            wait(0.4);
        }
        wave++;
        wait(10);
    }
}
```

A task that never ends is fine; it just keeps going alongside the game. Keep the `task` value if you might want to `stop()` it later.

### One task waiting for another

```cpp
task bossIntro(){
    task text = showText("A wild boss appears!");
    wait(text);                 // carry on when showText has finished
    bossActive = true;
}
```

### Tasks in a class

Methods of a **class** can be tasks. The object stays alive while its task runs, even if nothing else holds on to it:

```cpp
class Player : Body {
    color tint = color.ORANGE;

    task flash(){
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
}
```

Struct methods can't be tasks, because a struct gets copied around, so there's no single "it" to come back to.

## Rules

- `return;` ends a task early. A task can't return a value.
- A task's parameters are copies, kept for as long as the task runs.
- Tasks run on the same "thread" as the rest of the game. Nothing runs at exactly the same moment, so there's no need to worry about two things changing a variable at once. A task's code just slots in between frames.

## When tasks run

**With a window** (`<window>`): at the start of every frame, any task whose wait is over continues, before your `update` / `mainLoop` code runs. Time is measured in game frames' time (`window.deltaTime()`).

**Without a window** (console programs), nothing moves tasks forward automatically. Include `<tasks>` and either:

- call `tasks.update(dt)` yourself, which moves time forward by `dt` seconds and continues any task that's due; or
- call `tasks.runAll()`, which really waits, in real time, until every task has finished.

```cpp
#include <stdio>
#include <tasks>

task hello(){
    print("Hello...");
    wait(1);
    print("...world!");
}

int main(){
    hello();
    tasks.runAll();          // waits one second, then prints "...world!"
    return 0;
}
```

| `<tasks>` function | What it does |
|---|---|
| `tasks.update(float dt)` | moves task time forward by `dt` seconds |
| `tasks.runAll()` | keeps going (really waiting) until no tasks are left |
| `tasks.count()` | how many tasks are waiting |
| `tasks.stopAll()` | stops every waiting task |

## Tasks vs timers

For simple "has enough time passed?" checks, a [`Timer`](../libraries/time.md#timer) can be simpler (`if (cooldown.isDone()) { ... }`). Use a task when there's a **sequence**: do this, wait, do that, wait, do something else.

## What's next

[Errors](12-errors.md): reading Forge's error messages.
