# 1. The Basics

This chapter covers how a Forge program is put together: where it starts, what a statement is, how comments work, and the rules for names.

## Where a program starts

Every Forge program is a **folder** containing a file called `main.frg`. That file must have a function called `main`:

```cpp
#include <stdio>

int main(){
    print("The game starts here");
    return 0;
}
```

When the program runs, it starts at the first line inside `main` and goes down. When `main` reaches `return`, the program ends.

- `main` must be written exactly as `int main()`, with no parameters.
- `main` must end with `return 0;` (everything went fine) or `return 1;` (something went wrong). Forge reports an error if `main` can reach its end without returning.
- Other files and folders can be part of the program too; see [Files and folders](07-files-and-folders.md).

## Statements and semicolons

A **statement** is one instruction. Statements end with a semicolon `;`:

```cpp
int score = 0;
score += 10;
print(score);
```

Forgetting the `;` is the most common mistake. Forge points at the exact spot:

```
error: missing ';' at the end of this line
  --> main.frg:4:18
   |
 4 |     int score = 0
   |                  ^
```

You can put several statements on one line, or spread one statement over several lines. Forge goes by the `;`, not by line breaks:

```cpp
int a = 1; int b = 2;
int total = a +
            b;
```

## Blocks and braces

A **block** is a group of statements inside `{ }`. Functions, `if`, loops and types all use blocks:

```cpp
if (score > 100) {
    print("High score!");
    score = 0;
}
```

**Braces are always required** after `if`, `else`, `while` and `for`, even for a single statement. This avoids a classic bug where a second line looks like it's inside the `if` but isn't:

```cpp
if (score > 100) print("High score!");   // error: 'if' needs { } around its code
```

**Indentation doesn't matter to Forge**: the braces decide what's inside what. Indent anyway (4 spaces per level is usual), because it makes code far easier to read.

## Comments

Comments are notes for people. Forge ignores them.

```cpp
// A line comment: everything after // until the end of the line.

int lives = 3;   // comments can go at the end of a line too

```
A block comment: everything between the two sets of three backticks,
over as many lines as you like.
```
```

`/* ... */` comments from C and C++ don't work in Forge; Forge will tell you to use ` ``` ` instead. A block comment that's never closed is an error.

## The order things are written in

**Functions, types and global variables can be used before or after the place they're written**, even in other files. Forge reads everything first, then checks it:

```cpp
int main(){
    greet("Ana");       // fine, even though greet is written below
    return 0;
}

void greet(string name){
    print("Hi {name}!");
}
```

Variables made *inside* a function are different: they can only be used after the line that makes them.

```cpp
int main(){
    print(x);       // error: unknown name 'x'
    int x = 5;
    return 0;
}
```

## Top level

Outside of any function (the **top level** of a file) you can only write:

- `#include` lines
- functions
- global variables (`int score = 0;`)
- types: `struct`, `class`, `enum`

Any other code, like `print("hi");`, has to go inside a function. Forge says "this code has to go inside a function" if it isn't.

## Names

Names for variables, functions, types and fields:

- can use letters, numbers and `_`
- can't start with a number (`player2` is fine, `2player` isn't)
- are case-sensitive: `score`, `Score` and `SCORE` are three different names
- can't be a reserved word, like `if`, `class`, `int`, `print` or a library name like `window` (see the [full list](13-reserved-words.md))
- can't be the name of one of your top-level files or folders. If you have `player.frg`, nothing can be called `player`, because `player.something` always means the file.

Common styles (Forge doesn't enforce them, but the docs and examples use them):

| Kind | Style | Example |
|---|---|---|
| variables, functions, fields, methods | `camelCase` | `jumpSpeed`, `takeDamage()` |
| types (struct, class, enum) | `PascalCase` | `Player`, `GameState` |
| constants | `UPPER_CASE` | `MAX_HP`, `GRAVITY` |
| files and folders | `lower_case` | `player.frg`, `enemies/` |

## Expressions vs statements

An **expression** is anything that has a value: `5`, `score`, `score + 10`, `hp > 0`, `max(a, b)`.

A **statement** does something. Most statements are one of:

| Statement | Example |
|---|---|
| Make a variable | `int hp = 10;` |
| Change a variable | `hp = 5;` / `hp -= 1;` / `hp++;` / `set hp to 5;` |
| Call a function | `print("hi");` |
| Control flow | `if`, `while`, `for`, `switch`, `return`, `break`, `continue` |

A line that's just an expression with nothing done to it is an error, because it would do nothing:

```cpp
hp == 5;    // error: this compares two values but doesn't use the answer
```

## What's next

[Variables and types](02-variables-and-types.md): storing numbers, text and true/false values.
