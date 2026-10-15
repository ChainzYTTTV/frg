# The Forge Language (v0.4)

This guide covers everything Forge can do **right now**. Planned features are listed at the end.

**Contents:** [A Forge game](#a-forge-game) · [Variables](#variables-and-types) · [Text](#text) · [Operators](#operators) · [If and loops](#if-loops-and-switch) · [Functions](#functions) · [Files and folders](#multiple-files) · [Lists and maps](#lists-and-maps) · [Structs and classes](#structs-and-classes) · [Enums](#enums) · [Optional values and null](#optional-values-and-null) · [Pointers](#pointers) · [Built-in types](#built-in-types-vec2-color-rect) · [Libraries](#libraries) · [Making a game](#making-a-game) · [Errors](#errors)

## A Forge game

Every game is a folder. The top folder must contain a file named **`main.frg`** with an **`int main()`** function. That's where the game starts.

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 0;   // 0 = success, 1 = an error happened
}
```

| Command | What it does |
|---|---|
| `frg new mygame` | makes a folder `mygame/` with a starter `main.frg` |
| `frg build` | compiles the game into `build/<folder name>.exe` |
| `frg run` | builds, then runs the game |

## Comments

````cpp
// a single-line comment

```
a multi-line comment
can go over many lines
```
````

## Variables and types

Every variable has a type, written before its name:

```cpp
int hp = 15;
float speed = 2.5;
double price = 19.99;
bool alive = true;
string name = "Hero";
```

| Type | Holds | Notes |
|---|---|---|
| `int` | whole numbers | 32 bits, about ±2 billion |
| `int8` `int16` `int64` `int128` | whole numbers | smaller or bigger than `int` |
| `uint` `uint8` `uint16` `uint64` `uint128` | whole numbers ≥ 0 | "u" means *unsigned* (no negatives) |
| `float` | decimal numbers | about 7 digits of precision. Use it for game math. |
| `double` | decimal numbers | about 15 digits of precision |
| `bool` | `true` or `false` | |
| `string` | text | |

A number variable with no value starts at zero (`0`, `0.0`, `false`, `""`).

`const` makes a variable that can never be set to something else:

```cpp
const int MAX_HP = 100;
```

### Converting between types

Forge converts automatically when nothing can be lost. For example, an `int` fits in a `float`. Otherwise you convert yourself:

```cpp
int whole = int(3.9);         // 3 (the decimals are cut off)
float f = float(7) / 2;       // 3.5
int n = int("42");            // text to number
string s = string(99);        // anything to text
```

Watch out: `7 / 2` is `3`, because both sides are whole numbers. Use `7.0 / 2` to get `3.5`.

Whole numbers **wrap around** when they go past their limit. A `uint8` holding `250` becomes `4` after `+= 10`.

## Text

Put any value inside `{ }` in a string to show it. This works for numbers, lists, structs and anything else:

```cpp
print("HP: {hp}/{MAX_HP}");
print("Is it there? {name.contains("ero")}");
```

| Write this | To get |
|---|---|
| `\n` `\t` | a new line, a tab |
| `\"` `\\` | a quote, a backslash |
| `{{` and `}}`, or `\{` and `\}` | a literal `{` or `}` |

Text has `.length` plus these methods: `contains(t)` `indexOf(t)` `startsWith(t)` `endsWith(t)` `upper()` `lower()` `trim()` `replace(a, b)` `split(sep)` `substring(start, count)` `charAt(i)` `repeat(n)` `isEmpty()`.

## Operators

| | |
|---|---|
| Math | `+  -  *  /  %` (`%` is the remainder) |
| Changing a variable | `=  +=  -=  *=  /=  %=  ++  --` |
| Comparing | `==  !=  <  >  <=  >=` |
| Logic | `and` / `&&`, `or` / `\|\|`, `not` / `!`. Each pair means the same thing. |
| Pick one | `condition ? a : b`, which is `a` if the condition is true, otherwise `b` |
| Backup value | `maybe ?? backup`, which is `backup` if `maybe` is null |

