# Error messages

Every message Forge can show while building or running a game, with a small example that causes it, what it means, and how to fix it. Each example is built (and run) by Forge's tests, so the messages here are exactly what Forge prints.

To find a message, search this page (Ctrl+F) for a few words of it. Parts in quotes, like `'score'`, change with your code.

## How a message is laid out

<!-- check: error -->

```cpp
int main(){
    int score = "ten";
    return 0;
}
```

```text
error: 'score' needs an int, but this is a string
  --> main.frg:2:17
   |
 2 |     int score = "ten";
   |                 ^^^^^
  hint: use int(...) to turn text into a number
```

1. **`error:`** is what's wrong. Errors stop the build; **`warning:`** is something that's probably a mistake, but the game still builds.
2. **`-->`** is the file, the line and the column (how many characters into the line).
3. The line itself, with **`^^^`** under the exact part.
4. **`hint:`** (not always there) is how to fix it.

Forge shows every error it finds in a file at once, up to the first mistake in each function's punctuation. After fixing those, a few more can show up: the ones in code that couldn't be read until the punctuation was right.

Runtime errors happen while the game runs. They show the file and line, and the calls that led there (`called from ...`), and stop the game with exit code 1. A `try { ... } catch (Error e) { ... }` around the code catches them instead.

**The C++ compiler rejected Forge's output**: if you ever see an error from g++ (with C++ code in it) instead of a Forge message, that's a bug in Forge, not in your game. Please report it, with the code that caused it.

## Writing code: punctuation, braces and text

### `missing ';' at the end of this line`

A statement has to end with `;`. The arrow points at where it was expected, usually the end of the line before.

<!-- check: error -->

```cpp
int main(){
    int lives = 3
    lives--;
    return 0;
}
```

```text
error: missing ';' at the end of this line
  --> main.frg:2:18
   |
 2 |     int lives = 3
   |                  ^
```

**Fix:** add `;` at the end of the line the error points at.

### `expected a value, but found ';'`

Forge read something it didn't expect at this point. The message says what it wanted and what it found instead.

<!-- check: error -->

```cpp
int main(){
    int lives = ;
    return 0;
}
```

```text
error: expected a value, but found ';'
  --> main.frg:2:17
   |
 2 |     int lives = ;
   |                 ^
```

**Fix:** finish what the line started: here, give `lives` a value (`int lives = 3;`).

### `this '{' is never closed`

A `{` was opened and never closed. The arrow points at the `{`, not at where the `}` is missing.

<!-- check: error -->

```cpp
int main(){
    if (true) {
        return 1;
    return 0;
}
```

```text
error: this '{' is never closed
  --> main.frg:1:11
   |
 1 | int main(){
   |           ^
  hint: add a matching '}'
```

**Fix:** add the missing `}`. `frg fmt` indents the code by its braces, which shows where one is missing.

### `this '}' doesn't close anything`

A `}` with no `{` before it to close.

<!-- check: error -->

```cpp
int main(){
    return 0;
}
}
```

```text
error: this '}' doesn't close anything
  --> main.frg:4:1
   |
 4 | }
   | ^
  hint: remove it - or, if a function or type ends too early, look for an extra '}' above it
```

**Fix:** remove the extra `}`, or add the `{` it was meant to close.

### `'if' needs { } around its code`

`if`, `else`, `while` and `for` always need `{ }`, even around a single line.

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

**Fix:** put braces around it: `if (lives > 0) { lives--; }`.

### `'=' sets a variable - to compare two values, use '=='`

A single `=` sets a variable. Comparing is `==` (or the word `equals`).

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

**Fix:** write `if (x == 5)`.

### `this 'else' doesn't belong to an 'if'`

`else` has to come straight after the `}` of an `if`, with nothing in between.

<!-- check: error -->

```cpp
int main(){
    int x = 0;
    if (x > 0) {
        x--;
    }
    x++;
    else {
        x++;
    }
    return 0;
}
```

```text
error: this 'else' doesn't belong to an 'if'
  --> main.frg:7:5
   |
 7 |     else {
   |     ^^^^
  hint: else has to come right after the } of an if
```

**Fix:** move whatever is between the `}` and `else` (here, `x++;`) somewhere else.

### `this ';' is on its own, so it doesn't end anything`

A `;` with nothing before it. In C++ `};` is common, but in Forge a `}` is never followed by `;`.

<!-- check: error -->

```cpp
int main(){
    int x = 0;
    if (x > 0) {
        x--;
    };
    return 0;
}
```

```text
error: this ';' is on its own, so it doesn't end anything
  --> main.frg:5:6
   |
 5 |     };
   |      ^
  hint: remove it - a '}' doesn't need a ';' after it
```

**Fix:** remove the `;`.

### `text in Forge uses double quotes`

Text is written in double quotes.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print('hello');
    return 0;
}
```

```text
error: text in Forge uses double quotes
  --> main.frg:4:11
   |
 4 |     print('hello');
   |           ^
  hint: write "hello" instead of 'hello'
```

**Fix:** write `"hello"`. One letter is text too: `"a"`.

### `this text is missing its closing "`

Text has to start and end on the same line.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print("hello);
    return 0;
}
```

```text
error: this text is missing its closing "
  --> main.frg:4:11
   |
 4 |     print("hello);
   |           ^
  hint: text has to start and end on the same line
```

**Fix:** add the closing `"`. For text over several lines, use `\n`, or raw text between `"""`.

### `unknown escape '\g'`

A `\` in text starts an escape, and only some letters can follow it.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print("C:\games");
    return 0;
}
```

```text
error: unknown escape '\g'
  --> main.frg:4:14
   |
 4 |     print("C:\games");
   |              ^^
  hint: the escapes are \n (new line), \t (tab), \\ (backslash), \" (quote), \{ and \}
```

**Fix:** write `\\` for a backslash: `"C:\\games"`, or use `/`: `"C:/games"`.

### `this text is missing its closing "`

In text, `{` starts a value to show, like `{score}`. Without its `}`, Forge reads the rest of the line as part of that value, so the text never seems to end.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print("score: {score");
    return 0;
}
```

```text
error: this text is missing its closing "
  --> main.frg:4:11
   |
 4 |     print("score: {score");
   |           ^
  hint: text has to start and end on the same line
