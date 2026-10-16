# 2. Variables and Types

A **variable** is a named box that holds a value. Every variable has a **type**, which says what kind of value it holds. The type is written before the name.

```cpp
int hp = 15;
float speed = 2.5;
double price = 19.99;
bool alive = true;
string name = "Hero";
```

Once a variable has a type, it keeps it: you can't put text into an `int`.

## Making and changing variables

```cpp
int score = 0;      // make a variable with a starting value
int lives;          // without a value, it starts at zero (see below)

score = 100;        // change it (no type this time)
score += 50;        // add 50 to it (see Operators for all the ways)
```

Writing the type a second time makes a **new** variable. That's an error if one with the same name already exists in the same place:

```cpp
int score = 0;
int score = 5;      // error: there's already a variable called 'score' here
score = 5;          // this is what was meant
```

## Starting values

A variable made without a value starts at the "zero" of its type:

| Type | Starts as |
|---|---|
| any number | `0` (or `0.0`) |
| `bool` | `false` |
| `string` | `""` (empty text) |
| `list`, `map` | empty |
| struct | every field at its starting value |
| optional (`Type?`) | `null` |
| `task` | a finished task |

**Classes have no zero**, so a variable of a class type must be given a value: `Enemy e = Enemy();`. If it should start empty, make it optional instead: `Enemy? e = null;`. See [Structs and classes](09-structs-and-classes.md).

## Where variables live

| Where it's made | Name | Who can use it | How long it lasts |
|---|---|---|---|
| Inside a function or block | **local** | only code after it, inside the same `{ }` | until the `}` |
| At the top level of a file | **global** | every function in the file, and other files (`player.score`) | the whole program |
| A function's parameters | **parameter** | that function | until the function returns |

```cpp
int highScore = 0;          // global

void play(){
    int score = 50;         // local to play
    if (score > highScore) {
        int bonus = 10;     // local to this if block
        highScore = score + bonus;
    }
    print(bonus);           // error: bonus only exists inside the if block
}
```

Global variables are created before `main` starts, in the order they're written.

## Constants

`const` makes a variable that can never be set to a new value. It must be given one straight away:

```cpp
const int MAX_HP = 100;
const float GRAVITY = 980;

MAX_HP = 50;    // error: 'MAX_HP' is const, so it can't be changed
```

`const` only stops the **variable** from being set again. If it holds a list or an object, the contents can still change: `const list<int> scores = []; scores.add(5);` is allowed.

## The types

### Whole numbers

| Type | Smallest | Biggest | Notes |
|---|---|---|---|
| `int8` | -128 | 127 | |
| `int16` | -32,768 | 32,767 | |
| `int` (same as `int32`) | -2,147,483,648 | 2,147,483,647 | **the everyday whole number** |
| `int64` | about -9.2 × 10¹⁸ | about 9.2 × 10¹⁸ | |
| `int128` | about -1.7 × 10³⁸ | about 1.7 × 10³⁸ | |
| `uint8` | 0 | 255 | `u` = unsigned: no negative numbers |
| `uint16` | 0 | 65,535 | |
| `uint` (same as `uint32`) | 0 | 4,294,967,295 | |
| `uint64` | 0 | about 1.8 × 10¹⁹ | |
| `uint128` | 0 | about 3.4 × 10³⁸ | |

Use `int` unless you have a reason not to. Use the smaller types when you have huge numbers of values (each `uint8` is 1 byte instead of 4), and the bigger ones when values can grow past 2 billion.

Whole numbers can be written in several ways:

```cpp
int a = 1000000;
int b = 1_000_000;      // underscores are ignored, for readability
int c = 0xFF;           // hexadecimal (base 16): 255
int d = 0b1010;         // binary (base 2): 10
```

**Wrapping around:** when a whole number goes past its biggest or smallest value, it wraps around to the other end, like a car's odometer:

```cpp
uint8 x = 250;
x += 10;            // 260 doesn't fit in a uint8, so it wraps around to 4
int8 y = 127;
y++;                // wraps around to -128
```

Writing a number that doesn't fit is an error: `uint8 z = 300;` → "this number doesn't fit in a uint8". So is a negative number in an unsigned type: `uint n = -1;`.

### Decimal numbers

| Type | Precision | Use for |
|---|---|---|
| `float` | about 7 digits | **game math**: positions, speeds, angles, timers |
| `double` | about 15 digits | when you need more accuracy (money totals, very large worlds) |

```cpp
float speed = 2.5;
double distance = 1234567.891;
float tiny = 0.000001;
float big = 1.5e6;      // 1.5 × 10⁶ = 1,500,000
```

Decimal numbers are stored in binary, so some values can't be stored exactly, the same as in every language. `0.1 + 0.2` as a `double` prints `0.30000000000000004`. Never use `==` to check whether two calculated decimal numbers are equal; check whether they're close instead: `math.abs(a - b) < 0.0001`.

