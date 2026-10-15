# `<save>`: Remembering Things Between Runs

```cpp
#include <save>
```

Keeps values (a high score, the music volume, which level the player reached) after the game closes, so they're still there next time. Each value has a name, called its **key**.

```cpp
#include <stdio>
#include <save>

int main(){
    int runs = save.getInt("runs") + 1;     // 0 the first time, so this is 1
    save.setInt("runs", runs);
    print("You've started this game {runs} times");
    return 0;
}
```

Run it a few times and the number goes up.

## Functions

| Function | What it does |
|---|---|
| `save.setInt(string key, int value)` | saves a whole number |
| `save.getInt(string key, int backup = 0)` | the saved whole number, or `backup` if there isn't one |
| `save.setFloat(string key, float value)` | saves a decimal number |
| `save.getFloat(string key, float backup = 0)` | the saved decimal number, or `backup` |
| `save.setString(string key, string value)` | saves text |
| `save.getString(string key, string backup = "")` | the saved text, or `backup` |
| `save.setBool(string key, bool value)` | saves `true` or `false` |
| `save.getBool(string key, bool backup = false)` | the saved bool, or `backup` |
| `save.has(string key)` | `true` if something is saved under that key |
| `save.remove(string key)` | forgets one value |
| `save.clear()` | forgets everything (for a "Reset save" button) |

The **backup** is what you get when nothing is saved yet, like the very first time the game runs. It's also what you get if the value was saved as a different kind: `getInt` on a key that holds `"hello"` gives the backup.

Saving the same key again replaces the old value.

## Where it's saved

Everything goes into one file, **`save.txt`, in the game's folder**: the folder with `main.frg` while you're making the game, or the folder with the `.exe` in a finished game. Each `set...`, `remove` and `clear` writes the file **straight away**, so nothing is lost if the game crashes or is closed suddenly. (It's a small file, so this is fast.)

The file is plain text, one `key=value` per line, so you can open it to see what's saved:

```
# Forge save file: one key=value per line
best=120
music=true
name=Ana
volume=0.75
```

To start over while testing, delete `save.txt` (or call `save.clear()`).

## Example: a best score

```cpp
#include <save>

int best = save.getInt("best");

void gameOver(int score){
    if (score > best) {
        best = score;
        save.setInt("best", best);
    }
}
```

## Example: settings

```cpp
#include <save>
#include <audio>

void loadSettings(){
    audio.setVolume(save.getFloat("volume", 0.8));    // 0.8 until the player changes it
}

void setVolume(float v){
    audio.setVolume(v);
    save.setFloat("volume", v);
}
```

## Things to know

- Keys can be any text except `""`. Saving with an empty key stops the game with "the key can't be empty text".
- `save` is a library name, so your own functions and variables can't be called `save`. Use something like `saveGame`.
- For your own file formats (a level made of characters, a list of lines), use [`<file>`](file.md) instead.