```

**Fix:** close it (`{score}`), or write `{{` or `\{` to show a `{`.

### `Forge uses ''' for multi-line comments, not /* */`

Forge's comments over several lines start and end with three backticks.

<!-- check: error -->

```cpp
/* the player */
int main(){
    return 0;
}
```

```text
error: Forge uses ``` for multi-line comments, not /* */
  --> main.frg:1:1
   |
 1 | /* the player */
   | ^^
  hint: start the comment with ``` and end it with ```
```

**Fix:** write the comment between ``` and ```, or use `//` on each line.

### `this ''' comment is never closed`

A comment started with three backticks never ends.

<!-- check: error -->

~~~cpp
```
the start of a comment
int main(){
    return 0;
}
~~~

```text
error: this ``` comment is never closed
  --> main.frg:1:1
   |
 1 | ```
   | ^^^
  hint: add ``` where the comment should end
```

**Fix:** add three backticks where the comment should end.

### `unexpected letter right after a number`

A number with a letter stuck to it, like `10px` or `3d`. Names can't start with a digit, and numbers have no units.

<!-- check: error -->

```cpp
int main(){
    int width = 10px;
    return 0;
}
```

```text
error: unexpected letter right after a number
  --> main.frg:2:19
   |
 2 |     int width = 10px;
   |                   ^
  hint: put a space or an operator between them
```

**Fix:** remove the letters (`int width = 10;`), or rename the variable so it doesn't start with a digit.

### `unexpected character '@'`

A character Forge doesn't use anywhere, like `@`, `$` or `#` in the middle of code.

<!-- check: error -->

```cpp
int main(){
    int cost = 5 @ 2;
    return 0;
}
```

```text
error: unexpected character '@'
  --> main.frg:2:18
   |
 2 |     int cost = 5 @ 2;
   |                  ^
```

**Fix:** remove it. If it came from copying code from somewhere else, check that line.

### `this code has to go inside a function`

Only declarations can be at the top level of a file: variables, functions, types and `#include`s. Code that does something goes inside a function.

<!-- check: error -->

```cpp
#include <stdio>

print("hello");

int main(){
    return 0;
}
```

```text
error: this code has to go inside a function
  --> main.frg:3:1
   |
 3 | print("hello");
   | ^^^^^
  hint: put it inside main(){ ... } - or, if you're making a new function, give it a return type like 'void print()'
```

**Fix:** move it into `main` (or a function `main` calls).

### `Forge doesn't need function declarations - just write the function with its { body }`

In C and C++ a function is often declared first, then written later. Forge doesn't need that: every function can be used from anywhere.

<!-- check: error -->

```cpp
int add(int a, int b);

int main(){
    return 0;
}
```

```text
error: Forge doesn't need function declarations - just write the function with its { body }
  --> main.frg:1:22
   |
 1 | int add(int a, int b);
   |                      ^
  hint: functions can be used before or after the place they're written
```

**Fix:** remove the declaration; just write the function with its `{ }`.

### `make one variable per line`

Each variable gets its own line in Forge.

<!-- check: error -->

```cpp
int main(){
    int a, b;
    return 0;
}
```

```text
error: make one variable per line
  --> main.frg:2:10
   |
 2 |     int a, b;
   |          ^
  hint: for example: int a = 1;  int b = 2;
```

**Fix:** write `int a = 0;` and `int b = 0;` on two lines.

### `struct types have to be made outside of functions`

Types (structs, classes, enums, interfaces) are made at the top level of a file, not inside a function.

<!-- check: error -->

```cpp
int main(){
    struct Point {
        int x = 0;
    }
    return 0;
}
```

```text
error: struct types have to be made outside of functions
  --> main.frg:2:5
   |
 2 |     struct Point {
   |     ^^^^^^
  hint: move it to the top level of the file
```

**Fix:** move the type outside `main`.

### `nothing inside { }`

A `{ }` in text has to hold a value to show.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print("{}");
    return 0;
}
```

```text
error: nothing inside { }
  --> main.frg:4:12
   |
 4 |     print("{}");
   |            ^^
  hint: put a value inside, like {score}
```

**Fix:** put a value inside (`{score}`), or write `{{}}` to show the braces.

### `unexpected '++' inside { }`

`++` and `--` change a variable, so they're statements of their own, not part of a value.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    int u = 5;
    print("{u++}");
    return 0;
}
```

```text
error: unexpected '++' inside { }
  --> main.frg:5:14
   |
 5 |     print("{u++}");
   |              ^^
```

**Fix:** do it on its own line first: `u++;` then `print("{u}");`.

### `'new' isn't available yet`

Words that Forge keeps for the future can't be used yet.

<!-- check: error -->

```cpp
int main(){
    int x = new int(5);
    return 0;
}
```

```text
error: 'new' isn't available yet
  --> main.frg:2:13
   |
 2 |     int x = new int(5);
   |             ^^^
  hint: it's saved for a future version of Forge
```

**Fix:** objects are made by calling their type: `Enemy()`, not `new Enemy()`.

## Writing code: if, loops, switch, try

### `expected 'case' or 'default' inside the switch, but found 'n'`

Inside a `switch`, every part starts with `case value:` or `default:`.

<!-- check: error -->

```cpp
int main(){
    int n = 1;
    switch (n) {
        n++;
    }
    return 0;
}
```

```text
error: expected 'case' or 'default' inside the switch, but found 'n'
  --> main.frg:4:9
   |
 4 |         n++;
   |         ^

error: expected a function, a variable, a type or an #include, but found 'return'
  --> main.frg:6:5
   |
 6 |     return 0;
   |     ^^^^^^
```

**Fix:** start the part with `case 1:` (or `default:`).

### `a switch doesn't need 'break'`

In Forge each `case` stops by itself at the next one, so `break` isn't needed (and isn't allowed) in a switch.

<!-- check: error -->

```cpp
int main(){
    int n = 1;
    switch (n) {
        case 1:
            n++;
            break;
    }
    return 0;
}
```

```text
error: a switch doesn't need 'break'
  --> main.frg:6:13
   |
 6 |             break;
   |             ^^^^^
  hint: in Forge, each case stops by itself at the next case - remove this break
```

**Fix:** remove the `break`.

### `this switch already has a default`

A switch can have only one `default`.

<!-- check: error -->

```cpp
int main(){
    int n = 1;
    switch (n) {
        default:
            n++;
        default:
            n--;
    }
    return 0;
}
```

```text
error: this switch already has a default
  --> main.frg:6:9
   |
 6 |         default:
   |         ^^^^^^^
```

**Fix:** merge the two.

### `switch works on whole numbers, text and enums, but this is a float`

`switch` works on whole numbers, text and enums.

<!-- check: error -->

```cpp
int main(){
    float speed = 1.5;
    switch (speed) {
        case 1.5:
            speed = 0;
    }
    return 0;
}
```

```text
error: switch works on whole numbers, text and enums, but this is a float
  --> main.frg:3:13
   |
 3 |     switch (speed) {
   |             ^^^^^
```

**Fix:** use `if` / `else if` for decimal numbers.

### `a 'do { ... }' needs 'while (...)' after its closing }`

`do { ... }` needs `while (condition);` after its `}`.

<!-- check: error -->

```cpp
int main(){
    int n = 0;
    do {
        n++;
    }
    return 0;
}
```

```text
error: a 'do { ... }' needs 'while (...)' after its closing }
  --> main.frg:6:5
   |
 6 |     return 0;
   |     ^^^^^^
  hint: for example: do { ... } while (lives > 0);
```

**Fix:** add the condition: `} while (n < 3);`.

### `a 'try { ... }' needs 'catch (Error e) { ... }' after its closing }`

`try { ... }` needs a `catch` part after it.

<!-- check: error -->

```cpp
int main(){
    try {
        int x = 1;
    }
    return 0;
}
```

```text
error: a 'try { ... }' needs 'catch (Error e) { ... }' after its closing }
  --> main.frg:5:5
   |
 5 |     return 0;
   |     ^^^^^^
  hint: the catch part runs if something goes wrong in the try part
```

**Fix:** add `catch (Error e) { ... }` after the `}`.

### `this 'catch' doesn't belong to a 'try'`

`catch` has to come straight after the `}` of a `try`.

<!-- check: error -->

```cpp
int main(){
    catch (Error e) {
        int x = 1;
    }
    return 0;
}
```

```text
error: this 'catch' doesn't belong to a 'try'
  --> main.frg:2:5
   |
 2 |     catch (Error e) {
   |     ^^^^^
  hint: catch has to come right after the } of a try
```

**Fix:** put a `try { ... }` before it, or remove it.

### `catch takes an Error, like: catch (Error message)`

`catch` always catches an `Error`.

<!-- check: error -->

```cpp
int main(){
    try {
        int x = 1;
    } catch (string message) {
        int y = 2;
    }
    return 0;
}
```

```text
error: catch takes an Error, like: catch (Error message)
  --> main.frg:4:14
   |
 4 |     } catch (string message) {
   |              ^^^^^^
  hint: e.message is the error's text, and e.where is the file and line it happened on
```

**Fix:** write `catch (Error e)`, and use `e.message` for the text.

### `throw takes text (the error message), but this is an int`

`throw` takes the error's message, as text.

<!-- check: error -->

```cpp
int main(){
    throw 404;
    return 0;
}
```

```text
error: throw takes text (the error message), but this is an int
  --> main.frg:2:11
   |
 2 |     throw 404;
   |           ^^^
  hint: for example: throw "the save file is broken";
```

**Fix:** `throw "page 404 not found";`.

### `'break' can only be used inside a loop`

`break` and `continue` only make sense inside a loop.

<!-- check: error -->

```cpp
int main(){
    break;
    return 0;
}
```

```text
error: 'break' can only be used inside a loop
  --> main.frg:2:5
   |
 2 |     break;
   |     ^^^^^
```

**Fix:** remove it, or use `return` to leave the function.

### `for (... in ...) goes through a list, but this is an int`

A `for (... in ...)` loop goes through a list (or a range like `1..10`).

<!-- check: error -->

```cpp
int main(){
    int count = 5;
    for (int i in count) {
        count--;
    }
    return 0;
}
```

```text
error: for (... in ...) goes through a list, but this is an int
  --> main.frg:3:19
   |
 3 |     for (int i in count) {
   |                   ^^^^^
```

**Fix:** count with a range: `for (int i in 1..count)`, or loop through a list.

### `a counting loop counts whole numbers, so 'x' has to be an int`

The numbers in a range loop are whole numbers.

<!-- check: error -->

```cpp
int main(){
    for (float x in 1..5) {
        int y = 0;
    }
    return 0;
}
```

```text
error: a counting loop counts whole numbers, so 'x' has to be an int
  --> main.frg:2:10
   |
 2 |     for (float x in 1..5) {
   |          ^^^^^
  hint: write it as: for (int x in ...)
```

**Fix:** use `int` for the loop variable, and work out the decimal inside: `float x = i * 0.5;`.

### `for (... in ...) goes through a list, but this is a map<string, int>`

A loop over a map goes through its keys.

<!-- check: error -->

```cpp
int main(){
    map<string, int> ages = {"Ana": 12};
    for (int age in ages) {
        age++;
    }
    return 0;
}
```

```text
error: for (... in ...) goes through a list, but this is a map<string, int>
  --> main.frg:3:21
   |
 3 |     for (int age in ages) {
   |                     ^^^^
  hint: go through its keys: for (K key in m.keys())
```

**Fix:** `for (string name in ages.keys()) { int age = ages[name]; }`.

### `the condition of 'if' must be true or false, but this is an int`

The thing between `if (` and `)` has to be `true` or `false`. Forge doesn't treat numbers or objects as true/false.

<!-- check: error -->

```cpp
int main(){
    int lives = 3;
    if (lives) {
        lives--;
    }
    return 0;
}
```

```text
error: the condition of 'if' must be true or false, but this is an int
  --> main.frg:3:9
   |
 3 |     if (lives) {
   |         ^^^^^
  hint: compare it, like (x > 0)
```

**Fix:** compare it: `if (lives > 0)`, or for an optional, `if (target != null)`.

## Names

### `unknown name 'scroe'`

Nothing with this name exists here. Often a typo (Forge suggests the closest name), or a variable made inside `{ }` and used outside them.

<!-- check: error -->

```cpp
int main(){
    int score = 0;
    scroe++;
    return 0;
}
```

```text
error: unknown name 'scroe'
  --> main.frg:3:5
   |
 3 |     scroe++;
   |     ^^^^^
```

**Fix:** fix the spelling, or make the variable where both places can see it.

### `unknown type 'Enemey'`

The type isn't known: a typo, a type in another file without its file name in front, or a library not included.

<!-- check: error -->

```cpp
class Enemy {
    int hp = 10;
}

int main(){
    Enemey e = Enemey();
    return 0;
}
```

```text
error: unknown type 'Enemey'
  --> main.frg:6:5
   |
 6 |     Enemey e = Enemey();
   |     ^^^^^^
  hint: did you mean 'Enemy'?

error: unknown name 'Enemey'
  --> main.frg:6:16
   |
 6 |     Enemey e = Enemey();
   |                ^^^^^^
  hint: did you mean 'Enemy'?
```

**Fix:** fix the spelling. A type from another file is written `file.Type`.

### `'print' is part of the stdio library, which this file doesn't include`

A function from a library needs that library's `#include`.

<!-- check: error -->

```cpp
int main(){
    print("hi");
    return 0;
}
```

```text
error: 'print' is part of the stdio library, which this file doesn't include
  --> main.frg:2:5
   |
 2 |     print("hi");
   |     ^^^^^
  hint: add #include <stdio> at the top of main.frg
```

**Fix:** add `#include <stdio>` at the top of the file. Each file includes what it uses.

### `this file doesn't include the math library`

Using a library (here `math`) without including it in this file.

<!-- check: error -->

```cpp
int main(){
    float r = math.sqrt(16);
    return 0;
}
```

```text
error: this file doesn't include the math library
  --> main.frg:2:15
   |
 2 |     float r = math.sqrt(16);
   |               ^^^^
  hint: add #include <math> at the top of main.frg
```

**Fix:** add `#include <math>` at the top of this file.

### `there's no library called <maths>`

There's no library with that name.

<!-- check: error -->

```cpp
#include <maths>

int main(){
    return 0;
}
```

```text
error: there's no library called <maths>
  --> main.frg:1:11
   |
 1 | #include <maths>
   |           ^^^^^
  hint: did you mean <math>?
```

**Fix:** check the spelling. The libraries are listed in [the library overview](../libraries/README.md).

### `there's already a variable called 'score' here`

Two variables with the same name where both can be seen.

<!-- check: error -->

```cpp
int main(){
    int score = 0;
    int score = 5;
    return 0;
}
```

```text
error: there's already a variable called 'score' here
  --> main.frg:3:9
   |
 3 |     int score = 5;
   |         ^^^^^
  hint: pick a different name - or to change the existing one, leave off the type: score = ...;
```

**Fix:** give the second one another name, or leave off the type to change the first one: `score = 5;`.

### `'score' is defined twice in this file`

Two functions, globals or types with the same name in one file.

<!-- check: error -->

```cpp
int score = 0;
int score = 1;

int main(){
    return 0;
}
```

```text
error: 'score' is defined twice in this file
  --> main.frg:2:5
   |
 2 | int score = 1;
   |     ^^^^^
  hint: the first one is on line 1
```

**Fix:** rename one of them.

### `'window' is the name of a Forge library`

A name can't be a library's name, since `window.open` would be ambiguous.

<!-- check: error -->

```cpp
int main(){
    int window = 3;
    return 0;
}
```

```text
error: 'window' is the name of a Forge library
  --> main.frg:2:9
   |
 2 |     int window = 3;
   |         ^^^^^^
  hint: pick a different name
```

**Fix:** pick another name, like `windowCount`.

### `'string' is a built-in type name, so it can't be used as a name`

Type names like `int`, `string` and `list` can't be used as names.

<!-- check: error -->

```cpp
int main(){
    int string = 3;
    return 0;
}
```

```text
error: 'string' is a built-in type name, so it can't be used as a name
  --> main.frg:2:9
   |
 2 |     int string = 3;
   |         ^^^^^^
```

**Fix:** pick another name.

### `'negative' is a reserved word in Forge, so it can't be a parameter name`

A reserved word (like `negative`, `and`, `is` or `do`) where a name goes: a variable, a parameter, a field or a method. The end of the message says which kind of name it was.

<!-- check: error -->

```cpp
int add(int a, int negative){
    return a + negative;
}

int main(){
    return add(1, 2);
}
```

```text
error: 'negative' is a reserved word in Forge, so it can't be a parameter name
  --> main.frg:1:20
   |
 1 | int add(int a, int negative){
   |                    ^^^^^^^^
  hint: pick a different name - the reserved words are listed in the docs (Reserved words)
```

**Fix:** pick another name, like `amount`. The full list is in [Reserved words](../language/13-reserved-words.md).

### `'feed' is already the name of a function in this file`

A variable with the same name as a function in the same file. Only this line gets an error: the lines after it that use the variable don't.

<!-- check: error -->

```cpp
void feed(){
}

int main(){
    int feed = 5;
    feed++;
    return feed;
}
```

```text
error: 'feed' is already the name of a function in this file
  --> main.frg:5:9
   |
 5 |     int feed = 5;
   |         ^^^^
  hint: pick a different name
```

**Fix:** rename the variable (like `feedCount`), or the function.

### `'enemies' is a file, not a value`

A file's name is used to reach what's inside it (`enemies.spawn()`), so it isn't a value itself.

<!-- check: error -->

```cpp
// file: main.frg
#include <stdio>

int main(){
    print(enemies);
    return 0;
}
```

```cpp
// file: enemies.frg
int count = 3;
```

```text
error: 'enemies' is a file, not a value
  --> main.frg:5:11
   |
 5 |     print(enemies);
   |           ^^^^^^^
  hint: use it to reach things inside it, like enemies.something
```

**Fix:** use something inside it: `enemies.count`, `enemies.spawn()`.

### `'this' only works inside the methods of a struct or class`

`this` means "the object this method belongs to", so it only exists inside methods.

<!-- check: error -->

```cpp
int main(){
    int x = this.x;
    return 0;
}
```

```text
error: 'this' only works inside the methods of a struct or class
  --> main.frg:2:13
   |
 2 |     int x = this.x;
   |             ^^^^
```

**Fix:** use it in a struct's or class's method, or remove it.

### `'code' is private to Safe`

Something marked `private` can only be used inside the type (or file) it belongs to.

<!-- check: error -->

```cpp
class Safe {
    private int code = 1234;
}

int main(){
    Safe s = Safe();
    int c = s.code;
    return 0;
}
```

```text
error: 'code' is private to Safe
  --> main.frg:7:15
   |
 7 |     int c = s.code;
   |               ^^^^
  hint: only Safe's own methods (and classes that inherit from it) can use it
```

**Fix:** use it from inside `Safe`, or remove `private` if other code needs it.

## Types and values

### `'score' needs an int, but this is a string`

A value of one type was put where another type is needed. The hint says how to convert, when that's possible.

<!-- check: error -->

```cpp
int main(){
    int score = "ten";
    return 0;
}
```

```text
error: 'score' needs an int, but this is a string
  --> main.frg:2:17
   |
 2 |     int score = "ten";
   |                 ^^^^^
  hint: use int(...) to turn text into a number
```

**Fix:** give a value of the right type, or convert: `int("10")`, `string(10)`, `float(n)`.

### `'x' needs an int, but this is a float`

Decimal numbers don't turn into whole numbers by themselves, since the decimals would be lost.

<!-- check: error -->

```cpp
int main(){
    int x = 2.5;
    return 0;
}
```

```text
error: 'x' needs an int, but this is a float
  --> main.frg:2:13
   |
 2 |     int x = 2.5;
   |             ^^^
  hint: use int(...) to turn it into a whole number (the decimals get cut off)
```

**Fix:** say how: `int(2.5)` (cuts off: 2), `math.round(2.5)` (3), `math.floor`, `math.ceil`.

### `can't add text and an int with +`

`+` on text only joins two pieces of text.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    int hp = 5;
    print("HP: " + hp);
    return 0;
}
```

```text
error: can't add text and an int with +
  --> main.frg:5:18
   |
 5 |     print("HP: " + hp);
   |                  ^
  hint: put the value inside the text instead, like "Score: {score}"
```

**Fix:** put the value inside the text: `"HP: {hp}"`.

### `can't use '*' on a bool and an int`

Math works on numbers (and on `vec2`, `vec3`...), not on text, bools or objects without math methods.

<!-- check: error -->

```cpp
int main(){
    bool done = true;
    int x = done * 2;
    return 0;
}
```

```text
error: can't use '*' on a bool and an int
  --> main.frg:3:18
   |
 3 |     int x = done * 2;
   |                  ^
```

**Fix:** use a number, or `done ? 2 : 0`.

### `this list needs an int, but this is a string`

A list holds one type of item.

<!-- check: error -->

```cpp
int main(){
    list<int> xs = [1, "two", 3];
    return 0;
}
```

```text
error: this list needs an int, but this is a string
  --> main.frg:2:24
   |
 2 |     list<int> xs = [1, "two", 3];
   |                        ^^^^^
  hint: use int(...) to turn text into a number
```

**Fix:** make every item the same type.

### `Forge can't tell what kind of list [] is`

`[]` on its own doesn't say what the list will hold.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    print([]);
    return 0;
}
```

```text
error: Forge can't tell what kind of list [] is
  --> main.frg:4:11
   |
 4 |     print([]);
   |           ^
  hint: say the type first, like: list<int> numbers = [];
