# Syntax: every way to write Forge

This page lists every form the language has, in one place, with the exact shape of each. The chapters in [The language](../README.md#the-language) explain them slowly; this page is for looking things up. The examples are run by Forge's tests, so they're exactly right.

How to read the shapes:

- `name`, `Type`, `value`, `condition` stand for something you write there.
- `[ ... ]` (in a shape, not in code) means that part can be left out.
- `...` means "more of the same".

## Files and lines

- A game is a folder with a `main.frg`, plus any other `.frg` files next to it ([Files and folders](../language/07-files-and-folders.md)).
- A **statement** (a line that does something) ends with `;`. Blocks of statements go inside `{ }`.
- **Braces are required**, even around one line: `if (x > 0) { x--; }`. Never `if (x > 0) x--;`.
- Spaces, tabs and line breaks between words don't matter (except inside text). `frg fmt` tidies them.
- Names are case-sensitive: `score`, `Score` and `SCORE` are three different names.

## Comments

~~~cpp
// from here to the end of the line

```
Several lines,
until the next three backticks.
```
~~~

`/* ... */` isn't used in Forge (it's an error that says to use three backticks).

## Names

A name (of a variable, function, type, field...) is made of letters, digits and `_`, and doesn't start with a digit. It can't be one of the [reserved words](../language/13-reserved-words.md) (`if`, `class`, `and`...), a type name (`int`, `string`, `list`...), or a library's name (`window`, `math`...).

The usual style, which `frg fmt` doesn't change: `camelCase` for variables and functions, `PascalCase` for types, `UPPER_CASE` for constants and enum values, `lower_case` for file names.

## Values written in the code (literals)

| Kind | How to write it | Notes |
|---|---|---|
| whole number | `42`, `-7`, `1_000_000` | `_` can separate digits anywhere |
| hexadecimal | `0xFF`, `0x1F_FF` | base 16 |
| binary | `0b1010` | base 2 |
| decimal number | `3.14`, `0.5`, `1.5e3`, `2e-4` | needs a digit before the dot: `0.5`, not `.5`. `e3` means "times 1000" |
| true / false | `true`, `false` | |
| nothing | `null` | only for optional types (`Player?`) |
| text | `"Hello"` | on one line. `{value}` puts a value inside; `\n` new line, `\t` tab, `\"` quote, `\\` backslash, `\{` and `\}` (or `{{`) braces |
| raw text | `"""` ... `"""` | many lines, exactly as written: no `{value}`s, no `\` escapes |
| list | `[1, 2, 3]`, `[]` | `[]` needs its type known: `list<int> xs = [];` |
| map | `{"Ana": 12, "Ben": 9}` | key `:` value |
| struct or class | `Point(1, 2)`, `Point()`, or `{1, 2}` where the type is known | see [Making objects](#making-objects) |
| function value | `(int n) { return n * 2; }` | see [Function values](#function-values) |

Text with values inside:

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    int hp = 7;
    float speed = 3.14159;
    print("HP: {hp} of {hp * 2}");
    print("Speed: {speed:.2}");
    print("Braces: {{hp}} or \{hp\}");
    print("Tab:\tafter, quote: \"hi\"");
    print("""Raw text: {hp} stays as it is, and \n too""");
    print(0xFF + 0b11 + 1_000);
    return 0;
}
```

```text
HP: 7 of 14
Speed: 3.14
Braces: {hp} or {hp}
Tab:	after, quote: "hi"
Raw text: {hp} stays as it is, and \n too
1258
```

## Types

| Shape | Example | Meaning |
|---|---|---|
| a built-in type | `int` `float` `double` `bool` `string` `int8`...`uint128` | see [Variables and types](../language/02-variables-and-types.md) |
| `vec2` `vec3` `vec4` `color` `rect` | `vec2 pos = vec2(1, 2);` | always available ([Built-in types](../libraries/core-types.md)) |
| `list<T>` | `list<Enemy>` | a list of T |
| `map<K, V>` | `map<string, int>` | keys K to values V. K is a number, `string`, `bool` or an enum |
| `T?` | `Player?`, `int?` | a T, or `null` |
| `Function<R(P1, P2)>` | `Function<bool(Enemy)>`, `Function<void()>` | a function value that takes P1, P2 and gives back R |
| `Weak<T>` | `Weak<Player>` | holds a class object without keeping it alive |
| your own type | `Enemy`, `Box<int>` | |
| a type from another file | `enemies.Goblin`, `world.items.Sword` | `file.Type`, or `folder.file.Type` |
| `void` | `void update(float dt)` | "nothing": only as what a function gives back |

## The top level of a file

Outside every function, a file can have only these:

```cpp
#include <stdio>                       // a library
#include <"enemies">                   // a folder of your own .frg files, next to this file

const int MAX_HP = 100;                // a constant
int score = 0;                         // a global variable
private int secret = 7;                // only this file can use it

int add(int a, int b){ ... }           // a function
task blink(){ ... }                    // a task (a function that can wait)

struct Point { ... }                   // types
class Enemy { ... }
class Goblin : Enemy, Drawable { ... } // inherits from Enemy, has the interface Drawable
abstract class Shape { ... }
interface Drawable { ... }
enum State { IDLE, RUNNING }
class Box<T> { ... }                   // a generic type
```

