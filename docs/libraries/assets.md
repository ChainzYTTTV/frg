# `<assets>`: Images, Animations and Fonts

```cpp
#include <assets>
```

Loads pictures (`Texture`) and fonts (`Font`) from files, and draws them. `Animation` plays the frames of a sprite sheet. See the [sprites guide](../guides/sprites-and-animation.md) for a walk-through.

## Where files go

Put asset files in the game's top folder (next to `main.frg`), or in a subfolder: `assets.loadTexture("art/player.png")`. When loading, Forge looks:

1. relative to the folder the game was started from (with `frg run`, that's the game's top folder),
2. next to the `.exe`,
3. in the folder above the `.exe`.

So the game finds its files whether it's started with `frg run` or by double-clicking `build/game.exe`. A missing file stops the game with "can't find the file 'player.png' - put it in the game's folder (next to main.frg), or check the spelling". Capitals matter for the error message, though Windows itself ignores them.

## Loading

| Function | Gives back | What it does |
|---|---|---|
| `assets.loadTexture(string path)` | `Texture` | loads an image: `.png`, `.jpg`, `.bmp`, `.tga`, `.gif` (first frame) or `.qoi` |
| `assets.loadFont(string path, int quality = 64)` | `Font` | loads a font: `.ttf` or `.otf`. `quality` is the size the letters are prepared at; use about the biggest size you'll draw it at. |
| `assets.exists(string path)` | `bool` | true if the file can be found (using the same search) |

**Load once, draw many times.** Load assets once, as global variables or at the start of the game, never inside `draw()`, which runs 60 times a second:

```cpp
Texture player = assets.loadTexture("player.png");    // good: loaded once
```

Loading works even **before** `window.open(...)`. The file is read straight away (so a missing file is reported straight away), and sent to the graphics card the first time it's drawn.

## Texture

A loaded image.

| Method | What it does |
|---|---|
| `width()` | the image's width in pixels (`int`) |
| `height()` | the image's height in pixels (`int`) |
| `draw(float x, float y, color tint = color.WHITE)` | draws it at its normal size with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | draws it `scale` times as big (2 = double size) |
| `drawRotated(vec2 center, float degrees, float scale = 1, color tint = color.WHITE)` | draws it **centered** on `center`, turned `degrees` clockwise |
| `drawPart(rect source, rect destination, color tint = color.WHITE)` | draws the `source` part of the image into the `destination` rectangle on screen, stretched to fit. This is how sprite sheets are drawn. |

**Tint** multiplies the image's colors: `color.WHITE` leaves it as it is, `color.RED` makes it reddish, and `color.WHITE.fade(0.5)` makes it half see-through.

**Flipping:** give `drawPart` a source with a negative width to flip the image left-to-right: `rect(0, 0, -16, 16)`. A negative height flips it upside down.

```cpp
Texture hero = assets.loadTexture("hero.png");

void draw(){
    hero.draw(100, 100);
    hero.drawScaled(200, 100, 4);                    // pixel art, 4x bigger
    hero.drawRotated(vec2(400, 200), angle, 2);
    hero.drawPart(rect(0, 0, 16, 16), rect(500, 100, 64, 64));
}
```

Pixel art stays sharp when scaled up; there's no blurring.

## Animation

Plays the frames of a **sprite sheet** (one image holding every frame of an animation, side by side) one after the other. The [sprites and animation guide](../guides/sprites-and-animation.md#animation) explains it step by step.

```cpp
Texture sheet = assets.loadTexture("hero.png");
Animation walk = Animation(sheet, 16, 16, 4, 8);     // 16x16 frames, 4 of them, 8 per second

void update(float dt){
    walk.update(dt);
}

void draw(){
    walk.drawScaled(100, 100, 4);
}
```

| Making one | |
|---|---|
| `Animation(Texture sheet, int frameWidth, int frameHeight, int frames, float fps)` | frames starting at the top-left of the sheet, read left to right (carrying on in the next row if needed) |
| `Animation(sheet, frameWidth, frameHeight, frames, fps, int row)` | frames starting at the beginning of row `row` (0 is the top row), for sheets with one row per animation |

| Method or field | What it does |
|---|---|
| `update(float dt)` | moves the animation on by `dt` seconds. Call it once per frame |
| `draw(float x, float y, color tint = color.WHITE)` | draws the current frame at its real size, with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | the same, `scale` times bigger |
| `play()` / `pause()` | starts and stops it. A paused animation stays on its frame |
| `reset()` | back to the first frame, playing |
| `flip(bool flipped)` | `flip(true)` draws it mirrored left-to-right, `flip(false)` normally |
| `isFlipped()`, `isPlaying()` | what was chosen |
| `frame()`, `setFrame(int frame)`, `frameCount()` | the frame being shown (from 0), jumping to one, and how many there are |
| `float fps` | frames per second; change it any time (0 stops it) |
| `bool looping` | `true` (the start): after the last frame it starts again. `false`: it stops on the last frame |
| `isDone()` | `true` when an animation that doesn't loop has reached its last frame |

If the sheet doesn't have room for the frames you asked for, the game stops with a message saying how big the sheet really is. `Animation` is a class, so `Animation current = walk;` makes `current` mean the very same animation, not a copy.

## Font

A loaded font.

| Method | Gives back | What it does |
|---|---|---|
| `drawText(string text, float x, float y, float size, color c)` | — | draws text with its top-left corner at (x, y). `size` is the height in pixels. |
| `measure(string text, float size)` | `float` | how wide the text would be, in pixels |

```cpp
Font title = assets.loadFont("fonts/Title.ttf", 96);

void draw(){
    string text = "Forge Quest";
    float w = title.measure(text, 72);
    title.drawText(text, (window.width() - w) / 2.0, 80, 72, color.GOLD);
}
```

`Font()` (made directly, not loaded) is the built-in pixel font.

Fonts are prepared for drawing the first time they're used, which needs the window to be open. `measure` also needs the window.

## Memory

Textures and fonts are freed automatically when nothing uses them anymore, and everything is cleaned up when the window closes. There's no need to unload anything.
