# Forge for VS Code

Support for the Forge (`.frg`) game language.

- **Syntax colors**, including `{values}` inside text, `"""raw text"""` and ` ``` ` comments
- **Errors and warnings when you save**: errors are underlined in red and warnings in yellow, with Forge's hint attached
- **Run button** (▶ in the editor's title bar, or **F5**) builds and runs the game. **Ctrl+Shift+B** only builds it.
- **Console on/off**: **Forge: Turn the Console On/Off** (from Ctrl+Shift+P) chooses whether games run with a console window for `print` output and errors. **Forge: Build Release** builds the finished game, which never has one, and **Forge: Package Game** makes it and zips it with everything it needs (`frg package`).
- **Autocomplete** for your own code and the libraries: your variables, functions, types and files, and after a dot what's inside (`player.`, `enemies.goblin.`, `window.`, `color.`, `Key.`). **Ctrl+Space** shows it any time.
- **Hover help**: what a name is, and how a library function is used
- **Go to definition** (F12 or Ctrl+click), **find all uses** (Shift+F12), **rename everywhere** (F2), and the **Outline** panel
- **Format Document** (Shift+Alt+F) tidies the file with `frg fmt`: indentation, `int main(){` braces, blank lines. Turn on `editor.formatOnSave` to tidy as you save.
- **Debugging**: the bug button (or **F6**) runs the game in the debugger. Put breakpoints on `.frg` lines, pause, look at variables, and go on line by line. It uses gdb (from w64devkit) through Microsoft's C/C++ extension.
- **Snippets**: `main`, `game`, `gameloop`, `class`, `struct`, `enum`, `foreach`, `switch`, `body`, and more

## Settings

| Setting | |
|---|---|
| `forge.path` | Full path to `frg.exe`. Leave it empty to use the one the extension was built with, or the one on your PATH. |
| `forge.console` | Run and build games with a console window (on by default). |
| `forge.checkOnSave` | Look for errors whenever a `.frg` file is saved (on by default). |

## Building it

```
.\editors\vscode\package.ps1 -Install
```

Then reload VS Code.
