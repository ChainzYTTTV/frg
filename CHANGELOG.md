# Changelog

What changed in each version of Forge. Plans for later versions are in the [roadmap](ROADMAP.md).

## 0.9.5 (2026-10-18)

3D physics, and the names settled for 1.0. Code written for 0.9.4 that uses an old name gets an error saying what it's called now, and how to change it everywhere at once (Replace in Files, with Match Case and Match Whole Word).

**Language**
- **A library's name only blocks names in the files that `#include` it.** A file without `#include <time>` can have a variable called `time`, and a library added in a later Forge never breaks a game (`network` isn't reserved any more either). Files and folders can be named like libraries too.
- Errors for a name that's taken (a reserved word, or the name of an included library) say to rename it everywhere at once, with Match Case and Match Whole Word.

**Renamed** (the old names give an error that says the new one)
- `Camera` is now **`Camera3D`**, to match `Camera2D`.
- `graphics3d.setCulling` / `isCulling` are now **`setViewCulling`** / **`isViewCulling`**, so they don't share a name with the toolkits' `setCulling(mode)`, which is about triangles' sides.
- `Font.measure` is now **`textWidth`**, like `graphics2d.textWidth`. Both take decimal sizes and give back a `float` (`int w = graphics2d.textWidth(...)` becomes `float w`). `graphics2d.drawText` takes a decimal size too.
- `time.clock()` is now **`time.timeOfDay()`**: it gives the time of day as text, like `"14:05:09"`.

**Engine**
- `math.toRadians(degrees)` and `math.toDegrees(radians)`.
- **`<physics3d>`: 3D physics.** `Body3D` boxes and balls (`sphere = true`) that fall, move, collide, bounce and stack, in the same world as `<graphics3d>` (y up). `stepHeight` lets walking bodies climb stairs. `physics3d.raycast(from, direction, maxDistance)` finds what a line hits (point, surface direction, distance and body), for shooting and line of sight. Collision layers, a fixed-rate `step` (the same at any frame rate), and `physics3d.drawDebug()`. New example: `examples/physics3d`.

## 0.9.4 (2026-10-17)

The engine extras before 1.0 (3D culling and light, balls and ramps in `<physics>`, sprite effects, textures for the low-level toolkits), and an installer.

**Installing and updating**
- **An installer:** `Forge-0.9.4-setup.exe`, below on this release (and one on every release from now on). Everything in one step, for your Windows account only (no admin rights): `frg`, its own C++ compiler (w64devkit), the docs, the examples and the VS Code extension. It can put Forge on the PATH and give `.frg` files their icon; uninstalling undoes everything. Each installer is built and tested by installing it, building a game with it and uninstalling it.
- **An installed Forge uses its own compiler**, whatever else is on the PATH.
- **`frg update`**: checks for a newer Forge and, if you say yes, downloads and starts its installer. Once a day, building or running also notices a new version and says so in one line. Updating is never required; `frg update --off` (or `FRG_NO_UPDATE_CHECK=1`) stops the daily check.
- The VS Code extension also finds an installed Forge before VS Code is restarted.

**Toolkits**
- **Textures in `<Vulkan>`:** `loadTexture(path)`, `bindTexture(texture)` and `setTextureFilter(smooth)`. Every pipeline has room for one texture (`layout(set = 0, binding = 0) uniform sampler2D`).
- **Drawing into a texture** in `<OpenGL>` (framebuffers and renderbuffers: `genFramebuffer`, `bindFramebuffer`, `framebufferTexture2D`, `checkFramebufferStatus`, `texImage2D`...) and `<DirectX>` (`createRenderTexture(width, height)` and `setRenderTarget(texture)`).