```cpp
if (alive and not paused) { ... }
string label = hp > 50 ? "healthy" : "hurt";
```

## If, loops and switch

`{ }` braces are always required. Indentation is just for readability.

```cpp
if (hp <= 0) {
    print("Game over");
} else if (hp < 10) {
    print("Low health!");
} else {
    print("OK");
}

for (int i = 0; i < 10; i++) {
    print(i);
}

for (Enemy e in enemies) {      // goes through every item in a list
    e.update();
}

while (lives > 0) {
    lives--;
}
```

`break` leaves a loop early. `continue` skips to the next time around.

`switch` picks a case by value. A case ends where the next one starts, so it never needs `break`:

```cpp
switch (state) {
    case State.Idle:
        print("waiting");
    case State.Running, State.Jumping:
        print("moving");
    default:
        print("something else");
}
```

## Functions

```cpp
int add(int a, int b){
    return a + b;
}

void greet(string who, string greeting = "Hi"){   // greeting has a default value
    print("{greeting} {who}!");
}
```

**Order doesn't matter.** A function can be used above or below the place it's written. Forge reads every file before it runs anything.

## Multiple files

Every `.frg` file next to `main.frg` is part of the game automatically. To use something from another file, always start with the file's name. This works for functions, variables and types:

```cpp
// player.frg
int score = 0;
class Player { ... }
void jump(float height){ ... }
```

```cpp
// main.frg
player.jump(2.5);
player.score += 100;
player.Player p = player.Player();
```

### Folders

Files in a subfolder only join the game when a file next to that folder includes it:

```
mygame/
  main.frg               #include <"enemies">
  enemies/
    goblin.frg           #include <"bosses">
    bosses/
      dragon.frg
```

You always reach things by their full path from the top folder: `enemies.goblin.spawn()` or `enemies.bosses.dragon.roar()`.

## Lists and maps

```cpp
list<int> scores = [10, 25, 7];
scores.add(40);
scores[0] = 99;
print(scores.length);          // 4

map<string, int> ages = {"Ana": 12, "Ben": 14};
ages["Cy"] = 9;                // adds a new key
print(ages["Ana"]);
if (ages.has("Zed")) { ... }
```

| | Methods |
|---|---|
| `list<T>` | `.length` `add(x)` `insert(i, x)` `removeAt(i)` `remove(x)` `contains(x)` `indexOf(x)` `first()` `last()` `pop()` `clear()` `isEmpty()` `sort()` `reverse()` `copy()` |
| `map<K, V>` | `.length` `has(key)` `get(key, backup)` `remove(key)` `keys()` `values()` `clear()` `isEmpty()` |

Positions in a list start at 0. Using a position that isn't in the list, or a key that isn't in the map, stops the game with a clear message.

## Structs and classes

Both group data (**fields**) with functions that work on it (**methods**). Inside a method, fields can be used by name or as `this.field`.

```cpp
struct Point {
    float x;
    float y;

    float length() {
        return math.sqrt(x * x + y * y);
    }
}

Point p = Point(3, 4);   // fields in order
Point q = {3, 4};        // the same thing
Point zero = Point();    // every field at its starting value
```

The difference between them is what happens when you copy one:

- A **struct** is a value. `Point b = a;` makes a separate copy, so changing `b` doesn't change `a`. Use structs for small bits of data like positions and colors.
- A **class** is shared. `Player b = a;` makes `b` another name for the *same* player. Use classes for game objects: players, enemies, items.

Classes can **inherit** from another class. They get all its fields and methods, and can replace methods with their own version:

```cpp
class Entity {
    string name;
    int hp = 100;

    Entity(string name) {        // a constructor: runs when one is made
        this.name = name;
    }

    string describe() {
        return "an entity";
    }
}

class Goblin : Entity {          // a Goblin is a kind of Entity
    string describe() {
        return "a goblin, not just " + super.describe();
    }
}

Entity e = Goblin("Grok");       // a Goblin can go anywhere an Entity can
print(e.describe());             // "a goblin, not just an entity"
```

