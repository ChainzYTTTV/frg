# How Forge Works

You don't need any of this to *use* Forge. It's for the curious: what actually happens between your `.frg` files and a running `.exe`, and how to find your way around Forge's own code.

## The big picture

```
your .frg files
      │
      ▼
 1. read files ──── project.cpp: finds main.frg, the files next to it, and #included folders
      │
      ▼
 2. lex ─────────── lexer.cpp:   turns text into tokens (words, numbers, symbols)
      │
      ▼
 3. parse ───────── parser.cpp:  turns tokens into a syntax tree (functions, statements, expressions)
      │
      ▼
 4. check ───────── checker.cpp: works out types and names, and reports errors
      │
      ▼
 5. generate ────── codegen.cpp: writes the program out as C++ (build/forge/)
      │
      ▼
 6. compile ─────── g++ turns that C++, plus Forge's runtime, into build/<game>.exe
```

Steps 1–5 are Forge itself (`frg.exe`, written in C++). Step 6 uses the g++ compiler from w64devkit.

## 1. Reading the project

`frg` starts at `main.frg`, reads every `.frg` file in the same folder, and then follows `#include <"folder">` lines into subfolders. Each file becomes a **module** named after the file, which is why other files reach it as `player.something`. Folders become nested names (`enemies.goblin`).

## 2. Lexing

The **lexer** reads characters and groups them into **tokens**: `int`, `score`, `=`, `10`, `;`. It skips spaces and comments, and keeps track of each token's line and column for error messages. Text with `{interpolation}` is kept whole, and the parts inside the braces are lexed separately later.

## 3. Parsing

The **parser** reads the tokens and builds a **syntax tree**: a structure that captures meaning, like "a variable declaration, type `int`, name `score`, value: the number 10". Expressions follow the order of operations, so `2 + 3 * 4` becomes "add 2 to (multiply 3 by 4)".

The parser recognizes both spellings of every operator (`+` and `plus`) and produces the same tree for both. When it finds a syntax error, it reports it, skips to the end of that statement, and carries on, so several syntax errors can be reported at once.

## 4. Checking

The **checker** is where most of Forge's friendliness lives. It runs in two passes:

- **Pass 1** learns every type, function and global variable in every file. That's why things can be used before they're written, even across files.
- **Pass 2** goes through every function body, working out the type of every expression and making sure everything fits: you can't put a `float` into an `int`, conditions must be `bool`, and every path of a function returns.

Each error comes with a location and, often, a hint. The checker also gives **warnings** for code that works but probably isn't what was meant (a changed copy in a `for ... in` loop, `int / int` going into a float, unused variables, code after `return`, `==` on decimals). Warnings are collected while checking and printed afterwards, in yellow; they're only shown to people when there are no errors, and they never stop the build. "Did you mean...?" suggestions come from comparing the unknown name with known names and picking the closest one.

Built-in types and libraries (`vec2`, `window`, `physics`...) are written in **Forge syntax** inside the compiler (in `prelude.cpp`). The checker reads them like any other code, which is why they follow the same rules and give the same kind of error messages.

## 5. Generating C++

The **code generator** writes the checked program as C++:

| Forge | C++ |
|---|---|
| each file | a C++ namespace: `player.frg` → `m_player` |
| every name you choose | the name plus `_`, so `score` → `score_`. That way nothing can clash with C++ keywords. |
| `struct` | a C++ `struct` (copied by value) |
| `class` | a C++ `struct` shared through `std::shared_ptr` (reference counted) |
| `list<T>`, `map<K, V>` | `std::vector`, `std::map` |
| `T?` | `std::optional<T>` (or a null-able pointer for classes) |
| `task` functions | C++20 coroutines (`co_await`) |
| `"HP: {hp}"` | `::frg::cat({...})`: the pieces joined in order |
| `list[i]` | a checked access that stops with a clear message if `i` is out of range |

Forge's own helper names contain `$`, which Forge names can't, so they never clash with yours either. You can read the result in the `build/forge/` folder.

