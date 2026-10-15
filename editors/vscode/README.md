# Forge for VS Code

Support for the Forge (`.frg`) game language.

- **Syntax colors**, including `{values}` inside text, `"""raw text"""` and ` ``` ` comments
- **Errors and warnings when you save**: errors are underlined in red and warnings in yellow, with Forge's hint attached
- **Run button** (▶ in the editor's title bar, or **F5**) builds and runs the game. **Ctrl+Shift+B** only builds it.
- **Console on/off**: **Forge: Turn the Console On/Off** (from Ctrl+Shift+P) chooses whether games run with a console window for `print` output and errors. **Forge: Build Release** builds the finished game, which never has one, and **Forge: Package Game** makes it and zips it with everything it needs (`frg package`).
- **Autocomplete and hover help** for the libraries: type `window.`, `graphics2d.`, `color.` or `Key.` (and `OpenGL.`, `DirectX.`, `Vulkan.` for the low-level toolkits)
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
