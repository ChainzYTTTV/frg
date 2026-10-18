# Roadmap

What's planned for Forge between now (0.9.2) and 1.0.0. The order and the version numbers can change. What's already done is in the [changelog](CHANGELOG.md).

Sizes are rough: **S** is small, **M** medium, **L** large.

## 0.7.0: Polish and game-making helpers

| Feature | Why | Size | Status |
|---|---|---|---|
| Levels from text or tile maps (`#` is a wall, `C` a coin, `P` the player) | build levels by drawing them instead of placing every platform in code | M | done (`<tilemap>`) |
| Particles (dust, sparks, explosions) | makes games feel alive with a few lines | M | done (`<particles>`) |
| Tweens: smooth movement over time | menus sliding in, things bouncing and fading | S | done (`<tween>`) |
| A text input box in `<ui>` | typing a name for the high score | M | done (`ui.textBox`) |
| `Pointer.remove()` | throw a pointer away; using it afterwards gives a clear error | S | done |
| `do ... while` loops | some tutorials use them | S | done |
| A size limit for the build cache | it can grow to hundreds of MB | S | done (`frg cache`) |
| Check gamepads and the `.exe` icon on real hardware | the two 0.6 features nobody could test | S | done: both tested on real hardware (an Xbox controller, and `examples/gamepad`'s icon.png) |

## 0.8.0: More power in the language

These belong before 1.0, because they're harder to add once people rely on how Forge works.

| Feature | Why | Size | Status |
|---|---|---|---|
| Functions as values (lambdas) | `list.removeWhere(...)`, `list.sortBy(...)`, buttons that run code, events | L | done |
| Several functions with the same name and different parameters | `drawHealth(int)` and `drawHealth(float)` | M | done |
| `static` fields and methods in your own types | shared counters, helper functions on a class | M | done |
| Interfaces and abstract classes | "every enemy must have `attack()`" | M | done |
| Your own generic types (`class Box<T>`) | reusable containers | L | done |
| Weak references | objects that point at each other get freed | M | done |
| `try` / `catch` | handling errors like a missing file without the game stopping | M | done |
| A short counting loop, `for (int i in 0..10)`, alongside the C-style one | easier for beginners to read | S | done |
| Built-in info on every struct and class, with nothing to write (like Python's `__name__`, `__class__` and `vars()`): the type's name, its field names, each field's value as text, and the type it inherits from | look inside any object while debugging or printing, for free | M | done |
| Operators for your own types, using Forge's operator words as method names (Forge's version of Python's dunder methods): a method `plus(Money other)` makes both `a + b` and `a plus b` work; `less(...)` makes `<` work and lets `list.sort()` sort your own structs; `equals(...)` replaces the automatic `==`; `toText()` chooses what `print` and `"{x}"` show | your types work with the same symbols and words as numbers | M | done |

### Decided for 0.8.0

These were decided by Forge's designer (2026-10-16). Build them this way.

1. **Counting loop.** `for (int i in 0..10)` counts 0, 1, 2 ... 10. **Both ends are included**, because that's what a beginner expects "0 to 10" to mean. It works with variables too (`0..n`).
   - Because lists start at 0, `0..list.length` would go one past the end. When the end of a range is a `.length`, Forge gives a **warning** with the fix: `0..list.length - 1`.
   - The C-style loop stays as it is.

2. **Functions as values** are written like a function without a name:

   ```cpp
   list.removeWhere((Enemy e) { return e.hp <= 0; });
   ```

   How to write the *type* of a function value (for storing one in a variable or parameter) is up to the builder. Keep it close to Forge's existing look, and document it.

3. **Built-in info: `describe`.** Every struct and class gets it, with nothing to write.

   ```cpp
   Goblin g = Goblin();
   print(g.describe);            // Goblin (an Enemy): hp = 30, name = "Grub"
   print(g.describe.name);       // Goblin
   print(g.describe.parent);     // Enemy ("" if it doesn't inherit)
   print(g.describe.fields);     // ["hp", "name"]
   print(g.describe.values);     // {"hp": "30", "name": "Grub"}
   ```

   - On its own, `describe` gives everything as text. A part after it gives only that part.
   - `private` fields are left out.
   - `describe` becomes a reserved member name, so your own field or method can't be called `describe`.

4. **Operators for your own types.** Methods named after Forge's operator words: `plus`, `minus`, `times`, `div`, `mod`, `less`, `equals` and `toText`.
   - `less(T other)` gives all four comparisons: `<` (less), `>` (greater), `<=` (less or equal) and `>=` (greater or equal). It also lets `list.sort()` sort lists of that type.
   - `equals(T other)` gives `==` and `!=` (equals / not equals).
   - `toText()` decides what `print(x)` and `"{x}"` show.
   - The symbol and the word form both work: `a + b` and `a plus b`.

5. **`try` / `catch`.**
   - Every runtime error can be caught: list positions, null, missing files, `int("abc")`, and so on.
   - `try { ... } catch (Error e) { print(e.message); }`
   - Your own code can raise an error with `throw "message";`.

6. **Interfaces and abstract classes** use the keywords `interface` and `abstract`. A class lists them with the same `:` as inheritance: `class Goblin : Enemy, Attacker`.

7. **Weak references** are a type you choose to use. They point at an object without keeping it alive, and `get()` gives `null` once it's gone:

   ```cpp
   Weak<Player> target = Weak(player);
   Player? p = target.get();
   ```

8. **Generics:** `class Box<T>`, with no limits on what `T` can be.

9. **`static`** is the keyword for shared fields and methods.

10. **Brace style** (for `frg fmt` in 0.9.0): the designer's style, `int main(){`, with no space before the brace. Use it everywhere in the docs and examples too.

## 0.9.0: Better tools

| Feature | Why | Size | Status |
|---|---|---|---|
| Split builds: one C++ file per `.frg` file instead of one big one, compiled in parallel on every CPU core, with unchanged files reused from the last build | big games rebuild in seconds after a small change. Measured with one big file: 50 files take 6 s, 500 files 48 s, 1,000 files 103 s, every time | L | done: big games are split into parts (small ones stay one part); a one-file edit in 1,000 files takes 4 s |
| A language server: autocomplete and hover help for your own code, go to definition, rename | the biggest editor upgrade | L | done (`frg query`), plus find all uses and the Outline |
| A debugger: breakpoints in `.frg` files, and seeing variables while paused | debugging without only `print` | L | done (`frg build --debug`, Forge: Debug Game) |
| `frg fmt` | tidies spacing and indentation automatically | M | done |
| Runtime errors show which function calls led there | easier crash fixing | M | done |
| `frg test`: tests for your own games | checking game logic without playing | M | done (`<testing>`, the `tests` folder) |

## 0.9.1: Trustworthy foundations

| Item | Why | Status |
|---|---|---|
| Remove `Pointer<T>` | a pointer to a list item or an object's field could quietly change freed memory | done |
| Physics the same at every frame rate | a jump went 5% higher at 144 FPS than at 30 FPS | done (equal parts of at most 1/240 s) |
| No C++ errors from comparing things that can't be compared | `==`, `contains`... on function values passed Forge's checks, then failed in g++ | done |
| Run every example in CI, not only build it | drawing, input and game code could break without a test failing | done (Linux, Wine, software drawing) |
| Docs that match Forge | `<ECS>` described honestly, and wrong claims fixed | done |
| Call a function value from anywhere: `fs[0]()` | only variables and fields could be called | done |

## 0.9.2: Exact docs, and stopping a game properly

| Item | Why | Status |
|---|---|---|
| No single key closes the window (Escape is a normal key) | a stray key press shouldn't end the game; Escape is for pause menus | done |
| Hide, show, move and shape the mouse pointer | own pointers, menus, mouse-look games | done (`hideCursor`, `setMousePosition`, `setCursor`...) |
| Reference docs whose examples are run by the tests | no guessing: syntax, exact behavior, every command, every error message | done (`docs/reference/`) |
| Tests that run games and check what they do | every way a game stops, custom loops, frame order, key timing | done (`=== window` tests, run in CI) |
| `Box<int>(5)` | the natural way to make a generic object gave a confusing error | done |

## 0.9.3: What a 3D first-person game needed

| Item | Why | Status |
|---|---|---|
| Games find their files however they're started | double-clicking the `.exe` looked in `build/` | done |
| Turn, move and scale drawings (`pushTransform` / `popTransform`) | characters facing where they walk, a tilting gun | done |
| Draw in front of everything (`clearDepth`) | a first-person gun that never sinks into walls | done |
| Scripted mouse movement in tests | tests that click through menus and check mouse-look | done |
| The side mouse buttons (`MouseButton.BACK` / `FORWARD`) | games couldn't use them, so players couldn't bind them | done |
| Clearer errors for reserved words and name clashes | "expected a parameter name" didn't say why; one clash gave many errors | done |

## 0.9.4 and 0.9.5: Engine extras

| Feature | Why | Size |
|---|---|---|
| Circle collisions, slopes and collision layers in `<physics>` | balls, ramps, "bullets don't hit bullets" | done (`circle`, `slope`, `bounce`, `layer`) |
| Automatic 3D culling (skip what the camera can't see) | big 3D worlds only cost what's in view | done |
| Simple 3D lighting | 3D shapes stop looking flat | done (`graphics3d.setLight`) |
| Custom shaders on sprites in the engine (glow, flash white, outlines) | effects without a low-level toolkit | done (`beginFlash`, `beginOutline`, `beginGlow`) |
| Textures in `<Vulkan>`, and drawing into textures in `<OpenGL>` and `<DirectX>` | fills the toolkits' gaps | done |
| `<physics3d>`: 3D boxes and balls, stairs, rays | 3D games without writing their own collision | done (0.9.5) |

## 1.0.0: Ready for other people

1.0 is a promise: games written for 1.0 keep working in every later 1.x version.

| Item | Why | Size |
|---|---|---|
| An installer that includes g++, and gives `.frg` files their icon in File Explorer (`editors/windows/register-frg-files.ps1`) | one-step setup | done (`installer/`, and `frg update`) |
| A final check of every name and rule | they can't change after 1.0 | done (`Camera3D`, `setViewCulling`, `textWidth`, `timeOfDay`; a library's name is only taken in files that include it) |
| **Forge on Linux**: the compiler, the 2D and 3D engine, physics and the other libraries (not the low-level toolkits yet; DirectX is Windows-only) | learning and making games on a Linux laptop | done in 0.9.6 (`Forge-<version>-linux-x64.tar.gz`, and every test runs on Linux) |
| A full review of the docs | the docs are how people learn Forge | M |
| Testing on Windows 10 and 11, on several PCs, and the toolkits on NVIDIA and laptop graphics | no surprises for new users; only one AMD card has run the toolkits so far | S |

## After 1.0 (ideas)

- `<network>` for multiplayer games
- macOS support, and the `<OpenGL>` and `<Vulkan>` toolkits on Linux
- Values scrambled in memory, to make cheating harder
- Exporting games to the web

## Not planned

- `goto`, and a separate `char` type (one-letter strings do the job)
- Hot reload
