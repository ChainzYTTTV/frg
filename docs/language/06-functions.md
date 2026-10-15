# 6. Functions

A **function** is a named piece of code you can run ("call") whenever you need it. Functions keep programs short (write something once, use it many times) and readable (a good name explains what the code does).

## Writing a function

```cpp
int add(int a, int b){
    return a + b;
}
```

| Part | Here | Meaning |
|---|---|---|
| return type | `int` | the type of value the function gives back |
| name | `add` | what you call it by |
| parameters | `(int a, int b)` | values it's given, each with a type and a name |
| body | `{ return a + b; }` | the code that runs |

Calling it:

```cpp
int total = add(2, 3);    // 5
print(add(10, 20));       // 30
```

## void: functions that give nothing back

```cpp
void greet(string name){
    print("Hi {name}!");
}

greet("Ana");
```

`void` means "nothing". A `void` function can use `return;` (with no value) to stop early. Using its result, as in `int x = greet("Ana");`, is an error: "this doesn't give back a value".

## return

`return value;` ends the function and gives back the value. Code after a `return` doesn't run.

**A function with a return type must return on every path.** Forge checks this:

```cpp
string grade(int score){
    if (score >= 90) {
        return "A";
    }
}   // error: 'grade' doesn't always return a value - what if score is 50?
```

Fix it by covering every case:

```cpp
string grade(int score){
    if (score >= 90) {
        return "A";
    } else if (score >= 80) {
        return "B";
    }
    return "F";
}
```

The return value is converted to the return type with the usual rules: returning an `int` from a `float` function is fine, but returning a `float` from an `int` function is an error.

## Parameters

Parameters are variables that get their starting value from the call:

```cpp
void spawn(string kind, float x, float y){
    print("spawning {kind} at {x}, {y}");
}
spawn("goblin", 100, 250);
```

- The values are matched in order, and there must be the right number of them. A mismatch is an error that shows how the function is written: "spawn() needs 3 values, but got 2 - it's written as: void spawn(string kind, float x, float y)".
- Each value must fit the parameter's type, using the same rules as making a variable.
- Two parameters can't have the same name.

### What a function can change

What happens when a function changes a parameter depends on its type:

| Parameter type | The function gets | Changes affect the caller? |
|---|---|---|
| numbers, bools, strings, structs, lists, maps | its own **copy** | no |
| classes | the **same object** | yes |
| `Pointer<T>` | a pointer to the caller's variable | yes, through `.value` |

```cpp
void tryToChange(int n, list<int> xs, Enemy e){
    n = 100;          // only changes the copy
    xs.add(5);        // only changes the copy of the list
    e.hp = 0;         // changes the real enemy
}
```

To change a number or a list for the caller, either return the new value, or pass a `Pointer`. See [Pointers](10-enums-optionals-pointers.md#pointers).

### Default values

A parameter can have a value to use when the caller leaves it out:

```cpp
void shout(string text, int times = 3){
    print(text.repeat(times));
}

shout("hey");         // heyheyhey
shout("hey", 1);      // hey
```

Once a parameter has a default, every parameter after it needs one too.

## Order doesn't matter

A function can be called from anywhere in the game, above or below where it's written, and from other files (with the file's name in front; see [Files and folders](07-files-and-folders.md)). Forge reads all the code before checking any of it.

## Functions calling themselves

A function may call itself. That's called **recursion**:

```cpp
int fib(int n){
    if (n < 2) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
```

Make sure there's always a way out (here, `n < 2`); otherwise it calls itself forever and the program crashes.

## Rules

- Each function name can be used only once per file. Two functions with the same name in one file is an error, even with different parameters.
- A function's name can't be a [reserved word](13-reserved-words.md), a built-in function name (`print`, `wait`), or the name of one of your top-level files.
- Functions are always written at the top level of a file (or inside a struct or class, where they're called methods). You can't write a function inside another function.
- Functions can't be stored in variables or passed to other functions.

## Methods

Functions written inside a `struct` or `class` are called **methods**, and they're called on a value: `player.jump()`. See [Structs and classes](09-structs-and-classes.md).

## Tasks

A function whose return type is `task` can pause itself with `wait(seconds)` while the rest of the game keeps running. See [Tasks](11-tasks.md).

## What's next

[Files and folders](07-files-and-folders.md): splitting a game across several files.
