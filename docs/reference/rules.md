# How Forge behaves, exactly

This page answers the "what happens if...?" questions, so you don't have to guess. Every example here is run when Forge's tests run, and the output shown under it is what it really prints. If Forge ever behaves differently from this page, the tests fail.

- [The order a program runs in](#the-order-a-program-runs-in)
- [Numbers](#numbers)
- [Text](#text)
- [Copies and shared objects](#copies-and-shared-objects)
- [Loops](#loops)
- [null](#null)
- [A frame, step by step](#a-frame-step-by-step)
- [Stopping the game](#stopping-the-game)
- [Testing a game that opens a window](#testing-a-game-that-opens-a-window)

## The order a program runs in

1. **Global variables** (the ones outside every function) get their starting values first, before `main`: those in `main.frg` first, then the other files of the top folder in alphabetical order, then the files of each included folder. Within one file, from top to bottom.
2. Then **`main()`** runs, from its first line to its `return`.
The game runs **in its own folder** (the one with `main.frg`, or the `.exe`'s folder in a packaged game), so files like `"data/level.txt"` are found however it was started.

3. When `main` returns, the window closes (if one is open), sound stops, and the program ends with `main`'s number as its [exit code](commands.md#exit-codes).

<!-- check: output -->

```cpp
#include <stdio>

int first = note("a global in main.frg", 1);

int note(string what, int value){
    print(what);
    return value;
}

int main(){
    print("main starts");
    return 0;
}
```

```cpp
// file: enemies.frg
int count = main.note("a global in enemies.frg", 2);
```

```text
a global in main.frg
a global in enemies.frg
main starts
```

Inside one line, things happen **left to right**: in `add(next(), next())` the first `next()` runs first; in `a() + b()`, `a()` runs first; in `"{a()} {b()}"`, `a()` runs first.

<!-- check: output -->

```cpp
#include <stdio>

int counter = 0;

int next(){
    counter++;
    return counter;
}

int main(){
    print("{next()} {next()} {next()}");
    print(next() * 10 + next());
    return 0;
}
```

```text
1 2 3
45
```

A runtime error in a global's starting value stops the game before `main` starts, with the line it happened on:

<!-- check: runtime-error exit 1 -->

```cpp
list<int> scores = [];
int best = scores[0];

int main(){
    return 0;
}
```

```text
Runtime error in main.frg:2: position 0 is outside the list: the list is empty
```

## Numbers

### Whole numbers

`int` holds whole numbers from -2,147,483,648 to 2,147,483,647. The other sizes (`int8` to `int128`, and the unsigned `uint` ones) are listed in [Variables and types](../language/02-variables-and-types.md).

#### Numbers that get too big

Going past the biggest value **wraps around** to the smallest (and the other way). It never stops the game. Writing a number that doesn't fit, straight into the code, is an error instead (`uint8 z = 300;`).

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    int big = 2147483647;
    big = big + 1;
    print(big);
    uint8 small = 250;
    small += 10;
    print(small);
    return 0;
}
```

```text
-2147483648
4
```

#### Dividing whole numbers

- `int / int` gives an `int`: the decimals are **cut off** (toward zero), not rounded. `7 / 2` is `3`, and `-7 / 2` is `-3`.
- `%` gives what's left over, with the **sign of the left number**: `-7 % 2` is `-1`, and `7 % -2` is `1`.
- Dividing a whole number by `0` (or `% 0`) stops the game with "division by zero".
- Numbers written straight into the code are the exception: on their own they adapt to where the result goes. `float half = 7 / 2;` is `3.5`, but with a variable, `int seven = 7; float half = seven / 2;` is `3.0`. When in doubt, write `7.0 / 2`.

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    int seven = 7;
    print(seven / 2);
    print(-seven / 2);
    print(-seven % 2);
    print(seven % -2);
    float a = seven / 2;
    float b = 7 / 2;
    float c = seven / 2.0;
    print("{a} {b} {c}");
    return 0;
}
```

```text
3
-3
-1
1
3.0 3.5 3.5
```

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

### Decimal numbers

- `float` keeps about 7 digits, `double` about 16. Text shows the shortest form that reads back as the same number.
- Dividing a decimal number by zero doesn't stop the game: it gives `inf` (or `-inf`, or `nan` for `0.0 / 0`).
- `int(...)` cuts off the decimals (toward zero): `int(2.9)` is `2`, `int(-2.9)` is `-2`.
- `math.round(...)` rounds halves away from zero: `2.5` becomes `3`, and `-2.5` becomes `-3`.
- Decimals are often a tiny bit off (`0.1` is really `0.100000001...`), so compare them with a small margin: `math.abs(a - b) < 0.001`. Forge warns about `==` on decimals.

<!-- check: output -->

```cpp
#include <stdio>
#include <math>

int main(){
    float third = 1.0 / 3;
    double precise = 1.0 / 3;
    print(third);
    print(precise);
    float zero = 0;
    print(1.0 / zero);
    print(int(2.9));
    print(int(-2.9));
    print(math.round(2.5));
    print(math.round(-2.5));
    return 0;
}
```

```text
0.33333334
0.3333333333333333
inf
2
-2
3.0
-3.0
```

## Text

- `text.length` counts **bytes**, not letters. Letters outside plain English (`é`, `ß`, emoji) take 2 to 4 bytes each, so `"héllo".length` is 6.
- Comparing text with `<` and `>` goes character by character by their code numbers: every capital letter comes before every small one, so `"Zebra" < "apple"` is `true`. Use `.lower()` on both sides to compare without caring about capitals.
- `text[i]` doesn't work; use `text.charAt(i)` (it gives a one-character `string`).
- `+` joins text, but only with text: `"HP: " + 5` is an error. Put the value inside the text instead: `"HP: {hp}"`.

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    print("hello".length);
    print("héllo".length);
    print("apple" < "banana");
    print("Zebra" < "apple");
    print("Zebra".lower() < "apple".lower());
    print("forge".charAt(0));
    return 0;
}
```

```text
5
6
true
true
false
f
```

## Copies and shared objects

When you put a value in another variable, pass it to a function, or put it in a list, Forge either **copies** it or **shares** it:

| Type | Assigning / passing it |
|---|---|
| numbers, `bool`, `string`, enums | copied |
| `vec2`, `vec3`, `color`, `rect` and your own `struct`s | copied: changing the copy doesn't change the original |
| `list<...>` and `map<...>` | **copied too**, with everything in them |
| your own `class` objects | **shared**: both names are the same object, so a change through one is seen through the other |

<!-- check: output -->

```cpp
#include <stdio>

struct Point {
    int x = 0;
}

class Player {
    int hp = 10;
}

void hurtPoint(Point p){
    p.x = 99;
}

void hurtPlayer(Player p){
    p.hp = 1;
}

void addItem(list<int> items){
    items.add(4);
}

int main(){
    Point a = Point();
    hurtPoint(a);
    print("struct after the function: {a.x}");

    Player hero = Player();
    hurtPlayer(hero);
    print("class after the function: {hero.hp}");

    list<int> mine = [1, 2, 3];
    addItem(mine);
    list<int> other = mine;
    other.add(5);
    print("list: {mine}, the copy: {other}");
    return 0;
}
```

```text
struct after the function: 0
class after the function: 1
list: [1, 2, 3], the copy: [1, 2, 3, 5]
```

To let a function change a struct, list or number, **give back the new value** and store it: `scores = withBonus(scores);`. Or keep it in a class object, which the function can change directly.

A class object stays alive as long as anything still holds it (a variable, a field or a list), and is freed as soon as nothing does. Two objects that hold each other are never freed; use [`Weak`](../language/09-structs-and-classes.md) for one of the two.

## Loops

- `for (int i in 1..5)` counts 1, 2, 3, 4, 5: **both ends are included**.
- A range that goes down, like `5..1`, counts nothing. To count down, use `for (int i = 5; i >= 1; i--)`.
- `for (Item x in items)` gives a **copy** of each struct item; changing `x` doesn't change the list (Forge warns about it). Loop with a number to change the items: `items[i].hp = 0;`.
- Items **added** to the list during a `for ... in` loop are visited too.
- **Removing** items during a `for ... in` loop skips the item right after each removed one. Use `items.removeWhere(...)` instead, or loop with a number from the end down.

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    for (int i in 1..3) {
        print("up {i}");
    }
    for (int i in 3..1) {
        print("this never prints");
    }
    list<int> grows = [1, 2];
    for (int x in grows) {
        if (x == 1) {
            grows.add(3);
        }
        print("saw {x}");
    }
    list<int> shrinks = [1, 2, 3, 4];
    for (int x in shrinks) {
        if (x == 2) {
            shrinks.remove(2);
        }
        print("visited {x}");
    }
    print(shrinks);
    return 0;
}
```

```text
up 1
up 2
up 3
saw 1
saw 2
saw 3
visited 1
visited 2
visited 4
[1, 3, 4]
```

## null

Only optional types (with a `?`, like `Player?` or `int?`) can be `null`. Using a `null` value as if it were there stops the game with a runtime error that names it. Check first with `if (x != null)`, or give a backup with `x ?? backup`.

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

## A frame, step by step

A game with a window runs in **frames**, normally 60 a second. With `window.run()`, every frame does this, in this order:

1. Show the frame drawn last time, and read the keyboard, mouse and controllers.
2. If the player clicked the window's X (or `window.close()` was called), stop: `window.run()` returns, and `main` carries on after it.
3. Clear the screen to the background color.
4. Wake up every [task](../language/11-tasks.md) whose `wait(...)` is over.
5. Call `update(dt)`. `dt` is the time since the last frame, in seconds (about `0.0167` at 60 frames a second). On the very first frame it's `0`.
6. Call `draw()`, unless `update` closed the window.

Calling a task starts it straight away: it runs until its first `wait`, then the caller carries on.

Input is read once per frame, in step 1, so it can't change halfway through `update`. `keyPressed` is `true` on exactly one frame (the first one the key is down), `keyDown` on every frame it's down, and `keyReleased` on the first frame it's up again.

This game prints what happens in its first 4 frames, with Space held down during frames 1 and 2 (see [Testing a game that opens a window](#testing-a-game-that-opens-a-window)):

<!-- check: output frames=4 input=SPACE:1-2 -->

```cpp
#include <window>
#include <input>
#include <stdio>

int frame = 0;

task ticker(){
    for (int i in 1..2) {
        print("  the task runs (frame {frame})");
        wait(0);
    }
}

int main(){
    window.open(320, 200, "Frames");
    ticker();
    print("before window.run()");
    window.run();
    print("after window.run(): {frame} frames");
    return 0;
}

void update(float dt){
    string first = dt > 0 ? "" : " (dt is 0)";
    print("update {frame}{first}");
    if (input.keyPressed(Key.SPACE)) {
        print("  Space was pressed");
    }
    if (input.keyDown(Key.SPACE)) {
        print("  Space is down");
    }
    if (input.keyReleased(Key.SPACE)) {
        print("  Space was let go");
    }
}

void draw(){
    print("draw {frame}");
    frame++;
}
```

```text
  the task runs (frame 0)
before window.run()
  the task runs (frame 0)
update 0 (dt is 0)
draw 0
update 1
  Space was pressed
  Space is down
draw 1
update 2
  Space is down
draw 2
update 3
  Space was let go
draw 3
after window.run(): 4 frames
```

With your own loop (`while (!window.shouldClose()) { ... }`), each call to `window.shouldClose()` does steps 1 to 4, and your loop's body is steps 5 and 6.

## Stopping the game

No single key closes a Forge game: **Escape is a normal key**, so games can use it for a pause menu. A game stops in one of these ways:

| What happens | What stops | What runs after it |
|---|---|---|
| The player clicks the window's X, or presses Alt+F4 | `window.run()` returns, or `window.shouldClose()` gives `true` | the rest of `main` (save the game there!) |
| Your code calls `window.close()` | the same, at the end of this frame: `draw()` isn't called after an `update` that closed the window | the rest of `main` |
| `main` returns a number | the whole program; an open window closes | nothing. The number is the [exit code](commands.md#exit-codes) |
| A runtime error that no `try` catches | the whole program, with the error's message | nothing. The exit code is 1 |
| `throw "message";` that no `try` catches | the same as a runtime error | nothing. The exit code is 1 |

So the pattern for "the player wants to quit" is the same however they ask: let `window.run()` (or your loop) end, then do your last things in `main`.

### Closing from your own code

`window.close()` is how the game itself decides to stop, for example from a Quit button or after the last level. In `window.run()`, `update` finishes, `draw` is skipped, and `window.run()` returns:

<!-- check: output exit 3 frames=60 -->

```cpp
#include <window>
#include <graphics2d>
#include <stdio>

int frames = 0;

int main(){
    window.open(320, 200, "Quit after 3 frames");
    window.run();
    print("window.run() returned after {frames} frames");
    return 3;
}

void update(float dt){
    frames++;
    if (frames == 3) {
        print("closing");
        window.close();
        print("update still finishes");
    }
}

void draw(){
    graphics2d.drawRect(10, 10, 20, 20, color.RED);
}
```

```text
closing
update still finishes
window.run() returned after 3 frames
```

### Your own loop

With your own loop, `window.close()` makes the next `window.shouldClose()` give back `true`, so the loop ends. Everything after `window.close()` in that turn of the loop still runs, but **drawing after it is an error**, because the window is gone (use `break` to leave the loop straight away).

<!-- check: output frames=60 -->

```cpp
#include <window>
#include <graphics2d>
#include <stdio>

int main(){
    window.open(320, 200, "My own loop");
    int frames = 0;
    while (!window.shouldClose()) {
        frames++;
        graphics2d.drawRect(10, 10, 20, 20, color.RED);
        if (frames == 4) {
            window.close();
        }
    }
    print("the loop ended after {frames} frames");
    return 0;
}
```

```text
the loop ended after 4 frames
```

<!-- check: runtime-error exit 1 frames=60 -->

```cpp
#include <window>
#include <graphics2d>

int main(){
    window.open(320, 200, "Drawing after closing");
    while (!window.shouldClose()) {
        window.close();
        graphics2d.drawRect(10, 10, 20, 20, color.RED);
    }
    return 0;
}
```

```text
Runtime error in graphics2d.drawRect(): the window has been closed (by window.close(), or by the player), so there's nothing to draw on - check window.shouldClose() before drawing, or stop drawing after closing it
  called from main.frg:8 (in main)
```

### The player clicks the X

Here the player "clicks the X" on frame 5 (with scripted input), and the game saves before it ends. Escape, held during frames 1 to 3, doesn't close anything: the game reads it like any key.

<!-- check: output frames=60 input=ESCAPE:1-3,CLOSE:5-5 -->

```cpp
#include <window>
#include <input>
#include <stdio>

int frames = 0;

int main(){
    window.open(320, 200, "Quitting");
    window.run();
    print("the player closed the window after {frames} frames - saving");
    return 0;
}

void update(float dt){
    frames++;
    if (input.keyPressed(Key.ESCAPE)) {
        print("Escape: open the pause menu");
    }
}

void draw(){
}
```

```text
Escape: open the pause menu
the player closed the window after 5 frames - saving
```

### A window after a window

After `window.close()`, `window.open(...)` opens a new window, so a game can show a launcher window, close it, and open the game's own:

<!-- check: output frames=60 -->

```cpp
#include <window>
#include <graphics2d>
#include <stdio>

int main(){
    window.open(320, 200, "Launcher");
    int frames = 0;
    while (!window.shouldClose() and frames < 3) {
        frames++;
    }
    window.close();
    print("the launcher closed");
    window.open(640, 360, "The game");
    frames = 0;
    while (!window.shouldClose() and frames < 3) {
        graphics2d.drawText("Playing", 10, 10, 20, color.WHITE);
        frames++;
    }
    print("the game's window is done");
    return 0;
}
```

```text
the launcher closed
the game's window is done
```

## Testing a game that opens a window

Three [environment variables](commands.md#environment-variables) let a test play a game without anyone at the keyboard:

- `FRG_AUTOTEST_FRAMES=N` runs **exactly N frames** (frames 0 to N-1), then closes the window as if the player had clicked the X. So `window.run()` returns, and the rest of `main` runs.
- `FRG_AUTOTEST_SHOT=name.png` saves a picture of the last frame (normally `autotest.png`, in the game's folder).
- `FRG_AUTOTEST_INPUT=SPACE:1-2,CLOSE:5-5` holds keys and mouse buttons during those frames, both ends included. `CLOSE` clicks the window's X, `MOUSE@x/y` moves the pointer (for clicking menus) and `TURN@dx/dy` moves the mouse (for mouse-look). See [Scripted input](commands.md#scripted-input).

Forge's own tests use these to check the examples on this page, and every example game, on every change.
