# `<window>`: The Game Window

```cpp
#include <window>
```

Opens the window your game is drawn in, runs the game loop, and keeps track of time between frames. See the [game loop guide](../guides/game-loop.md) for the ideas behind it.

## A minimal game

```cpp
#include <window>
#include <graphics2d>

int main(){
    window.open(800, 450, "My Game");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    graphics2d.drawCircle(400, 225, 50, color.RED);
}
```

## Functions

### Opening and closing

| Function | What it does |
|---|---|
| `window.open(int width, int height, string title)` | Opens the window, `width` × `height` pixels. Frames are limited to 60 per second to start with. Calling it again while it's open does nothing. |
| `window.close()` | Closes the window. It's also closed automatically when `main` ends. |

### The game loop: two ways

**Your own loop:**

```cpp
while (!window.shouldClose()) {
    // update and draw one frame here
}
window.close();
```

| Function | What it does |
|---|---|
| `window.shouldClose()` | Shows the frame drawn since the last call, then starts the next frame. Gives back `true` when the player has clicked the window's X (or `window.close()` was called), so the loop ends. |

**The engine loop:**

```cpp
window.run();
```

| Function | What it does |
|---|---|
| `window.run()` | Runs the game until the window closes. Every frame, it calls **`update(float dt)`** and then **`draw()`**, and then shows the frame. Both functions must be written in **main.frg** with exactly those names and parameters, or it's a compile error. When the window closes, `run()` closes it and returns. |

Both loops do the same things each frame:

1. Clear the screen to the background color.
2. Continue any [tasks](../language/11-tasks.md) whose `wait` is over.
3. Your code: update, then draw.
4. Show the frame, read the keyboard and mouse, and wait if needed to keep to the frame rate.

### Time

| Function | Gives back | What it does |
|---|---|---|
| `window.deltaTime()` | `float` | seconds since the last frame, usually about 0.0167 at 60 FPS. This is the same `dt` that `update(dt)` gets. |
| `window.time()` | `double` | seconds since the window opened |
| `window.fps()` | `int` | frames per second right now |
| `window.setFPS(int fps)` | — | the most frames per second to draw (60 to start with); 0 means no limit |

**Multiply movement by `dt`** so the game runs at the same speed on fast and slow computers: `x += 200 * dt;` moves 200 pixels per **second**, however many frames that takes.

### Size and look

| Function | What it does |
|---|---|
| `window.width()` | the window's width in pixels (`int`) |
| `window.height()` | the window's height in pixels (`int`) |
| `window.setTitle(string title)` | changes the title bar text |
| `window.setBackground(color c)` | the color the screen is cleared to every frame (dark blue-gray to start with) |
| `window.toggleFullscreen()` | switches between fullscreen and windowed |
| `window.screenshot(string fileName)` | saves what's on screen as a `.png` in the game's folder |

## Stopping the game

No single key closes a Forge game. **Escape is a normal key** that your game can use (for a pause menu, say). The window closes when:

- the player clicks its **X**, or presses **Alt+F4**;
- your code calls **`window.close()`** (from a Quit button, after the last level...);
- **`main` returns**, or the game stops with a runtime error.

With `window.run()`, the first two make `window.run()` return, and `main` carries on after it: that's the place to save the game. If `update` calls `window.close()`, `update` finishes, `draw` isn't called again, and `window.run()` returns. With your own loop, they make the next `window.shouldClose()` give back `true`.

<!-- check: builds -->

```cpp
#include <window>
#include <input>
#include <stdio>

bool paused = false;
int coins = 0;

int main(){
    window.open(960, 540, "My game");
    window.run();           // returns when the player closes the window, or window.close() is called
    print("saving {coins} coins");   // so this runs every time the game ends normally
    return 0;
}

void update(float dt){
    if (input.keyPressed(Key.ESCAPE)) {
        paused = !paused;   // Escape opens and closes the pause menu
    }
    if (paused and input.keyPressed(Key.Q)) {
        window.close();     // Quit, from the pause menu
    }
}

void draw(){
}
```

Every way a game can stop, what runs after each, and examples that are checked by Forge's tests: [Stopping the game](../reference/rules.md#stopping-the-game). The exact order of everything in a frame: [A frame, step by step](../reference/rules.md#a-frame-step-by-step).

## Notes

- Drawing can happen any time after `open()`; a frame starts by itself if one isn't running.
- Calling drawing functions before `window.open(...)` stops the game with "the window isn't open yet".
- Escape doesn't close the window: it's a normal key your game can use. The window closes when the player clicks its X, when your code calls `window.close()`, or when `main` ends.
- If the game's folder has an `icon.png`, it becomes the window's icon (and the `.exe`'s icon). See [The game's icon](../getting-started.md#the-games-icon).
- [`<input>`](input.md), [`<graphics2d>`](graphics2d.md), [`<graphics3d>`](graphics3d.md) and [`<ui>`](ui.md) all work with this window.

## Testing games automatically

Games read three environment variables, so they can be tested without anyone at the keyboard (Forge's own tests use this):

| Variable | Effect |
|---|---|
| `FRG_AUTOTEST_FRAMES=90` | stop after 90 frames |
| `FRG_AUTOTEST_SHOT=shot.png` | take this screenshot just before stopping |
| `FRG_AUTOTEST_INPUT=RIGHT:10-60,SPACE:30-31` | pretend these keys are held during those frames (mouse buttons are `MOUSE_LEFT`, `MOUSE_RIGHT`, `MOUSE_MIDDLE`, `MOUSE_BACK` and `MOUSE_FORWARD`) |

## Limits

- One window per game, and **its size is fixed**: the player can't drag its edges to resize it. `toggleFullscreen()` is the only other size.
- The window's icon comes from `icon.png` when the game is built; it can't be changed while the game runs.
