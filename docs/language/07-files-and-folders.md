# 7. Files and Folders

Small programs fit in `main.frg`. Bigger games are easier to work with when they're split up: one file for the player, one for enemies, one for the level. Forge makes this simple.

## Files next to main.frg

**Every `.frg` file in the same folder as `main.frg` is part of the game automatically.** There's no need to list or include them.

```
mygame/
  main.frg
  player.frg
  level.frg
```

## The file-name prefix rule

To use something from another file, **start with that file's name and a dot**. This works for functions, global variables and types:

```cpp
// player.frg
int score = 0;

void jump(float height){
    print("jumping {height}");
}

class Player {
    int hp = 100;
}
```

```cpp
// main.frg
#include <stdio>

int main(){
    player.jump(2.5);                         // a function from player.frg
    player.score += 100;                      // a global variable from player.frg
    player.Player p = player.Player();        // a type from player.frg
    print(player.score);
    return 0;
}
```

- Things in the **same** file are used without a prefix.
- The prefix is **always required** for other files, even when there's no confusion. That way, `player.jump()` always tells you exactly where `jump` comes from.
- Forgetting the prefix gives a helpful error: "unknown name 'jump' - 'jump' is in player.frg - use player.jump".
- Because the file name is used like this, nothing else can share that name. With a `player.frg` in the top folder, a variable called `player` is an error.
- Each file has its own names, so two files can both have a function called `update`. They're `player.update()` and `enemy.update()`.

## Folders

A folder's files only join the game when a file **in the folder above it** includes the folder by name, in quotes:

```
mygame/
  main.frg               #include <"enemies">
  enemies/
    goblin.frg           #include <"bosses">
    orc.frg
    bosses/
      dragon.frg
```

```cpp
// main.frg
#include <"enemies">
```

```cpp
// enemies/goblin.frg
#include <"bosses">      // bosses/ is next to goblin.frg, so goblin.frg (or orc.frg) includes it
```

You reach things in folders by their **full path from the top folder**, from anywhere in the game:

```cpp
enemies.goblin.spawn();
enemies.bosses.dragon.roar();
enemies.orc.Orc o = enemies.orc.Orc();
```

The full path is used even between files in the same folder: inside `goblin.frg`, `orc.frg`'s functions are `enemies.orc.something()`.

Rules:

- A folder only needs to be included once, by any one file next to it.
- Include one folder at a time: `#include <"enemies/bosses">` doesn't work. Files inside `enemies` include `bosses` themselves.
- Using a folder that isn't included is an error that tells you which `#include` to add.
- A folder and a file in the same place can't have the same name (`enemy.frg` and `enemy/`).
- File and folder names may only use letters, numbers and `_`, can't start with a number, and can't be a keyword, type name or library name (so not `map.frg`, `class.frg`, `window.frg` or `math.frg`).

## Libraries

Libraries are included with angle brackets and **no** quotes:

```cpp
#include <stdio>
#include <window>
```

**Each file includes the libraries it uses.** If `player.frg` calls `print`, then `player.frg` needs `#include <stdio>`, even if `main.frg` has it too. Forgetting gives: "'print' is part of the stdio library, which this file doesn't include - add #include <stdio> at the top of player.frg".

Library functions keep the library's name: `window.open(...)`, `math.sqrt(2)`. The exceptions are `<stdio>`'s functions (`print`, `write`, `input`), which are used on their own. Types from a library (`Key`, `Body`, `Texture`, ...) are used by their plain name once the library is included.

See the [library list](../libraries/README.md).

## private: keeping things inside a file

Put `private` in front of a function, global variable or type to make it usable **only inside its own file**:

```cpp
// player.frg
private int jumpsUsed = 0;          // only player.frg can see this

private void resetJumps(){          // ...or call this
    jumpsUsed = 0;
}

void land(){                        // other files can call player.land()
    resetJumps();
}
```

From another file, `player.jumpsUsed` is an error: "'jumpsUsed' is private to player.frg". This is useful for helper functions and values that other files shouldn't touch. `public` means the opposite (usable everywhere), which is also what you get with nothing written.

## Where things run

- Global variables in every file are created before `main` starts, in file order (main.frg first, then the others alphabetically, then folders as they're included).
- A global variable's starting value can call functions, even from other files. However, a global's starting value shouldn't depend on another file's global, because that one might not be created yet.

## What's next

[Lists and maps](08-lists-and-maps.md): holding many values in one variable.
