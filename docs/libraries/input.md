# `<input>`: Keyboard and Mouse

```cpp
#include <input>
```

Reads the keyboard and mouse in a windowed game. Input is updated once per frame (when the frame is shown), so check it every frame in your `update` / loop.

## Keyboard

| Function | Gives back | True when |
|---|---|---|
| `input.keyDown(Key key)` | `bool` | the key is **held down** (true every frame while held) |
| `input.keyPressed(Key key)` | `bool` | the key was **just pressed** (true for one frame only) |
| `input.keyReleased(Key key)` | `bool` | the key was **just let go** (true for one frame only) |

Use `keyDown` for things that continue while held (walking), and `keyPressed` for things that happen once per press (jumping, shooting, menus):

```cpp
if (input.keyDown(Key.RIGHT)) {
    x += speed * dt;
}
if (input.keyPressed(Key.SPACE) and onGround) {
    velocityY = -600;
}
```

### Keys

| Group | Values |
|---|---|
| Letters | `Key.A` … `Key.Z` |
| Number row | `Key.NUM0` … `Key.NUM9` |
| Arrows | `Key.LEFT` `Key.RIGHT` `Key.UP` `Key.DOWN` |
| Common | `Key.SPACE` `Key.ENTER` `Key.ESCAPE` `Key.BACKSPACE` `Key.TAB` |
| Modifiers | `Key.LEFT_SHIFT` `Key.RIGHT_SHIFT` `Key.LEFT_CONTROL` `Key.RIGHT_CONTROL` `Key.LEFT_ALT` `Key.RIGHT_ALT` |
| Function keys | `Key.F1` … `Key.F12` |

`Key` is an [enum](../language/10-enums-optionals-pointers.md#enums), so keys can be stored in variables and lists: `Key jumpKey = Key.SPACE;`.

Escape also closes the window by default.

## Mouse

| Function | Gives back | What it is |
|---|---|---|
| `input.mousePosition()` | `vec2` | the mouse's position in the window (top-left is `(0, 0)`) |
| `input.mouseX()` | `float` | just the x |
| `input.mouseY()` | `float` | just the y |
| `input.mouseDown(MouseButton b)` | `bool` | the button is held down |
| `input.mousePressed(MouseButton b)` | `bool` | the button was just pressed (one frame) |
| `input.mouseReleased(MouseButton b)` | `bool` | the button was just let go (one frame) |
| `input.mouseWheel()` | `float` | how far the wheel moved this frame (positive = away from you) |

Buttons: `MouseButton.LEFT`, `MouseButton.RIGHT`, `MouseButton.MIDDLE`.

```cpp
if (input.mousePressed(MouseButton.LEFT)) {
    vec2 m = input.mousePosition();
    if (rect(100, 100, 200, 50).contains(m)) {
        print("clicked the box");
    }
}
```

## input vs input()

`<stdio>` has a function `input(...)` for typing into the console, and this library is also called `input`. With both included, `input.keyDown(...)` is this library and `input("Name? ")` is the console function.

## Examples

### Eight-way movement

```cpp
vec2 move = vec2.ZERO;
if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) { move.x -= 1; }
if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) { move.x += 1; }
if (input.keyDown(Key.UP) or input.keyDown(Key.W)) { move.y -= 1; }
if (input.keyDown(Key.DOWN) or input.keyDown(Key.S)) { move.y += 1; }
position += move.normalized() * speed * dt;   // normalized: diagonals aren't faster
```

### Toggling a pause menu

```cpp
if (input.keyPressed(Key.P)) {
    paused = not paused;
}
```
