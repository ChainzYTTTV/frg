# `<physics>`: Boxes, Gravity and Collisions

```cpp
#include <physics>
```

Simple 2D physics made for platformers. Every **body** is an axis-aligned box (never rotated). Each step, moving bodies fall with gravity, move by their velocity, and get pushed out of anything solid they run into.

It works in [screen coordinates](graphics2d.md#coordinates): y grows **downward**, so gravity pulls toward +y and jumping means a **negative** y velocity.

This library doesn't draw anything: draw bodies yourself, for example with `graphics2d.drawRect(body.bounds(), color.ORANGE)`. It doesn't need a window either.

## Body

```cpp
Body crate = Body(100, 50, 32, 32);    // x, y (top-left corner), width, height
physics.add(crate);
```

| Making one | |
|---|---|
| `Body(float x, float y, float width, float height)` | a moving, solid box |

| Field | Type | Meaning |
|---|---|---|
| `position` | `vec2` | top-left corner |
| `size` | `vec2` | width and height |
| `velocity` | `vec2` | pixels per second |
| `isStatic` | `bool` | `true` for things that never move: floors, walls, platforms. Static bodies ignore gravity and velocity. (`false` to start with.) |
| `solid` | `bool` | `false` for things that shouldn't push or be pushed, like coins and trigger zones. (`true` to start with.) |
| `oneWay` | `bool` | for static bodies: `true` makes a **one-way platform** that you can jump up through from below and walk through sideways, but land on from above. (`false` to start with.) |
| `gravityScale` | `float` | how strongly gravity pulls this body: 1 is normal, 0 floats, 2 is twice as heavy |
| `onGround` | `bool` | **set by Forge:** true if it landed on something during the last step |
| `onCeiling` | `bool` | **set by Forge:** true if it bumped its head |
| `onWall` | `bool` | **set by Forge:** true if it hit something sideways |

| Method | Gives back | What it is |
|---|---|---|
| `bounds()` | `rect` | the body's rectangle, handy for drawing it |
| `center()` | `vec2` | the middle point |
| `overlaps(Body other)` | `bool` | true if the two boxes overlap right now |

## The world

| Function | What it does |
|---|---|
| `physics.add(Body body)` | adds a body to the world. Adding it twice does nothing. |
| `physics.remove(Body body)` | takes it out of the world |
| `physics.step(float dt)` | moves everything forward by `dt` seconds. Call it **once per frame**. |
| `physics.setGravity(vec2 gravity)` | gravity, in pixels per second per second. Starts at `vec2(0, 980)`. |
| `physics.gravity()` | the current gravity |
| `physics.clear()` | removes every body |
| `physics.count()` | how many bodies are in the world |

Bodies only move during `physics.step`, and only if they've been added.

## Seeing the bodies: the debug view

When something collides in a way you don't expect, draw what physics sees:

```cpp
#include <graphics2d>    // physics.drawDebug() draws with graphics2d, so the file needs both
#include <physics>

void draw(){
    level.draw();
    player.draw();
    physics.drawDebug();     // on top of everything else
}
```

`physics.drawDebug()` draws the outline of every body in the world:

| Outline | Body |
|---|---|
| blue | static (`isStatic = true`): floors, walls, platforms |
| light blue, with a thick top edge | a one-way platform (`oneWay = true`) |
| green | moving |
| purple | not solid (`solid = false`): coins, triggers |
| a yellow bar along the bottom | a moving body that's standing on something (`onGround` is true) |

It's drawn like anything else in `<graphics2d>`, so with a [2D camera](graphics2d.md#camera-scrolling-and-zooming), call it before `resetCamera()` so the outlines line up with the level. Take the line out (or put it behind a key, like `if (showDebug) { ... }`) when you're done.

The physics library itself never draws (it doesn't use raylib), which is why the file calling `drawDebug` needs `#include <graphics2d>`. Without it, Forge says so.

## What happens in a step

For each moving (not static) body:

1. `onGround`, `onCeiling` and `onWall` are reset to false.
2. Gravity is added to its velocity: `velocity += gravity × gravityScale × dt`.
3. It moves sideways by `velocity.x × dt`. If that runs it into a static solid body, it's pushed back out, `velocity.x` becomes 0, and `onWall` becomes true.
4. It moves up or down by `velocity.y × dt`. If that runs it into a static solid body, it's pushed out, `velocity.y` becomes 0, and `onGround` (when moving down) or `onCeiling` (when moving up) becomes true. A one-way platform (`oneWay = true`) only counts here, and only when the body is falling onto it from above; in step 3 it never counts.
5. Fast bodies move in several small steps, so they can't pass straight through thin platforms.

Then moving solid bodies that overlap each other are pushed apart, in whichever direction they overlap least. A body resting on the ground isn't pushed down into it; the one on top is pushed up instead (and gets `onGround`), so things stack.

To keep things stable after a pause, `dt` is capped at 0.05 seconds per step.

## Patterns

### Walking and jumping

```cpp
void update(float dt){
    float dir = 0;
    if (input.keyDown(Key.LEFT)) { dir -= 1; }
    if (input.keyDown(Key.RIGHT)) { dir += 1; }
    player.velocity.x = dir * 300;                    // set x speed directly, every frame

    if (input.keyPressed(Key.SPACE) and player.onGround) {
        player.velocity.y = -600;                     // up is negative
    }
    physics.step(dt);
}
```

Read `onGround` **before** calling `physics.step` in the same frame: it describes the previous step.

### One-way platforms

Platforms you can jump up through from below, and then stand on, like in many classic platformers:

```cpp
Body ledge = Body(300, 320, 160, 12);
ledge.isStatic = true;
ledge.oneWay = true;
physics.add(ledge);
```

A body only lands on it when it's **falling** and was **above** the platform's top just before. From below and from the sides, it passes straight through. `oneWay` only matters for static bodies. In the [debug view](#seeing-the-bodies-the-debug-view), one-way platforms are light blue with a thick top edge.

### Platforms and walls

To build a whole level of walls and platforms, drawing it as text with [`<tilemap>`](tilemap.md) is usually easier: `level.addBodies("#")` makes the static bodies for you.

```cpp
void addPlatform(float x, float y, float w, float h){
    Body b = Body(x, y, w, h);
    b.isStatic = true;
    physics.add(b);
}
```

### Collectibles and triggers

```cpp
Body coin = Body(300, 200, 16, 16);
coin.isStatic = true;
coin.solid = false;          // the player passes through it
physics.add(coin);

// every frame:
if (player.overlaps(coin)) {
    physics.remove(coin);
    score++;
}
```

### Your own body classes

Classes can inherit from `Body` to add their own fields and methods. They use `Body`'s constructor if they don't have one:

```cpp
class Player : Body {
    int lives = 3;
    void respawn(){
        position = vec2(80, 300);
        velocity = vec2.ZERO;
    }
}

Player hero = Player(80, 300, 32, 32);
```

## Limits

- Boxes only: no circles, slopes or rotation.
- Static bodies don't move by themselves, but you can change their `position` yourself (moving platforms). Bodies standing on one aren't carried along automatically.
- There's no bounciness or friction: set `velocity` yourself for those effects.