When printed, a whole-valued decimal number always shows `.0`, so you can tell it apart from an `int`: `print(5.0)` shows `5.0`, while `print(5)` shows `5`.

### bool

`true` or `false`. Comparisons give back a `bool`:

```cpp
bool alive = hp > 0;
bool canJump = onGround and not paused;
```

Conditions in `if` and `while` must be `bool`. Writing `if (hp)` is an error; write `if (hp > 0)` or `if (hp != 0)`.

### string

Text. See [Text](03-text.md) for everything you can do with it.

```cpp
string name = "Hero";
string greeting = "Hi {name}!";
```

### Other types

| Type | Chapter |
|---|---|
| `list<T>`, `map<K, V>` | [Lists and maps](08-lists-and-maps.md) |
| your own `struct`, `class`, `enum` | [Structs and classes](09-structs-and-classes.md), [Enums](10-enums-and-optionals.md) |
| `Type?` (optional) | [Enums and optionals](10-enums-and-optionals.md) |
| `task` | [Tasks](11-tasks.md) |
| `vec2`, `vec3`, `vec4`, `color`, `rect` | [Built-in types](../libraries/core-types.md) |

## Converting between types

### Automatic conversions

Forge converts a value automatically **only when nothing can be lost**:

| From | To | Automatic? |
|---|---|---|
| a smaller whole number type | a bigger one (`int8` → `int`) | yes |
| any whole number | `float` or `double` | yes |
| `float` | `double` | yes |
| a decimal number | a whole number | **no**: decimals would be lost |
| a bigger whole number type | a smaller one (`int` → `int8`) | **no**: it might not fit |
| signed (`int`) | unsigned (`uint`) | **no**: negatives don't fit |
| a number | `string` | **no** |
| `string` | a number | **no** |

```cpp
int whole = 7;
float f = whole;        // fine: 7.0
int back = f;           // error: 'back' needs an int, but this is a float
```

### Converting yourself

Write the type's name like a function:

| Conversion | Example | Result | Notes |
|---|---|---|---|
| decimal → whole | `int(3.9)` | `3` | The decimals are cut off, not rounded. Use `math.round` first to round. |
| whole → decimal | `float(7)` | `7.0` | |
| bigger → smaller | `int8(300)` | `44` | Wraps around if it doesn't fit |
| text → number | `int("42")`, `float("2.5")` | `42`, `2.5` | Spaces around are ignored. If the text isn't a number, the game stops with a runtime error. |
| anything → text | `string(42)`, `string(true)` | `"42"`, `"true"` | Works for every type, the same as `"{x}"` |
| enum → whole | `int(State.Running)` | its position, from 0 | |
| whole → enum | `State(1)` | the value at that position | |

## Numbers in calculations

When two numbers of different types are combined, the result has the "bigger" type:

| Combination | Result |
|---|---|
| `int` with `float` | `float` |
| `float` with `double` | `double` |
| `int8` with `int` | `int` |
| `int` with `uint` | `int64` (big enough for both) |

### How division works

**Dividing two whole numbers gives a whole number.** The answer is cut toward zero, and the remainder is thrown away:

| Expression | Result | Why |
|---|---|---|
| `7 / 2` | `3` | 3.5 cut to 3 |
| `-7 / 2` | `-3` | -3.5 cut toward zero |
| `70 / 30` | `2` | 2.33 cut to 2 |
| `7 % 2` | `1` | the remainder |
| `-7 % 3` | `-1` | the remainder takes the sign of the left side |
| `7.0 / 2` | `3.5` | one side is a decimal, so it's decimal division |
| `float(7) / 2` | `3.5` | same |

So to get a decimal answer from two whole numbers, make one of them a decimal first:

```cpp
int coins = 70;
int players = 30;
float each = float(coins) / players;    // 2.3333333
float wrong = coins / players;          // 2.0: the division happened first, with whole numbers
```

Forge gives a **warning** for the `wrong` line (see [Warnings](12-errors.md#warnings)): the game still builds, but the hint shows the fix, `float(coins) / players`.

**One special case: numbers written straight into the code** (like `70`, as opposed to a variable) take on the type of wherever the result goes. So this gives a decimal answer:

```cpp
float result = 70 / 30;     // 2.3333333: both numbers are written out, and the result goes into a float
print(70 / 30);             // 2: nothing says "float" here, so it's whole-number division
```

The same rule makes `int8 x = 100;` work without converting: `100` takes on the type `int8`. It also means `x * 1000` (with `x` an `int8`) is calculated as an `int8` and wraps around. Convert first if you need a bigger result: `int(x) * 1000`.

**Dividing by zero** with whole numbers stops the game with "Runtime error in main.frg:12: division by zero". With decimal numbers, it gives `inf` (infinity) or `nan` (not a number) instead.

## What's next

[Text](03-text.md): strings, and putting values inside them.
