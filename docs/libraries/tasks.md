# `<tasks>`: Tasks Without a Window

```cpp
#include <tasks>
```

[Tasks](../language/11-tasks.md) (functions returning `task` that call `wait`) are part of the core language. With a window open, they move forward by themselves every frame. **This library is only needed in programs without a window**, where something has to move task time forward.

| Function | What it does |
|---|---|
| `tasks.update(float dt)` | moves task time forward by `dt` seconds, and continues every task whose wait is over |
| `tasks.runAll()` | keeps going, really waiting in real time, until there are no tasks left |
| `tasks.count()` | how many tasks are waiting |
| `tasks.stopAll()` | stops every waiting task |

## Running tasks in a console program

```cpp
#include <stdio>
#include <tasks>

task countdown(){
    for (int i = 3; i > 0; i--) {
        print(i);
        wait(1);
    }
    print("Liftoff!");
}

int main(){
    countdown();
    tasks.runAll();       // takes about 3 seconds
    return 0;
}
```

## Stepping by hand

`tasks.update(dt)` doesn't wait in real time; it just moves task time forward. That makes it useful for simulations and tests:

```cpp
countdown();
tasks.update(1);      // prints 2
tasks.update(1);      // prints 1
tasks.update(1);      // prints Liftoff!
```

Notes:

- `tasks.runAll()` returns straight away if there are no tasks, and stops if the only tasks left are waiting on each other.
- A task that loops forever (`while (true) { wait(1); }`) makes `runAll` run forever. Stop it first, or use `update`.
- In a windowed game, don't call `tasks.update` yourself; the window already does, and calling it too would move time forward twice.

## Limits

- With a window open, every task's `wait` moves on with every frame, **even while your game shows its own pause menu**. A task that should stop during a pause has to check (for example `while (paused) { wait(); }`, which waits a frame at a time).
