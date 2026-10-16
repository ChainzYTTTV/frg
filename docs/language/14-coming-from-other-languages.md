# 14. Coming From Other Languages

If you already know C++, C#, Java, JavaScript or Python, Forge will feel familiar. This page lists where it's **different**, so your habits don't trip you up.

## Overall

| | Forge |
|---|---|
| Looks like | C++ / C#: braces, semicolons, typed variables (`int x = 5;`) |
| Compiled? | Yes. Forge is checked fully before running, then compiled to native code through C++. |
| Memory | Automatic: class objects are reference-counted (freed when nothing uses them). No `new`, `delete`, garbage collector pauses or manual freeing. |
| Entry point | `int main()` in `main.frg`, returning 0 for success |
| Type inference | None. Every variable states its type (no `var` or `auto`). |
| Declaration order | Doesn't matter for functions, types and globals, even across files. No forward declarations or headers. |

## Habits that need changing

| You might write | In Forge | Why |
|---|---|---|
| `print(a, b)` (Python) | `print("{a} {b}")` | `print` takes one value. Put several values in one string. |
| `"Score: " + score` | `"Score: {score}"` | `+` only joins two strings; Forge doesn't guess number formatting. |
| `f"{x}"` (Python), `` `${x}` `` (JS), `$"{x}"` (C#) | `"{x}"` | every string can hold `{values}` |
| `/* comment */` | ` ``` comment ``` ` | |
| `if (x) return;` | `if (x) { return; }` | braces are always required |
| `if (count)`, `if (obj)`, `while (n)` | `if (count > 0)`, `if (obj != null)` | conditions must be `bool`; there's no "truthy" |
| `x = y = 0;`, `if ((n = next()) > 0)` | separate statements | assignment is a statement, not a value |
| `y = x++;` | `x++;` then `y = x;` | `++`/`--` are statements only |
| `case 1: ...; break;` | `case 1: ...` | switch cases never fall through; `break` in a switch is an error |
| `new Player()` | `Player()` | there's no `new` |
| `Player p;` (C++/C#) | `Player p = Player();` or `Player? p = null;` | class variables must hold an object, or be optional |
| `#include "player.h"` / `import player` | nothing (same folder); `#include <"folder">` for folders | every `.frg` file next to `main.frg` is included automatically |
| calling `jump()` from another file | `player.jump()` | other files' things always need the file-name prefix |
| `void jump();` declarations | just write the function | no forward declarations or headers |
| `arr.size()`, `len(arr)`, `arr.Count` | `arr.length` (no parentheses) | |
| `str[0]` | `str.charAt(0)` | text can't be indexed |
| `'c'` (a char) | `"c"` | there's no `char` type; single quotes are an error |
| `for i in range(10)` (Python) | `for (int i in 0..9)` or `for (int i = 0; i < 10; i++)` | a range includes **both** ends: `0..9` is ten numbers |
| `for (auto& x : list)` (C++) | `for (int x in list)` | the item type is written out |
| `x ** 2`, `x ^ 2`, `pow(x, 2)` | `x * x` or `math.pow(x, 2)` | no power operator; `^` is bitwise xor |
| `7 / 2 == 3.5` (Python 3) | `7 / 2` is `3` | whole-number division, like C, C++, Java and C# |
| `null` for any object (Java/C#) | only for `Type?` | plain types can never be null |
| `x?.y` (C#/JS) | `if (x != null) { x.y }` | an optional is used directly; it stops the game if it's null |
| `(Goblin)e` / `e as Goblin` (C#) | `e as Goblin` gives `Goblin?` | |
| `protected` | `private` | `private` already includes classes that inherit |
| `static` members | `static int count = 0;` and `static Enemy boss() { ... }` | reached as `Enemy.count`, `Enemy.boss()` |
| overloading `void f(int)` / `void f(float)` | the same: `void f(int)` and `void f(float)` | top-level functions and constructors can share a name; methods can't |
| `class B extends A` / `class B : public A` | `class B : A` | |
| `interface I` / `implements I` / abstract classes | `interface Attacker { int power(); }`, `class Goblin : Enemy, Attacker`, `abstract class Enemy { abstract int reward(); }` | at most one class after the `:`, then interfaces. See [Interfaces and abstract classes](09-structs-and-classes.md#interfaces-and-abstract-classes) |
| `@Override` / `override` | nothing | a method with the same name and types replaces the parent's |
| `base.method()` / `super.method()` | `super.method()` | |
| `try` / `catch` / `throw` | `try { ... } catch (Error e) { print(e.message); }` and `throw "message";` | every runtime error can be caught; you throw text, not objects; there's no `finally`. See [Catching runtime errors](12-errors.md#catching-runtime-errors-try-and-catch) |
| lambdas, function pointers, callbacks | `(Enemy e) { return e.hp <= 0; }`, typed `Function<bool(Enemy)>` | a function value gets a copy of the local variables it uses. See [Functions as values](06-functions.md#functions-as-values) |
| templates / generics you write yourself | `class Box<T> { T item; }`, used as `Box<int> b = Box(5);` | structs and classes only, not functions; inside, a `T` can only be stored, passed, compared and printed. See [Generic types](09-structs-and-classes.md#generic-types-one-type-that-holds-any-type) |
| `weak_ptr`, `WeakReference`, `weakref` | `Weak<Player> w = Weak(p);` and `Player? x = w.get();` | only for class types. See [Weak](09-structs-and-classes.md#weak-pointing-at-an-object-without-keeping-it-alive) |
| `__name__`, `__class__`, `vars(obj)` (Python), reflection | `obj.describe`, `obj.describe.name`, `.parent`, `.fields`, `.values` | built into every struct and class. See [describe](09-structs-and-classes.md#describe-looking-inside-any-object) |
| threads | not available | use [tasks](11-tasks.md) for things that happen over time |

## Things Forge adds

- **Word operators**: every symbol has a word form: `a plus b`, `x greater or equal y`, `set x to 5;`, `increase score by 10;`. See [Operators](04-operators.md).
- **Tasks**: `task` functions with `wait(seconds)`, like coroutines or `async`, but simpler. See [Tasks](11-tasks.md).
- **A file is a namespace**: `player.frg`'s contents are reached as `player.x`, and folders nest: `enemies.goblin.spawn()`.
- **Literals adapt to their destination**: `int8 x = 100;` needs no cast, and `float r = 7 / 2;` is `3.5` (because both numbers are written out). See [How division works](02-variables-and-types.md#how-division-works).
- **Runtime checks** on list positions, map keys, null and division by zero, with the file and line in the message.
- **Game types built in**: `vec2`, `vec3`, `color`, `rect`, with math operators.

## How Forge compares, roughly

| | Forge | C++ | C# | Python |
|---|---|---|---|---|
| Typing | static, explicit | static | static | dynamic |
| Speed | close to C++ | fast | fast | slower |
| Memory | automatic (reference counting) | manual / RAII | garbage collected | garbage collected |
| Null safety | only `Type?` can be null | no | partial (nullable types) | no |
| Game libraries | built in | external | external (Unity, MonoGame) | external (pygame) |