- A type with no constructor is made with `Type()` or `Type(every field, in order)`.
- A class without its own constructor uses the one it inherits. In a constructor, `super(...)` runs the parent class's constructor.
- `print` shows a struct or class as `Point(x: 3.0, y: 4.0)`. To show it differently, add a method `string toString()`.

## Enums

An enum is a type with a fixed set of named values:

```cpp
enum State { Idle, Running, Jumping }

State s = State.Running;
print(s);              // Running
int n = int(s);        // 1
State t = State(0);    // Idle
```

## Optional values and null

Adding `?` to a type means the variable may also hold `null` (nothing):

```cpp
Enemy? target = null;
int? bestScore = null;

if (target != null) {
    target.takeHit(5);
}
int shown = bestScore ?? 0;   // 0 when there's no best score yet
```

Variables of class type **without** a `?` must always hold an object, so they need a starting value. You can use an optional anywhere its plain type is needed. If it turns out to be null, the game stops and tells you exactly where.

## Pointers

A `Pointer<T>` points at a variable, so you can change that variable from somewhere else. `.value` reads or changes the variable it points at:

```cpp
void addOne(Pointer<int> counter) {
    counter.value += 1;
}

int score = 10;
addOne(score);        // score is now 11
```

Don't keep a pointer to a function's local variable after that function ends.

## Built-in types: vec2, color, rect

These are always available:

| Type | Fields | Useful extras |
|---|---|---|
| `vec2` | `x` `y` | `length()` `normalized()` `dot(v)` `distance(v)` `angle()` `rotated(r)` `lerp(v, t)`, `vec2.ZERO` `ONE` `UP` `DOWN` `LEFT` `RIGHT` |
| `vec3` | `x` `y` `z` | `length()` `normalized()` `dot(v)` `cross(v)` `distance(v)` `lerp(v, t)`, `vec3.ZERO` `ONE` `UP` `FORWARD` `RIGHT` |
| `vec4` | `x` `y` `z` `w` | `length()` `normalized()` `dot(v)` |
| `color` | `r` `g` `b` `a` (0 to 255) | `color(r, g, b)` or `color(r, g, b, a)`, `fade(alpha)` `lerp(c, t)`, and `color.WHITE` `BLACK` `RED` `GREEN` `BLUE` `YELLOW` `ORANGE` `PURPLE` `PINK` `GOLD` `SKYBLUE` `GRAY` `LIGHTGRAY` `DARKGRAY` `BROWN` `MAGENTA` `LIME` `MAROON` `VIOLET` `BEIGE` `DARKGREEN` `DARKBLUE` `BLANK` |
| `rect` | `x` `y` `width` `height` | `overlaps(r)` `contains(point)` `center()` `position()` `size()` |

Vectors work with math: `pos += velocity * dt;`

## Libraries

Each file includes the libraries it uses. Functions are reached through the library name, like `window.open(...)`. Types from a library, like `Key` or `Body`, are used by their plain name.