Code that *does* something (`print("hi");`, `score++;`) has to be inside a function.

## Functions

```cpp
ReturnType name(Type param, Type param = defaultValue, ...){
    statements
}
```

- `void` as the return type means it gives nothing back; otherwise every way through it must `return value;`.
- Parameters with a default value come after the ones without.
- Several functions can share a name if their parameters differ (overloading).
- `int main()` is where the game starts. It takes nothing and returns an `int` (0 = everything went well).
- `private` before a function hides it from other files.
- `task` instead of a return type makes a [task](../language/11-tasks.md): `task flash(){ ... wait(0.5); ... }`.

Calling: `name(value, value)`, `file.name(value)` for one in another file, `object.method(value)` for a method.

## Statements

Every kind of statement, with its exact shape:

| Statement | Shape |
|---|---|
| a variable | `Type name = value;` or `Type name;` (numbers start at 0, text at `""`) |
| a constant | `const Type NAME = value;` |
| changing a variable | `name = value;` `name += value;` (also `-=` `*=` `/=` `%=`) `name++;` `name--;` |
| the same in words | `set x to 5;` `increase x by 2;` `decrease x by 2;` `multiply x by 2;` `divide x by 2;` `increase x;` `decrease x;` |
| calling a function | `name(values);` |
| if | `if (condition) { ... }` then any number of `else if (condition) { ... }`, then at most one `else { ... }` |
| while | `while (condition) { ... }` |
| do-while | `do { ... } while (condition);` (runs at least once) |
| counting for | `for (int i = 0; i < 10; i++) { ... }` (each of the three parts can be left empty) |
| for each | `for (Type name in list) { ... }` |
| for a range | `for (int i in first..last) { ... }` (both ends included) |
| switch | `switch (value) { case 1: ... case 2, 3: ... default: ... }` |
| leaving a loop | `break;` (out of the loop) `continue;` (on to the next turn) |
| giving back a value | `return value;` or `return;` in a `void` function |
| errors | `try { ... } catch (Error e) { ... }` and `throw "message";` |
| waiting (only in a task) | `wait(seconds);` or `wait(otherTask);` |
| a block | `{ ... }` on its own: its variables only exist inside it |

`switch` works on whole numbers, text and enums. Each `case` stops by itself at the next one (no `break` needed; `break` in a switch is an error). One case can list several values: `case 2, 3:`.

<!-- check: output -->

```cpp
#include <stdio>

enum Weather { SUN, RAIN, SNOW }

int main(){
    int total = 0;
    for (int i = 1; i <= 3; i++) {
        total += i;
    }
    for (int i in 4..5) {
        increase total by i;
    }
    int n = 0;
    do {
        n++;
    } while (n < 3);
    while (true) {
        n++;
        if (n == 5) {
            break;
        }
    }
    Weather today = Weather.RAIN;
    switch (today) {
        case Weather.SUN:
            print("sunny");
        case Weather.RAIN, Weather.SNOW:
            print("wet");
        default:
            print("never shown");
    }
    try {
        throw "something went wrong";
    } catch (Error e) {
        print("caught: {e.message}");
    }
    print("total {total}, n {n}");
    return 0;
}
```

```text
wet
caught: something went wrong
total 15, n 5
```

What isn't a statement in Forge: assigning inside a condition (`if (x = 5)` is an error: use `==`), `++` inside an expression (`print(x++)` is an error: write `x++;` on its own line), and a value on its own that doesn't do anything (`x + 1;`).

## Expressions

An **expression** is anything that gives a value. All the forms:

| Form | Example |
|---|---|
| a literal | `42`, `"hi"`, `[1, 2]`, `null` |
| a name | `score`, `MAX_HP`, `enemies.count` |
| a call | `add(1, 2)`, `player.jump()`, `fs[0]()`, `makeAdder(10)(5)` |
| a field | `player.hp`, `pos.x` |
| a list or map item | `items[0]`, `ages["Ana"]` |
| an operator | `a + b`, `not done`, `x is Goblin` ([all of them](../language/04-operators.md)) |
| choosing | `condition ? a : b` or `a if condition else b` |
| a backup for null | `maybe ?? backup` or `maybe otherwise backup` |
| converting | `int(3.9)`, `float(n)`, `string(42)`, `int("12")` |
| checking a class | `enemy is Goblin`, `enemy as Goblin` (gives `Goblin?`) |
| making an object | `Point(1, 2)`, `Box<int>(5)`, `{1, 2}` |
| a function value | `(int n) { return n * 2; }` |

