# Library Overview

The core language (variables, functions, types, lists...) needs no includes. Everything else lives in a **library** that a file brings in with `#include <name>`:

```cpp
#include <window>
#include <graphics2d>
```

How libraries work:

- **Each file includes the libraries it uses.** If two files both draw, both include `<graphics2d>`.
- **Functions** are reached through the library's name: `window.open(...)`, `math.sqrt(2)`. The `<stdio>` functions (`print`, `write`, `input`) are the exception; they're used on their own.
- **Types** from a library are used by their plain name: `Key.SPACE`, `Body`, `Texture`, `Timer`.
- Using a library without including it is an error that says exactly which `#include` to add.

| Library | One-line summary | Needs a window? |
|---|---|---|
| (built in) [`vec2` `vec3` `vec4` `color` `rect`](core-types.md) | Vectors, colors and rectangles, always available | no |
| [`<stdio>`](stdio.md) | `print`, `write`, `input` for the console | no |
| [`<math>`](math.md) | `sqrt`, `sin`, `cos`, `pow`, `floor`, `abs`, `min`, `max`, `clamp`, `lerp`, `PI` | no |
| [`<random>`](random.md) | Random whole and decimal numbers, chances | no |
| [`<window>`](window.md) | Opens the game window, runs the game loop, frame timing | **is** the window |
| [`<input>`](input.md) | Keyboard, mouse and controllers (gamepads) | yes |
| [`<graphics2d>`](graphics2d.md) | Rectangles, circles, lines, triangles, text | yes |
| [`<graphics3d>`](graphics3d.md) | 3D camera, cubes, spheres, grids, 3D models | yes |
| [`<assets>`](assets.md) | Loading images (`Texture`) and fonts (`Font`) | to draw them |
| [`<audio>`](audio.md) | Sound effects (`Sound`) and music (`Music`) | no |
| [`<physics>`](physics.md) | Box bodies with gravity and collisions | no |
| [`<time>`](time.md) | The clock and date, `sleep`, and the `Timer` class | no |
| [`<save>`](save.md) | Remembering values (high scores, settings) between runs | no |
| [`<file>`](file.md) | Reading and writing text files | no |
| [`<scenes>`](scenes.md) | Splitting a game into scenes (menu, level...) | no |
| [`<ui>`](ui.md) | Buttons, checkboxes, sliders, labels, panels | yes |
| [`<ECS>`](ecs.md) | Entities built from components | no |
| [`<tasks>`](tasks.md) | Running [tasks](../language/11-tasks.md) in programs without a window | no |

"Needs a window" means the library only works after `window.open(...)`. Calling a drawing function earlier stops the game with "the window isn't open yet".

## The built-in engine

`<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>`, `<audio>` and `<ui>` are Forge's built-in game engine. They're built on [raylib](https://www.raylib.com/), which Forge includes, and they all work together: one window, one game loop, and 2D and 3D drawing mixed in one frame.

## Low-level toolkits: OpenGL, DirectX and Vulkan

For advanced users who want to write their own rendering, there are three **separate** toolkits:

| Toolkit | For |
|---|---|
| [`<OpenGL>`](opengl.md) | Drawing with OpenGL 3.3 and GLSL shaders |
| [`<DirectX>`](directx.md) | Drawing with Direct3D 11 and HLSL shaders |
| [`<Vulkan>`](vulkan.md) | Drawing with Vulkan and SPIR-V shaders (compiled from GLSL by `frg build`) |

They are **not** part of the engine, and the engine can't be switched over to use them. A game that includes one makes its own window and does all its own drawing; it can't use `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`, and it can use only one toolkit. Libraries that don't draw (`<math>`, `<audio>`, `<physics>`...) still work. [Low-Level Toolkits](toolkits.md) explains it all, and lists what the three have in common.
