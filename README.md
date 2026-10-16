# Forge

<img src="art/forge-icon-512.png" width="96" alt="Forge's logo: an orange anvil with sparks" align="right">

[![tests](https://github.com/ChainzYTTTV/frg/actions/workflows/tests.yml/badge.svg)](https://github.com/ChainzYTTTV/frg/actions/workflows/tests.yml) · Version 0.8.1 · [Changelog](CHANGELOG.md) · [Roadmap](ROADMAP.md) · [License: MIT](LICENSE)

> **This project was made with AI.** I designed the language: what it looks like, how files and folders work, and which features it has. The compiler, runtime, tools and docs were written by an AI (Claude), not by me.
>
> I'm not trying to get anyone to use it. I thought a game language built around my own ideas would be cool to see as something real, and I'm using it to learn how programming languages, and programming in general, actually work. If AI-made projects aren't your thing, that's completely fair.

Forge (`.frg`) is a compiled language for making 2D and 3D games. It looks like C++ and aims to read like plain English.

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 0;
}
```

**Documentation:** start at [docs/README.md](docs/README.md), which has a getting-started guide, a chapter for each part of the language, a page for every library, and guides. [docs/LANGUAGE.md](docs/LANGUAGE.md) is a one-page cheat sheet. For VS Code support (colors, errors as you save, a Run button), see [editors/vscode](editors/vscode/README.md).

## What it looks like

| | |
|---|---|
| ![The example platformer, with a camera that follows the player](docs/images/platformer.png) | ![A 3D scene with a camera circling around it](docs/images/world3d.png) |
| The example platformer (`examples/platformer`) | 3D shapes and a moving camera (`examples/world3d`) |
| ![A main menu made with the ui library](docs/images/menu.png) | ![A spinning cube drawn with the Vulkan toolkit](docs/images/vulkan.png) |
| A menu with buttons and sliders (`examples/menu`) | Drawing with Vulkan yourself (`examples/vulkan`) |

## How it works

`frg build` checks your `.frg` files and translates them to C++. It then uses g++ to turn that C++ into a standalone `.exe`. The C++ is an internal step: it's saved as `build/forge_generated.cpp`, but you never need to read it.

```
src/
  main.cpp         the frg command: build / run / new
  lexer.cpp        source text  -> tokens
  parser.cpp       tokens       -> syntax tree
  project.cpp      finds main.frg, the other files and #included folders
  prelude.cpp      built-in types and libraries (vec2, color, window, physics...), written in Forge syntax
  checker.cpp      types, names and the friendly error messages
  codegen.cpp      syntax tree  -> C++
  diagnostics.cpp  error printing
runtime/
  forge.hpp             core runtime: text, lists, maps, optionals, tasks (coroutines), vec2/color/rect, random
  forge_window.hpp      <window>, <input>, <graphics2d> on top of raylib
  forge_graphics3d.hpp  <graphics3d>
  forge_assets.hpp      <assets>: textures and fonts
  forge_audio.hpp       <audio>: sounds and music
  forge_physics.hpp     <physics>
  forge_physics_draw.hpp  physics.drawDebug() (added only when a game uses <physics> and <graphics2d>)
  forge_ui.hpp          <ui>
  forge_scenes.hpp      <scenes>
  forge_ecs.hpp         <ECS>
  forge_time.hpp        <time>
  forge_file.hpp        <file>
  forge_save.hpp        <save>
  forge_tween.hpp       <tween>
  forge_particles.hpp   <particles>
  forge_native.hpp      the plain Windows window shared by the low-level toolkits (code in native_window.inl)
  forge_opengl.hpp      <OpenGL> toolkit (code in forge_opengl_impl.cpp)
  forge_directx.hpp     <DirectX> toolkit (code in forge_directx_impl.cpp)
  forge_vulkan.hpp      <Vulkan> toolkit (code in forge_vulkan_impl.cpp)
  raylib/               raylib 5.5 (zlib license)
  vulkan/               Vulkan-Headers 1.3.296 (Apache 2.0 / MIT license)
  tools/glslang.exe     glslang 16, compiles <Vulkan> shaders to SPIR-V (licenses in tools/glslang-LICENSE.txt)
tests/             .\tests\run_tests.ps1
examples/          hello, tour, shapes, platformer, sprites, menu, world3d, gamepad, opengl, directx, vulkan
```

Games that use the window, graphics, input, assets, audio or UI libraries are linked with raylib. Everything is linked statically, so the `.exe` runs on any Windows PC.

The low-level toolkits (`<OpenGL>`, `<DirectX>`, `<Vulkan>`) are separate from that engine: each opens its own Win32 window, and its `_impl.cpp` file is compiled into the game only when the game uses it. `<Vulkan>` loads `vulkan-1.dll` when the game starts, so building needs no Vulkan SDK.

### Testing games automatically

Graphical games read three environment variables, so they can be tested without a person at the keyboard:

```
FRG_AUTOTEST_FRAMES=90               quit after 90 frames
FRG_AUTOTEST_SHOT=shot.png           ...taking this screenshot first
FRG_AUTOTEST_INPUT=RIGHT:10-60,SPACE:30-31   hold keys during those frames
```

## Building Forge

You need [w64devkit](https://github.com/skeeto/w64devkit) (g++, CMake, Ninja) on your PATH.

```
cmake -S . -B build -G Ninja
cmake --build build
```

This produces `bin/frg.exe`, with `bin/runtime/` next to it. Add `bin` to your PATH to use `frg` from anywhere. Run the tests (they need PowerShell 7, `pwsh`) with:

```
.\tests\run_tests.ps1
```

## License and contributing

Forge is under the [MIT license](LICENSE). Games you make with Forge are yours, and they don't need to include Forge's license. raylib, the Vulkan headers and glslang keep their own licenses (listed in [LICENSE](LICENSE)).

Bug reports and ideas are welcome: see [CONTRIBUTING.md](CONTRIBUTING.md). What changed in each version is in the [changelog](CHANGELOG.md), and what's planned is in the [roadmap](ROADMAP.md).
