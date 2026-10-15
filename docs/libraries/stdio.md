# `<stdio>`: Console Input and Output

```cpp
#include <stdio>
```

"stdio" stands for **st**an**d**ard **i**nput and **o**utput: the text console. Unlike other libraries, its functions are used without a prefix: `print(...)`, not `stdio.print(...)`.

With a window open, `print` output still goes to the console window behind the game. That's handy for debugging. In a `--release` build there's no console, so the output isn't visible.

## print

```cpp
void print(value)
void print()
```

Shows one value, then starts a new line. `print()` with nothing just starts a new line. Any type can be printed; see [how values look as text](../language/03-text.md#how-values-look-as-text).

```cpp
print("Hello!");
print(42);
print([1, 2, 3]);
print("x = {x}, y = {y}");     // to show several values, put them in one string
```

`print(a, b)` is an error: `print` takes one value.

## write

```cpp
void write(value)
```

Like `print`, but doesn't start a new line afterwards, so the next output continues on the same line.

```cpp
write("Loading");
for (int i = 0; i < 3; i++) {
    write(".");
}
print();        // Loading...
```

## input

```cpp
string input()
string input(string prompt)
```

Waits for the player to type a line and press Enter, then gives back what they typed (without the Enter). With a `prompt`, that text is shown first, on the same line.

```cpp
string name = input("What's your name? ");
int age = int(input("How old are you? "));
```

- To get a number, convert: `int(input(...))`. If the player types something that isn't a number, the game stops with a runtime error.
- If there's no more input (the input was closed), it gives back `""`.
- `input` is for console programs. In a windowed game, use [`<input>`](input.md) for the keyboard. With both included, `input(...)` is this function and `input.keyDown(...)` is the library.
