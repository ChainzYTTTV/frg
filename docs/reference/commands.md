# Commands: everything `frg` can do

`frg` is the program that turns your `.frg` files into a game. You type its commands in a **terminal** (also called a command line or console). This page lists every command, every option, what each one prints, which files it makes, and what it means when something goes wrong.

The error messages on this page are checked against the real `frg` every time Forge's tests run, so they're exactly what you'll see.

## Where to type commands

- **In VS Code:** open the terminal with **Ctrl+`** (the key above Tab), or the menu *Terminal > New Terminal*. It starts in the folder you have open.
- **In Windows:** open **PowerShell** or **Terminal** from the Start menu.

`frg` has to be on your `PATH` (see [Getting Started](../getting-started.md#what-you-need)). Check with:

```text
frg version
```

which prints `Forge 0.9.2` (or whichever version you have). If it says `frg is not recognized`, the folder with `frg.exe` isn't on your `PATH`.

### Moving between folders

Most commands work on "the game in this folder". Moving around in a terminal:

| Type | What it does |
|---|---|
| `cd mygame` | go into the folder `mygame` |
| `cd ..` | go up one folder |
| `cd D:\games\mygame` | go to that exact folder |
| `ls` (PowerShell) or `dir` | list what's in this folder |
| `pwd` | show which folder you're in |
| Tab | finish a folder or file name you started typing |
| Up arrow | bring back the last command |
| Ctrl+C | stop the program that's running (a game started with `frg run`, for example) |

## All the commands

| Command | What it does |
|---|---|
| [`frg new <name>`](#frg-new) | makes a new game folder with a `main.frg` |
| [`frg build [folder]`](#frg-build) | checks the game and turns it into `build/<name>.exe` |
| [`frg run [folder]`](#frg-run) | builds the game, then runs it |
| [`frg check [folder]`](#frg-check) | only looks for errors (fast, makes no files) |
| [`frg test [folder]`](#frg-test) | runs the tests in the game's `tests` folder |
| [`frg fmt [folders or files]`](#frg-fmt) | tidies the layout of `.frg` files |
| [`frg package [folder]`](#frg-package) | makes a finished build and zips it, ready to share |
| [`frg cache`](#frg-cache) | shows the build cache; `frg cache clear` deletes it |
| [`frg version`](#frg-version-and-frg-help) | shows which Forge this is |
| [`frg help`](#frg-version-and-frg-help) | shows the list of commands |
| [`frg query ...`](#frg-query) | answers questions about the code, for editors like VS Code |

### The `[folder]` part

`[folder]` (in square brackets) means you **can** give it, but don't have to. It's the game's top folder: the one with `main.frg` in it. Without it, `frg` uses the folder the terminal is in. These do the same thing:

```text
cd D:\games\mygame
frg run
```

```text
frg run D:\games\mygame
```

The game's name is its folder's name: a game in `D:\games\space_rocks` becomes `space_rocks.exe`.

---

## frg new

```text
frg new <name> [--template console | window | platformer | 3d]
```

Makes a new folder called `<name>`, with a `main.frg` to start from.

| Template | What `main.frg` holds |
|---|---|
| `console` (the default) | a program that prints `Hello, World!` |
| `window` | a window with a square you move with the arrow keys |
| `platformer` | a small platformer with physics |
| `3d` | a 3D scene with a camera |

What it prints:

```text
     Created mygame/main.frg

To run it:
    cd mygame
    frg run
```

Rules for the name: only letters, numbers and `_`, and it can't start with a number (it becomes the `.exe`'s name). The folder must not exist yet.

<!-- check: command fails new 2fast -->

```text
error: '2fast' can't be the name of a game
  hint: use only letters, numbers and _, and don't start with a number - for example: frg new my_game
```

<!-- check: command fails new x --template windw -->

```text
error: there's no template called 'windw'
  hint: did you mean window? The templates are console, window, platformer and 3d
