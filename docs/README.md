# Forge Documentation

Forge (`.frg`) is a compiled programming language for making 2D and 3D games. It looks like C++, has optional plain-English words for every operator, and its error messages explain what went wrong in everyday language.

These docs are for **anyone using or learning Forge**, including people who have never programmed before. If you're new, read them in order. If you already know another language, skim [Getting Started](getting-started.md), then read [Things that surprise people from other languages](language/14-coming-from-other-languages.md).

> Forge was designed by a person and written by an AI (see the main [README](../README.md)). These docs describe **Forge 0.6**. If something here disagrees with what the compiler does, the compiler is right; please report the mismatch.

## Start here

| | |
|---|---|
| [Getting Started](getting-started.md) | Setup, the `frg` command, VS Code, your first program, and building a finished game |
| [Cheat sheet](LANGUAGE.md) | The whole language on one page, for looking things up quickly |

## The language

Read these in order the first time.

| # | Chapter | What's in it |
|---|---|---|
| 1 | [The basics](language/01-basics.md) | How a program is laid out, `main`, statements, comments, braces, names |
| 2 | [Variables and types](language/02-variables-and-types.md) | Every type, number sizes, `const`, converting between types, how division works |
| 3 | [Text](language/03-text.md) | Strings, putting values inside text, escapes, raw text, every text method |
| 4 | [Operators](language/04-operators.md) | Every operator in symbol and word form, and which ones happen first |
| 5 | [Making decisions and repeating](language/05-control-flow.md) | `if`, `while`, `for`, `for ... in`, `switch`, `break`, `continue` |
| 6 | [Functions](language/06-functions.md) | Writing and calling functions, parameters, default values, `return` |
| 7 | [Files and folders](language/07-files-and-folders.md) | Splitting a game into files, the file-name prefix rule, folders, `#include` |
| 8 | [Lists and maps](language/08-lists-and-maps.md) | `list<T>`, `map<K, V>` and every method they have |
| 9 | [Structs and classes](language/09-structs-and-classes.md) | Your own types: fields, methods, constructors, inheritance, `is`/`as`, `private` |
| 10 | [Enums, optionals and pointers](language/10-enums-optionals-pointers.md) | `enum`, `Type?` and `null`, `Pointer<T>` |
| 11 | [Tasks](language/11-tasks.md) | `task` functions and `wait()`: timed events without freezing the game |
| 12 | [Errors](language/12-errors.md) | How to read error messages, and a list of common ones with fixes |
| 13 | [Reserved words](language/13-reserved-words.md) | Every keyword, built-in name and word operator |
| 14 | [Coming from other languages](language/14-coming-from-other-languages.md) | Where Forge differs from C++, C#, Java, JavaScript and Python |

## The libraries

Everything that isn't the core language lives in a library you `#include`. See the [library overview](libraries/README.md) for a one-line summary of each.

| Library | For |
|---|---|
| [Built-in types](libraries/core-types.md) | `vec2`, `vec3`, `vec4`, `color`, `rect` (always available) |
| [`<stdio>`](libraries/stdio.md) | Printing to and reading from the console |
| [`<math>`](libraries/math.md) | Square roots, trigonometry, rounding, min/max/clamp |
| [`<random>`](libraries/random.md) | Random numbers |
| [`<window>`](libraries/window.md) | Opening the game window and running the game loop |
| [`<input>`](libraries/input.md) | Keyboard and mouse |
| [`<graphics2d>`](libraries/graphics2d.md) | Drawing shapes and text |
| [`<graphics3d>`](libraries/graphics3d.md) | Cameras, 3D shapes and 3D models |
| [`<assets>`](libraries/assets.md) | Loading images and fonts |
| [`<audio>`](libraries/audio.md) | Sound effects and music |
| [`<physics>`](libraries/physics.md) | Gravity and collisions for boxes |
| [`<time>`](libraries/time.md) | The clock, the date, and timers |
| [`<save>`](libraries/save.md) | Remembering values between runs: high scores, settings, progress |
| [`<file>`](libraries/file.md) | Reading and writing text files |
| [`<scenes>`](libraries/scenes.md) | Switching between menu, levels and game over |
| [`<ui>`](libraries/ui.md) | Buttons, checkboxes, sliders |
| [`<ECS>`](libraries/ecs.md) | Building game objects out of components |
| [`<tasks>`](libraries/tasks.md) | Running tasks in programs without a window |

### Low-level toolkits

For advanced users who want to write their own rendering with a real graphics API. These are **separate from the engine**: a game uses either the engine or one toolkit, never both. Start with [Low-Level Toolkits](libraries/toolkits.md).

| Toolkit | For |
|---|---|
| [`<OpenGL>`](libraries/opengl.md) | OpenGL 3.3, with GLSL shaders |
| [`<DirectX>`](libraries/directx.md) | Direct3D 11, with HLSL shaders |
| [`<Vulkan>`](libraries/vulkan.md) | Vulkan, with GLSL shaders compiled to SPIR-V |

## Guides

| | |
|---|---|
| [The game loop](guides/game-loop.md) | How a game runs frame by frame, and `dt` |
| [Sprites and animation](guides/sprites-and-animation.md) | Drawing images, sprite sheets, flipping, animation timing |
| [Platformer walkthrough](guides/platformer-walkthrough.md) | The example platformer, explained line by line |
| [Practice projects](guides/practice-projects.md) | 18 projects with steps and hints, from a guessing game to a 3D solar system |

## Under the hood

| | |
|---|---|
| [How Forge works](how-forge-works.md) | What happens between your `.frg` files and a `.exe`, for the curious |
