# 12. Errors

Everyone's code has mistakes. Forge's job is to find them early and explain them plainly. There are two kinds:

| Kind | When | Example |
|---|---|---|
| **Compile errors** | before the game runs, while Forge checks your code | a missing `;`, a misspelled name, a float put into an int |
| **Runtime errors** | while the game is running | a list position that doesn't exist, using something that's null |

Most mistakes are compile errors, so they're caught before you ever see the game.

Forge also gives **warnings**: code that works, but probably doesn't do what you meant. A warning never stops the game from building. See [Warnings](#warnings) below.

## Reading a compile error

```
error: 'hp' needs an int, but this is a float
  --> main.frg:4:14
   |
 4 |     int hp = 7.5;
   |              ^^^
  hint: use int(...) to turn it into a whole number (the decimals get cut off)
```

| Part | Meaning |
|---|---|
| `error: ...` | what's wrong |
| `--> main.frg:4:14` | the file, the line (4) and the column (14) |
| the code line and `^^^` | exactly which part of the line |
| `hint: ...` | how to fix it (not every error has one) |

Tips:

- **Fix the first error first.** One mistake can cause others further down, and fixing it often makes them disappear.
- Forge reports every syntax error in a file in one go, and all type errors in the whole game. At the end it says how many: `Build failed: 3 errors.`
- In VS Code, the same errors appear as red underlines when you save. Hover over one to read it, or open the Problems panel (Ctrl+Shift+M).

## Common compile errors

### Writing code

