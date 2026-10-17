# `<input>`: Keyboard, Mouse and Controllers

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

`Key` is an [enum](../language/10-enums-and-optionals.md#enums), so keys can be stored in variables and lists: `Key jumpKey = Key.SPACE;`.

Escape is a normal key: it doesn't close the window, so a game can use it for a pause menu or to let go of the mouse. The player closes the window with its X button (see [Stopping the game](window.md#stopping-the-game)).

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
| `input.mouseDelta()` | `vec2` | how far the mouse moved since the last frame, in pixels (right and down are positive) |
| `input.isMouseInWindow()` | `bool` | the pointer is over the window |

Buttons: `MouseButton.LEFT`, `MouseButton.RIGHT`, `MouseButton.MIDDLE`, and the two side (thumb) buttons many mice have: `MouseButton.BACK` (the one nearer you) and `MouseButton.FORWARD`.

```cpp
if (input.mousePressed(MouseButton.LEFT)) {
    vec2 m = input.mousePosition();
    if (rect(100, 100, 200, 50).contains(m)) {
        print("clicked the box");
    }
}
```

## The mouse pointer (cursor)

| Function | Gives back | What it does |
|---|---|---|
| `input.hideCursor()` | | hides the pointer. It still moves, and can leave the window |
| `input.showCursor()` | | shows it again (and lets go of it, if it was locked) |
| `input.isCursorHidden()` | `bool` | whether it's hidden (also while it's locked) |
| `input.lockMouse(bool locked)` | | `true` hides the pointer **and** keeps it in the window, so the mouse can move forever (for looking around in 3D); `false` gives it back, shown |
| `input.isMouseLocked()` | `bool` | whether it's locked |
| `input.setMousePosition(float x, float y)` | | moves the pointer to that place in the window (also takes a `vec2`) |
| `input.setCursor(Cursor shape)` | | the pointer's shape: `Cursor.ARROW` (normal), `HAND` (something to click), `TEXT` (typing), `CROSSHAIR`, `RESIZE_ALL`, `RESIZE_LEFT_RIGHT`, `RESIZE_UP_DOWN`, `NOT_ALLOWED` |

These need the window to be open (they stop the game with "the window isn't open yet" before `window.open(...)`).

**Hidden or locked?**

| | `hideCursor()` | `lockMouse(true)` |
|---|---|---|
| the pointer shows | no | no |
| `mousePosition()` follows the mouse | yes | no: use `mouseDelta()` |
| it can leave the window | yes | no |
| it stops at the edge of the screen | yes | no: the mouse can turn forever |
| for | drawing your own pointer, games played with a controller | first-person cameras, mouse-steered games |

### Drawing your own pointer

Hide the real one and draw something at `mousePosition()` at the end of `draw()`, so it's on top of everything:

<!-- check: builds -->

```cpp
#include <window>
#include <input>
#include <graphics2d>

rect button = rect(140, 120, 120, 40);

int main(){
    window.open(400, 300, "Own pointer");
    input.hideCursor();
    window.run();
    return 0;
}

void update(float dt){
    // the real pointer comes back over the window's edge, so it's usable outside the game
    if (input.isMouseInWindow()) {
        input.hideCursor();
    } else {
        input.showCursor();
    }
}

void draw(){
    vec2 m = input.mousePosition();
    bool over = button.contains(m);
    graphics2d.drawRect(button, over ? color.ORANGE : color.GRAY);
    graphics2d.drawText("Play", button.x + 35, button.y + 10, 20, color.BLACK);
    // a crosshair pointer, drawn last
    graphics2d.drawLine(m.x - 8, m.y, m.x + 8, m.y, color.WHITE, 2);
    graphics2d.drawLine(m.x, m.y - 8, m.x, m.y + 8, color.WHITE, 2);
}
```

To keep the normal pointer but show that something can be clicked, change its shape instead: `input.setCursor(over ? Cursor.HAND : Cursor.ARROW);`.

### Looking around with the mouse (first-person)

First-person 3D games lock the mouse and turn the camera by how far it moved. `mouseDelta()` keeps working when the pointer would have hit the edge of the screen. Since Escape is a normal key, it's the usual way to let go of the mouse (to reach the window's X, or a menu), and a click takes it back:

<!-- check: builds -->

