# Roadmap

What's planned for Forge between now (0.6.0) and 1.0.0. The order and the version numbers can change. What's already done is in the [changelog](CHANGELOG.md).

Sizes are rough: **S** is small, **M** medium, **L** large.

## 0.7.0: Polish and game-making helpers

| Feature | Why | Size |
|---|---|---|
| Levels from text or tile maps (`#` is a wall, `C` a coin, `P` the player) | build levels by drawing them instead of placing every platform in code | M |
| Particles (dust, sparks, explosions) | makes games feel alive with a few lines | M |
| Tweens: smooth movement over time | menus sliding in, things bouncing and fading | S |
| A text input box in `<ui>` | typing a name for the high score | M |
| `Pointer.remove()` | throw a pointer away; using it afterwards gives a clear error | S |
| `do ... while` loops | some tutorials use them | S |
| A size limit for the build cache | it can grow to hundreds of MB | S |
| Check gamepads and the `.exe` icon on real hardware | the two 0.6 features nobody could test | S |

## 0.8.0: More power in the language

These belong before 1.0, because they're harder to add once people rely on how Forge works.

| Feature | Why | Size |
|---|---|---|
| Functions as values (lambdas) | `list.removeWhere(...)`, `list.sortBy(...)`, buttons that run code, events | L |
| Several functions with the same name and different parameters | `drawHealth(int)` and `drawHealth(float)` | M |
| `static` fields and methods in your own types | shared counters, helper functions on a class | M |
| Interfaces and abstract classes | "every enemy must have `attack()`" | M |
| Your own generic types (`class Box<T>`) | reusable containers | L |
| Weak references | objects that point at each other get freed | M |
| `try` / `catch` | handling errors like a missing file without the game stopping | M |
| Maybe a short counting loop like `for (i in 0..10)`, alongside the C-style one | still to be decided | S |

## 0.9.0: Better tools

| Feature | Why | Size |
|---|---|---|
| A language server: autocomplete and hover help for your own code, go to definition, rename | the biggest editor upgrade | L |
| A debugger: breakpoints in `.frg` files, and seeing variables while paused | debugging without only `print` | L |
| `frg fmt` | tidies spacing and indentation automatically | M |
| Runtime errors show which function calls led there | easier crash fixing | M |
| `frg test`: tests for your own games | checking game logic without playing | M |

## 0.9.x: Engine extras

| Feature | Why | Size |
|---|---|---|
| Circle collisions, slopes and collision layers in `<physics>` | balls, ramps, "bullets don't hit bullets" | M |
| Simple 3D lighting | 3D shapes stop looking flat | M |
| Custom shaders on sprites in the engine (glow, flash white, outlines) | effects without a low-level toolkit | M |
| Textures in `<Vulkan>`, and drawing into textures in `<OpenGL>` and `<DirectX>` | fills the toolkits' gaps | M |
| Test the toolkits on NVIDIA and laptop graphics | only one AMD card has run them so far | S |

## 1.0.0: Ready for other people

1.0 is a promise: games written for 1.0 keep working in every later 1.x version.

| Item | Why | Size |
|---|---|---|
| An installer that includes g++ | one-step setup | M |
| A final check of every name and rule | they can't change after 1.0 | M |
| A full review of the docs and the teaching brief | the docs are how people learn Forge | M |
| Testing on Windows 10 and 11, on several PCs | no surprises for new users | S |

## After 1.0 (ideas)

- `<network>` for multiplayer games
- Linux and macOS support
- Values scrambled in memory, to make cheating harder
- Exporting games to the web

## Not planned

- `goto`, and a separate `char` type (one-letter strings do the job)
- Hot reload
