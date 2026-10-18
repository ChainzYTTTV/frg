# Low-Level Toolkits: OpenGL, DirectX and Vulkan

Forge has three extra libraries for people who want to write their **own** rendering code with a real graphics API:

| Toolkit | Graphics API | Shaders are written in | Page |
|---|---|---|---|
| `<OpenGL>` | OpenGL 3.3 (core profile) | GLSL, as text in the program | [OpenGL](opengl.md) |
| `<DirectX>` | Direct3D 11 | HLSL, as text in the program | [DirectX](directx.md) |
| `<Vulkan>` | Vulkan 1.0 | GLSL files, compiled to SPIR-V by `frg build` | [Vulkan](vulkan.md) |

> **Windows only, for now.** The toolkits open their own windows with Windows' own code, so on Linux a game that includes one gets an error saying so. (DirectX will always be Windows-only: it's Microsoft's.)

> **New to Forge or to making games? You don't need any of these.** Forge's built-in engine (`<window>`, `<graphics2d>`, `<graphics3d>`...) already draws 2D and 3D for you. The toolkits are for learning how graphics cards work, or for drawing things the engine can't.

## Toolkits are not part of the engine

The most important thing to know: **a toolkit is a separate way of making a game, not a setting of the engine.**

- The engine is **not** built on top of these toolkits, and you can't switch the engine over to use one of them. The engine always draws with its own code.
- A game that includes a toolkit **can't use the engine at all**: no `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`. The toolkit opens its own window, and your code does all the drawing: shaders, buffers, matrices, everything.
- A game can use **only one** toolkit.

Both rules are checked when the game is built:

```cpp
#include <window>
#include <OpenGL>     // error: <OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)
```

```cpp
#include <OpenGL>
#include <Vulkan>     // error: a game can only use one of <OpenGL>, <DirectX> and <Vulkan>
```

The rule is per **game**, not per file: if any file of the game includes `<window>`, no file can include a toolkit.

### What still works

Libraries that don't draw work the same with a toolkit as with the engine:

| Works with a toolkit | Doesn't (it's part of the engine) |
|---|---|
| `<stdio>`, `<math>`, `<random>`, `<time>`, `<file>` | `<window>`: use the toolkit's own `openWindow` and `shouldClose` |
| `<audio>`: sounds, and music that keeps playing by itself | `<input>`: the toolkit has its own `keyDown`, `mousePosition`... |
| `<physics>`, `<ECS>`, `<scenes>` | `<graphics2d>`, `<graphics3d>`: you draw with shaders instead |
| [tasks](../language/11-tasks.md) (`wait` moves forward every frame by itself) | `<assets>`: `<OpenGL>` and `<DirectX>` have their own `loadTexture` |
| the core types (`vec2`, `vec3`, `color`...) | `<ui>` |

## What every toolkit has

All three toolkits share the same window, keyboard and mouse functions. They're written here once, with `OpenGL.` in front; use `DirectX.` or `Vulkan.` instead with those toolkits.

### The window

| Function | What it does |
|---|---|
| `openWindow(int width, int height, string title)` | opens the window and starts the graphics API. Call it once, before anything else. |
| `shouldClose()` | call once per frame, at the top of the game loop. It shows the frame you just drew, reads the keyboard and mouse, and gives back `true` when the player clicks the window's X (Escape is a normal key, it doesn't close the window). |
| `closeWindow()` | frees everything and closes the window. It also happens by itself when the program ends. |
| `width()`, `height()` | the window's size in pixels (`int`). The window can't be resized. |
| `deltaTime()` | seconds since the last frame (`float`), for moving things smoothly |
| `time()` | seconds since the window opened (`double`) |
| `setTitle(string title)` | changes the window's title |
| `setVSync(bool on)` | `true` (the start) waits for the screen before showing each frame, so the game runs at the screen's speed (often 60 frames a second) without tearing. `false` draws as fast as it can. |

The game loop always looks like this:

```cpp
OpenGL.openWindow(800, 600, "My game");
while (!OpenGL.shouldClose()) {
    // update the game, then draw the frame
}
OpenGL.closeWindow();
```

There's no `window.run()` with `update` and `draw` functions here: toolkit games always write their own loop.

### Keyboard and mouse

