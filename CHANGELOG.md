# Changelog

What changed in each version of Forge. Plans for later versions are in the [roadmap](ROADMAP.md).

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