```

**Fix:** write the type where it's made: `list<int> xs = [];`.

### `list positions are whole numbers, but this is a float`

List positions are whole numbers.

<!-- check: error -->

```cpp
int main(){
    list<int> xs = [1, 2, 3];
    int x = xs[1.5];
    return 0;
}
```

```text
error: list positions are whole numbers, but this is a float
  --> main.frg:3:16
   |
 3 |     int x = xs[1.5];
   |                ^^^
```

**Fix:** use `int(...)` to make the position a whole number.

### `[ ] only works on lists and maps, but this is an int`

`[ ]` reads an item of a list or map.

<!-- check: error -->

```cpp
int main(){
    int n = 5;
    int x = n[0];
    return 0;
}
```

```text
error: [ ] only works on lists and maps, but this is an int
  --> main.frg:3:14
   |
 3 |     int x = n[0];
   |              ^
```

**Fix:** use a list, or remove the `[0]`.

### `text can't be used with [ ]`

Text can't be read with `[ ]`.

<!-- check: error -->

```cpp
int main(){
    string name = "Ana";
    string first = name[0];
    return 0;
}
```

```text
error: text can't be used with [ ]
  --> main.frg:3:24
   |
 3 |     string first = name[0];
   |                        ^
  hint: use text.charAt(i) or text.substring(start, count)
```

