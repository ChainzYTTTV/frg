# Getting Started

This page gets you from nothing to a running game, and covers every `frg` command.

## What you need

| | Why |
|---|---|
| **Windows 10 or 11** | Forge only builds Windows games for now |
| **Forge** | The installer has everything: `frg`, its own C++ compiler (w64devkit's `g++`, which Forge uses to make `.exe` files), the docs and the examples |
| **VS Code** (optional, recommended) | With the Forge extension: colors, errors as you save, a Run button |

### Installing

1. Get `Forge-<version>-setup.exe` from the [releases page](https://github.com/ChainzYTTTV/frg/releases/latest) and run it. It installs for your Windows account only, so it doesn't need admin rights.
2. Keep the three choices ticked: Forge on your PATH (so `frg` works in any terminal), the Forge icon for `.frg` files, and the VS Code extension (only shown if VS Code is installed).
3. Open a **new** terminal (one opened before installing doesn't know about Forge yet).

It goes into `%LOCALAPPDATA%\Programs\Forge`. To remove it: Windows Settings > Apps > Forge > Uninstall. That also takes it off your PATH and puts `.frg` files back to a plain icon; your games aren't touched.

**Updates** are optional. Once a day `frg` notices when a newer Forge is out and says so in one line after a build. `frg update` installs it if you want; see [frg update](reference/commands.md#frg-update). Running a newer installer over an older Forge updates it in place.

To check that everything is set up, open a terminal and run:

```
frg version
g++ --version
```

The first should print `Forge 0.9.5`. If `frg` isn't found, open a new terminal; if it's still not found, run the installer again with "Add Forge to my PATH" ticked.

### Without the installer

You can also use Forge from a copy of its folder: put its `bin` folder on your PATH, and install [w64devkit](https://github.com/skeeto/w64devkit) with its `bin` folder on your PATH too (Forge uses its `g++`). `g++ --version` should then print a GCC version. An installed Forge always uses the compiler that came with it, whatever else is on the PATH.

### Building Forge from source

You only need this if you're changing Forge itself. From the Forge folder:

```
cmake -S . -B build -G Ninja
cmake --build build
```

That produces `bin/frg.exe`, with `bin/runtime/` next to it. The runtime folder must stay next to `frg.exe`.

## Your first program

```
frg new hello
cd hello
frg run
```

`frg new` makes a folder with one file, `main.frg`:

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 0;
}
```

`frg run` checks your code, compiles it and runs it. You should see:

```
   Compiling hello (1 file)
    Finished build/hello.exe in 1.02s
     Running hello.exe
Hello, World!
```

What each line of the program means:

| Line | Meaning |
|---|---|
| `#include <stdio>` | Use the `stdio` library, which is where `print` comes from |
| `int main(){` | The start of `main`, the function every program begins in. `int` means it gives back a whole number when it finishes. |
| `print("Hello, World!");` | Show some text. Every statement ends with `;`. |
| `return 0;` | Finish `main` and report "everything went fine" (0). Returning 1 means "something went wrong". |
| `}` | The end of `main` |

### Starting from a template

`frg new` can also start you off with a small working game. Add `--template` and one of these:

| Command | You get |
|---|---|
| `frg new mygame` (or `--template console`) | the hello world above: a program that prints text, no window |
| `frg new mygame --template window` | a window with a ball you move with the arrow keys, using `window.run()` with `update(dt)` and `draw()` |
| `frg new mygame --template platformer` | a small platformer: a box that runs and jumps across platforms, with physics, a pit, and a camera that follows it through a level wider than the window |
| `frg new mygame --template 3d` | a 3D scene: a cube and a ball on a grid, and a camera you turn with Left / Right |

Each one is a single `main.frg` with comments, ready to `frg run` and then change into your own game. The game's name (`mygame`) becomes its window title and the name of its `.exe`, so it can only use letters, numbers and `_`, and can't start with a number.

## The `frg` command

Run these in a terminal. `[folder]` is the game's top folder (the one with `main.frg`); leave it out to use the current folder. Every command, option and message is explained in [Commands](reference/commands.md).

| Command | What it does |
|---|---|
| `frg new <name>` | Makes a folder `<name>/` containing a starter `main.frg`. Add `--template window`, `platformer` or `3d` to [start from a small game](#starting-from-a-template) |
| `frg run [folder]` | Checks, compiles and runs the game |
| `frg build [folder]` | Checks and compiles into `build/<folder name>.exe`, without running it |
| `frg check [folder]` | Only looks for errors (fast, makes no `.exe`) |
| `frg fmt [folder or files]` | Tidies the layout of your `.frg` files (see below) |
| `frg test [folder]` | Runs the tests in the game's `tests` folder ([`<testing>`](libraries/testing.md)) |
| `frg package [folder]` | Makes the finished game and zips it, ready to share: `dist/<name>.zip` (see [Sharing your game](#sharing-your-game)) |
| `frg cache` | Shows how big the build cache is ([why there is one](#why-the-first-build-is-slower)). `frg cache clear` deletes it |
| `frg version` | Shows the Forge version |
| `frg help` | Shows this list |

### The console window: on for testing, off for players

A game built with `frg build` or `frg run` opens with a black **console window** next to it. That's where `print` output and error messages appear, which is what you want while you're making the game. Players don't want it, though, so it can be turned off:

| Add this to `build` or `run` | Console | Use it for |
|---|---|---|
| (nothing) | on | testing and debugging |
| `--no-console` | off | trying the game the way players will see it |
| `--release` | off, and the `.exe` is a bit smaller | the finished game you give to people |
| `--release --console` | on | a finished build that still shows what's going on |

```
frg run                  // console on: see print() output and errors
frg run --no-console     // no console
frg build --release      // the finished game, no console
```

Without a console, `print` output isn't shown anywhere. **Runtime errors still are**: they pop up in a small box titled "The game stopped" that says what went wrong, instead of the game just disappearing. A program that never opens a window (a text game) needs its console: build it without one and nothing shows at all. Forge prints a note if you try.

In VS Code, the **Forge: Turn the Console On/Off** command switches the console for the ▶ button, and **Forge: Build Release** builds the finished game.

### Why the first build is slower

The very first time you build a game (and the first time with `--debug`, `--release` or without a console), Forge shows:

```
   Preparing Forge's runtime for this kind of build (once each for normal, --debug and --release builds, and again after Forge is updated)
```

It's getting Forge's own code ready once, so every build after that is quicker, for all your games. It's kept in a `cache` folder next to `frg.exe`. Each kind of build (normal, `--debug`, `--release`, with or without a window) gets its own prepared set of 100-150 MB, and Forge keeps the whole folder under about **1 GB** by deleting the sets that haven't been used for the longest time. When Forge (or w64devkit) is updated, the old sets are deleted by themselves too, so the message shows once more after an update.

```
frg cache          // shows where the cache is and how big it is
frg cache clear    // deletes it; the next build prepares it again
```

`--release` builds take a little longer, because they make the game run as fast as possible. Normal builds skip some of that work: they're quicker to make, and the game behaves exactly the same.

When `frg run` starts your game, the game's **working folder** is the game's top folder. That matters for files your game loads or saves; see [Files](libraries/file.md) and [Assets](libraries/assets.md).

If the game crashes or `main` returns something other than 0, `frg run` says so after the game ends, for example `[hello.exe exited with code 1]`.

## A game's folder

```
mygame/
  main.frg          <- required: the game starts in its main()
  player.frg        <- any other .frg files next to main.frg are part of the game automatically
  enemies/          <- subfolders join the game with #include <"enemies">
    goblin.frg
  player.png        <- images, sounds, fonts and models your game loads
  icon.png          <- optional: the game's icon (see below)
  build/            <- made by frg: mygame.exe, plus the C++ Forge generated (you can ignore it)
  dist/             <- made by frg package: the finished game, ready to share
```

- `main.frg` must be in the **top** folder, not in a subfolder, and it must contain `int main()`.
- File and folder names may only use letters, numbers and `_`, and can't start with a number. They also can't be a Forge keyword or library name, so no `map.frg` or `math.frg`.
- See [Files and folders](language/07-files-and-folders.md) for how files use each other.
- If you keep your game in **git**, don't save the folders Forge makes. Put a file called `.gitignore` in the game's folder with these lines:

  ```
  build/
  dist/
  save.txt
  ```

  (`save.txt` is where [`<save>`](libraries/save.md) keeps your own test saves.)

### The game's icon

Put a picture called **`icon.png`** in the game's top folder (next to `main.frg`), and `frg build` makes it the icon of the `.exe`, which shows in File Explorer and the taskbar. In window games it's the window's icon too. A square picture works best, ideally 256 × 256 pixels. Without an `icon.png`, the game gets Windows' plain program icon.

If `icon.png` isn't really a PNG picture, Forge gives a warning and builds the game without an icon.

## Sharing your game

The easy way is one command:

```
frg package
```

It makes a finished (`--release`) build, then puts the `.exe` and every file the game needs into `dist/<name>/`, and zips that folder into **`dist/<name>.zip`**. Send that zip to people: they unzip it and double-click the `.exe`.

What goes into the package: everything in the game's folder (images, sounds, fonts, models, level files, `icon.png`, sub-folders...), **except** your `.frg` files, the `build/` and `dist/` folders, hidden files and folders (names starting with `.`, like `.git`), shader sources (`.vert`, `.frag` and the others; their compiled `.spv` files are included), and `save.txt`, so players don't start with your test save.

`frg package --console` keeps the console window in the packaged game, for testers who should see errors.

The zip is made with `tar.exe`, which comes with Windows 10 and newer. If it can't be made, the `dist/<name>/` folder is still there, and you can zip it yourself (right-click it, then **Send to → Compressed (zipped) folder**).

### Doing it by hand

The `.exe` in `build/` runs on any Windows PC, with nothing else installed. Everything is built into it.

If your game loads images, fonts, sounds or 3D models, copy them next to the `.exe`, or into the folder above it, keeping the same sub-folders as in your game's folder. Forge looks in both places.

Text files read and written with [`<file>`](libraries/file.md) work differently: they're found relative to the folder the game was **started from**. When someone double-clicks the `.exe`, that's the folder the `.exe` is in.

## VS Code

The Forge extension adds:

| Feature | How |
|---|---|
| Syntax colors | Automatic for `.frg` files |
| Errors while you work | Saving a file checks the whole game; mistakes are underlined in red, with the hint attached. The Problems panel (Ctrl+Shift+M) lists them all. |
| Run the game | The ▶ button at the top-right of the editor, or **F5** |
| Debug the game | The bug button next to ▶, or **F6**: runs the game in the debugger (see below) |
| Build only | **Ctrl+Shift+B** |
| Autocomplete | Suggests what fits where you're typing: your variables, functions, types and files, and after a dot, what's inside (`player.` shows its fields and methods, `enemies.goblin.` what that file has, `window.` the library's functions). **Ctrl+Space** shows the list any time. |
| Hover help | Hold the mouse over a name to see what it is: a variable's type, a function's parameters, a library function's description |
| Go to definition | **F12** (or Ctrl+click) on a name jumps to where it's written, even in another file |
| Find all uses | **Shift+F12** on a name lists everywhere it's used |
| Rename | **F2** on a name renames it everywhere it's used, in every file. Forge says so if the new name isn't allowed. |
| Outline | The Outline panel (and **Ctrl+Shift+O**) lists the file's types, functions and variables |
| Snippets | Type `main`, `game`, `gameloop`, `class`, `struct`, `enum`, `func`, `if`, `for`, `foreach`, `while`, `switch`, `task`, `scene`, `component` or `body`, then press Tab |
| Comments | **Ctrl+/** toggles `//` comments; **Shift+Alt+A** wraps a selection in a ` ``` ` comment |

To install or update it, run `.\editors\vscode\package.ps1 -Install` from the Forge folder, then in VS Code run **Developer: Reload Window**.

Settings: `forge.path` is the full path to `frg.exe` if VS Code can't find it. `forge.checkOnSave` turns checking on save on or off.

Autocomplete, hover help, go to definition and rename all read your game the same way a build does, including what you've typed but not saved yet in the file you're in. Other files are read as they are on disk, so save them first if you've changed them.

### Tidying your code: frg fmt

`frg fmt` tidies the layout of every `.frg` file in the game (or just the files you name), without changing what the code does:

- each line is indented 4 spaces for each `{ }` level it's inside, and in a `switch`, the `case`s go one level in and their code one more;
- a function's or method's opening brace goes straight after the `)`: `int main(){` (an `if`, `while` or `for` keeps its space: `if (x) {`);
- spaces at the ends of lines go, and several blank lines in a row become one.

It never touches what's inside ` ``` ` comments or `"""raw text"""`. A line that carries on a statement (like the second line of a long list) keeps how you lined it up, moved along with the statement. After tidying, Forge reads the file again, and if anything except spaces would change, it leaves the file as it was and says so.

`frg fmt --check` only lists the files that aren't tidy, and `frg fmt main.frg --print` shows the tidied file without saving it. In VS Code, **Format Document** (**Shift+Alt+F**) does the same for the open file; turn on the `editor.formatOnSave` setting to tidy every time you save.

### Debugging: pausing the game to look inside

The **debugger** lets you pause the game on a line and look at every variable, then go on one line at a time. It's the best way to find out why something doesn't do what you expected.

1. Click in the margin just left of a line number in a `.frg` file. A red dot appears: a **breakpoint**.
2. Press **F6** (or the bug button next to ▶). Forge makes a debug build, and the game runs until it reaches a line with a breakpoint, then pauses there.
3. The **Variables** panel on the left shows the variables and their values. Hold the mouse over a name in the code to see its value too. The **Call Stack** shows which function called which, file by file.
4. **F10** runs one line, **F11** goes into the function called on this line, **Shift+F11** comes back out, **F5** carries on to the next breakpoint, and **Shift+F5** stops the game.

Good to know:

- It uses **gdb**, the debugger that comes with w64devkit, through Microsoft's **C/C++ extension** for VS Code. Forge asks you to install the extension the first time if you don't have it.
- In the Variables panel, names have a `_` on the end (`score_`), because that's how Forge names things in the C++ it makes. Numbers, `bool`s and objects show normally. Text shows as C++ details: the letters are in the part called `_M_p`. A class object is a pointer: open it to see its fields.
- A line that calls several functions (like `a() + b()`) can pause twice, because Forge works the calls out one by one there.
- `frg build --debug` makes the same build from a terminal. Debug builds aren't optimized, so they can be slower than normal ones.

### The `.frg` icon in File Explorer

Windows shows `.frg` files as blank pages until it's told about them. To give them Forge's icon (an orange anvil) and make double-clicking one open it in VS Code, run this once from the Forge folder:

```
.\editors\windows\register-frg-files.ps1
```

It only changes settings for your own Windows account. `-Remove` undoes it. The icon file is `art\frg-file.ico` inside the Forge folder, so run it again if you move Forge somewhere else.

## Troubleshooting

| Problem | Fix |
|---|---|
| `frg` is not recognized | Forge's `bin` folder isn't on your PATH. Add it, then open a new terminal. |
| `couldn't find g++` | w64devkit's `bin` folder isn't on your PATH. |
| `there's no main.frg in ...` | You're in the wrong folder, or the file is named differently. `main.frg` must be in the game's top folder. |
| `couldn't replace build\game.exe` | The game is still running. Close it and try again. |
| `the C++ compiler rejected Forge's output` | That's a bug in Forge, not in your code. The message shows where the generated C++ is; please report it. |
| `Forge's runtime files are missing` | The `runtime` folder must be next to `frg.exe`. Rebuild Forge, or copy it back. |
| The game can't find `player.png` | Put the file in the game's top folder (next to `main.frg`), and check the spelling and capitals. |
| VS Code shows old red underlines | Save the file again; they update on every save. |