These work the same as in [`<input>`](input.md) and use the same `Key` and `MouseButton` names (`Key.SPACE`, `Key.A`, `MouseButton.LEFT`...):

| Function | Gives back `true`... |
|---|---|
| `keyDown(Key key)` | while the key is held |
| `keyPressed(Key key)` | only on the frame the key went down |
| `keyReleased(Key key)` | only on the frame the key came up |
| `mouseDown(MouseButton button)`, `mousePressed(...)`, `mouseReleased(...)` | the same, for mouse buttons |

| Function | Gives back |
|---|---|
| `mousePosition()` | where the mouse is, in pixels from the window's top-left corner (`vec2`) |
| `mouseWheel()` | how far the wheel turned this frame (`float`, positive is away from you) |

### Matrices

Graphics APIs move and turn things with 4×4 **matrices**. Every toolkit has helpers that make them as a `list<float>` of 16 numbers, column by column, which is the order shaders expect:

| Function | Makes a matrix that... |
|---|---|
| `identity()` | does nothing (the starting point) |
| `translate(vec3 offset)` | moves things by `offset` |
| `rotate(float degrees, vec3 axis)` | turns things around `axis` |
| `scale(vec3 amount)` | stretches things (`vec3(2, 2, 2)` is twice as big) |
| `multiply(list<float> a, list<float> b)` | does `b` first, then `a` |
| `lookAt(vec3 eye, vec3 target, vec3 up)` | a camera at `eye` looking at `target` (the "view" matrix) |
| `perspective(float fovDegrees, float aspect, float near, float far)` | 3D with things further away looking smaller (the "projection" matrix). `aspect` is width / height. |
| `orthographic(float left, float right, float bottom, float top, float near, float far)` | flat projection with no perspective, for 2D |

The helpers are the same in each toolkit, except that each one's `perspective` and `orthographic` fit that API's screen (DirectX and Vulkan put depth from 0 to 1, and Vulkan's screen y points down). So always use the helpers from the toolkit you're using. Then the same numbers give the same picture in all three.

Putting them together, the usual order is `projection × view × model`:

```cpp
list<float> model = OpenGL.multiply(OpenGL.translate(vec3(0, 1, 0)), OpenGL.rotate(angle, vec3.UP));
list<float> view = OpenGL.lookAt(vec3(0, 2, 6), vec3(0, 0, 0), vec3.UP);
list<float> projection = OpenGL.perspective(45, float(OpenGL.width()) / OpenGL.height(), 0.1, 100);
list<float> all = OpenGL.multiply(projection, OpenGL.multiply(view, model));
```

### Handles

Graphics objects (shaders, buffers, textures, pipelines) are given back as an `int` **handle**: a number that stands for the object. Keep it in a variable and pass it to the functions that use the object. A handle of `0` means "nothing".

### Files

Files the toolkit loads (images, Vulkan's `.spv` shaders) are found the same way as [the engine's assets](assets.md#where-files-go): put them in the game's folder, next to `main.frg`.

### When something goes wrong

Mistakes the toolkit can spot stop the game with a clear message, like the rest of Forge:

```
Runtime error in OpenGL.buildProgram(): the vertex shader has an error:
0(4) : error C1008: undefined variable "positon"
```

Some mistakes can't be spotted, though, because graphics APIs trust you to get the details right. The usual sign is a black or empty window. Check, in this order:

1. the vertex data and the sizes you gave for each corner;
2. that the matrices are multiplied in the right order;
3. that the shader's inputs match the buffer's layout.

## Which one should I pick?

- **OpenGL** is the easiest to start with and has by far the most tutorials online (search for "learnopengl"). Its functions are almost exactly the real OpenGL ones, so tutorials translate line by line.
- **DirectX** (Direct3D 11) is what most Windows games used for years. Its ideas are close to OpenGL's, with more set up at the start.
- **Vulkan** is the most modern and the most work: you describe a whole drawing setup (a *pipeline*) at the start, and record each frame's commands. Forge does the longest part, the setup, for you.

Examples of each, all drawing the same spinning cube: [`examples/opengl`](../../examples/opengl/main.frg), [`examples/directx`](../../examples/directx/main.frg), [`examples/vulkan`](../../examples/vulkan/main.frg).
