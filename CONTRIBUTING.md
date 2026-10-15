# Contributing to Forge

Thanks for looking! A heads-up first: Forge is a personal learning project. I designed the language, an AI (Claude) wrote the code and docs, and I'm using it to learn programming. So replies to issues and pull requests may be slow, and I might say no to changes that don't fit what I want Forge to be. That's nothing personal.

## Reporting a bug

Open an issue on GitHub with:

- the smallest `.frg` program that shows the problem (the whole `main.frg`, if you can);
- what you expected to happen, and what happened instead (paste the full error message);
- your Forge version (`frg version`) and Windows version.

If the message says "this is a bug in Forge, not in your code", please also attach `build/forge_generated.cpp` from your game's folder.

## Suggesting a feature

Open an issue describing what you'd like to write in Forge, with a short example. Check [the roadmap](ROADMAP.md) first: it may already be planned.

## Working on the code

Forge only builds and runs on **Windows**. You need:

- a recent [w64devkit](https://github.com/skeeto/w64devkit), which has g++, CMake and Ninja, with its `bin` folder on your PATH;
- PowerShell 7 (`pwsh`) for the tests. The older Windows PowerShell 5 doesn't work.

Build and test:

```
cmake -S . -B build -G Ninja
cmake --build build
pwsh -File tests\run_tests.ps1
```

`bin\frg.exe` is the compiler, with the `runtime` folder next to it. The [README](README.md#how-it-works) and [How Forge works](docs/how-forge-works.md) explain how the parts fit together.

### The rules for changes

- **Every change comes with tests.** Tests are `.test` files in `tests/cases`; the top of `tests/run_tests.ps1` explains their sections (`=== main.frg`, `=== output`, `=== error`, `=== warning`, `=== build`...).
- **Every change that users can see updates the docs:**
  - the right page in `docs/`;
  - the cheat sheet `docs/LANGUAGE.md`;
  - the teaching brief `docs/TEACH_ME_FORGE.md`.
  Then run `.\docs\make-all-docs.ps1` to rebuild the combined file. The tests check that it's up to date.
- **New library functions** are declared in `src/prelude.cpp` and added to the autocomplete table in `editors/vscode/extension.js`.
- **Errors and docs are written for beginners:** plain words, no jargon, and every error explains the fix in a `hint:`.
- **Keep the design decisions.** Please don't change these:
  - the file-name prefix rule (`player.jump()`);
  - folders joined with `#include <"folder">`;
  - braces are always required, and ` ``` ` marks multi-line comments;
  - whole-number division stays whole-number division;
  - every operator also has a word form;
  - `main` returns an `int`;
  - `<OpenGL>`, `<DirectX>` and `<Vulkan>` are separate toolkits, not engine backends.

All tests must pass before a pull request is merged. GitHub runs them automatically on every push.
