# 13. Reserved Words

These words have special meanings, so they can't be used as names for variables, functions, types, fields, files or folders.

## Keywords

| Group | Words |
|---|---|
| Control flow | `if` `else` `while` `do` `for` `in` `switch` `case` `default` `return` `break` `continue` `try` `catch` `throw` |
| Values | `true` `false` `null` `this` `super` |
| Declarations | `struct` `class` `enum` `interface` `const` `void` |
| Modifiers | `private` `public` `static` `abstract` |
| Logic | `and` `or` `not` |
| Classes | `is` `as` |
| Word operators | `negative` `bitnot` |
| Saved for the future | `new` `delete` |

## Built-in type names

`void` `bool` `string` `task` `int` `int8` `int16` `int32` `int64` `int128` `uint` `uint8` `uint16` `uint32` `uint64` `uint128` `float` `double` `list` `map` `Function` `Weak` `vec2` `vec3` `vec4` `color` `rect` `Error`

## Built-in member names

`describe` is built into every struct and class (`obj.describe`), so no field or method can be called that.

## Built-in functions

`print` `write` `input` (from `<stdio>`), `wait` (inside tasks), and `Weak(...)`.

## Library names

`stdio` `math` `random` `window` `input` `graphics2d` `graphics3d` `assets` `audio` `physics` `physics3d` `time` `file` `save` `tween` `particles` `tilemap` `scenes` `ui` `ECS` `tasks` `testing` `OpenGL` `DirectX` `Vulkan`

A library's name can't be the name of something of yours **in a file that `#include`s that library**, since `time.now()` has to mean the library there. In every other file it's free: a file without `#include <time>` can have a variable called `time`. That's also why a library added in a later Forge never breaks a game: only files that include it are affected.

Types that come from a library (`Key`, `MouseButton`, `GamepadButton`, `Body`, `Texture`, `Animation`, `Font`, `Sound`, `Music`, `Camera3D`, `Camera2D`, `Model`, `Timer`, `Tween`, `Ease`, `Emitter`, `TileMap`, `Scene`, `Entity`, `Component`) can't be reused as names of your own types **in a file that includes that library**.

## Your own top-level files and folders

If the game has `player.frg` or an `enemies/` folder in its top folder, then `player` and `enemies` can't be used as names anywhere in the game, because `player.something` always means the file.

## When a newer Forge takes a name you used

Before Forge 1.0, a few names were changed (like `Camera` to `Camera3D`), and later Forges may add keywords. Either way, the error says exactly what happened, and, when something was renamed, what it's called now:

```text
error: unknown type 'Camera'
  hint: it's called Camera3D since Forge 0.9.5 - change 'Camera' to 'Camera3D' everywhere at once: in VS Code, Replace in Files (Ctrl+Shift+H), with Match Case (Alt+C) and Match Whole Word (Alt+W) turned on, so only this exact name changes
```

Change it **everywhere at once** rather than one line at a time: in VS Code, **Replace in Files** (Ctrl+Shift+H), type the old name and the new one, and turn on **Match Case** (Alt+C) and **Match Whole Word** (Alt+W) before replacing. Match Case keeps `camera` (a variable) from turning into `camera3D`; Match Whole Word keeps `myCamera` and `CameraShake` as they are. Then look over the list of matches before **Replace All**: words inside quotes match too, so text a player sees, like `"Bulletproof Camera"`, would change as well. Click the **×** next to a match to leave it alone. For a name of your own that's now taken, pick a new name and do the same.

## Word operators that are *not* reserved

These words are only operators in specific places, so they can still be used as names:

| Words | Count as operators when |
|---|---|
| `plus` `minus` `times` `div` `mod` `equals` `greater` `less` `otherwise` `bitand` `bitor` `bitxor` `shiftleft` `shiftright` | they come right after a value: `a plus b` |
| `equal` | right after `greater or` / `less or` |
| `set` ... `to`, `increase` / `decrease` ... `by`, `multiply` / `divide` ... `by` | at the start of a statement with the matching `to` / `by` later on |
| `increase x;` / `decrease x;` | at the start of a statement that ends right after the name |

For example, this is fine, though not recommended:

```cpp
int times = 3;
int set = 2;
print(times times set);    // 6
```

See [Operators](04-operators.md) for what they all do.

## Names Forge uses internally

Forge's generated code adds `_` to the end of every name you write, and uses `$` in its own names, so nothing you name can clash with C++ or with Forge's runtime. You never see these unless you open the generated C++.