**Fix:** `name.charAt(0)` gives the first letter, `name.substring(0, 2)` the first two.

### `this number doesn't fit in a uint8`

A number written in the code is too big (or negative) for its type.

<!-- check: error -->

```cpp
int main(){
    uint8 level = 300;
    return 0;
}
```

```text
error: this number doesn't fit in a uint8
  --> main.frg:2:19
   |
 2 |     uint8 level = 300;
   |                   ^^^
  hint: uint8 can only hold 0 and positive whole numbers
```

**Fix:** use a bigger type (`int`), or a number that fits (0 to 255 for `uint8`).

### `'MAX_HP' is const, so it can't be changed`

`const` variables can't change after they're made.

<!-- check: error -->

```cpp
const int MAX_HP = 100;

int main(){
    MAX_HP = 200;
    return 0;
}
```

```text
error: 'MAX_HP' is const, so it can't be changed
  --> main.frg:4:5
   |
 4 |     MAX_HP = 200;
   |     ^^^^^^
```

**Fix:** remove `const` if it has to change.

### `only variables, fields and list items can be changed`

Only variables, fields and list items can be set. Not a function's result, a number, or a calculation.

<!-- check: error -->

```cpp
int main(){
    int x = 1;
    x + 1 = 3;
    return 0;
}
```

```text
error: only variables, fields and list items can be changed
  --> main.frg:3:7
   |
 3 |     x + 1 = 3;
   |       ^
```

**Fix:** set a variable: `x = 3 - 1;`.

### `length can't be changed directly`

`length` is read-only.

<!-- check: error -->

```cpp
int main(){
    list<int> xs = [1, 2, 3];
    xs.length = 0;
    return 0;
}
```

```text
error: length can't be changed directly
  --> main.frg:3:8
   |
 3 |     xs.length = 0;
   |        ^^^^^^
  hint: use add, remove or clear instead
```

**Fix:** use `xs.clear()`, `add` or `remove`.

### `void means 'nothing', so a variable can't be void`

`void` means "nothing": it's only for functions that give nothing back.

<!-- check: error -->

```cpp
int main(){
    void nothing = 0;
    return 0;
}
```

```text
error: void means 'nothing', so a variable can't be void
  --> main.frg:2:5
   |
 2 |     void nothing = 0;
   |     ^^^^
  hint: use a type like int, float, bool or string
```

**Fix:** use a real type.

### `map keys have to be numbers, text, bools or enums - not a Point`

Map keys are numbers, text, bools or enums.

<!-- check: error -->

```cpp
struct Point {
    int x = 0;
}

int main(){
    map<Point, int> scores = {};
    return 0;
}
```

```text
error: map keys have to be numbers, text, bools or enums - not a Point
  --> main.frg:6:9
   |
 6 |     map<Point, int> scores = {};
   |         ^^^^^

error: Forge can't tell what { } should make here
  --> main.frg:6:30
   |
 6 |     map<Point, int> scores = {};
   |                              ^
  hint: { } builds structs, classes and maps, but <error> is none of those
```

**Fix:** use a number or text as the key (like a name or an id).

### `both sides of ? : need the same type, but they're an int and a string`

`? :` picks one of two values, so both have to be the same type.

<!-- check: error -->

```cpp
int main(){
    int lives = 3;
    int x = lives > 0 ? 1 : "none";
    return 0;
}
```

```text
error: both sides of ? : need the same type, but they're an int and a string
  --> main.frg:3:23
   |
 3 |     int x = lives > 0 ? 1 : "none";
   |                       ^
```

**Fix:** make both sides the same type.

### `'p' can't be null, because a Player isn't optional`

Only optional types (with `?`) can be `null`.

<!-- check: error -->

```cpp
class Player {
    int hp = 10;
}

int main(){
    Player p = null;
    return 0;
}
```

```text
error: 'p' can't be null, because a Player isn't optional
  --> main.frg:6:16
   |
 6 |     Player p = null;
   |                ^^^^
  hint: null can only go in an optional type - add ? to the type, like Player?
```

**Fix:** make it optional: `Player? p = null;`.

### `{...:.2} shows decimals, so it needs a float or a double, but this is an int`

`{value:.2}` shows a decimal number with 2 decimals, so the value has to be a `float` or `double`.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    int hp = 5;
    print("{hp:.2}");
    return 0;
}
```

```text
error: {...:.2} shows decimals, so it needs a float or a double, but this is an int
  --> main.frg:5:13
   |
 5 |     print("{hp:.2}");
   |             ^^
  hint: turn it into a decimal number first, like {float(hp):.2}
```

**Fix:** use `{float(hp):.2}`, or leave off the `:.2`.

### `can't use '==' on a Button: its field 'onClick' is a Function<void()>, and function values can't be compared`

Some types can't be compared with `==`: function values, `Weak` values, tasks, and structs that hold them.

<!-- check: error -->

```cpp
struct Button {
    string name;
    Function<void()> onClick;
}

