# The Game Loop

A console program runs from top to bottom and ends. A game is different: it runs **in a loop**, many times a second, until the player quits. Each time around is a **frame**.

## What happens in a frame

Every frame, a game does three things:

1. **Read input**: which keys are held, where the mouse is.
2. **Update**: move things, check collisions, change scores, all by a small step of time.
3. **Draw**: clear the screen and draw everything where it is now.

At 60 frames per second, this happens 60 times every second. Each frame is a still picture, and showing them quickly makes the motion.

## The two ways to write it in Forge

### 1. The engine loop: window.run()

Forge runs the loop and calls two functions you write in `main.frg`:

```cpp
#include <window>
#include <graphics2d>
#include <input>

float x = 100;

int main(){
    window.open(800, 450, "Engine loop");
    window.run();               // only returns when the window closes
    return 0;
}

void update(float dt){          // called first, every frame
    if (input.keyDown(Key.RIGHT)) {
        x += 200 * dt;
    }
}

void draw(){                    // called second, every frame
    graphics2d.drawRect(x, 200, 40, 40, color.ORANGE);
}
```

Keeping `update` (changing things) separate from `draw` (showing things) keeps code tidy. **This is the recommended style.**

### 2. Your own loop

You write the loop yourself. `window.shouldClose()` ends the previous frame (showing it), starts a new one, and gives back `true` when it's time to quit:

```cpp
int main(){
    window.open(800, 450, "Own loop");
    while (!window.shouldClose()) {
        mainLoop();
    }
    window.close();
    return 0;
}

void mainLoop(){
    float dt = window.deltaTime();
    if (input.keyDown(Key.RIGHT)) {
        x += 200 * dt;
    }
    graphics2d.drawRect(x, 200, 40, 40, color.ORANGE);
}
```

This gives you more control, for example over the order of things, or skipping updates while paused.

### What Forge does around your code

| Step | Engine loop | Own loop |
|---|---|---|
| clear the screen to the background color | before `update` | at the start of each frame (inside `shouldClose`) |
| continue tasks whose `wait` is over | before `update` | at the start of each frame |
| your code | `update(dt)`, then `draw()` | the loop body |
| show the frame, read new input, keep to 60 FPS | after `draw` | at the next `shouldClose()` |

## dt: delta time

`dt` (or `window.deltaTime()`) is **the number of seconds since the last frame**: about 0.0167 at 60 frames per second.

**Always multiply movement by `dt`:**

```cpp
x += 200 * dt;      // 200 pixels per SECOND, on any computer
x += 3;             // 3 pixels per FRAME: twice as fast at 120 FPS, half as fast when the game lags
```

The same goes for anything that changes over time: velocity, rotation, timers, fading.

```cpp
velocity.y += gravity * dt;        // speed changes by gravity per second
position += velocity * dt;          // position changes by velocity per second
timer -= dt;                        // counts down in seconds
angle += 90 * dt;                   // 90 degrees per second
```

## Game state

Values that must last from frame to frame (positions, scores, lists of enemies) need to be **global variables**, or fields of objects that live in globals. A local variable in `update` starts fresh every frame:

```cpp
float x = 100;        // global: keeps its value between frames

void update(float dt){
    float speed = 200;    // local: made fresh every frame (fine for things that don't change)
    x += speed * dt;
}
```

## Organizing a bigger game

- Split things into files: `player.frg`, `enemies.frg`, `level.frg`, each with its own `update` and `draw` functions, called from `main.frg`'s: `player.update(dt);`, `player.draw();`.
- Use [scenes](../libraries/scenes.md) for menus, levels and game-over screens.
- Use [tasks](../language/11-tasks.md) for things that happen over several seconds.
- Use [ECS](../libraries/ecs.md) when many different kinds of objects share behavior.

See the [platformer walkthrough](platformer-walkthrough.md) for a complete, small example.