```

<!-- check: command fails new -->

```text
error: frg new needs a name
  hint: for example: frg new mygame, or frg new mygame --template platformer
```

---

## frg build

```text
frg build [folder] [--release | --debug] [--console | --no-console]
```

Reads every `.frg` file of the game, checks it for mistakes, translates it to C++, and has g++ turn that into `build/<name>.exe`. It doesn't run the game.

What it prints, when it works:

```text
   Compiling mygame (3 files)
    Finished build/mygame.exe in 1.15s
```

- `(3 files)` counts your `.frg` files.
- If nothing changed since the last build, it says `(nothing changed)` and takes a fraction of a second.
- A big game is split into parts that are compiled at the same time; it then says how many had to be compiled again, like `(2 of 8 parts compiled)`.
- The very first build that uses a library also prints `Preparing Forge's runtime for these libraries (only the first time, to make later builds faster)` and takes longer. See [frg cache](#frg-cache).

If the game has mistakes, it prints each one and stops without making an `.exe`:

<!-- check: command fails build {game} -->

```cpp
int main(){
    int score = "ten";
    return 0;
}
```

```text
error: 'score' needs an int, but this is a string
  --> main.frg:2:17
   |
 2 |     int score = "ten";
   |                 ^^^^^

Build failed: 1 error.
```

Every error message is explained in [Error messages](error-messages.md). Warnings (yellow) are printed too, but they never stop the build.

### What a build makes

```text
mygame/
    main.frg
    build/
        mygame.exe          the game
        forge/              the C++ Forge made from your code (you never need to open it)
        tests/              only after frg test: the game with its tests
```

You can delete `build/` whenever you like; the next build makes it again (and takes a little longer).

### The options