int main(){
    Button b = Button("ok", () { });
    bool same = b == b;
    return 0;
}
```

```text
error: can't use '==' on a Button: its field 'onClick' is a Function<void()>, and function values can't be compared
  --> main.frg:8:19
   |
 8 |     bool same = b == b;
   |                   ^^
  hint: give Button a method that says when two of them are equal: bool equals(Button other) { ... } - or compare one of its fields instead
```

**Fix:** compare a field instead (`a.name == b.name`), or give the struct `bool equals(Button other) { ... }`.

### `contains() compares the items with ==, but a Button can't be compared: its field 'onClick' is a Function<void()>, and function values can't be compared`

`list.contains`, `indexOf` and `remove` compare with `==`, so the items must be comparable.

<!-- check: error -->

```cpp
struct Button {
    string name;
    Function<void()> onClick;
}

int main(){
    list<Button> buttons = [];
    bool has = buttons.contains(Button("ok", () { }));
    return 0;
}
```

```text
error: contains() compares the items with ==, but a Button can't be compared: its field 'onClick' is a Function<void()>, and function values can't be compared
  --> main.frg:8:24
   |
 8 |     bool has = buttons.contains(Button("ok", () { }));
   |                        ^^^^^^^^
  hint: give Button a method that says when two of them are equal: bool equals(Button other) { ... } - or compare one of its fields instead
```

**Fix:** use `any((Button b) { return b.name == "ok"; })`, or give `Button` an `equals` method.

### `Forge doesn't have pointers (Pointer<T> was removed)`

Forge doesn't have pointers.

<!-- check: error -->

```cpp
int main(){
    int score = 0;
    Pointer<int> p = score;
    return 0;
}
```

```text
error: Forge doesn't have pointers (Pointer<T> was removed)
  --> main.frg:3:5
   |
 3 |     Pointer<int> p = score;
   |     ^^^^^^^^^^^^
  hint: to let a function change a number, return the new value: score = addOne(score); - or keep the value in a class object, which functions can change
```

**Fix:** to let a function change a number, give back the new value: `score = addOne(score);`. Or keep it in a class object.

## Functions and calls

### `add() needs 2 values, but got 3`

A function was called with the wrong number of values.

<!-- check: error -->

```cpp
int add(int a, int b){
    return a + b;
}

int main(){
    int x = add(1, 2, 3);
    return 0;
}
```

```text
error: add() needs 2 values, but got 3
  --> main.frg:6:13
   |
 6 |     int x = add(1, 2, 3);
   |             ^^^
  hint: it's written as: int add(int a, int b)
```

**Fix:** give exactly the values it takes. The hint shows the function's parameters.

### `'n' in twice() needs an int, but this is a string`

A value given to a function is the wrong type.

<!-- check: error -->

```cpp
int twice(int n){
    return n * 2;
}

int main(){
    int x = twice("4");
    return 0;
}
```

```text
error: 'n' in twice() needs an int, but this is a string
  --> main.frg:6:19
   |
 6 |     int x = twice("4");
   |                   ^^^
  hint: use int(...) to turn text into a number
```

**Fix:** give the type it takes, or convert it.

### `none of the versions of area() take these values`

Several functions share this name (overloading), and none takes these values.

<!-- check: error -->

```cpp
int area(int side){
    return side * side;
}

int area(int w, int h){
    return w * h;
}

int main(){
    int x = area("big");
    return 0;
}
```

```text
error: none of the versions of area() take these values
  --> main.frg:10:13
    |
 10 |     int x = area("big");
    |             ^^^^
  hint: it can be used as:
      int area(int side)
      int area(int w, int h)
```

**Fix:** use one of the versions the hint lists.

### `'sign' doesn't always return a value`

A function that gives back a value has to `return` one on every way through it, including after the last `if`.

<!-- check: error -->

```cpp
int sign(int n){
    if (n > 0) {
        return 1;
    }
}

int main(){
    return 0;
}
```

```text
error: 'sign' doesn't always return a value
  --> main.frg:1:5
   |
 1 | int sign(int n){
   |     ^^^^
  hint: add a return at the end, like 'return 0;'
```

**Fix:** add a `return` at the end.

### `'reset' is a void function, so it can't return a value`

A `void` function gives nothing back, so `return` can't have a value.

<!-- check: error -->

```cpp
void reset(){
    return 5;
}

int main(){
    return 0;
}
```

```text
error: 'reset' is a void function, so it can't return a value
  --> main.frg:2:12
   |
 2 |     return 5;
   |            ^
  hint: remove the value, or change 'void' to the type it should return
```

**Fix:** write `return;`, or change `void` to the type it gives back.

### `'five' has to return an int`

A function that gives back a value needs one in every `return`.

<!-- check: error -->

```cpp
int five(){
    return;
}

int main(){
    return 0;
}
```

```text
error: 'five' has to return an int
  --> main.frg:2:5
   |
 2 |     return;
   |     ^^^^^^
  hint: for example: return 0;
```

**Fix:** `return 5;`.

### `'speed' is a variable, not a function`

A variable that isn't a function value can't be called with `( )`.

<!-- check: error -->

```cpp
int main(){
    int speed = 5;
    speed();
    return 0;
}
```

```text
error: 'speed' is a variable, not a function
  --> main.frg:3:5
   |
 3 |     speed();
   |     ^^^^^
```

**Fix:** remove the `( )`, or call the function you meant.

### `main.frg needs a main function`

Every game starts in `main`, in `main.frg`.

<!-- check: error -->

```cpp
int start(){
    return 0;
}
```

```text
error: main.frg needs a main function
  --> main.frg:1:1
   |
 1 | int start(){
   | ^
  hint: every game starts in main:

      int main(){
          return 0;
      }
```

**Fix:** rename the function to `main`, or add `int main(){ ... return 0; }`.

### `main has to return an int (0 means success, 1 means an error)`

`main` gives back a whole number: 0 means the game ended normally.

<!-- check: error -->

```cpp
void main(){
}
```

```text
error: main has to return an int (0 means success, 1 means an error)
  --> main.frg:1:1
   |
 1 | void main(){
   | ^^^^
  hint: write it as: int main()
```

**Fix:** write `int main(){ ... return 0; }`.

### `main can't take parameters`

`main` takes no parameters.

<!-- check: error -->

```cpp
int main(int level){
    return 0;
}
```

```text
error: main can't take parameters
  --> main.frg:1:5
   |
 1 | int main(int level){
   |     ^^^^
  hint: write it as: int main()
```

**Fix:** remove the parameter.

### `'y' needs a default value too`

After a parameter with a default value, every parameter needs one.

<!-- check: error -->

```cpp
void spawn(int x = 0, int y){
}

int main(){
    return 0;
}
```

```text
error: 'y' needs a default value too
  --> main.frg:1:27
   |
 1 | void spawn(int x = 0, int y){
   |                           ^
  hint: once a parameter has a default value, the ones after it need one as well
```

**Fix:** give `y` a default too, or move it before `x`.

### `print() takes one value`

`print` shows one value. Several go inside one piece of text.

<!-- check: error -->

```cpp
#include <stdio>

int main(){
    int x = 1;
    int y = 2;
    print(x, y);
    return 0;
}
```

```text
error: print() takes one value
  --> main.frg:6:5
   |
 6 |     print(x, y);
   |     ^^^^^
  hint: to show several values, put them in the text: print("x = {x}, y = {y}")
```

**Fix:** `print("x = {x}, y = {y}");`.

### `window.run() calls update(float dt) and draw() every frame, so main.frg needs both`

`window.run()` calls your `update(float dt)` and `draw()` every frame, so `main.frg` needs both.

<!-- check: error -->

```cpp
#include <window>

int main(){
    window.open(320, 200, "game");
    window.run();
    return 0;
}
```

```text
error: window.run() calls update(float dt) and draw() every frame, so main.frg needs both
  --> main.frg:5:12
   |
 5 |     window.run();
   |            ^^^
  hint: add them to main.frg:

      void update(float dt){
          // move things here
      }

      void draw(){
          // draw things here
      }
```

**Fix:** add `void update(float dt){ }` and `void draw(){ }` to `main.frg`.

### `for window.run(), update has to be written as: void update(float dt)`

`window.run()` calls `update` with the frame's time, so it has to take a `float`.

<!-- check: error -->

```cpp
#include <window>

int main(){
    window.open(320, 200, "game");
    window.run();
    return 0;
}

void update(){
}

void draw(){
}
```

```text
error: for window.run(), update has to be written as: void update(float dt)
  --> main.frg:9:6
   |
 9 | void update(){
   |      ^^^^^^
```

