# 3. Text

Text is called a **string** in programming: a "string" of characters. In Forge, the type is `string`, and text values are written between double quotes.

```cpp
string name = "Hero";
string empty = "";
```

Single quotes don't work: `'hi'` is an error. Text also has to start and end on the same line.

## Putting values inside text

Anything inside `{ }` in a string is worked out and turned into text. This is called **interpolation**:

```cpp
int hp = 15;
int maxHp = 20;
print("HP: {hp}/{maxHp}");                   // HP: 15/20
print("Half: {maxHp / 2}");                  // Half: 10
print("Alive? {hp > 0}");                     // Alive? true
print("Name: {name.upper()}");               // Name: HERO
print("Has an e? {name.contains("e")}");     // quotes inside { } are fine
```

Any expression works inside `{ }`, and any type can be shown: numbers, bools, lists, maps, structs, classes, enums and optionals. See [How values look as text](#how-values-look-as-text) below.

### Joining text with +

`+` joins two strings:

```cpp
string full = "Game" + " " + "Over";     // "Game Over"
full += "!";                             // "Game Over!"
```

`+` only works between two strings. `"Score: " + 5` is an error, because Forge won't guess how you want the number written. Use `{ }` instead: `"Score: {5}"`.

## Escapes: special characters

A backslash `\` inside a string starts an **escape**:

| Write | You get |
|---|---|
| `\n` | a new line |
| `\t` | a tab |
| `\"` | a double quote `"` |
| `\\` | a backslash `\` |
| `\{` or `{{` | a `{` (instead of starting a `{value}`) |
| `\}` or `}}` | a `}` |
| `\'` | a single quote |
| `\r` | a carriage return (rarely needed) |

```cpp
print("Line one\nLine two");
print("She said \"hi\"");
print("C:\\Games\\forge");          // C:\Games\forge
print("Use {{braces}} like this");   // Use {braces} like this
```

Any other letter after `\` is an error.

## Raw text

Normal text has to start and end on the same line. For longer text, write it between **three** double quotes, `"""`. This is called **raw text**:

```cpp
string HELP = """
Arrow keys: move
Space: jump
Escape: quit
""";
print(HELP);
```

Raw text is used exactly as written:

- **It can go over many lines.** A line break straight after the opening `"""` is left out, so the text above starts with `Arrow keys`. The line break before the closing `"""` is kept.
- **Escapes don't work.** `\n` stays as a backslash and an `n`. `{ }` doesn't put values in: `"""{score}"""` is the text `{score}`.
- **It can contain `"` and `\`.** The only thing it can't contain is `"""`, which ends it.

That makes raw text the way to write things like shader code for [`<OpenGL>`](../libraries/opengl.md) and [`<DirectX>`](../libraries/directx.md), maps made of characters, or Windows paths:

```cpp
string LEVEL = """
##########
#..P....$#
##########
""";
string folder = """C:\Games\forge""";
```

To put values into longer text, add them on with `+`: `"""Score: """ + "{score}"`.

## Comparing text

| | Example | |
|---|---|---|
| Equal | `name == "Hero"` (or `name equals "Hero"`) | Capitals matter: `"Hero" == "hero"` is false |
| Not equal | `name != "Hero"` | |
| Alphabetical order | `"apple" < "banana"` | Compares character by character; capital letters come before lowercase ones |

To compare without caring about capitals: `a.lower() == b.lower()`.

## Text methods

A **method** is a function that belongs to a value, called with a dot: `name.upper()`. Text methods never change the original string; they give back a new one.

| Method | Gives back | What it does | Example | Result |
|---|---|---|---|---|
| `.length` | `int` | How many characters (bytes) there are. No `()`. | `"Hero".length` | `4` |
| `isEmpty()` | `bool` | True if it's `""` | `"".isEmpty()` | `true` |
| `upper()` | `string` | All capitals | `"Hero".upper()` | `"HERO"` |
| `lower()` | `string` | All lowercase | `"Hero".lower()` | `"hero"` |
| `trim()` | `string` | Removes spaces, tabs and new lines from both ends | `"  hi  ".trim()` | `"hi"` |
| `contains(t)` | `bool` | True if `t` appears anywhere | `"Hello".contains("ell")` | `true` |
| `startsWith(t)` | `bool` | True if it begins with `t` | `"Hello".startsWith("He")` | `true` |
| `endsWith(t)` | `bool` | True if it ends with `t` | `"file.png".endsWith(".png")` | `true` |
| `indexOf(t)` | `int` | The position where `t` first appears (from 0), or `-1` if it doesn't | `"Hello".indexOf("l")` | `2` |
| `replace(a, b)` | `string` | Replaces every `a` with `b` | `"a-b-c".replace("-", "+")` | `"a+b+c"` |
| `split(sep)` | `list<string>` | Cuts it into pieces at every `sep` | `"a,b,c".split(",")` | `["a", "b", "c"]` |
| `substring(start, count)` | `string` | `count` characters starting at position `start` | `"Forge".substring(1, 3)` | `"org"` |
| `charAt(i)` | `string` | The character at position `i`, as one-letter text | `"Forge".charAt(0)` | `"F"` |
| `repeat(n)` | `string` | The text `n` times over | `"ab".repeat(3)` | `"ababab"` |

Notes:

- Positions start at **0**: in `"Forge"`, `F` is at 0 and `e` is at 4.
- `substring` and `charAt` stop the game with a runtime error if the positions are outside the text.
- `split("")` (an empty separator) splits into single characters.
- Text can't be indexed with `[ ]`. Use `charAt(i)`.
- Forge has no `char` type. A single character is a one-letter `string`.
- `.length` counts bytes. Letters with accents and emoji take more than one byte each, so `"é".length` is 2.

### Example: reading "key=value" lines

```cpp
string line = "speed=320";
list<string> parts = line.split("=");
string key = parts[0];                  // "speed"
float value = float(parts[1]);          // 320.0
```

## Turning text into numbers and back

```cpp
int n = int("42");          // text → whole number
float f = float("2.5");     // text → decimal number
string s = string(99);      // anything → text
string t = "{99}";          // the same thing
```

If the text isn't a valid number (`int("abc")`), the game stops with: `Runtime error in main.frg:7: couldn't turn "abc" into an int`. Spaces around the number are fine.

## How values look as text

This is what `print(x)` and `"{x}"` produce for each kind of value:

| Value | Shows as |
|---|---|
| whole number | `42` |
| decimal number | `2.5`, `5.0`, `1e+20`, `inf`, `nan` |
| bool | `true` / `false` |
| string | the text itself, with no quotes |
| list | `[1, 2, 3]` |
| map | `{Ana: 12, Ben: 14}` (keys in sorted order) |
| enum | the value's name: `Running` |
| struct or class | `Point(x: 1.0, y: 2.0)`. Classes inside a class show as `<Name>` to keep it short. |
| null | `null` |
| `vec2` / `color` / `rect` | `vec2(1.0, 2.0)`, `color(255, 0, 0, 255)`, `rect(0.0, 0.0, 10.0, 10.0)` |
| task | `task (running)` / `task (done)` |

A struct or class can choose how it's shown by having a method `string toString()`:

```cpp
class Player {
    string name;
    int hp = 100;
    string toString() {
        return "{name} ({hp} HP)";
    }
}
print(Player("Ana", 100));     // Ana (100 HP)
```

## Reading what the player types

In a console program (no window), `input` from `<stdio>` waits for the player to type a line and press Enter:

```cpp
string name = input("What's your name? ");
int age = int(input("How old are you? "));
print("Hi {name}, next year you'll be {age + 1}");
```

## What's next

[Operators](04-operators.md): every way to combine values, in symbols and in words.