**Engine**
- **Sprite effects:** `graphics2d.beginFlash(color, amount)` (the "got hit" flash), `beginOutline(color, thickness)`, `beginGlow(color, size)` and `endEffect()` for textures and animations, without writing a shader.
- **`<physics>`: balls, ramps, bounce and layers.** `circle = true` makes a body a ball; `slope = 1` / `-1` makes a static body a ramp that bodies walk up and down (staying `onGround`); `bounce` (0-1) makes things bounce; every body has a `layer`, and `physics.collide(a, b, false)` makes two layers pass through each other ("bullets don't hit bullets"). `overlaps()` follows the round shapes. The debug view draws balls and ramps. Boxes behave exactly as before.
- **3D light:** `graphics3d.setLight(direction, light, ambient)` lights every shape, mesh and model from one direction (like the sun), with a soft light everywhere, so 3D stops looking flat. `lightOff()` and `isLit()`. Lines and outlines stay unlit; see-through shapes stay see-through.
- **3D culling:** anything entirely outside what the camera sees (behind it, off to a side, too far) isn't drawn, so big 3D worlds only cost what's in view. On by default; it follows `pushTransform`. `graphics3d.setCulling(false)` turns it off, and `graphics3d.culledLastFrame()` counts what was skipped.

## 0.9.3 (2026-10-17)

Found while building a full 3D first-person game in Forge.

**Engine**
- **Games run in their own folder.** `file.read("data/level.txt")`, `<save>` and loading pictures and sounds now find the game's files however it was started (`frg run`, double-clicking `build/game.exe`, or from another folder). Before, double-clicking the `.exe` looked in `build/`.
- `graphics3d.clearDepth()`: 3D drawn after it shows in front of everything drawn before it, for a first-person gun that never sinks into walls.
- `graphics3d.pushTransform(position, axis, degrees, scale)` and `popTransform()`: turn, move and scale everything drawn between them (characters facing where they walk, a gun tilting while it reloads). They nest, and a missing `popTransform` is a clear error.
- `FRG_AUTOTEST_HIDDEN=1` is silent too: a hidden test window plays no sound.
- **The side mouse buttons:** `MouseButton.BACK` (the thumb button nearer you) and `MouseButton.FORWARD`, in the engine and in the OpenGL / DirectX / Vulkan toolkits. Tests can script them as `MOUSE_BACK` and `MOUSE_FORWARD`.

**Language**
- A reserved word where a name goes says so: `int add(int a, int negative)` gives "'negative' is a reserved word in Forge, so it can't be a parameter name" instead of "expected a parameter name". The same for fields, methods and other names.
- A variable named like a function (or a global) in the same file gives one error, on that line. The lines that use the variable no longer each add a misleading "'feed' is a function".

**Tools**
- **Builds stop preparing Forge's runtime over and over.** Each kind of build (normal, `--debug`, `--release`) needs its own prepared runtime of 100-150 MB, and the build cache was capped at 400 MB, so they pushed each other out: switching between kinds of build showed "Preparing Forge's runtime ... (only the first time)" every time. The cache is now kept under about 1 GB, and the message says what it really is: once for each kind of build, and again after Forge is updated.
- `FRG_AUTOTEST_INPUT` can move the pointer (`MOUSE@400/300:10-10`) and the mouse (`TURN@-5/0:20-40`), so tests can click through menus and check mouse-look.

**Docs**
- The docs say which way the 3D directions go: looking along +z, +x is on the left.
- Two more entries in [Error messages](docs/reference/error-messages.md), for the messages above.

## 0.9.2 (2026-10-17)

**Engine**
- **No key closes the window by itself.** Escape is a normal key now (games can use it for a pause menu); the window closes with its X, Alt+F4, `window.close()`, or when `main` ends. Alt+F4 also works in the OpenGL / DirectX / Vulkan toolkits now.
- **The mouse pointer:** `input.hideCursor()`, `showCursor()`, `isCursorHidden()`, `setMousePosition(x, y)`, `setCursor(Cursor.HAND)` (8 shapes) and `isMouseInWindow()`, next to `lockMouse` for first-person cameras.
- `window.close()` in `update` no longer crashes `draw`: `window.run()` stops before drawing. Drawing after the window was closed says so.
- `FRG_AUTOTEST_FRAMES=N` runs exactly N frames (it ran one more). `FRG_AUTOTEST_INPUT` can click the window's X (`CLOSE:5-5`), and `FRG_AUTOTEST_HIDDEN=1` runs without showing the window.