```cpp
#include <window>
#include <input>
#include <graphics3d>
#include <math>

float yaw = 0;     // turning left and right, in radians (0 looks along +z)
float pitch = 0;   // looking up and down
vec3 eye = vec3(0, 2, -6);

int main(){
    window.open(960, 540, "Mouse look");
    input.lockMouse(true);
    window.run();
    return 0;
}

void update(float dt){
    if (input.isMouseLocked()) {
        vec2 d = input.mouseDelta();
        yaw -= d.x * 0.0025;
        pitch = math.clamp(pitch - d.y * 0.0025, -1.5, 1.5);
    }
    if (input.keyPressed(Key.ESCAPE)) {
        input.lockMouse(false);          // let go of the mouse
    }
    if (!input.isMouseLocked() and input.mousePressed(MouseButton.LEFT)) {
        input.lockMouse(true);           // clicking the game takes it back
    }
    vec3 look = vec3(math.cos(pitch) * math.sin(yaw), math.sin(pitch), math.cos(pitch) * math.cos(yaw));
    vec3 forward = vec3(math.sin(yaw), 0, math.cos(yaw));
    if (input.keyDown(Key.W)) {
        eye += forward * (5 * dt);
    }
    if (input.keyDown(Key.S)) {
        eye -= forward * (5 * dt);
    }
    graphics3d.setCamera(Camera(eye, eye + look));
}

void draw(){
    graphics3d.drawGrid(20, 1);
    graphics3d.drawCube(vec3(0, 0.5, 0), vec3(1, 1, 1), color.ORANGE);
}
```

[examples/voxel](../../examples/voxel/player.frg) does this in a full game, with walking, jumping and collisions.

## Controllers (gamepads)

Xbox-style controllers (and most others) plugged in by USB or Bluetooth work too. Buttons are named like on an Xbox controller:

| `GamepadButton.` | Button |
|---|---|
| `A` `B` `X` `Y` | the four buttons on the right (on a PlayStation controller: cross, circle, square, triangle) |
| `UP` `DOWN` `LEFT` `RIGHT` | the D-pad (the cross on the left) |
| `LEFT_BUMPER` `RIGHT_BUMPER` | the shoulder buttons |
| `LEFT_TRIGGER` `RIGHT_TRIGGER` | the triggers, as buttons (pressed or not). For how far, use `leftTrigger()` |
| `BACK` `START` | the two small buttons in the middle |
| `LEFT_STICK` `RIGHT_STICK` | pressing a stick down like a button |

| Function | Gives back | What it does |
|---|---|---|
| `input.gamepadDown(GamepadButton button, int gamepad = 0)` | `bool` | true **while** the button is held |
| `input.gamepadPressed(GamepadButton button, int gamepad = 0)` | `bool` | true only in the frame it was pressed |
| `input.gamepadReleased(GamepadButton button, int gamepad = 0)` | `bool` | true only in the frame it was let go |
| `input.leftStick(int gamepad = 0)` | `vec2` | the left stick: x from -1 (left) to 1 (right), y from -1 (up) to 1 (down) |
| `input.rightStick(int gamepad = 0)` | `vec2` | the right stick, the same way |
| `input.leftTrigger(int gamepad = 0)` | `float` | how far the left trigger is pressed, from 0 to 1 |
| `input.rightTrigger(int gamepad = 0)` | `float` | the same for the right trigger |
| `input.gamepadConnected(int gamepad = 0)` | `bool` | true if that controller is plugged in |
| `input.gamepadName(int gamepad = 0)` | `string` | the controller's name, or `""` |

`gamepad` picks the controller: `0` (the default) is the first one plugged in, `1` the second, and so on, for local multiplayer.

Things to know:

- Sticks never rest exactly in the middle, so small movements (less than a fifth of the way) read as exactly `(0, 0)`. That's called a **dead zone**. Past it, the value still goes all the way to 1.
- The stick's `y` points **down**, like the screen, so `position += input.leftStick() * speed * dt;` moves the right way.
- A stick pushed all the way, in any direction, has a length of 1. The d-pad doesn't: up + right is `(1, -1)`, which is about 1.41 long, so diagonals would be faster. When a game mixes the stick with the d-pad (or keys), cap the length instead of always normalizing, so gentle stick pushes stay gentle: `if (move.length() > 1) { move = move.normalized(); }`. [`examples/gamepad`](../../examples/gamepad/main.frg) does this.
- Without a controller, every function just says "not pressed" (`false`, `vec2(0, 0)`, `0`), so a game can always check the keyboard and the controller together:

```cpp
bool jump = input.keyPressed(Key.SPACE) or input.gamepadPressed(GamepadButton.A);

vec2 move = input.leftStick();
if (input.keyDown(Key.LEFT) or input.gamepadDown(GamepadButton.LEFT)) {
    move.x = -1;
}
if (input.keyDown(Key.RIGHT) or input.gamepadDown(GamepadButton.RIGHT)) {
    move.x = 1;
}
player.velocity.x = move.x * MOVE_SPEED;
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

## Limits

- **Only the keys in the [table](#keys) can be read**: letters, the number row, the arrows, Space, Enter, Escape, Backspace, Tab, the Shift / Control / Alt keys and F1-F12. The number pad, punctuation keys (like `,` `.` `-` `=`), Delete, Home, End, Page Up / Down, Insert, Caps Lock and the Windows key can't.
- Typing text is done with [`ui.textBox`](ui.md#typing-text-textbox).
- Up to four controllers (`gamepad` 0 to 3). Their buttons have the Xbox names; controllers can't vibrate.
- No touch screens.