**Fix:** write `void update(float dt)`.

### `this line doesn't do anything`

A value on its own line that isn't used: it calculates something and throws it away.

<!-- check: error -->

```cpp
int main(){
    int x = 1;
    x + 1;
    return 0;
}
```

```text
error: this line doesn't do anything
  --> main.frg:3:7
   |
 3 |     x + 1;
   |       ^
  hint: a line should call a function or change a variable
```

**Fix:** store it (`x = x + 1;`), or remove the line.

### `a function value can't change 'count', a local variable from outside it`

A function value gets a copy of the local variables around it, so changing one inside it would change only its copy.

<!-- check: error -->

```cpp
int main(){
    int count = 0;
    Function<void()> add = () { count++; };
    return 0;
}
```

```text
error: a function value can't change 'count', a local variable from outside it
  --> main.frg:3:33
   |
 3 |     Function<void()> add = () { count++; };
   |                                 ^^^^^
  hint: it only gets a copy of the variables around it. To share a value, use a global variable or a field
```

**Fix:** use a global variable or a class object's field for values that have to be shared.

### `'twice' needs 1 value, but got 2`

A number of values doesn't match the function value's type.

<!-- check: error -->

```cpp
int main(){
    Function<int(int)> twice = (int n) { return n * 2; };
    int x = twice(1, 2);
    return 0;
}
```

```text
error: 'twice' needs 1 value, but got 2
  --> main.frg:3:13
   |
 3 |     int x = twice(1, 2);
   |             ^^^^^
  hint: it's a Function<int(int)>
```

**Fix:** give exactly the values in its `Function<...>` type.

### `only a type can be followed by types inside < >, like Box<int>(5)`

Only types can have `<types>` after their name.

<!-- check: error -->

```cpp
int twice(int n){
    return n * 2;
}

int main(){
    int x = twice<int>(3);
    return 0;
}
```

```text
error: only a type can be followed by types inside < >, like Box<int>(5)
  --> main.frg:6:13
   |
 6 |     int x = twice<int>(3);
   |             ^^^^^^^^^^
  hint: 'twice' isn't a type, so write it without the < >
```

**Fix:** remove the `<int>`.

## Structs, classes and enums

### `Enemy has no field or method called 'hpp'`

The type has nothing with this name. Forge suggests the closest one.

<!-- check: error -->

```cpp
class Enemy {
    int hp = 10;
}

int main(){
    Enemy e = Enemy();
    e.hpp = 5;
    return 0;
}
```

```text
error: Enemy has no field or method called 'hpp'
  --> main.frg:7:7
   |
 7 |     e.hpp = 5;
   |       ^^^
  hint: did you mean 'hp'?
```

**Fix:** fix the spelling.

### `'e' needs a starting value`

A class object has to start as something: Forge can't make one up.

<!-- check: error -->

```cpp
class Enemy {
    int hp = 10;
}

int main(){
    Enemy e;
    return 0;
}
```

```text
error: 'e' needs a starting value
  --> main.frg:6:11
   |
 6 |     Enemy e;
   |           ^
  hint: make one, like: Enemy e = Enemy(...);  - or if it can be empty, make it optional: Enemy? e = null;
```

**Fix:** make one (`Enemy e = Enemy();`), or let it be empty: `Enemy? e = null;`.

### `the field 'weapon' needs a starting value`

A class-type field needs a starting value too.

<!-- check: error -->

```cpp
class Weapon {
    int damage = 1;
}

class Player {
    Weapon weapon;
}

int main(){
    return 0;
}
```

```text
error: the field 'weapon' needs a starting value
  --> main.frg:6:12
   |
 6 |     Weapon weapon;
   |            ^^^^^^
  hint: give it one (Weapon weapon = Weapon(...);) or make it optional (Weapon? weapon;)
```

**Fix:** give it one (`Weapon weapon = Weapon();`), or make it optional (`Weapon? weapon;`).

### `Point(...) takes either nothing or all 2 of its fields, but got 1`

Without a constructor, `Type(...)` takes all of its fields, in order, or none of them.

<!-- check: error -->

```cpp
struct Point {
    int x = 0;
    int y = 0;
}

int main(){
    Point p = Point(5);
    return 0;
}
```

```text
error: Point(...) takes either nothing or all 2 of its fields, but got 1
  --> main.frg:7:15
   |
 7 |     Point p = Point(5);
   |               ^^^^^
  hint: its fields, in order, are: int x, int y - or give Point a constructor
```

**Fix:** give both (`Point(5, 0)`), none (`Point()`), or write a constructor.

### `Shape is an abstract class, so it can't be made directly`

Abstract classes and interfaces only describe what other classes have; you make one of those.

<!-- check: error -->

```cpp
abstract class Shape {
    abstract float area();
}

int main(){
    Shape s = Shape();
    return 0;
}
```

```text
error: Shape is an abstract class, so it can't be made directly
  --> main.frg:6:15
   |
 6 |     Shape s = Shape();
   |               ^^^^^
  hint: make one of the classes that inherit from it instead
```

**Fix:** make a class that inherits from it and writes `area()`, and make that.

### `Tree has to have the method 'draw', because Drawable (an interface it has) says so`

A class that has an interface (or inherits an abstract class) has to write each of its methods.

<!-- check: error -->

```cpp
interface Drawable {
    void draw();
}

class Tree : Drawable {
    int height = 3;
}

int main(){
    return 0;
}
```

```text
error: Tree has to have the method 'draw', because Drawable (an interface it has) says so
  --> main.frg:5:7
   |
 5 | class Tree : Drawable {
   |       ^^^^
  hint: add it to Tree: void draw() { ... }
```

**Fix:** add the method: `void draw(){ ... }`.

### `struct 'Node' ends up containing itself through 'next'`

A struct is stored inside whatever holds it, so it can't hold itself (it would be infinitely big).

<!-- check: error -->

```cpp
struct Node {
    int value = 0;
    Node next;
}

int main(){
    return 0;
}
```

```text
error: struct 'Node' ends up containing itself through 'next'
  --> main.frg:3:10
   |
 3 |     Node next;
   |          ^^^^
  hint: a struct is stored inside whatever holds it, so it can't contain itself - use a class instead
```

**Fix:** make it a class, and the field optional: `Node? next;`.

### `structs can't inherit from other types`

Only classes can inherit from other classes.

<!-- check: error -->

```cpp
struct Base {
    int x = 0;
}

struct Child : Base {
    int y = 0;
}

int main(){
    return 0;
}
```

```text
error: structs can't inherit from other types
  --> main.frg:5:14
   |
 5 | struct Child : Base {
   |              ^
  hint: use a class if you need inheritance
```

**Fix:** make both classes, or put a `Base` field in the struct.

### `a class can only inherit from one class, but C already inherits from A`

A class inherits from at most one class (it can have many interfaces).

<!-- check: error -->

```cpp
class A {
}

class B {
}

class C : A, B {
}

int main(){
    return 0;
}
```

```text
error: a class can only inherit from one class, but C already inherits from A
  --> main.frg:7:14
   |
 7 | class C : A, B {
   |              ^
  hint: the others after the : must be interfaces
```

**Fix:** inherit from one, and make the other an interface, or a field.

### `this constructor of Boss is private`

A `private` constructor can only be used by the type's own methods.

<!-- check: error -->

```cpp
class Boss {
    private Boss(){
    }
}

int main(){
    Boss b = Boss();
    return 0;
}
```

```text
error: this constructor of Boss is private
  --> main.frg:7:14
   |
 7 |     Boss b = Boss();
   |              ^^^^
  hint: only Boss's own methods can use it
```

**Fix:** use the method the type gives for making one, or remove `private`.

### `a static method belongs to the type, not to one object, so it can't use the field 'hp'`

A static method belongs to the type, not to one object, so it has no `this` and no fields.

<!-- check: error -->

```cpp
class Enemy {
    int hp = 10;
    static int total(){
        return hp;
    }
}

int main(){
    return 0;
}
```

```text
error: a static method belongs to the type, not to one object, so it can't use the field 'hp'
  --> main.frg:4:16
   |
 4 |         return hp;
   |                ^^
  hint: make it a normal method (without static), or give it the object as a parameter
```

**Fix:** remove `static`, or give it the object as a parameter.

### `super only works inside a class that inherits from another class`

`super` reaches the class this one inherits from, so it needs one.

<!-- check: error -->

```cpp
class Enemy {
    void hit(){
        super.hit();
    }
}

int main(){
    return 0;
}
```