The order operators happen in, and every word version, is in [Operators](../language/04-operators.md#order-of-operations). In short: calls, fields and `[ ]` first; then `-a` and `!a`; then `* / %`; then `+ -`; then comparisons; then `==`; then `and`; then `or`; then `??`; then `? :`. Parentheses `( )` change the order. Within one expression, the parts [happen left to right](rules.md#the-order-a-program-runs-in).

## Making objects

| Shape | When |
|---|---|
| `Type(values)` | calls the type's constructor; without one, gives the fields in order (all of them, or none) |
| `Type()` | every field at its starting value |
| `{values}` | the same as `Type(values)`, when the type is already known: `Point p = {1, 2};`, or as an argument |
| `Box<int>(5)` | a generic type, saying its types |
| `Box(5)` | the same, when the type is known from where it goes: `Box<int> b = Box(5);` |

## Your own types

```cpp
struct Point {
    int x = 0;                     // a field, with its starting value
    int y = 0;
    static int made = 0;           // one value for the whole type: Point.made

    Point(int x, int y){           // a constructor (optional; several are allowed)
        this.x = x;
        this.y = y;
    }

    int sum(){                     // a method
        return x + y;
    }

    static Point origin(){         // a static method: Point.origin()
        return Point(0, 0);
    }
}

class Goblin : Enemy {             // inherits Enemy's fields and methods
    Goblin(){
        super(10);                 // runs Enemy's constructor first
    }
    void attack(){                 // replaces Enemy's attack
        super.attack();            // runs Enemy's version too
    }
}

interface Drawable {
    void draw();                   // only the shape: classes that have the interface write the code
}

abstract class Shape {
    abstract float area();         // every class that inherits Shape has to write area()
}

enum Direction { UP, DOWN, LEFT, RIGHT }

class Box<T> {                     // a generic type: Box<int>, Box<string>...
    T item;
    Box(T item){
        this.item = item;
    }
}
```

- `private` before a field, method or constructor: only the type's own methods can use it (for a class, also the classes that inherit from it).
- A struct is copied; a class is shared ([Copies and shared objects](rules.md#copies-and-shared-objects)).
- Methods named `plus`, `minus`, `times`, `div`, `less`, `equals` and `toText` make `+`, `-`, `*`, `/`, `<`, `==` and `"{x}"` work on your type ([Structs and classes](../language/09-structs-and-classes.md)).
- `x.describe` shows any object's fields; `x.describe.name`, `.parent`, `.fields`, `.values` give one part.

## Function values

A function can be stored in a variable, passed to a function, and kept in a list:

```cpp
Function<int(int)> twice = (int n) { return n * 2; };
Function<void()> hello = () { print("hi"); };
list<Enemy> alive = enemies.filter((Enemy e) { return e.hp > 0; });
```

A function value gets a **copy** of the local variables around it, at the moment it's made. It can read them, but changing them only changes its copy; use a global variable or a class object's field to share a value.

<!-- check: output -->

```cpp
#include <stdio>

Function<int(int)> makeAdder(int amount){
    return (int n) { return n + amount; };
}

int main(){
    Function<int(int)> twice = (int n) { return n * 2; };
    list<Function<int(int)>> steps = [twice, makeAdder(1)];
    int value = 5;
    for (Function<int(int)> step in steps) {
        value = step(value);
    }
    print(value);
    print(steps[0](10));
    print(makeAdder(10)(5));
    list<int> big = [3, 8, 1, 9].filter((int n) { return n > 2; });
    print(big);
    return 0;
}
```

```text
11
20
15
[3, 8, 9]
```

## Things Forge doesn't have

So you don't have to wonder:

| Not in Forge | Instead |
|---|---|
| pointers (`*`, `&`, `Pointer<T>`) | return the new value, or keep it in a class object |
| `new` and `delete` | `Enemy()` makes one; it's freed when nothing holds it |
| `x++` or `x = 5` inside an expression | do them on their own line |
| falling through from one `case` to the next | list several values in one case: `case 1, 2:` |
| `goto` | loops, `break`, `continue`, functions |
| `'c'` single characters | one-letter text: `"c"`, and `text.charAt(i)` |
| functions inside functions | function values: `Function<int(int)> f = (int n) { ... };` |
| code outside functions | put it in `main`, or in a function `main` calls |
| `#define`, macros, header files | `const` values, and files see each other with the file-name prefix |
| exceptions of different types | one `Error` type; `throw "message";` and `e.message` |
| generic functions (`T max<T>(T a, T b)`) | generic types only: `class Box<T>` |

<!-- check: error -->

```cpp
int main(){
    int x = 0;
    if (x = 5) {
        x++;
    }
    return 0;
}
```

```text
error: '=' sets a variable - to compare two values, use '=='
  --> main.frg:3:11
   |
 3 |     if (x = 5) {
   |           ^
```

<!-- check: error -->

```cpp
int main(){
    int lives = 3;
    if (lives > 0) lives--;
    return 0;
}
```

```text
error: 'if' needs { } around its code
  --> main.frg:3:20
   |
 3 |     if (lives > 0) lives--;
   |                    ^^^^^
  hint: for example: if (...) { ... }
```