**Left to right.** Forge promises that the parts of a call (`f(a(), b())`), of an operator (`a() + b()`) and of text with values are worked out left to right. C++ doesn't promise any order for those (g++ often goes right to left), so when two or more parts call something, the generator works them out one by one first, inside a small lambda: `[&]() -> auto { auto&& t0 = a(); auto&& t1 = b(); return f(t0, t1); }()`. Parts that only read values are written as plain C++, as before. Text with values is joined with `::frg::cat({...})`, because C++ always works out a `{ }` list left to right.

## 6. Compiling

The generated C++ is in pieces, in `build/forge/`:

| File | What's in it |
|---|---|
| `game.hpp` | every type, function and global variable's declaration, plus generic types' code (C++ templates). Every part includes it. |
| `game.cpp` | a small game: all the code, and C++'s `main()` |
| `part1.cpp`, `part2.cpp` ... `main.cpp` | a bigger game, split into parts: each part has some of the `.frg` files' code, and `main.cpp` has the global variables and `main()` |

`frg` compiles each part into a `.o` file, then links them into the `.exe`:

```
g++ -std=gnu++20 -O1 -fwrapv -w -I<runtime> -I<cache> -c build/forge/game.cpp -o build/forge/game.o
g++ build/forge/game.o -o game.exe -static
```

**Split builds.** Compiling a part always costs about half a second, just to load Forge's runtime. So a small game is one part, and a game whose C++ is bigger than about 64 KB is split into 2, 4, 8 ... up to 64 parts of about 48 KB each:

- **The parts are compiled at the same time**, one per processor core (fewer if memory is short).
- **Each `.frg` file always goes in the same part.** The part is chosen from the file's name, not its size, so editing one file can't move the others. The number of parts only changes when the game's size passes a power of two.
- **Only changed parts are compiled again.** Each part has a `.key` file saying everything it was made from (Forge, g++, the flags, the declarations and its own code). If that matches, its `.o` is reused. Editing the inside of one function recompiles just its part, and a build where nothing changed doesn't even link.
- **A big game's declarations are precompiled too.** When `game.hpp` is big (over 96 KB), every part would spend time reading it, so it's compiled once together with the runtime into `game_pch.hpp.gch`, in the game's own `build/forge/` folder. It's only made again when a declaration changes (a type, a function's parameters, a global), not when the code inside a function changes.

Global variables all go in one part, because C++ sets them in the order they're written within one file, but in no fixed order between files, and Forge sets them in a fixed order.

- If the game's folder has `icon.png`, Forge wraps the PNG in an `.ico` file (an `.ico` can simply hold a PNG), writes a one-line resource script `GLFW_ICON ICON "icon.ico"`, and runs **windres** (from w64devkit) to turn it into `build/icon_res.o`, which is linked in. That gives the `.exe` its icon; the name `GLFW_ICON` is the one raylib's window library looks for, and the window also loads `icon.png` with raylib's `SetWindowIcon`.
- `-O1` optimizes a little, and compiles fast. `--release` builds use `-O2`, which optimizes more but takes longer.
- **Precompiled headers.** Most of a build's time goes into reading Forge's runtime headers (and raylib's). So the generated file starts with `#include "forge_pch.hpp"`, and Forge compiles that header once into `forge_pch.hpp.gch` in a `cache` folder next to `frg.exe` (or in the game's `build/` folder if that one can't be written). There are two versions: one with the core and the libraries that don't use raylib, and one that adds every raylib library, so all games share them. The toolkits' own `.cpp` files are cached as `.o` files the same way. The cache folder's name comes from everything that affects the result (Forge's version, the runtime files' dates, which g++ is used, and the flags), so it's never stale. Each cache folder also has a `stamp.txt` naming the Forge and g++ that made it; when a new folder is made, folders from an older Forge or g++ are deleted, so the cache doesn't grow after updates. Using a folder touches its `stamp.txt`, and after each build the folders used least recently are deleted until the cache is under about 400 MB (the ones that build used always stay). `frg cache` shows the size, and `frg cache clear` deletes it all. If a cached file is unusable anyway, g++ just reads the real headers, so the cache only changes speed, never behavior.
- `-fwrapv` makes whole numbers wrap around predictably when they overflow.
- `-static` puts everything into the `.exe`, so it runs anywhere without extra files.
- Games that use the engine libraries also link **raylib** (`-lraylib -lopengl32 -lgdi32 -lwinmm`).
- Without a console (`--release` or `--no-console`), it adds `-mwindows` and defines `FRG_NO_CONSOLE`, which makes runtime errors show in a message box. `--release` also adds `-s` (a smaller file).

## The runtime

The **runtime** (`runtime/*.hpp`) is the C++ code every game is built with:

| File | Contains |
|---|---|
| `forge.hpp` | printing, text conversion, lists and maps, optionals, tasks, the core types (`vec2`, `color`...), random |
| `forge_window.hpp` | `<window>`, `<input>`, `<graphics2d>` on top of raylib |
| `forge_graphics3d.hpp`, `forge_assets.hpp`, `forge_audio.hpp`, `forge_ui.hpp`, `forge_particles.hpp` | the other engine libraries |
| `forge_physics.hpp`, `forge_scenes.hpp`, `forge_ecs.hpp`, `forge_time.hpp`, `forge_file.hpp`, `forge_save.hpp`, `forge_tween.hpp` | libraries that don't need raylib |
| `forge_physics_draw.hpp` | `physics.drawDebug()`, which draws with raylib. Forge adds it only to games that use both `<physics>` and `<graphics2d>`, so `<physics>` itself never needs raylib |
| `raylib/` | raylib 5.5 (by Ramon Santamaria and contributors, zlib license) |

## Forge's source code

```
src/
  main.cpp          the frg command: new / build / run / check
  common.hpp        source locations and error reporting
  diagnostics.cpp   prints error messages (and --machine output for editors)
  lexer.cpp         text → tokens
  parser.cpp        tokens → syntax tree
  ast.hpp           what the syntax tree is made of
  project.cpp       loading files and folders
  types.cpp         Forge's types (int, list<T>, classes...)
  prelude.cpp       built-in types and libraries, in Forge syntax
  builtins.cpp      print, math and other functions with special rules
  checker.cpp       type checking and error messages
  checker_util.hpp  small helpers shared by the checker and code generator
  codegen.cpp       syntax tree → C++
runtime/            the C++ runtime (see above)
tests/              .test files and the test runner
editors/vscode/     the VS Code extension
examples/           example games
docs/               these docs
```

## Tests

`tests/cases/*.test` files each hold a small program and what should happen:

```
=== main.frg
#include <stdio>
int main(){ print(1 + 1); return 0; }
=== output
2
```

The other kinds are `=== error` (the build must fail with these messages) and `=== build` (it must compile; it isn't run, for games with windows). `=== new` (with a template name) runs `frg new --template` and builds the result. `=== package` runs `frg package` and lists files that must (or, with `!`, must not) be there afterwards. `=== runtime error` runs the program and expects it to stop with these messages. `=== warning` can be added to any test: each of its lines must appear in the warnings. `=== no warnings` (empty) means there must be none. Run them all with `.\tests\run_tests.ps1` (in PowerShell 7).

## How the VS Code extension finds errors

When you save, the extension runs `frg check <game folder> --machine`. In machine mode, each error is printed as one tab-separated line (`E`, file, line, column, length, message, hint), which the extension turns into red underlines. Warnings are the same, but start with `W` and become yellow underlines.

## Adding a library

1. Write the Forge declarations (functions, and classes with their methods) in `src/prelude.cpp`, as a new entry in `preludeLibs()`.
2. Write the C++ in a new `runtime/forge_<name>.hpp`, in `namespace frg::<name>`. Every function's name is the Forge name plus `_`; classes are `struct Name_` with a static `frg$new(...)`.
3. Rebuild Forge, and add tests and docs.