```text
error: super only works inside a class that inherits from another class
  --> main.frg:3:9
   |
 3 |         super.hit();
   |         ^^^^^
```

**Fix:** remove it, or make the class inherit from one.

### `'is' checks what kind of class an object is, but this is an int`

`is` and `as` check what kind of class an object is, so they only work on class objects.

<!-- check: error -->

```cpp
class Enemy {
}

int main(){
    int n = 5;
    bool b = n is Enemy;
    return 0;
}
```

```text
error: 'is' checks what kind of class an object is, but this is an int
  --> main.frg:6:16
   |
 6 |     bool b = n is Enemy;
   |                ^^
```

**Fix:** use them on a class object.

### `the enum State has no value called 'JUMPING'`

The value isn't one of the enum's.

<!-- check: error -->

```cpp
enum State { IDLE, RUNNING }

int main(){
    State s = State.JUMPING;
    return 0;
}
```

```text
error: the enum State has no value called 'JUMPING'
  --> main.frg:4:21
   |
 4 |     State s = State.JUMPING;
   |                     ^^^^^^^
  hint: its values are: IDLE, RUNNING
```

**Fix:** use one of the values the hint lists, or add it to the enum.

### `Forge needs to know what T is for this Box`

A generic type has to know what its `T` is.

<!-- check: error -->

```cpp
#include <stdio>

class Box<T> {
    T item;
    Box(T item){
        this.item = item;
    }
}

int main(){
    print(Box(5).item);
    return 0;
}
```

```text
error: Forge needs to know what T is for this Box
  --> main.frg:11:11
    |
 11 |     print(Box(5).item);
    |           ^^^
  hint: say it here, like Box<int>(...), or make it where its type is written: Box<int> b = Box(...);
```

**Fix:** say it (`Box<int>(5)`), or make it where its type is written: `Box<int> b = Box(5);`.

### `can't use '*' on a T and an int`

Inside a generic type, a `T` could be anything, so only what every type can do works on it.

<!-- check: error -->

```cpp
class Box<T> {
    T item;
    Box(T item){
        this.item = item;
    }
    T doubled(){
        return item * 2;
    }
}

int main(){
    return 0;
}
```

```text
error: can't use '*' on a T and an int
  --> main.frg:7:21
   |
 7 |         return item * 2;
   |                     ^
```

**Fix:** do the math outside the Box, where the type is known.

### `toText() decides how a Point is shown, so it has to be written as: string toText()`

`toText()` decides how an object looks in text, so it has to be written exactly like this.

<!-- check: error -->

```cpp
struct Point {
    int x = 0;
    int toText(){
        return x;
    }
}

int main(){
    return 0;
}
```

```text
error: toText() decides how a Point is shown, so it has to be written as: string toText()
  --> main.frg:3:9
   |
 3 |     int toText(){
   |         ^^^^^^
```

**Fix:** `string toText(){ return "({x})"; }`.

### `Weak only works with classes, but int isn't one`

`Weak` holds a class object without keeping it alive, so it only works with classes.

<!-- check: error -->

```cpp
int main(){
    Weak<int> w = Weak(5);
    return 0;
}
```

```text
error: Weak only works with classes, but int isn't one
  --> main.frg:2:10
   |
 2 |     Weak<int> w = Weak(5);
   |          ^^^
  hint: structs, numbers and text are copied, so there's nothing to point at

error: Weak(...) needs a class object, but this is an int
  --> main.frg:2:24
   |
 2 |     Weak<int> w = Weak(5);
   |                        ^
  hint: structs, numbers and text are copied, so there's nothing to point at
```

**Fix:** use `Weak` with a class, like `Weak<Player>`.

## Files, folders and libraries

### `unknown name 'jump'`

Something from another file is used without the file's name in front. In Forge it's always `file.name`.

<!-- check: error -->

```cpp
// file: main.frg
int main(){
    jump();
    return 0;
}
```

```cpp
// file: player.frg
void jump(){
}
```

```text
error: unknown name 'jump'
  --> main.frg:3:5
   |
 3 |     jump();
   |     ^^^^
  hint: 'jump' is in player.frg - use player.jump
```

**Fix:** write `player.jump();` (the file is `player.frg`).

### `the folder 'enemies' isn't part of the game yet`

A subfolder's files join the game when a file next to that folder includes it.

<!-- check: error -->

```cpp
// file: main.frg
int main(){
    enemies.goblin.spawn();
    return 0;
}
```

```cpp
// file: enemies/goblin.frg
void spawn(){
}
```

```text
error: the folder 'enemies' isn't part of the game yet
  --> main.frg:3:5
   |
 3 |     enemies.goblin.spawn();
   |     ^^^^^^^
  hint: add #include <"enemies"> to a file in the top folder, like main.frg
```

**Fix:** add `#include <"enemies">` at the top of `main.frg`.

### `includes look like #include <stdio> or #include <"folder">`

`#include` takes a library in `< >`, or a folder in `<" ">`.

<!-- check: error -->

```cpp
#include "stdio"

int main(){
    return 0;
}
```

```text
error: includes look like #include <stdio> or #include <"folder">
  --> main.frg:1:10
   |
 1 | #include "stdio"
   |          ^^^^^^^
  hint: files in the same folder are already included automatically
```

**Fix:** `#include <stdio>`, or for a folder `#include <"enemies">`. Files in the same folder need no include.

### `unknown directive after '#'`

Forge has only `#include`; there's no `#define`, `#if` or `#pragma`.

<!-- check: error -->

```cpp
#define MAX 10

int main(){
    return 0;
}
```

```text
error: unknown directive after '#'
  --> main.frg:1:2
   |
 1 | #define MAX 10
   |  ^^^^^^
  hint: the only one is #include
```

**Fix:** use a constant: `const int MAX = 10;`.

### `<OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)`

A game uses the engine (`<window>`, `<graphics2d>`...) or one low-level toolkit, not both.

<!-- check: error -->

```cpp
#include <window>
#include <OpenGL>

int main(){
    return 0;
}
```

```text
error: <OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)
  --> main.frg:2:11
   |
 2 | #include <OpenGL>
   |           ^^^^^^
  hint: a game either uses the engine (<window>, <input>, <graphics2d>, <graphics3d>, <assets>, <ui>) or one toolkit, which opens its own window and does its own drawing - OpenGL has its own window, keyboard and mouse functions
```

**Fix:** pick one. See [Low-Level Toolkits](../libraries/toolkits.md).

### `the <network> library isn't built yet`

This library is planned, but not part of Forge yet.

<!-- check: error -->

```cpp
#include <network>

int main(){
    return 0;
}
```

```text
error: the <network> library isn't built yet
  --> main.frg:1:11
   |
 1 | #include <network>
   |           ^^^^^^^
  hint: it's planned for a later version of Forge
```

**Fix:** see the [roadmap](../../ROADMAP.md) for when it's coming.

## Tasks

### `wait() only works inside a task function`

`wait()` only works inside a `task` function.

<!-- check: error -->

```cpp
int main(){
    wait(1);
    return 0;
}
```

```text
error: wait() only works inside a task function
  --> main.frg:2:5
   |
 2 |     wait(1);
   |     ^^^^
  hint: change 'int main()' to 'task main()' - then calling it starts it, and it keeps going on its own
```

**Fix:** move the code into a task: `task later(){ wait(1); ... }`, and call it.

### `wait() needs a number of seconds (or a task to wait for), but this is a string`

`wait()` takes a number of seconds (or a task to wait for).

<!-- check: error -->

```cpp
task blink(){
    wait("soon");
}

int main(){
    return 0;
}
```

```text
error: wait() needs a number of seconds (or a task to wait for), but this is a string
  --> main.frg:2:10
   |
 2 |     wait("soon");
   |          ^^^^^^
```

**Fix:** `wait(0.5);`.

## Warnings

### `comparing decimal numbers with '==' often gives the wrong answer`

Decimal numbers are rarely exactly equal, so `==` on them often gives `false` when you'd expect `true`.

<!-- check: warning -->

```cpp
#include <stdio>

int main(){
    float x = 0.1 + 0.2;
    if (x == 0.3) {
        print("equal");
    }
    return 0;
}
```

```text
warning: comparing decimal numbers with '==' often gives the wrong answer
  --> main.frg:5:11
   |
 5 |     if (x == 0.3) {
   |           ^^
  hint: decimals are often a tiny bit off (0.1 + 0.2 isn't exactly 0.3), so check whether they're close instead: math.abs(a - b) < 0.001 (needs #include <math>)
```

**Fix:** check whether they're close: `math.abs(x - 0.3) < 0.001`.