| Include | Gives you |
|---|---|
| `<stdio>` | `print(x)` (adds a new line), `print()`, `write(x)` (no new line), `input()` / `input("prompt")` |
| `<math>` | `math.sqrt` `pow` `sin` `cos` `tan` `asin` `acos` `atan` `atan2` `exp` `log` `floor` `ceil` `round` `abs` `sign` `min` `max` `clamp` `lerp`, plus `math.PI` `TAU` `E` |
| `<random>` | `random.range(1, 6)` (whole numbers, both ends included), `random.range(0.5, 2.0)`, `random.value()` (0 to 1), `random.chance(0.25)`, `random.seed(n)` |
| `<window>` | `window.open(width, height, "title")` `shouldClose()` `run()` `close()` `deltaTime()` `time()` `width()` `height()` `fps()` `setFPS(n)` `setTitle(t)` `setBackground(color)` `toggleFullscreen()` `screenshot("file.png")` |
| `<input>` | `input.keyDown(Key.SPACE)` (held), `keyPressed` (just pressed this frame), `keyReleased`, `mouseDown(MouseButton.LEFT)` `mousePressed` `mouseReleased` `mousePosition()` `mouseX()` `mouseY()` `mouseWheel()`. Keys: `Key.A`…`Key.Z`, `Key.NUM0`…`Key.NUM9`, `SPACE` `ENTER` `ESCAPE` `BACKSPACE` `TAB` `LEFT` `RIGHT` `UP` `DOWN` `LEFT_SHIFT` `RIGHT_SHIFT` `LEFT_CONTROL` `RIGHT_CONTROL` `LEFT_ALT` `RIGHT_ALT` `F1`…`F12` |
| `<graphics2d>` | `graphics2d.drawRect(x, y, w, h, color)` or `drawRect(rect, color)`, `drawRectOutline(..., color, thickness)`, `drawCircle(x, y, radius, color)` or `drawCircle(vec2, radius, color)`, `drawCircleOutline`, `drawLine(from, to, color, thickness)`, `drawTriangle(a, b, c, color)`, `drawText(text, x, y, size, color)`, `textWidth(text, size)`, `drawFPS(x, y)`, `clear(color)` |
| `<physics>` | the `Body` class and `physics.add(body)` `remove(body)` `step(dt)` `setGravity(vec2)` `gravity()` `clear()` `count()` (see below) |

Screen coordinates start at the **top-left** corner, and **y goes down**.

### Physics

A `Body` is a box: `Body(x, y, width, height)`. It has `position`, `size`, `velocity`, `isStatic`, `gravityScale`, plus `onGround`, `onCeiling` and `onWall`, which say what it touched during the last step. It also has `bounds()`, `center()` and `overlaps(other)`.

```cpp
Body ground = Body(0, 500, 960, 40);
ground.isStatic = true;          // static bodies never move: floors, walls, platforms
physics.add(ground);

Body box = Body(100, 100, 32, 32);
physics.add(box);

// every frame:
physics.step(dt);                // falls with gravity, stops on static bodies
if (box.onGround) { ... }
```

Your own classes can inherit from `Body`: `class Player : Body { ... }`. Moving bodies don't collide with each other, so use `a.overlaps(b)` to check for that yourself.

## Making a game

There are two ways to write the game loop.

**1. Your own loop.** `shouldClose()` shows the frame you just drew and starts the next one:

```cpp
#include <window>
#include <graphics2d>

int main(){
    window.open(800, 450, "My Game");
    while (!window.shouldClose()) {
        mainLoop();
    }
    window.close();
    return 0;
}

void mainLoop(){
    graphics2d.drawCircle(400, 225, 50, color.RED);
}
```

**2. The engine loop.** `window.run()` calls `update(float dt)` and then `draw()` from main.frg every frame:

```cpp
int main(){
    window.open(800, 450, "My Game");
    window.run();
    return 0;
}

void update(float dt){
    // move things: dt is the time since the last frame, in seconds
}

void draw(){
    // draw things
}
```

The screen is cleared to the background color before each frame (`window.setBackground(color)` changes it). The window closes when you press Escape or click the X.

See `examples/platformer` for a complete game.

## Errors

Forge explains what went wrong, where it happened, and usually how to fix it:

```
error: 'hp' needs an int, but this is a float
  --> main.frg:4:14
   |
 4 |     int hp = 7.5;
   |              ^^^
  hint: use int(...) to turn it into a whole number (the decimals get cut off)
```

Problems that can only be found while the game runs stop it with the file and line:

```
Runtime error in main.frg:10: position 5 is outside the list (it has 3 items, at positions 0 to 2)
```

## Coming next

- **3D:** `<graphics3d>` with cameras and models (the `vec3` type is already here)
- **More libraries:** `<audio>`, `<assets>` (images, sounds, fonts), `<ECS>`, `<time>`, `<file>`, `<scenes>`, `<ui>`
- **Coroutines:** `wait(2.0)` inside game logic
- **Graphics backends:** `#include <OpenGL>`, `<Vulkan>`, `<DirectX>`
- **Language:** user-made generic types, `is` checks for classes, `private` fields