**Language**
- Generic types can name their types when they're made: `Box<int>(5)`, `Box<Button>()`.
- A runtime error in a global variable's starting value (or a static field's) shows its message and line, instead of crashing without one.
- Clearer messages for an extra `}` or `;`, `null` in a type that isn't optional, and `window.run()` after the window was closed.

**Tools**
- `frg help` lists `frg fmt`; `frg check` rejects options it doesn't know.

**Docs and tests**
- A new **Reference** section, whose examples are run by the tests so they're exactly what Forge does: [Syntax](docs/reference/syntax.md), [How Forge behaves, exactly](docs/reference/rules.md), [Commands](docs/reference/commands.md) (everything you type in the terminal) and [Error messages](docs/reference/error-messages.md) (134 of them, each with an example and a fix).
- 24 new tests that really run games (`=== window`): every way a game stops, custom loops, frame order, key timing, the mouse pointer. CI runs them on Linux.

## 0.9.1 (2026-10-16)

**Language**
- **Comparing things that can't be compared is now a Forge error.** `==`, `!=`, `contains`, `indexOf` and `remove` on function values, `Weak` values, tasks, or structs and lists holding them used to pass Forge's checks and then fail with hundreds of lines of C++ errors. Now Forge says which field is the problem and suggests an `equals` method. The same goes for a generic type that compares its `T` (like `items.contains(x)`) when it's made with such a `T`.
- **A function value can be called wherever it comes from:** `actions[0]()`, `handlers["jump"]()`, `makeAdder(10)(5)`. Before, only a variable or a field could be called. Also fixed: a global or `static` function value written in place (`static Function<string()> boss = () { ... };`) failed in the C++ compiler, and calling a `static` function value field didn't work.
- **`Pointer<T>` is removed.** A pointer to a list item, or to a field of an object, could quietly change the wrong memory once the list grew or the object was freed, with no error. To change a caller's number, return the new value (`score = addOne(score);`); class objects are shared, so a function can change one it's given. Using `Pointer` now gives an error that says this. The pointers docs page is now "Enums and optionals" (`docs/language/10-enums-and-optionals.md`).

**Engine**
- **Physics is the same at every frame rate.** `physics.step(dt)` now splits each frame into equal parts of at most 1/240 s. Before, the same jump went about 6.5 pixels (5%) higher at 144 FPS than at 30 FPS; now every frame rate agrees to within half a pixel. Jumps are about 3 pixels higher than before at 60 FPS, because the result is closer to exact. Physics takes about 1.7 times as long as before (still about 2.6 ms for 400 moving bodies).
- `onGround`, `onCeiling` and `onWall` now mean "at any moment during this `physics.step`". With several parts per frame, a wall that stopped a body early in the frame used to be forgotten by the end of it.
- **Meshes.** `graphics3d.makeMesh(positions, uvs, shades, texture)` sends your own triangles to the graphics card once, so thousands of them draw in one call. Mostly see-through pixels of the texture are cut out, so leaves and glass can have holes.
- **Looking around with the mouse.** `input.mouseDelta()` is how far the mouse moved this frame, and `input.lockMouse(true)` hides the pointer and keeps it in the window (`isMouseLocked()` to check).
- `FRG_AUTOTEST_INPUT` can press mouse buttons too: `MOUSE_LEFT`, `MOUSE_RIGHT`, `MOUSE_MIDDLE`.

**Tools**
- **The examples are run on every push, not only built.** A new CI job runs each example for 180 frames with scripted keys and keeps a picture of the last frame. GitHub's Windows machines have no graphics card, so it runs the Windows games on Linux under Wine with software drawing (OpenGL, DirectX and Vulkan all work there). A crash, a runtime error or a hang fails the run.

