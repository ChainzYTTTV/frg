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

## Several functions with the same name

Functions can share a name when their **parameters are different**: a different number of them, or different types. Forge picks the right one from the values you call it with:

```cpp
void drawHealth(int hp){
    graphics2d.drawText("HP: {hp}", 10, 10, 20, color.WHITE);
}

void drawHealth(float fraction){
    graphics2d.drawRect(10, 10, 200 * fraction, 20, color.GREEN);
}

drawHealth(75);      // the int version
drawHealth(0.75);    // the float version
```

How Forge chooses:

- Only the versions the values fit are considered.
- Of those, the one whose parameter types match the values most exactly wins: `drawHealth(75)` uses the `int` version even though `75` would also fit a `float`.
- If none fits, the error lists every version, so you can see what's possible.

Rules:

- Two versions with exactly the same parameter types are an error, even if they give back different types: a call couldn't tell them apart.
- There's only one `main`.
- Methods in a struct or class can't share a name (only constructors can).
- When a function with several versions is used as a [function value](#functions-as-values), the version whose parameters match the `Function<...>` type is used.

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

- Two functions in one file can only share a name if their parameters are different (see [Several functions with the same name](#several-functions-with-the-same-name)).
- A function's name can't be a [reserved word](13-reserved-words.md), a built-in function name (`print`, `wait`), or the name of one of your top-level files.
- Functions are always written at the top level of a file (or inside a struct or class, where they're called methods). You can't write a function inside another function.
- To store a function in a variable or pass it to another function, use a [function value](#functions-as-values).

## Functions as values

A function can be kept in a variable, handed to another function, or stored in a field, like any other value. That's how you tell a list *how* to pick items, or tell a button *what to do* when it's clicked.

### Writing one

A **function value** is written like a function without a return type or a name: the parameters in `( )`, then the code in `{ }`:

```cpp
list<Enemy> alive = enemies.filter((Enemy e) { return e.hp > 0; });
enemies.removeWhere((Enemy e) { return e.hp <= 0; });
```

Read `(Enemy e) { return e.hp <= 0; }` as "a function that takes an enemy `e` and says whether its hp is 0 or less". `removeWhere` calls it for every enemy, and removes the ones it says `true` for.

A function with no parameters has empty brackets: `() { print("Clicked!"); }`.

### Its type: Function<...>

To keep a function value in a variable, a field or a parameter, its type is `Function<` the type it gives back `(` the types it takes `)>`:

| Type | A function that... |
|---|---|
| `Function<void()>` | takes nothing and gives nothing back |
| `Function<bool(Enemy)>` | takes an `Enemy`, gives back a `bool` |
| `Function<int(int, int)>` | takes two `int`s, gives back an `int` |

```cpp
Function<bool(int)> isEven = (int n) { return n % 2 == 0; };
print(isEven(4));      // true: call it like a function
```

The part inside `< >` reads like the first line of a function, without the names: `bool isEven(int n)` becomes `Function<bool(int)>`.

A function you've written can be used as a value too: write its name **without** `()`.

```cpp
int twice(int n){
    return n * 2;
}

Function<int(int)> f = twice;    // f is twice, not the result of calling it
print(f(21));                    // 42
```

### Handing one to a function

```cpp
int applyTwice(Function<int(int)> f, int x){
    return f(f(x));
}

print(applyTwice(twice, 5));                       // 20
print(applyTwice((int n) { return n + 1; }, 5));   // 7
```

### In a field: code that runs later

```cpp
class Button {
    string label;
    Function<void()> onClick;
}

Button play = Button("Play", () { scenes.change(Playing()); });
...
play.onClick();      // runs the code
```

A field like `onClick` can be left empty until it's given a function. Calling an empty one stops the game with "this function value is empty". A variable needs a starting value, or can be optional (`Function<void()>? onClick = null;`).

### Using the variables around it

A function value can use the variables around the place it's written. It gets a **copy** of them, made when the function value is made:

```cpp
int bonus = 100;
Function<int(int)> addBonus = (int n) { return n + bonus; };
print(addBonus(1));     // 101
```

Because it's a copy, a function value **can't change** those local variables: `() { total++; }` is an error ("a function value can't change 'total'..."). It *can* change global variables, fields, and objects of a class (those are shared, not copied). So to count something from inside a function value, use a global variable or a field.

### Things to know

- The function value's parameters must have exactly the types of the `Function<...>` it goes into.
- If the `Function<...>` gives something back, every way through the function value must `return` a value.
- A task can't be a function value. Use a function value that starts it: `() { blink(); }`.
- A function value always goes where a `Function<...>` is expected (a variable, a parameter, a list method). On its own, Forge can't tell what kind it is.

## Methods

Functions written inside a `struct` or `class` are called **methods**, and they're called on a value: `player.jump()`. See [Structs and classes](09-structs-and-classes.md).

## Tasks

A function whose return type is `task` can pause itself with `wait(seconds)` while the rest of the game keeps running. See [Tasks](11-tasks.md).

## What's next

[Files and folders](07-files-and-folders.md): splitting a game across several files.
