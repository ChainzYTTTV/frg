# `<graphics2d>`: 2D Drawing

```cpp
#include <graphics2d>
```

Draws shapes and text in the [window](window.md). Everything is redrawn every frame: the screen is cleared before each frame, so draw everything you want to see, every frame.

## Coordinates

- `(0, 0)` is the **top-left** corner of the window.
- **x** grows to the right, and **y** grows **downward**.
- Sizes and positions are in pixels. Decimal positions are fine.
- Things drawn later appear **on top** of things drawn earlier.

## Functions

Several functions have two versions: one taking separate numbers, and one taking a [`vec2` or `rect`](core-types.md). Use whichever is handier.

### Rectangles

| Function | Draws |
|---|---|
| `graphics2d.drawRect(float x, float y, float width, float height, color c)` | a filled rectangle; (x, y) is the top-left corner |
| `graphics2d.drawRect(rect r, color c)` | the same, from a `rect` |
| `graphics2d.drawRectOutline(float x, float y, float width, float height, color c, float thickness = 1)` | the outline of a rectangle |
| `graphics2d.drawRectOutline(rect r, color c, float thickness = 1)` | the same, from a `rect` |

### Circles

| Function | Draws |
|---|---|
| `graphics2d.drawCircle(float x, float y, float radius, color c)` | a filled circle, centered on (x, y) |
| `graphics2d.drawCircle(vec2 center, float radius, color c)` | the same, from a `vec2` |
| `graphics2d.drawCircleOutline(float x, float y, float radius, color c)` | a 1-pixel circle outline |

### Lines and triangles

| Function | Draws |
|---|---|
| `graphics2d.drawLine(float x1, float y1, float x2, float y2, color c, float thickness = 1)` | a line between two points |
| `graphics2d.drawLine(vec2 from, vec2 to, color c, float thickness = 1)` | the same, from `vec2`s |
| `graphics2d.drawTriangle(vec2 a, vec2 b, vec2 c, color fill)` | a filled triangle (the corners can be in any order) |

### Text

| Function | Gives back | What it does |
|---|---|---|
| `graphics2d.drawText(string text, float x, float y, int size, color c)` | — | draws text with its top-left corner at (x, y). `size` is the height in pixels. |
| `graphics2d.textWidth(string text, int size)` | `int` | how wide that text would be, in pixels |
| `graphics2d.drawFPS(float x, float y)` | — | draws the current frames per second |

This uses the built-in pixel font. For other fonts, see [`Font`](assets.md#font).

Centering text:

```cpp
string title = "GAME OVER";
int w = graphics2d.textWidth(title, 48);
graphics2d.drawText(title, (window.width() - w) / 2.0, 200, 48, color.WHITE);
```

### Other

| Function | What it does |
|---|---|
| `graphics2d.clear(color c)` | fills the whole screen with a color (the window already clears to its background color each frame) |

## Colors and transparency

Every function takes a [`color`](core-types.md#color). Make one with `color(r, g, b)`, use a named one like `color.RED`, or make it see-through with `color(255, 0, 0, 100)` or `color.RED.fade(0.4)`.

## Images

To draw pictures from files, see [`<assets>`](assets.md) (`Texture`). For sprite sheets and animation, see the [sprites guide](../guides/sprites-and-animation.md).

## Mixing with 3D

2D drawing can be mixed with [`<graphics3d>`](graphics3d.md) in the same frame. 2D drawn after 3D appears on top of it, which is how you'd draw a HUD over a 3D scene.

## Example: a health bar

```cpp
void drawHealthBar(float x, float y, int hp, int maxHp){
    float width = 200;
    float filled = width * hp / maxHp;
    graphics2d.drawRect(x, y, width, 20, color.DARKGRAY);
    graphics2d.drawRect(x, y, filled, 20, hp > maxHp / 4 ? color.GREEN : color.RED);
    graphics2d.drawRectOutline(x, y, width, 20, color.WHITE, 2);
    graphics2d.drawText("{hp}/{maxHp}", x + 8, y + 2, 16, color.WHITE);
}
```