**Docs**
- `<ECS>` is described honestly: the entities-and-components style (like Unity's GameObjects), not a data-oriented ECS, with when to use plain classes and lists instead. The name stays.
- Fixed claims that didn't match Forge: the roadmap's "now (0.8.0)", "Forge reports all syntax mistakes in one go" (it's the first one in each function, and type mistakes come once the syntax is right), and `physics.step(dt)` once per frame (now true at any frame rate, see above).

**Examples**
- **`examples/voxel`**: a creative-mode block world. Hills, trees, 16 kinds of block from a sprite sheet of 32 x 32 textures, breaking and placing, a hotbar and a creative menu. Faces that touch other blocks are never built, each 16 x 16 x 16 section is one mesh, sections behind the camera aren't drawn, and changing a block rebuilds only its section (about 1.5 ms).

## 0.9.0 (2026-10-16)

**Tools**
- **Split builds.** A big game's C++ is split into parts that compile at the same time, one per processor core, and only the parts with changed files are compiled again. A build with no changes doesn't even link. On 12 threads: a one-file edit in a 1,000-file game takes 4 s (it took 103 s), and a full build 40 s. Small games stay one part, as fast as before.
- **Autocomplete, hover help, go to definition, find all uses, rename and the outline for your own code** in VS Code, through the new `frg query`. Autocomplete works while you type, after a dot too (`player.body.`), and only offers what exists where the cursor is.
- **A debugger.** `frg build --debug` makes a build where every line knows its `.frg` line, and VS Code's **Forge: Debug Game** (F6, or the bug button) runs it in gdb: breakpoints on `.frg` lines, the variables, stepping, and the call stack.
- **`frg fmt`** tidies the layout of `.frg` files (indentation, `int main(){` braces, blank lines) without changing anything else, and VS Code's Format Document uses it.
- **`frg test`** runs the tests in the game's `tests` folder: functions named `test...` that use the new `<testing>` library (`testing.check(h.hp == 15)` shows both values when it fails).

**Runtime**
- **Runtime errors show the calls that led to them**: "called from main.frg:6 (in main)", nearest first.
- `list[i]` on an empty list says "the list is empty".

## 0.8.1 (2026-10-16)

Two bug fixes, found by testing a big game spread across many files and folders.

- **Things now happen left to right.** In a call like `add(next(), next())`, in text like `"{a()} {b()}"`, and on the two sides of operators like `+` and `==`, Forge used to let C++ choose the order, and g++ often went right to left. So code whose parts *do* something (count, put into a list, read input) could quietly give the wrong answer. Now the left part always happens first. Code whose parts only read values didn't change, and isn't any slower.
- **Function values work inside `{ }` in text.** `"{numbers.filter((int n) { return n > 0; })}"` used to fail with "this '{' is never closed".

## 0.8.0 (2026-10-16)

**Language**
- Functions as values: `(Enemy e) { return e.hp <= 0; }`, typed `Function<bool(Enemy)>`. Lists get `removeWhere`, `filter`, `any` and `sortBy`.
- Several top-level functions can share a name when their parameters differ: `drawHealth(int)` and `drawHealth(float)`.
- `static` fields and methods in your own structs and classes: `Enemy.count`, `Enemy.boss()`.
- Interfaces and abstract classes: `interface Attacker { int power(); }`, `abstract class Enemy { abstract int reward(); }`, `class Goblin : Enemy, Attacker`.
- Generic structs and classes: `class Box<T> { T item; }`, used as `Box<int> b = Box(5);`.
- Weak references: `Weak<Player> target = Weak(player);` and `target.get()`, which is `null` once the player is gone.
- `try { ... } catch (Error e) { print(e.message); }` catches every runtime error, and `throw "message";` makes your own. `try`, `catch` and `throw` are now keywords.
- A counting loop: `for (int i in 0..10)` counts 0 to 10, both ends included. `0..list.length` warns.
- `describe` on every struct and class: `g.describe`, `.name`, `.parent`, `.fields`, `.values`. `describe` is now a reserved member name.
- Operators for your own types, with methods named after the operator words: `plus`, `minus`, `times`, `div`, `mod`, `less` (gives `<`, `>`, `<=`, `>=` and `list.sort()`), `equals` and `toText` (what `print` shows). `toString()` still works, with a warning to rename it.

**Docs**
- The designer's brace style, `int main(){`, is used for functions and methods everywhere in the docs and examples.

## 0.7.0 (2026-10-16)

**Language and compiler**
- `do { ... } while (condition);` loops, which always run at least once. `do` is now a keyword.
- `Pointer.remove()` and `isRemoved()`: using a removed pointer stops the game with a clear message instead of crashing.
- Using a reserved word as a variable name (`int do = 5;`) now says so clearly.

**Libraries**
- `<tilemap>`: levels drawn as text, one character per tile. Walls become physics bodies (merged into big boxes), and tiles can be drawn as colors or from a tileset image.
- `<particles>`: an `Emitter` for dust, sparks, smoke and explosions.
- `<tween>`: smooth changes over time, with easing (`LINEAR`, `IN`, `OUT`, `IN_OUT`, `BOUNCE`).
- `ui.textBox` for typing text (like a player's name), and `ui.isTyping()`.

**Tools**
- The build cache is kept under about 400 MB. `frg cache` shows its size, and `frg cache clear` deletes it.

**Examples**
- The platformer's level is now drawn as text, with one-way planks, dust when landing and sparkles when collecting a coin.
- The menu has a name box, and its title drops in with a bounce.

## 0.6.0 (2026-10-15)

**Language and compiler**
- Warnings: yellow messages for code that works but is probably wrong. They never stop the build. Forge warns about:
  - whole-number division going into a decimal;
  - changing a copy in a `for (... in ...)` loop;
  - unused variables;
  - code that can never run;
  - `==` on decimal numbers.
- `"{value:.2}"` shows a number with 2 decimal places.
- `text.isInt()` and `text.isFloat()` check text before converting it.
- `"""raw text"""` that can go over many lines, and `list.addAll(other)`.
- `color(...)` accepts whole numbers, and there's a new `random.color()`.

**Libraries**
- A 2D camera that scrolls and zooms (`Camera2D`).
- `physics.drawDebug()`, and one-way platforms (`Body.oneWay`).
- `<save>`, for remembering values between runs.
- `Animation`, for sprite sheets.
- Gamepad (controller) input.
- Low-level toolkits for writing your own rendering: `<OpenGL>`, `<DirectX>` and `<Vulkan>`. They're separate from the engine.

**Tools**
- Builds are much faster while testing, thanks to a prepared runtime and lighter optimization.
- `--console` / `--no-console`. Without a console, runtime errors pop up in a box.
- `frg package` makes a zip that's ready to share.
- `frg new --template console|window|platformer|3d`.
- An `icon.png` in the game folder becomes the game's icon.
- VS Code: warning squiggles, console on/off, Build Release and Package Game commands, and autocomplete for the toolkits.
- Tests run automatically on GitHub for every push.

**Docs**
- 18 practice projects.
- Pages for every new feature.
- `docs/FORGE_ALL_DOCS.md`: all the docs in one file.

## 0.5.0 (2026-10-15)

**Language**
- Every operator has a word form too: `plus`, `times`, `greater`, `set ... to` and more.
- Tasks: functions returning `task` can `wait()` without freezing the game.
- `is` / `as`, several constructors, and `private`.

**Libraries**
- `<assets>`, `<audio>`, `<time>`, `<file>`, `<scenes>`, `<ui>`, `<graphics3d>` and `<ECS>`.
- Moving physics bodies collide with each other.

**Tools**
- `--release` builds.
- Several syntax errors are reported at once.

**Docs**
- Detailed documentation for every part of the language and every library.
- `TEACH_ME_FORGE.md`, a teaching brief for an AI tutor.

## 0.4.0 (2026-10-14)

The first version:
- the compiler (`frg build`, `run`, `new`, `check`) and the core language;
- `<window>`, `<input>`, `<graphics2d>` and `<physics>`;
- the example platformer;
- the VS Code extension.
