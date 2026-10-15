# 13. Reserved Words

These words have special meanings, so they can't be used as names for variables, functions, types, fields, files or folders.

## Keywords

| Group | Words |
|---|---|
| Control flow | `if` `else` `while` `for` `in` `switch` `case` `default` `return` `break` `continue` |
| Values | `true` `false` `null` `this` `super` |
| Declarations | `struct` `class` `enum` `const` `void` |
| Modifiers | `private` `public` `static` (built-in libraries only) |
| Logic | `and` `or` `not` |
| Classes | `is` `as` |
| Word operators | `negative` `bitnot` |
| Saved for the future | `new` `delete` |

## Built-in type names

`void` `bool` `string` `task` `int` `int8` `int16` `int32` `int64` `int128` `uint` `uint8` `uint16` `uint32` `uint64` `uint128` `float` `double` `list` `map` `Pointer` `vec2` `vec3` `vec4` `color` `rect`

## Built-in functions

`print` `write` `input` (from `<stdio>`), and `wait` (inside tasks).

## Library names

`stdio` `math` `random` `window` `input` `graphics2d` `graphics3d` `assets` `audio` `physics` `time` `file` `scenes` `ui` `ECS` `tasks` `OpenGL` `DirectX` `Vulkan`, plus `network`, which is planned.

Types that come from a library (`Key`, `MouseButton`, `Body`, `Texture`, `Font`, `Sound`, `Music`, `Camera`, `Model`, `Timer`, `Scene`, `Entity`, `Component`) can't be reused as names of your own types **in a file that includes that library**.

## Your own top-level files and folders

If the game has `player.frg` or an `enemies/` folder in its top folder, then `player` and `enemies` can't be used as names anywhere in the game, because `player.something` always means the file.

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
