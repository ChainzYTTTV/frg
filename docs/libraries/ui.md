# `<ui>`: Buttons, Checkboxes, Sliders and Text Boxes

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
| `ui.textBox(rect area, string text, int maxLength = 0)` | `string` | draws a box to type in; gives back the **new** text. `maxLength` is the most letters it takes (0 means no limit) |
| `ui.isTyping()` | `bool` | true while the player is typing in a text box |
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

### Typing text: textBox

A text box works the same way: give it the text, and store what it gives back.

```cpp
string playerName = "Player";

void draw(){
    ui.label("Your name:", 360, 170);
    playerName = ui.textBox(rect(360, 195, 240, 40), playerName, 12);    // at most 12 letters
}
```

- **Click** the box to start typing in it. Its border lights up and a cursor blinks at the end.
- Type to add letters. **Backspace** deletes the last one (hold it to keep deleting).
- **Enter**, or a click anywhere else, finishes typing.
- With a `maxLength`, extra letters are simply ignored.
- Only one box takes the typing at a time. Each box is known by its `rect`, so two boxes need different rectangles.
- Text that's too long for the box shows its end, where the typing happens.

While the player types, the keys still reach the rest of the game. If the game uses keys for other things (like R to restart), check `ui.isTyping()` first:

```cpp
if (input.keyPressed(Key.R) and !ui.isTyping()) {
    restart();
}
```

## Look

- Buttons light up when the mouse is over them, and get darker while pressed.
- Text size follows the control's height.
- The default theme is a dark blue background, a light blue accent and white text. Change it with `ui.setTheme(...)`, for example `ui.setTheme(color(40, 30, 30), color.ORANGE, color.WHITE)`.

## Notes

- Controls need the window to be open, and must be drawn every frame to stay visible and clickable.
- Draw controls after the game world, so they appear on top.

See [examples/menu](../../examples/menu/main.frg) for a full menu screen.

## Limits

- `textBox`: typing adds letters at the end and Backspace removes the last one. There's no moving the cursor inside the text, selecting, copying or pasting.
- Controls use the built-in pixel font, and each one goes where its `rect` says: there's no automatic layout and no scrolling lists.