| Option | What it changes | Use it for |
|---|---|---|
| (none) | a normal build, with a console window for `print()` and errors | making and testing your game |
| `--release` | optimized more, made smaller, and **no console window** | the finished game you give to players |
| `--debug` | not optimized, and every line knows its `.frg` line | pausing the game in a debugger (VS Code's **Forge: Debug Game**, F6) |
| `--no-console` | no console window | seeing the game the way players will, while still testing |
| `--console` | keep the console window | a `--release` build where you still want to see `print()` |

Rules:

- `--release` and `--debug` can't be used together.
- `--console` and `--no-console` can't be used together.
- `--release` turns the console off unless you also write `--console`.
- Without a console, `print()` output goes nowhere, and a runtime error appears in a message box instead.

How each kind of build is made, for the curious (these are the g++ settings):

| Build | Speed of the game | g++ settings |
|---|---|---|
| normal | fast | `-O1` |
| `--release` | fastest | `-O2 -s` (and `-mwindows`: no console) |
| `--debug` | slow, but the debugger can see everything | `-O0 -g` |

In every build, whole numbers that go past their biggest value wrap around instead of breaking the game (`-fwrapv`): see [How Forge behaves](rules.md#numbers-that-get-too-big).

<!-- check: command fails build --release --debug -->

```text
error: --release and --debug can't both be used
  hint: --debug is for finding mistakes with a debugger, --release for the finished game
```

<!-- check: command fails build --fast -->

```text
error: there's no option called --fast
  hint: the options are --release, --debug, --console and --no-console
```

<!-- check: command fails build nofolder -->

```text
error: the folder 'nofolder' doesn't exist
```

<!-- check: command fails build {game} -->

```cpp
// file: player.frg
void jump(){
}
```

```text
error: there's no main.frg in D:\games\mygame
  hint: every Forge game needs a main.frg in its top folder. Make a new game with: frg new mygame
```

---

## frg run

```text
frg run [folder] [--release | --debug] [--console | --no-console]
```

Builds the game exactly like [frg build](#frg-build) (with the same options), then starts it. If the build fails, the game isn't started.

```text
   Compiling mygame (1 file)
    Finished build/mygame.exe in 0.78s
     Running mygame.exe
Hello, World!
```

- The game runs **in its own folder** (the one with `main.frg`), so `assets.loadTexture("hero.png")` finds `hero.png` there.
- `print()` output shows in the terminal.
- Ctrl+C in the terminal stops the game.
- When the game ends, `frg run` ends too. If `main` returned something other than `0`, it says so:

<!-- check: command fails run {game} -->

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 4;
}
```

```text
Hello, World!

[mygame.exe exited with code 4]
```

`frg run`'s own exit code is the game's exit code, so scripts can tell how the game ended (see [Exit codes](#exit-codes)).

Running the `.exe` yourself (double-clicking `build/mygame.exe`) works too. The game looks for its files in the folder it was started from, then next to the `.exe`, then one folder up (which is the game's folder, since the `.exe` is in `build/`).

---

## frg check

```text
frg check [folder]
```

Looks for mistakes without building anything, so it's much faster than `frg build`. It checks the `tests` folder too. VS Code runs it every time you save.

<!-- check: command check {game} -->

```cpp
int main(){
    return 0;
}
```

```text
     Checked mygame: no errors
```

With warnings it says `no errors, 2 warnings`. With errors it prints them, then `1 error found.`, and its exit code is 1.

<!-- check: command fails check --nope -->

```text
error: there's no option called --nope
  hint: frg check only takes a folder: frg check [folder]
```

`frg check --machine` prints the errors in a form that's easy for programs to read (one per line, with the file, line and column). It's what the VS Code extension uses.

---

## frg test

```text
frg test [folder] [--debug] [--console | --no-console]
```

Builds the game together with its `tests` folder (into `build/tests/<name>_tests.exe`), then runs every function in `tests` whose name starts with `test`. The game's own `main` doesn't run. How to write tests: [`<testing>`](../libraries/testing.md).

<!-- check: command fails test {game} -->

```cpp
int main(){
    return 0;
}
```

```cpp
// file: tests/math_test.frg
#include <testing>

void testAdding(){
    testing.check(1 + 1 == 2);
}

void testBroken(){
    int hp = 10;
    testing.check(hp - 3 == 8);
}
```

```text
   Compiling mygame (2 files)
     Testing mygame_tests.exe
  PASS  testAdding  (tests/math_test.frg)
  FAIL  testBroken  (tests/math_test.frg)
        tests/math_test.frg:10: testing.check(hp - 3 == 8);
          left side: 7, right side: 8

1 test passed, 1 failed (2 checks).
```

The exit code is 0 when every test passed, and 1 when any failed.

<!-- check: command fails test {game} -->

```cpp
int main(){
    return 0;
}
```

```text
error: there's no tests folder in D:\games\mygame
  hint: make a folder called tests next to main.frg, with files like tests/player_test.frg holding functions like void testJump(){ ... } that use testing.check(...) (they need #include <testing>)
```

---

## frg fmt

```text
frg fmt [folders or files] [--check | --print]
```

Tidies the **layout** of `.frg` files: indentation (4 spaces), `int main(){` braces, spaces, and blank lines. It never changes what the code does: it compares the code before and after, word by word, and refuses to save if anything but spacing would change. VS Code's **Format Document** (Shift+Alt+F) uses it.

| How you run it | What it does |
|---|---|
| `frg fmt` | tidies every `.frg` file in this folder and the folders inside it (not `build/`, `dist/` or hidden folders) |
| `frg fmt player.frg enemies` | tidies those files, and every `.frg` file in those folders |
| `frg fmt --check` | changes nothing; lists the files that aren't tidy. Exit code 1 if there are any |
| `frg fmt --print main.frg` | changes nothing; prints the tidied file |

What it prints:

```text
      Tidied player.frg
```

or, when everything is already tidy:

<!-- check: command fmt --check {game} -->

```cpp
int main(){
    return 0;
}
```

```text
Tidy 1 file is already tidy
```

A file that can't be read as Forge (a `{` that's never closed, for example) is left alone, with an error saying why.

---

## frg package

```text
frg package [folder] [--console]
```

Makes the finished game, ready to give to players:

1. a `--release` build (no console, unless you add `--console`);
2. a folder `dist/<name>/` with the `.exe` and **every file the game might load** (images, sounds, levels, `icon.png`...), keeping their folders;
3. `dist/<name>.zip` of that folder.

```text
      Copied mygame.exe and 12 files into dist/mygame/
    Packaged dist/mygame.zip

Your game is ready to share:
    D:\games\mygame\dist\mygame.zip
Players unzip it and double-click mygame.exe.
```

Left out of the package: `.frg` files, the `build/` and `dist/` folders, hidden files and folders (like `.git`), shader source files (`.vert`, `.frag` and so on; their compiled `.spv` files are included), and `save.txt` in the top folder, so players don't start with your saved game.

Making the `.zip` uses `tar.exe`, which comes with Windows 10 and newer. If it's missing, the folder is still made, and you can zip it yourself.

---

## frg cache

```text
frg cache
frg cache clear
```

The first build that uses a set of libraries prepares Forge's runtime for them, and keeps the result in the **build cache** (`bin\cache` next to `frg.exe`), so later builds are fast. Forge keeps it under about 400 MB by deleting the sets that haven't been used for the longest.

```text
       Cache D:\forge\bin\cache
    358 MB in 4 sets (Forge keeps it under about 400 MB)
    It makes builds faster. To delete it: frg cache clear
```

`frg cache clear` deletes it all. Nothing breaks: the next build just prepares it again. Use it if you need the disk space, or if builds act strangely after updating Forge.

<!-- check: command fails cache tidy -->

```text
error: frg cache doesn't know 'tidy'
  hint: use frg cache to see how big the build cache is, or frg cache clear to delete it
```

---

## frg version and frg help

`frg version` (or `frg --version`) prints the version, like `Forge 0.9.2`.

`frg help` (or `frg --help`, `frg -h`, or `frg` on its own) prints the list of commands and options.

<!-- check: command fails launch -->

```text
error: unknown command 'launch'
  hint: try frg build, frg run, or frg new <name>
```

---

## frg query

```text
frg query <question> <folder> <file> <line> <column> [--stdin] [new name]
```

For editors, not for typing yourself: it's how VS Code's autocomplete, hover help, go to definition, find all uses, rename and outline work. `<question>` is one of `definition`, `references`, `hover`, `complete`, `rename` and `symbols`, and the answer is printed as JSON. With `--stdin`, the file's text is read from standard input (the editor's unsaved version).

---

## Exit codes

Every program gives back a number when it ends, its **exit code**: 0 means it went well. Scripts and editors use it. In PowerShell, `$LASTEXITCODE` shows the last one.

| Command | Exit code |
|---|---|
| `frg build`, `frg check`, `frg new`, `frg package`, `frg cache` | 0 if it worked, 1 if it didn't |
| `frg run` | the game's own exit code (what `main` returned), or 1 if the build failed |
| `frg test` | 0 if every test passed, 1 if any failed or the build failed |
| `frg fmt` | 0 if it worked; with `--check`, 1 if any file isn't tidy |
| a game | what `main` returned; 1 if it stopped with a runtime error |

A game that crashed (instead of stopping with a Forge runtime error) shows a large number in hex, like `[mygame.exe crashed (code 0xC0000005)]`. That's a bug in Forge, not in your game: please report it.

## Environment variables

Settings you can give `frg` or a game for one run. In PowerShell, set one and run, then remove it:

```text
$env:FRG_VERBOSE = 1; frg build; Remove-Item Env:FRG_VERBOSE
```

| Variable | For | What it does |
|---|---|---|
| `FRG_VERBOSE=1` | `frg build` | shows how long each step of the build took |
| `FRG_AUTOTEST_FRAMES=N` | a game with a window | runs the game for N frames, then closes the window |
| `FRG_AUTOTEST_SHOT=name.png` | a game with a window | the picture saved of frame N (normally `autotest.png`) |
| `FRG_AUTOTEST_INPUT=...` | a game with a window | pretends keys and buttons are held during certain frames (below) |
| `FRG_AUTOTEST_HIDDEN=1` | a game with a window | runs without showing the window, and without sound (for tests that check what the game prints) |

### Scripted input

`FRG_AUTOTEST_INPUT` is a list of `NAME:first-last`, separated by commas. Each one holds that key or button down from frame `first` to frame `last` (both included). Frames count from 0.

```text
$env:FRG_AUTOTEST_FRAMES = 120
$env:FRG_AUTOTEST_INPUT = "RIGHT:10-60,SPACE:30-31,MOUSE_LEFT:50-50,CLOSE:100-100"
.\build\mygame.exe
```

| Name | What it pretends |
|---|---|
| `A` ... `Z`, `NUM0` ... `NUM9`, `SPACE`, `ENTER`, `ESCAPE`, `LEFT`, `F1` ... (every [`Key`](../libraries/input.md#keyboard) name) | that key |
| `MOUSE_LEFT`, `MOUSE_RIGHT`, `MOUSE_MIDDLE` | that mouse button |
| `CLOSE` | the player clicking the window's X: `window.shouldClose()` is `true` from that frame |
| `MOUSE@x/y` | the pointer at (x, y) in the window, from the first frame of the range on (`MOUSE@400/300:10-10`). Once any `MOUSE@` is given, the real mouse is ignored |
| `TURN@dx/dy` | the mouse moving by (dx, dy) on each frame of the range: what `input.mouseDelta()` gives, for testing mouse-look (`TURN@-5/0:20-40` turns left). On other frames it's (0, 0) |

While a key is scripted, the real key is ignored. Pressing (`keyPressed`) happens on the first frame of each range, and letting go (`keyReleased`) on the frame after the last. This is how Forge's own tests play games: see [Testing a game that opens a window](rules.md#testing-a-game-that-opens-a-window).

## The same things in VS Code

With a `.frg` file open, the Forge extension runs these commands for you. Open the command list with **Ctrl+Shift+P** and type "Forge":

| VS Code | Key | Runs |
|---|---|---|
| Forge: Run Game | F5 (or the ▶ button) | `frg run` |
| Forge: Debug Game | F6 | `frg build --debug`, then starts the debugger |
| Forge: Build Game | Ctrl+Shift+B | `frg build` |
| Forge: Build Release | | `frg build --release` |
| Forge: Check for Errors | (and every time you save) | `frg check --machine` |
| Forge: Run Tests | | `frg test` |
| Forge: Package Game | | `frg package` |
| Forge: Turn the Console On/Off | | adds or removes `--no-console` |
| Format Document | Shift+Alt+F | `frg fmt --stdin` |

The settings (File > Preferences > Settings, search "forge"): `forge.path` (where `frg.exe` is, if VS Code can't find it), `forge.console` (show the console window, on by default) and `forge.checkOnSave` (check for errors on save, on by default).

## Scripts for working on Forge itself

Only needed if you change Forge, not to make games. Run them from Forge's own folder, in PowerShell 7 (`pwsh`):

| Script | What it does |
|---|---|
| `cmake -S . -B build -G Ninja` then `cmake --build build` | builds `bin\frg.exe` from `src\` |
| `.\tests\run_tests.ps1` | runs every test in `tests\cases`, then checks the examples in the docs. `.\tests\run_tests.ps1 strings` runs just `strings.test` |
| `.\tests\check_doc_examples.ps1` | checks only the docs' examples (the `<!-- check: ... -->` ones). `-Page docs\reference\rules.md` checks one page |
| `.\docs\make-all-docs.ps1` | rebuilds `docs\FORGE_ALL_DOCS.md` from every docs page |
| `.\editors\vscode\package.ps1 -Install` | builds the VS Code extension and installs it |
| `.\editors\windows\register-frg-files.ps1` | gives `.frg` files Forge's icon in File Explorer |