### `this divides two whole numbers, so the decimals are cut off before it becomes a decimal number`

Dividing two whole numbers cuts off the decimals before the result becomes a decimal number.

<!-- check: warning -->

```cpp
#include <stdio>

int main(){
    int hits = 7;
    int shots = 10;
    float ratio = hits / shots;
    print(ratio);
    return 0;
}
```

```text
warning: this divides two whole numbers, so the decimals are cut off before it becomes a decimal number
  --> main.frg:6:24
   |
 6 |     float ratio = hits / shots;
   |                        ^
  hint: turn one side into a decimal number first, like float(hits) / shots
```

**Fix:** `float(hits) / shots`.

### `the variable 'unused' is never used`

A variable that's made but never used is often a typo, or left over.

<!-- check: warning -->

```cpp
int main(){
    int unused = 5;
    return 0;
}
```

```text
warning: the variable 'unused' is never used
  --> main.frg:2:9
   |
 2 |     int unused = 5;
   |         ^^^^^^
  hint: remove it if you don't need it
```

**Fix:** use it, or remove it.

### `this code can never run, because it comes after 'return'`

Code right after `return`, `break`, `continue` or `throw` can never run.

<!-- check: warning -->

```cpp
int five(){
    return 5;
    int never = 1;
}

int main(){
    return 0;
}
```

```text
warning: this code can never run, because it comes after 'return'
  --> main.frg:3:9
   |
 3 |     int never = 1;
   |         ^^^^^
  hint: remove it, or move it above the 'return'

warning: the variable 'never' is never used
  --> main.frg:3:9
   |
 3 |     int never = 1;
   |         ^^^^^
  hint: remove it if you don't need it
```

**Fix:** remove it, or move it above the `return`.

### `this changes 'e', which is only a copy of the item in the list, so the change is lost`

A `for (... in ...)` loop gives a copy of each struct, so changing it doesn't change the list.

<!-- check: warning -->

```cpp
struct Enemy {
    int hp = 10;
}

int main(){
    list<Enemy> enemies = [Enemy()];
    for (Enemy e in enemies) {
        e.hp = 0;
    }
    return 0;
}
```

```text
warning: this changes 'e', which is only a copy of the item in the list, so the change is lost
  --> main.frg:8:9
   |
 8 |         e.hp = 0;
   |         ^
  hint: a for (... in ...) loop gives you a copy of each struct. To change the list itself, loop with a number and use list[i], like: for (int i = 0; i < list.length; i++) { list[i]... }
```

**Fix:** loop with a number: `for (int i = 0; i < enemies.length; i++) { enemies[i].hp = 0; }`. (With a class instead of a struct, the loop gives the objects themselves.)

### `this counts one past the last position, because both ends of a range are included`

A range includes both ends, so `0..list.length` goes one past the last item.

<!-- check: warning -->

```cpp
#include <stdio>

int main(){
    list<int> xs = [1, 2, 3];
    for (int i in 0..xs.length) {
        print(i);
    }
    return 0;
}
```

```text
warning: this counts one past the last position, because both ends of a range are included
  --> main.frg:5:25
   |
 5 |     for (int i in 0..xs.length) {
   |                         ^^^^^^
  hint: lists start at 0, so write: 0..xs.length - 1
```

**Fix:** `0..xs.length - 1`, or `for (int x in xs)`.

### `toString() is called toText() in Forge now`

`toString()` was renamed `toText()`. It still works for now.

<!-- check: warning -->

```cpp
struct Point {
    int x = 0;
    string toString(){
        return "({x})";
    }
}

int main(){
    return 0;
}
```

```text
warning: toString() is called toText() in Forge now
  --> main.frg:3:12
   |
 3 |     string toString(){
   |            ^^^^^^^^
  hint: rename it to toText - toString still works for now
```

**Fix:** rename it to `toText`.

## Runtime errors (while the game runs)

### `position 3 is outside the list (it has 3 items, at positions 0 to 2)`

A position outside the list. Positions go from 0 to `length - 1`.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    list<int> xs = [1, 2, 3];
    int x = xs[3];
    return 0;
}
```

```text
Runtime error in main.frg:3: position 3 is outside the list (it has 3 items, at positions 0 to 2)
```

**Fix:** check first: `if (i < xs.length)`. The last item is `xs[xs.length - 1]`, or `xs.last()`.

### `position 0 is outside the list: the list is empty`

Any position in an empty list.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    list<int> xs = [];
    int x = xs[0];
    return 0;
}
```

```text
Runtime error in main.frg:3: position 0 is outside the list: the list is empty
```

**Fix:** check `xs.isEmpty()` (or `xs.length > 0`) first.

### `pop() was used on an empty list`

`first()`, `last()` and `pop()` need at least one item.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    list<int> xs = [];
    int x = xs.pop();
    return 0;
}
```

```text
Runtime error in main.frg:3: pop() was used on an empty list
```

**Fix:** check `xs.isEmpty()` first.

### `the key isn't in the map (use .has(key) to check first, or .get(key, backup))`

Reading a key that isn't in the map.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    map<string, int> ages = {"Ana": 12};
    int a = ages["Ben"];
    return 0;
}
```

```text
Runtime error in main.frg:3: the key isn't in the map (use .has(key) to check first, or .get(key, backup))
```

**Fix:** check with `ages.has("Ben")`, or give a backup: `ages.get("Ben", 0)`.

### `division by zero`

A whole number divided by zero (or `% 0`).

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    int zero = 0;
    int x = 10 / zero;
    return 0;
}
```

```text
Runtime error in main.frg:3: division by zero
```

**Fix:** check the number isn't 0 first. (Decimal numbers divided by 0 give `inf` instead.)

### `'hero' is null`

An optional value (`Type?`) was used while it was `null`.

<!-- check: runtime-error exit 1 -->

```cpp
class Player {
    int hp = 10;
}

int main(){
    Player? hero = null;
    int hp = hero.hp;
    return 0;
}
```

```text
Runtime error in main.frg:7: 'hero' is null
```

**Fix:** check `if (hero != null)` first, or give a backup: `hero ?? Player()`.

### `couldn't turn "ten" into an int`

Text that isn't a number was turned into one.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    int n = int("ten");
    return 0;
}
```

```text
Runtime error in main.frg:2: couldn't turn "ten" into an int
```

**Fix:** check with `"ten".isInt()` first.

### `position 5 is outside the text (it has 3 characters)`

A position outside the text.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    string s = "abc";
    string c = s.charAt(5);
    return 0;
}
```

```text
Runtime error in main.frg:3: position 5 is outside the text (it has 3 characters)
```

**Fix:** positions go from 0 to `length - 1`.

### `this function value is empty - give it a function before calling it`

A function value that was never given a function.

<!-- check: runtime-error exit 1 -->

```cpp
struct Button {
    string name;
    Function<void()> onClick;
}

int main(){
    Button b = Button();
    b.onClick();
    return 0;
}
```

```text
Runtime error in main.frg:8: this function value is empty - give it a function before calling it
```

**Fix:** give it one first: `b.onClick = () { ... };`, or make the button with one: `Button("ok", () { ... })`.

### `the level file is broken`

`throw` that no `try` catches stops the game with its message.

<!-- check: runtime-error exit 1 -->

```cpp
int main(){
    throw "the level file is broken";
    return 0;
}
```

```text
Runtime error in main.frg:2: the level file is broken
```

**Fix:** catch it with `try { ... } catch (Error e) { ... }`, or fix what caused it.

### `the window isn't open yet - call window.open(width, height, "title") first`

Drawing (or anything else that needs the window) before `window.open(...)`.

<!-- check: runtime-error exit 1 -->

```cpp
#include <graphics2d>

int main(){
    graphics2d.drawRect(0, 0, 10, 10, color.RED);
    return 0;
}
```

```text
Runtime error in graphics2d.drawRect(): the window isn't open yet - call window.open(width, height, "title") first
  called from main.frg:4 (in main)
```

**Fix:** call `window.open(width, height, "title")` first (it needs `#include <window>`).

### `can't find the file 'hero.png' - put it in the game's folder (next to main.frg), or check the spelling`

A file the game loads isn't in the game's folder.

<!-- check: runtime-error exit 1 -->

```cpp
#include <assets>

int main(){
    Texture hero = assets.loadTexture("hero.png");
    return 0;
}
```

```text
Runtime error in assets.loadTexture(): can't find the file 'hero.png' - put it in the game's folder (next to main.frg), or check the spelling
  called from main.frg:4 (in main)
```

**Fix:** put the file next to `main.frg` (or in a folder there, and write the path: `"art/hero.png"`), and check the spelling.