| Error | What it means | Fix |
|---|---|---|
| `missing ';' at the end of this line` | a statement isn't finished | add `;` where the arrow points |
| `expected ..., but found ...` | something's in the wrong place, like a missing `)` or an extra `,` | look just before the arrow; often a bracket isn't closed |
| `this '{' is never closed` | a `{` has no matching `}` | add the `}`; the arrow shows which `{` |
| `'if' needs { } around its code` | braces are required after `if`, `else`, `while`, `for` | `if (x) { ... }` |
| `'=' sets a variable - to compare two values, use '=='` | `if (x = 5)` | `if (x == 5)` |
| `this code has to go inside a function` | a statement at the top level of a file | move it into `main()` or another function |
| `text in Forge uses double quotes` | `'hello'` | `"hello"` |
| `Forge uses ``` for multi-line comments, not /* */` | | `` ``` comment ``` `` |
| `this ``` comment is never closed` | | add the closing ` ``` ` |
| `unknown escape '\x'` | a `\` followed by an unknown letter | see [escapes](03-text.md#escapes-special-characters); write `\\` for a backslash |
| `this { is never closed` (inside text) | a `{` in a string starts a `{value}` | write `{{` or `\{` for a literal `{` |
| `Forge doesn't need function declarations` | `void jump();` without a body | just write the function with its `{ }` body, anywhere |
| `make one variable per line` | `int a, b;` | `int a; int b;` |

### Names

| Error | What it means | Fix |
|---|---|---|
| `unknown name 'scroe'` + `did you mean 'score'?` | a typo, or something that doesn't exist yet | check the spelling; Forge suggests close matches |
| `unknown name 'jump'` + `'jump' is in player.frg - use player.jump` | it's in another file | add the file's name in front |
| `'print' is part of the stdio library, which this file doesn't include` | each file includes its own libraries | add `#include <stdio>` to **this** file |
| `this file doesn't include the math library` | `math.sqrt` without the include | add `#include <math>` |
| `there's already a variable called 'x' here` | `int x` written twice | to change it, leave off the type: `x = ...;` |
| `'x' is defined twice in this file` | two functions, globals or types with the same name | rename one |
| `'player' is already the name of the file player.frg` | names can't match top-level files | rename the variable |
| `'int' is a built-in type name, so it can't be used as a name` | | pick another name |
| `the folder 'enemies' isn't part of the game yet` | the folder isn't included | add `#include <"enemies">` |
| `unknown type 'Player'` + `use entities.Player` | the type is in another file | `entities.Player` |
| `'x' is private to Safe` / `... to player.frg` | it's `private` | use a public method instead, or remove `private` |

### Types

| Error | What it means | Fix |
|---|---|---|
| `'x' needs an int, but this is a float` | decimals don't go into whole numbers automatically | `int(value)` (cuts off the decimals) |
| `'x' needs an int, but this is a string` | text isn't a number | `int(text)` |
| `'x' needs a string, but this is an int` | numbers aren't text | `"{value}"` or `string(value)` |
| `this number doesn't fit in a uint8` | the value is outside the type's range | use a bigger type, or a value that fits |
| `can't add text and an int with +` | `"Score: " + 5` | `"Score: {5}"` |
| `the condition of 'if' must be true or false, but this is an int` | `if (hp)` | `if (hp > 0)` |
| `'and' needs true/false values on both sides` | `if (hp and alive)` | `if (hp > 0 and alive)` |
| `can't compare a string with an int` | | convert one side first |
| `null can only go in an optional type` | `int x = null;` | `int? x = null;` |
| `a Car isn't a kind of Cat` | the classes aren't related | use the right type |
| `'e' needs a starting value` | a class-type variable without a value | give it one, or make it optional: `Enemy? e = null;` |
| `this doesn't give back a value` | using a `void` function's result | the function needs a return type and `return` |

### Functions and types

| Error | What it means | Fix |
|---|---|---|
| `add() needs 2 values, but got 1` + how it's written | the wrong number of values in a call | match the function's parameters |
| `'f' doesn't always return a value` | some path through the function ends without `return` | add a `return` at the end |
| `'f' is a void function, so it can't return a value` | `return 5;` in a `void` function | remove the value, or change the return type |
| `main has to return an int` / `main.frg needs a main function` | | `int main(){ ... return 0; }` |
| `Point(...) takes either nothing or all 2 of its fields` | a type without a constructor got some fields | give all of them, or add a constructor |
| `none of the versions of graphics2d.drawRect() take these values` + the versions | no overload matches | use one of the listed forms |
| `'sound' replaces the method in Animal, so it has to match it` | a method with a parent's name but different types | match the parent's version exactly |
| `struct 'Node' ends up containing itself` | a struct inside itself would be infinitely big | make it a class |
| `wait() only works inside a task function` | | change the return type to `task` |
| `a switch doesn't need 'break'` | cases never fall through in Forge | remove the `break` |
| `'continue' can only be used inside a loop` | | |
| `'MAX' is const, so it can't be changed` | | remove `const`, or don't change it |
| `window.run() calls update(float dt) and draw() every frame, so main.frg needs both` | | add `void update(float dt){ }` and `void draw(){ }` to main.frg |

### Libraries and files

| Error | What it means | Fix |
|---|---|---|
| `there's no library called <stdoi>` + did you mean | a typo in an include | `#include <stdio>` |
| `the <network> library isn't built yet` | it's planned, but doesn't exist yet | |
| `<OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)` | the [low-level toolkits](../libraries/toolkits.md) replace the engine; they can't be mixed with it | use the engine **or** the toolkit, not both |
| `a game can only use one of <OpenGL>, <DirectX> and <Vulkan>` | | pick one toolkit |
| `the shader shape.frag has errors` | a `<Vulkan>` shader file has a mistake; the lines below say where | fix the shader |
| `there's no folder called 'x' next to this file` | `#include <"x">`, but no such folder | check the name and where the folder is |
| `the file name 'my game.frg' can't be used` | file names can only have letters, numbers and `_` | rename the file |

## Warnings

A warning looks like an error, but it's yellow and says `warning:`:

```
warning: this divides two whole numbers, so the decimals are cut off before it becomes a decimal number
  --> main.frg:5:24
   |
 5 |     float ratio = hits / shots;
   |                        ^
  hint: turn one side into a decimal number first, like float(hits) / shots
```

The game still builds and runs. The warning is there because the code probably doesn't do what you meant. In VS Code, warnings are underlined in yellow.

While there are errors, Forge only shows the errors, so they stand out. The warnings appear once the errors are fixed. (VS Code always shows both.)

| Warning | What it means | Fix |
|---|---|---|
| `this changes 'e', which is only a copy of the item in the list` | `for (Enemy e in enemies) { e.hp = 0; }` with a **struct**: the loop gets a copy, so the list doesn't change | loop with a number: `for (int i = 0; i < enemies.length; i++) { enemies[i].hp = 0; }` |
| `this divides two whole numbers, so the decimals are cut off` | `float ratio = hits / shots;` with two ints: `7 / 2` is `3`, and only then does it become `3.0` | `float(hits) / shots` gives `3.5` |
| `the variable 'x' is never used` | you made a variable and never used it | remove it, or use it |
| `this code can never run, because it comes after 'return'` | lines after `return`, `break` or `continue` in the same `{ }` | remove them, or move them above |
| `comparing decimal numbers with '==' often gives the wrong answer` | `0.1 + 0.2 == 0.3` is false, because decimals are a tiny bit off | check whether they're close: `math.abs(a - b) < 0.001` |

Divisions of plain numbers, like `float f = 70 / 30;`, don't give a warning: Forge already works those out as decimals, because it can see that the result goes into a float.

## Runtime errors

These stop the game with a message, the file and the line, and then **how the game got there**: which function called the one with the error, which called that one, and so on, nearest first:

```
Runtime error in main.frg:19: position 3 is outside the list (it has 3 items, at positions 0 to 2)
  called from main.frg:13 (in sumFirst)
  called from main.frg:6 (in main)
```

Read it from the top: the error happened on line 19, inside a function that `sumFirst` called on line 13, which `main` called on line 6. Often the real mistake is in one of the callers (here, `main` asked for 7 numbers from a list of 3), so the trace says where to look. A function value shows as `a function value in main`. Code in a `task` doesn't appear in the list, because tasks pause and carry on later.

`frg run` then shows `[game.exe exited with code 1]`. In a game without a console, the same text appears in the message box.

| Message | Cause | Prevent it with |
|---|---|---|
| `position N is outside the list (...)` | `list[N]` where `N` isn't between 0 and `length - 1` | check `i < list.length`; loop with `for ... in` |
| `position N is outside the list: the list is empty` | `list[N]` on a list with nothing in it | check `isEmpty()` first |
| `first() was used on an empty list` (also `last()`, `pop()`) | | check `isEmpty()` first |
| `can't insert at position N` | `insert(N, x)` with `N` greater than `length` | |
| `the key isn't in the map` | reading `map[key]` for a missing key | `map.has(key)` or `map.get(key, backup)` |
| `'target' is null` | using an optional that's `null` | `if (target != null)` |
| `division by zero` | whole-number `/` or `%` by 0 | check the divisor first |
| `couldn't turn "abc" into an int` | `int(text)` on text that isn't a number | check the text, or validate input |
| `substring(...) goes outside the text` / `position N is outside the text` | | check `.length` |
| `can't find the file 'player.png'` | an asset that isn't in the game folder | put it next to `main.frg`; check the spelling |
| `couldn't read 'x.png' as an image` | the file is broken or in an unsupported format | |
| `the window isn't open yet` | drawing before `window.open(...)` | open the window first |
| `file.read(): can't open 'x'` | the file doesn't exist | `file.exists(path)` first |

### Catching runtime errors: try and catch

Some runtime errors aren't mistakes in your code: a save file that's missing or broken, or text the player typed that isn't a number. Put code that might go wrong in `try { }`, and say what to do instead in `catch (Error e) { }`:

```cpp
try {
    string text = file.read("save.txt");
    level = int(text);
} catch (Error e) {
    print("couldn't load the save: {e.message}");
    level = 1;
}
```

If anything inside `try` has a runtime error, the rest of the `try` part is skipped and the `catch` part runs. If nothing goes wrong, the `catch` part is skipped.

- `e.message` is the error's text, like `couldn't turn "abc" into an int`, and `e.where` is the file and line, like `main.frg:12`.
- **Every** runtime error can be caught: list positions, `null`, missing files, `int("abc")`, division by zero, and the ones from libraries. This includes errors in functions called from inside the `try`, however deep.
- Your own code can make an error with `throw "message";`. If nothing catches it, the game stops with that message, like any runtime error:

  ```cpp
  int loadLevel(string name){
      if (name == "") {
          throw "the level needs a name";
      }
      ...
  }
  ```

- Inside a `catch`, `throw e;` sends the same error on to an outer `try`.
- `try` and `catch` both need `{ }`, and `catch` always takes `(Error e)` (the name can be anything).
- A `try` around starting a task doesn't catch errors inside the task: put the `try` **inside** the task instead. `wait()` works inside `try { }`, but not inside `catch { }`.
- Don't use `try` to hide mistakes: a list position that's outside the list is better fixed than caught. `try` is for things your game can't control, like files and what the player types.

Crashes that don't give a message (`[game.exe crashed (code 0xC0000005)]`) usually mean a [pointer](10-enums-optionals-pointers.md#pointers) was used after the thing it pointed at went away, or a function called itself forever.

## The C++ compiler rejected Forge's output

If you ever see:

```
error: the C++ compiler rejected Forge's output - this is a bug in Forge, not in your code
```

then your code passed all of Forge's checks, but Forge produced something invalid. That's Forge's fault. The message shows where the generated C++ file is; please report it, with the code that caused it.

## What's next

[Reserved words](13-reserved-words.md).
