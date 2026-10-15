# `<scenes>`: Menus, Levels and Game Over

```cpp
#include <scenes>
```

Most games have several **scenes**: a title menu, the levels, a pause screen, a game-over screen. Each needs its own update and draw code. This library keeps them apart, and switches between them cleanly.

## Scene

Each scene is a class that inherits from `Scene` and fills in any of these methods (they all do nothing to start with):

| Method | Runs |
|---|---|
| `void enter()` | once, when the scene starts |
| `void update(float dt)` | every frame, from `scenes.update(dt)` |
| `void draw()` | every frame, from `scenes.draw()` |
| `void exit()` | once, when switching to another scene |

```cpp
class Menu : Scene {
    void update(float dt) {
        if (input.keyPressed(Key.ENTER)) {
            scenes.change(Level(1));
        }
    }
    void draw() {
        graphics2d.drawText("Press Enter to play", 300, 200, 30, color.WHITE);
    }
}

class Level : Scene {
    int number;
    Level(int number) {
        this.number = number;
    }
    void enter() {
        print("starting level {number}");
    }
    void draw() {
        graphics2d.drawText("Level {number}", 20, 20, 30, color.WHITE);
    }
}
```

Scenes are ordinary classes, so they can have fields, constructors and helper methods, and they keep their state for as long as they're running.

## Functions

| Function | What it does |
|---|---|
| `scenes.change(Scene next)` | switches to `next` at the start of the next `scenes.update`. The very first scene starts straight away. |
| `scenes.update(float dt)` | does any pending switch (the old scene's `exit()`, then the new one's `enter()`), then runs the current scene's `update(dt)` |
| `scenes.draw()` | runs the current scene's `draw()` |
| `scenes.current()` | the running scene (`Scene?`, null before the first one) |

## Wiring it up

This uses the `Menu` and `Level` classes from above.

```cpp
#include <window>
#include <graphics2d>
#include <input>
#include <scenes>
#include <stdio>

int main(){
    window.open(960, 540, "Scenes");
    scenes.change(Menu());
    window.run();
    return 0;
}

void update(float dt){
    scenes.update(dt);
}

void draw(){
    scenes.draw();
}
```

Because the switch waits for the next `scenes.update`, it's safe to call `scenes.change` from inside a scene's own `update` or `draw`: the current frame finishes normally first.

## Checking which scene is running

```cpp
if (scenes.current() is Level) {
    Level lvl = scenes.current() as Level;
    print(lvl.number);
}
```

See [examples/menu](../../examples/menu/main.frg) for a menu with buttons, using [`<ui>`](ui.md).
