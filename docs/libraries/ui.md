# `<ui>`: Buttons, Checkboxes and Sliders

```cpp
#include <ui>
```

Simple interface controls for menus and settings screens. They're **immediate mode**: you call them every frame in your draw code, and each one both draws itself and tells you what the player did with it.

```cpp
if (ui.button(rect(380, 200, 200, 50), "Play")) {
    scenes.change(Level());        // runs on the frame the button is clicked
}
```

There are no objects to create or remember. Whatever the control controls lives in your own variables.

## Functions

| Function | Gives back | What it does |
|---|---|---|
| `ui.button(rect area, string label)` | `bool` | draws a button; **true on the frame it's clicked** (the mouse button released over it) |
| `ui.checkbox(rect area, string label, bool checked)` | `bool` | draws a box with a label to its right; gives back the **new** checked value |
| `ui.slider(rect area, float value, float min, float max)` | `float` | draws a slider; gives back the **new** value (drag it with the mouse) |
| `ui.label(string text, float x, float y, int size = 20, color c = color.WHITE)` | — | draws text |
| `ui.panel(rect area)` | — | draws a rounded box to put other controls on |
| `ui.setTheme(color background, color accent, color text)` | — | the colors of every control |

### Using checkbox and slider

They give back the new value, so store it back into your variable every frame:

```cpp
bool showFps = true;
float volume = 0.7;

void draw(){
    showFps = ui.checkbox(rect(360, 265, 30, 30), "Show FPS", showFps);
    volume = ui.slider(rect(360, 340, 240, 24), volume, 0, 1);
    ui.label("Volume: {int(volume * 100)}%", 360, 310);
}
```

For a checkbox, the `rect` is the clickable area; the box is a square as tall as the rect, with the label to its right.

## Look

- Buttons light up when the mouse is over them, and get darker while pressed.
- Text size follows the control's height.
- The default theme is a dark blue background, a light blue accent and white text. Change it with `ui.setTheme(...)`, for example `ui.setTheme(color(40, 30, 30), color.ORANGE, color.WHITE)`.

## Notes

- Controls need the window to be open, and must be drawn every frame to stay visible and clickable.
- Draw controls after the game world, so they appear on top.
- There's no text input box yet.

See [examples/menu](../../examples/menu/main.frg) for a full menu screen.
