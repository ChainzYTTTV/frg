# Sprites and Animation

A **sprite** is a 2D image for a game object. This guide covers drawing sprites, sprite sheets, animation, and flipping. The full reference is in [`<assets>`](../libraries/assets.md).

## Drawing one image

```cpp
#include <window>
#include <assets>

Texture ship = assets.loadTexture("ship.png");

int main(){
    window.open(800, 450, "Sprites");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    ship.draw(100, 100);                   // normal size, top-left at (100, 100)
    ship.drawScaled(300, 100, 3);          // three times bigger
}
```

Put `ship.png` in the game's folder, next to `main.frg`.

## Pixel art

Small pixel-art images (16×16, 32×32) are usually drawn several times bigger. Forge keeps them sharp, without blurring. Pick a whole-number scale (2, 3, 4...) so every pixel stays the same size.

## Sprite sheets

A **sprite sheet** is one image holding many pictures (**frames**) side by side, like a strip of film:

```
+------+------+------+------+
|  0   |  1   |  2   |  3   |      blob.png: 4 frames, each 16x16, so 64x16 overall
+------+------+------+------+
```

`drawPart(source, destination)` draws just one part of an image:

- `source` is a `rect` **inside the image**: which frame.
- `destination` is a `rect` **on the screen**: where to draw it, and how big.

```cpp
int frame = 2;
rect source = rect(frame * 16, 0, 16, 16);          // the 3rd frame
rect destination = rect(x, y, 64, 64);              // drawn 4x bigger
blob.drawPart(source, destination);
```

For a sheet with several rows (one row per animation), also pick the row: `rect(frame * 16, row * 16, 16, 16)`.

## Animation

To animate, show the frames one after the other. The `Animation` class from `<assets>` does that for you:

```cpp
#include <assets>

Texture blob = assets.loadTexture("blob.png");
Animation wobble = Animation(blob, 16, 16, 4, 8);    // the sheet, frame width, frame height, how many frames, frames per second

void update(float dt){
    wobble.update(dt);                 // moves on to the next frame when it's time
}

void draw(){
    wobble.drawScaled(x, y, 4);        // the current frame, 4 times bigger
}
```

Frames are read from the sheet **left to right**. If there are more frames than fit in a row, they carry on at the start of the next row.

| Making one | |
|---|---|
| `Animation(Texture sheet, int frameWidth, int frameHeight, int frames, float fps)` | frames starting at the top-left of the sheet |
| `Animation(sheet, frameWidth, frameHeight, frames, fps, int row)` | frames starting at the beginning of row `row` (0 is the top row), for sheets with one row per animation |

| Method or field | What it does |
|---|---|
| `update(float dt)` | moves the animation on by `dt` seconds. Call it once per frame |
| `draw(float x, float y, color tint = color.WHITE)` | draws the current frame at its real size, with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | the same, `scale` times bigger |
| `play()` / `pause()` | starts and stops it. A paused animation stays on its frame |
| `reset()` | back to the first frame, playing |
| `flip(bool flipped)` | `flip(true)` draws it mirrored left-to-right, `flip(false)` normally (see below) |
| `isFlipped()`, `isPlaying()` | what was chosen |
| `frame()`, `setFrame(int frame)`, `frameCount()` | the frame being shown (from 0), jumping to one, and how many there are |
| `float fps` | frames per second; change it any time (0 stops it) |
| `bool looping` | `true` (the start): after the last frame it starts again. `false`: it stops on the last frame |
| `isDone()` | `true` when an animation that doesn't loop has reached its last frame |

If the sheet doesn't have room for the frames you asked for, the game stops with a message saying how big the sheet really is.

### Several animations

Make one `Animation` per action, and keep the one being shown in a variable. With a sheet that has the idle frames in row 0 and the running frames in row 1:

```cpp
Texture sheet = assets.loadTexture("hero.png");
Animation idle = Animation(sheet, 16, 16, 2, 3);        // row 0: 2 frames, slow
Animation run = Animation(sheet, 16, 16, 6, 12, 1);     // row 1: 6 frames, fast
Animation current = idle;

void update(float dt){
    Animation wanted = moving ? run : idle;
    if (wanted != current) {
        current = wanted;
        current.reset();       // start the new animation from its first frame
    }
    current.update(dt);
}

void draw(){
    current.drawScaled(x, y, 4);
}
```

`Animation` is a class, so `current = run` makes `current` mean the very same animation as `run` (not a copy).

A one-time animation, like an explosion: set `looping = false`, and remove the explosion when `isDone()` is true.

### How it works

There's no magic inside: an animation counts time with `dt` and picks a part of the sheet with `drawPart`. Doing it by hand looks like this, which is useful if you ever need something `Animation` can't do:

```cpp
int frame = 0;
float frameTimer = 0;
const float FRAME_TIME = 0.1;      // 10 frames per second
const int FRAME_COUNT = 4;

void update(float dt){
    frameTimer += dt;
    if (frameTimer >= FRAME_TIME) {
        frameTimer -= FRAME_TIME;
        frame = (frame + 1) mod FRAME_COUNT;       // 0, 1, 2, 3, 0, 1, ...
    }
}

void draw(){
    blob.drawPart(rect(frame * 16, 0, 16, 16), rect(x, y, 64, 64));
}
```

## Facing left and right

Draw sprites facing right, then flip them when moving left:

```cpp
if (velocity.x < 0) {
    walk.flip(true);
}
if (velocity.x > 0) {
    walk.flip(false);
}
```

(Only change it when the player is actually moving, so they keep facing the same way when they stop.)

Without an `Animation`, a **negative width** in the source rectangle of `drawPart` flips the image the same way:

```cpp
bool facingLeft = velocity.x < 0;
float w = facingLeft ? -16 : 16;
sheet.drawPart(rect(frame * 16, 0, w, 16), rect(x, y, 64, 64));
```

## Rotating

`drawRotated(center, degrees, scale)` draws the image **centered** on a point and turned. Positive degrees turn clockwise:

```cpp
float angle = 0;

void update(float dt){
    angle += 180 * dt;         // half a turn per second
}

void draw(){
    ship.drawRotated(vec2(400, 225), angle, 2);
}
```

To point at something (like the mouse), work out the angle with `math.atan2` and convert radians to degrees:

```cpp
vec2 toMouse = input.mousePosition() - shipPosition;
float degrees = math.atan2(toMouse.y, toMouse.x) * 180 / math.PI;
```

## Tinting and fading

The `tint` color multiplies the image's colors:

```cpp
ship.draw(x, y, color.RED);                    // reddish (e.g. when hit)
ship.draw(x, y, color.WHITE.fade(0.5));        // half see-through
```

A hit flash with a [task](../language/11-tasks.md):

```cpp
color shipTint = color.WHITE;

task hitFlash(){
    shipTint = color.RED;
    wait(0.1);
    shipTint = color.WHITE;
}
```

## A working example

[examples/sprites](../../examples/sprites/main.frg) loads a 4-frame sprite sheet, animates it with an `Animation`, moves it with the arrow keys (flipping it to face the way it's going), and plays a sound when it hops.
