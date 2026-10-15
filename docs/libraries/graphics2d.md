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

## Camera: scrolling and zooming

When a level is bigger than the window, use a **camera**. Everything drawn after `graphics2d.setCamera(...)` is moved (and zoomed, and turned) so that the camera's `target`, a point in your level, shows at its `offset`, a point on the screen. `graphics2d.resetCamera()` goes back to drawing straight on the screen, which is what you want for the score and menus.

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));   // target, offset

void update(float dt){
    // glide toward the player: a bit of the way there each frame
    camera.target = camera.target.lerp(hero.center(), 5 * dt);
}

void draw(){
    graphics2d.setCamera(camera);      // the world...
    drawLevel();
    graphics2d.drawRect(hero.bounds(), color.ORANGE);
    graphics2d.resetCamera();          // ...and then the text, which stays put
    graphics2d.drawText("Coins: {coins}", 20, 20, 24, color.GOLD);
}
```

| `Camera2D` field | What it is |
|---|---|
| `vec2 target` | the point in the world the camera looks at |
| `vec2 offset` | where on the screen `target` shows. Use the middle of the window (`vec2(window.width() / 2.0, window.height() / 2.0)`) to keep the target centered |
| `float zoom` | `1` is normal size, `2` makes everything twice as big, `0.5` half. Must be more than 0 |
| `float rotation` | turns the view, in degrees |

Make one with `Camera2D(target, offset)` or `Camera2D(target, offset, zoom, rotation)`.

| Function | What it does |
|---|---|
| `graphics2d.setCamera(Camera2D camera)` | draw through this camera from now on. It stays set (even in later frames) until `resetCamera` |
| `graphics2d.resetCamera()` | stop using the camera: draw straight on the screen again |
| `graphics2d.camera()` | the camera that was set (a copy) |
| `graphics2d.screenToWorld(vec2 point)` | where a point on the screen is in the world. Use it for the mouse: `graphics2d.screenToWorld(input.mousePosition())` |
| `graphics2d.worldToScreen(vec2 point)` | where a point in the world shows on the screen |

Things to know:

- `Camera2D` is a struct, so `setCamera` takes a copy. After changing `camera.target`, call `setCamera(camera)` again (usually at the start of `draw()`, every frame).
- To stop the camera from showing past the edges of the level, keep the target away from them with `math.clamp`: `math.clamp(x, window.width() / 2.0, LEVEL_WIDTH - window.width() / 2.0)`.
- Physics doesn't know about the camera. Bodies keep their world positions; only the drawing moves.
- The 3D camera (`Camera`, in `<graphics3d>`) is a different thing. A game can use both: 3D drawing ignores the 2D camera, and the 2D camera comes back at the next 2D drawing.

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
