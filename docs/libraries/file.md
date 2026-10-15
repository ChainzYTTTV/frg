# `<file>`: Reading and Writing Text Files

```cpp
#include <file>
```

Reads and writes text files, for save games, settings, high scores and level data.

## Where files go

Paths are relative to the folder the game was **started from**:

- with `frg run`, that's the game's top folder (next to `main.frg`);
- when someone double-clicks the `.exe`, it's the folder the `.exe` is in.

Subfolders are fine (`"saves/slot1.txt"`). `write`, `append` and `writeLines` create missing folders by themselves.

## Functions

| Function | Gives back | What it does |
|---|---|---|
| `file.read(string path)` | `string` | the whole file as text. If the file doesn't exist, the game stops with a runtime error, so check with `exists` first. |
| `file.write(string path, string text)` | `bool` | replaces the file's contents with `text` (creating the file if needed). True if it worked. |
| `file.append(string path, string text)` | `bool` | adds `text` to the end of the file (creating it if needed) |
| `file.exists(string path)` | `bool` | true if there's a file there |
| `file.remove(string path)` | `bool` | deletes the file; true if there was one |
| `file.readLines(string path)` | `list<string>` | the file split into lines (without the line breaks) |
| `file.writeLines(string path, list<string> lines)` | `bool` | writes each item as a line |
| `file.files(string folder)` | `list<string>` | the names of the files in a folder (not subfolders), in alphabetical order; empty if the folder doesn't exist |

Windows line endings (`\r\n`) are turned into plain `\n` when reading.

## Saving and loading a high score

```cpp
#include <file>

int loadHighScore(){
    if (!file.exists("highscore.txt")) {
        return 0;
    }
    return int(file.read("highscore.txt").trim());
}

void saveHighScore(int score){
    file.write("highscore.txt", "{score}");
}
```

## Saving several values: one per line as key=value

```cpp
void save(){
    list<string> lines = [
        "level={level}",
        "coins={coins}",
        "name={playerName}"
    ];
    file.writeLines("save.txt", lines);
}

void load(){
    if (!file.exists("save.txt")) {
        return;
    }
    for (string line in file.readLines("save.txt")) {
        list<string> parts = line.split("=");
        if (parts.length != 2) {
            continue;
        }
        switch (parts[0]) {
            case "level":
                level = int(parts[1]);
            case "coins":
                coins = int(parts[1]);
            case "name":
                playerName = parts[1];
        }
    }
}
```

## Notes

- Files are text only (no images or raw bytes).
- `int(text)` stops the game if the text isn't a number. If players might edit your save files, check values before converting.
