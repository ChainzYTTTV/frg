# Teaching Brief: the Forge Programming Language

> **To the AI reading this:** you're going to teach a beginner a programming language called **Forge**. It's a brand-new language, so you have never seen it before, and nothing about it is on the internet. **This file is the only source of truth.** Read all of it before you start.

---

## Part 1: How to teach

### Who you're teaching

- They're **new to programming**. They know the very basics (variables, if statements, printing) but not deeper ideas such as memory, generics or design patterns.
- They designed Forge's rules together with the AI that built it, so they have opinions about how it looks. They didn't write the compiler, though, and they're learning to *use* the language.
- Their goal is **making games**. The first real target is a **2D platformer**: a cube that jumps between rectangle platforms. A finished example is in Part 5.
- They use **Windows** and **VS Code**, and Forge is installed. They can run any code you give them, so they're the one running things, not you.

### How to teach well

1. **Only teach what's in this file.** Never invent a feature, function or library. If Forge has something, it's listed here. If it isn't here, Forge doesn't have it yet. Say so plainly ("Forge can't do that yet") and show the closest way to do it with what *is* here.
2. **Many languages look like Forge, but don't assume they match.** Forge looks like C++ and reads a bit like Python, but its rules differ in places. Part 4 lists the differences you're most likely to get wrong.
3. **One small idea at a time.** Explain the idea in plain words. Then show a short, complete program that uses it, and give a small exercise. Wait for the learner to try it before moving on.
4. **Programs you give must be complete and runnable.** They need `main.frg`, `int main()`, the `#include` lines they use, and `return 0;`. A missing include is the most common beginner error.
5. **Let them run it, then read the output together.** Ask them to paste what they see, including any errors. Forge's error messages are written for beginners (examples in Part 3), so teach them to read the message and the `hint:` line.
6. **Use game examples** (health, scores, enemies, coins, jumping) instead of abstract ones like `foo` and `bar`.
7. **Check understanding.** Every few lessons, ask them to predict what a program prints before they run it, or to fix a broken program.
8. **Be encouraging, but correct mistakes clearly.** If their code is wrong, say what's wrong and why, then let them fix it themselves before showing the answer.

### The plan of lessons

Go in this order. Each stage ends with a small project. Move at the learner's pace and skip ahead if a stage is easy for them. **Part 6 has 18 more practice projects**, with steps and hints, for extra practice at each stage.

| Stage | Topics | Small project at the end |
|---|---|---|
| 1. First programs | `main.frg`, `print`, comments, running with `frg run`, reading errors | Print a short "character sheet" |
| 2. Values | variables, types (`int`, `float`, `bool`, `string`), math, `{}` in text, converting types | A damage calculator |
| 3. Decisions | `if`/`else`, comparisons, `and`/`or`/`not`, `switch` | Pick a message based on health |
| 4. Repeating | `for`, `while`, `break`/`continue` | Count down to launch, print a times table |
| 5. Functions | parameters, `return`, `void`, default values, using functions before they're written | Turn earlier projects into functions |
| 6. Collections | `list`, `map`, `for (x in list)`, list methods | A high-score table |
| 7. Your own types | `struct` vs `class`, fields, methods, constructors, `this` | `Point` struct, `Enemy` class |
| 8. Inheritance | `class Goblin : Enemy`, replacing methods, `super` | A few enemy types in one list |
| 9. Enums, optionals, null | `enum`, `Type?`, `null`, `??` | An enemy that may or may not have a target |
| 10. Multiple files | `player.frg` + `main.frg`, the file-name prefix rule, folders | Split a program into files |
| 11. Window and drawing | `window`, `graphics2d`, colors, coordinates (y goes **down**), the game loop | Draw a scene |
| 12. Input and movement | `input`, `deltaTime`, `vec2` | Move a square with the arrow keys |
| 13. Physics | `Body`, `physics.step`, `onGround`, jumping | A cube that jumps on a platform |
| 14. The platformer | read Part 5 together and change it | Add a coin, a level, or a moving hazard |
| 15. Tasks | `task` functions, `wait()` | Make the player flash, or a 3-2-1 countdown |
| 16. Images and sound | `<assets>`, `<audio>`, sprite sheets | Give the platformer a sprite and a jump sound |
| 17. Menus | `<scenes>`, `<ui>`, `<file>` | A main menu, and a best score saved to a file |
| 18. Checking kinds, hiding things | `is` / `as`, `private`, several constructors | Different enemy types in one list |
| 19. Components | `<ECS>` | Rebuild a small game out of components |
| 20. 3D | `<graphics3d>` | A spinning cube scene |
| 21. (Optional, advanced) Graphics APIs | `<OpenGL>` first, then `<DirectX>` or `<Vulkan>` if they're curious. Only if the learner asks: the engine is all they need for games. | One colored triangle, then a spinning cube |

---

## Part 2: Using Forge

Every game is a **folder**. The top folder must contain **`main.frg`**, and that file must contain **`int main()`**. That's where the game starts.

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 0;   // 0 = everything went fine, 1 = something went wrong
}
```

Commands, typed in a terminal inside the game's folder:

| Command | What it does |
|---|---|
| `frg new mygame` | makes a new folder `mygame/` with a starter `main.frg` |
| `frg run` | checks the code, compiles it, and runs the game |
| `frg build` | compiles into `build/<folder name>.exe` without running it |
| `frg check` | only looks for errors |
| `frg build --release` | the finished game: no console window, smaller `.exe` |
| `--no-console` / `--console` | add to `run` or `build` to turn the console window off or on |

The **console window** (the black window next to the game) shows `print` output and errors. It's on while testing. Turn it off with `--no-console` to see the game as players will, and `--release` turns it off for the finished game. Without a console, runtime errors pop up in a box titled "The game stopped". Text-only programs need the console: without it nothing shows. In VS Code, **Forge: Turn the Console On/Off** switches it for the ▶ button, and **Forge: Build Release** builds the finished game.

In **VS Code** (with the Forge extension): errors are underlined when you save, the **▶ button** at the top right (or **F5**) runs the game, typing `window.` or `color.` shows autocomplete, and snippets like `game`, `class` and `foreach` insert templates.

Forge is **compiled**. The whole program is checked before it runs, so most mistakes are caught before the game starts.

The first build (and the first `--release` build) shows "Preparing Forge's runtime ... (only the first time)" and takes longer: Forge is getting its own code ready once, so later builds of every game are faster. It's stored in a `cache` folder next to `frg.exe` that can be deleted at any time.

---

## Part 3: The language reference

### Comments

````cpp
// one line

```
many
lines
```
````

Multi-line comments use three backticks at the start and end. `/* */` is **not** Forge.

### Variables and types

Every variable has a type, written before its name. The semicolon at the end of each line is required.

```cpp
int hp = 15;
float speed = 2.5;
double price = 19.99;
bool alive = true;
string name = "Hero";
const int MAX_HP = 100;     // const: can never be set to a new value
```

| Type | Holds |
|---|---|
| `int` | whole numbers (about ±2 billion) |
| `int8` `int16` `int64` `int128` | smaller or bigger whole numbers |
| `uint` `uint8` `uint16` `uint64` `uint128` | whole numbers that can't be negative |
| `float` | decimal numbers. Use these for game math. |
| `double` | more precise decimal numbers |
| `bool` | `true` or `false` |
| `string` | text |

- A number, bool or string variable with no starting value is `0`, `0.0`, `false` or `""`.
- Forge converts automatically only when nothing is lost: an `int` can go into a `float`, but a `float` can't go into an `int`. You convert yourself with `int(3.9)` (gives 3, cutting off the decimals), `float(x)`, `int("42")` (text to number), and `string(x)` (anything to text).
- **Division** of two whole numbers gives a whole number, cut toward zero: `7 / 2` is `3`, and `int a = 70; int b = 30; float r = a / b;` gives `2.0` (the division happens first, with whole numbers). To get a decimal, make one side a decimal: `7.0 / 2` or `float(a) / b`. **One special case:** numbers written straight into the code adapt to where the result goes, so `float r = 70 / 30;` is `2.3333333`, while `print(70 / 30)` is `2`. The learner chose to keep this C-style behavior on purpose, so teach it clearly rather than treating it as a bug.
- Whole numbers **wrap around** past their limit: a `uint8` holding 250 becomes 4 after `+= 10`.
- Decimal numbers print with a `.0` when they're whole: `print(5.0)` shows `5.0`.

### Text

```cpp
print("HP: {hp}/{MAX_HP}");             // anything inside { } is shown as text
print("Half: {speed / 2}");
print("Has it? {name.contains("er")}");
print("Speed: {speed:.2}");             // :.2 shows a float or double with 2 decimals (3.14)
string full = "Game" + " Over";         // + joins two pieces of text
```

- `{value:.N}` (N from 0 to 9) shows a `float` or `double` with exactly N decimals, rounded. For an int, convert it first: `{float(score):.1}`. It doesn't clash with the ternary: in `{a ? 1 : 2}` the `:` is the ternary's, because numbers never start with a dot.

- Escapes: `\n` new line, `\t` tab, `\"` quote, `\\` backslash, `{{` or `\{` for a literal `{`, and `}}` or `\}` for a literal `}`.
- **Raw text** goes between three quotes, `"""like this"""`, and can cover many lines. It's used exactly as written: escapes and `{ }` don't work inside it, and it can contain `"` and `\`. A line break straight after the opening `"""` is left out. It's mainly for shader code, maps made of characters, and Windows paths.
- `"Score: " + 5` is an **error**. Write `"Score: {5}"` instead.
- Text has `.length` (no parentheses) and these methods: `contains(t)`, `indexOf(t)` (-1 if not found), `startsWith(t)`, `endsWith(t)`, `upper()`, `lower()`, `trim()`, `replace(old, new)`, `split(separator)` (gives a list), `substring(start, count)`, `charAt(i)` (gives a one-letter string), `repeat(n)`, `isEmpty()`, `isInt()` and `isFloat()` (true if `int(...)` / `float(...)` would work on the text, so input can be checked before converting it: `if (typed.isInt()) { int n = int(typed); }`).
- Text **can't** be indexed with `[ ]`. Use `charAt(i)`.

### Printing and reading (`#include <stdio>`)

- `print(x)` shows one value and starts a new line, and `print()` prints an empty line.
- `write(x)` shows a value without starting a new line.
- `string answer = input("Your name? ");` waits for the player to type a line.
- `print` takes **one** value. To show several, put them in text: `print("x = {x}, y = {y}")`.

### Operators

**Every operator has a symbol and a word form.** They mean exactly the same and can be mixed. Teach both, and let the learner use whichever they like:

| Symbol | Word | Meaning |
|---|---|---|
| `a + b` `a - b` `a * b` `a / b` `a % b` | `a plus b` `a minus b` `a times b` `a div b` `a mod b` | math (`%`/`mod` is the remainder) |
| `-a` | `negative a` | flip the sign |
| `a == b` / `a != b` | `a equals b` / `a not equals b` | |
| `a > b` / `a < b` | `a greater b` / `a less b` | |
| `a >= b` / `a <= b` | `a greater or equal b` / `a less or equal b` | |
| `&&` `\|\|` `!` | `and` `or` `not` | logic |
| `c ? a : b` | `a if c else b` | pick one (`if` must be on the same line) |
| `a ?? b` | `a otherwise b` | `a`, or `b` if `a` is null |
| `&` `\|` `^` `~` `<<` `>>` | `bitand` `bitor` `bitxor` `bitnot` `shiftleft` `shiftright` | bit operations (rarely needed) |
| `x = 5;` | `set x to 5;` | |
| `x += n;` `x -= n;` | `increase x by n;` `decrease x by n;` | |
| `x *= n;` `x /= n;` | `multiply x by n;` `divide x by n;` | |
| `x++;` `x--;` | `increase x;` `decrease x;` | |

- The words `plus`, `times`, `greater` and so on only act as operators between two values, so they can still be used as names. Only `negative` and `bitnot` are reserved.
- `not a == b` means `not (a == b)`, the same as in Python.
- There's **no power operator**; `^` is bitwise xor. Use `x * x` or `math.pow(x, 2)`.
- `++`, `--`, `increase x;` and `decrease x;` are statements only (`y = x++;` doesn't work).
- The order of operations follows C: `* / %`, then `+ -`, then comparisons, then `==`, then `and`, then `or`. Note that the bit operators come after `==`, so `(x & 1) == 0` needs its parentheses.

### If, loops, switch

Braces `{ }` are **always required**, even for one line. Parentheses around conditions are required. Conditions must be true/false values: `if (hp)` is an error, so write `if (hp > 0)`.

```cpp
if (hp <= 0) {
    print("Game over");
} else if (hp < 10) {
    print("Low health!");
} else {
    print("OK");
}

for (int i = 0; i < 10; i++) {   // C-style loop
    print(i);
}

for (string name in names) {     // goes through each item of a list
    print(name);
}

while (lives > 0) {
    lives--;
}
```

`break` leaves a loop and `continue` skips to the next time around.

```cpp
switch (state) {
    case State.Idle:
        print("waiting");
    case State.Running, State.Jumping:   // one case can match several values
        print("moving");
    default:
        print("something else");
}
```

`switch` works on whole numbers, text, bools and enums. **A case never falls into the next one, and `break` inside a switch is an error.**

### Functions

```cpp
int add(int a, int b){
    return a + b;
}

void greet(string who, string greeting = "Hi"){   // a default value
    print("{greeting} {who}!");
}
```

- A function can be used **before or after** the place it's written, and in any file.
- A function that returns a value must return on every path, or Forge reports an error.
- `void` means it gives nothing back.
- Each function name can be used only once per file: no two functions with the same name.

### Multiple files

Every `.frg` file next to `main.frg` is part of the game automatically. To use anything from another file, **always** put the file's name first. This is a rule even when there's no confusion.

```cpp
// player.frg
int score = 0;
void jump(float height){ ... }
class Player { ... }

// main.frg
player.jump(2.5);
player.score += 100;
player.Player p = player.Player();
```

Things in the same file are used without a prefix.

**Folders:** a subfolder only joins the game when a file next to it includes it with `#include <"folder name">` (note the quotes). Files inside that folder include their own subfolders the same way. Things are always reached by their full path from the top folder: `enemies.goblin.spawn()`, `enemies.bosses.dragon.roar()`.

Each file includes the libraries **it** uses. If `player.frg` uses `print`, then `player.frg` needs `#include <stdio>` itself.

### Lists and maps

```cpp
list<int> scores = [10, 25, 7];
scores.add(40);
scores[0] = 99;              // positions start at 0
print(scores.length);        // 4

map<string, int> ages = {"Ana": 12, "Ben": 14};
ages["Cy"] = 9;              // setting a new key adds it
print(ages["Ana"]);
if (ages.has("Zed")) { print("found"); }
```

- **List methods:** `.length`, `add(x)`, `addAll(otherList)` (adds every item of another list to the end), `insert(i, x)`, `removeAt(i)` (gives back the removed item), `remove(x)` (removes the first match and gives true/false), `contains(x)`, `indexOf(x)`, `first()`, `last()`, `pop()` (removes and gives back the last item), `clear()`, `isEmpty()`, `sort()` (numbers or text only), `reverse()`, `copy()`.
- **Map methods:** `.length`, `has(key)`, `get(key, backup)`, `remove(key)`, `keys()` (a list), `values()` (a list), `clear()`, `isEmpty()`.
- Map keys can be numbers, text, bools or enums.
- An empty list or map needs its type written out: `list<int> xs = [];` or `map<string, int> m = {};`.
- Lists can hold lists: `list<list<int>> grid = [[1, 2], [3, 4]]; grid[1][0] = 30;`.
- To loop through a map: `for (string k in ages.keys()) { ... }`.
- Using a position that isn't in a list, or a key that isn't in a map, stops the game with a runtime error that names the file and line.
- To remove items while looping, loop **backwards** with a C-style `for` (`for (int i = xs.length - 1; i >= 0; i--)`).

### Structs and classes

Both group **fields** (data) and **methods** (functions that belong to it). Inside a method, fields are used by name or as `this.field`.

```cpp
struct Point {
    float x;
    float y;

    float length() {
        return math.sqrt(x * x + y * y);   // needs #include <math>
    }
}

Point a = Point(3, 4);    // with no constructor: give every field, in order
Point b = {3, 4};         // the same thing
Point c = Point();        // every field at its starting value
```

**The key difference:**
- A **struct** is a *value*. `Point b = a;` makes a separate **copy**. Use structs for small data like positions.
- A **class** is *shared*. `Player b = a;` makes `b` another name for the **same** player, and passing a class to a function lets the function change it. Use classes for game objects.

```cpp
class Entity {
    string name;
    int hp = 100;                 // field with a starting value

    Entity(string name) {         // constructor: same name as the class, no return type
        this.name = name;         // this.name is the field, name is the parameter
    }

    void takeHit(int amount) {
        hp -= amount;
    }

    string describe() {
        return "an entity";
    }
}

class Goblin : Entity {           // inheritance: a Goblin is a kind of Entity
    string describe() {           // replaces Entity's version (must match it exactly)
        return "a goblin";
    }
}

class Player : Entity {
    int score = 0;
    Player(string name) {
        super(name);              // runs Entity's constructor
        hp = 150;
    }
    string describe() {
        return "the player, " + super.describe();   // calls Entity's version
    }
}

Entity e = Goblin("Grok");        // Goblin has no constructor, so it uses Entity's
print(e.describe());              // "a goblin": the replaced version runs
list<Entity> all = [Player("Hero"), Goblin("Grok")];
```

Rules:
- A type can have **several constructors** if each takes different types: `Size(float side)` and `Size(float w, float h)`. Forge picks the one matching the values given.
- A class with no constructors uses the ones it inherits. If there are none at all, it's made with `Type()` or `Type(every field, in order)`, where inherited fields come first.
- **Structs can't inherit.** Only classes can.
- A variable or field whose type is a class must start with a value: `Enemy e = Enemy(...);`. To start empty, make it optional: `Enemy? e = null;`.
- A struct can't contain itself (a list of itself is fine).
- `print(obj)` shows something like `Point(x: 3.0, y: 4.0)`. Adding a method `string toString()` changes what's shown.
- `==` on structs compares every field, and `==` on classes checks whether both are **the same object**.
- In `for (Point p in points)`, `p` is a **copy** of a struct, so changing it doesn't change the list. Use `points[i].x = ...` instead. With classes, the loop variable is the real object.

**Checking the real kind of an object** (classes only):

```cpp
for (Entity e in all) {
    if (e is Goblin) {              // true for a Goblin, or a class that inherits from Goblin
        Goblin g = e as Goblin;     // `as` gives Goblin?, which is null if e isn't a Goblin
        g.cackle();
    }
}
```

**`private`** hides things. A private field or method can only be used inside its class (and classes that inherit from it). A private function, global variable or type at the top of a file can only be used inside that file. `public` is allowed too and means the same as writing nothing.

```cpp
class Safe {
    private int code = 1234;
    bool tryOpen(int guess) { return guess == code; }
}
private int attempts = 0;           // only this file can see it
```

### Tasks: waiting without freezing the game

A function whose return type is **`task`** can call **`wait(seconds)`**. Only that task pauses; the game keeps running.

```cpp
task flash() {
    for (int i = 0; i < 5; i++) {
        tint = color.WHITE;
        wait(0.1);
        tint = color.ORANGE;
        wait(0.1);
    }
}

flash();                 // starts it; it runs until the first wait(), then carries on by itself
task t = flash();        // keep it to use t.isDone() or t.stop()
```

- `wait()` alone waits one frame. `wait(otherTask)` waits until that task finishes. `return;` stops a task early.
- `wait` only works inside a `task` function, or you get a compile error.
- With a window, tasks carry on by themselves every frame. In a program without a window, `#include <tasks>` and call `tasks.update(dt)`, or `tasks.runAll()` to wait for every task. `tasks.count()` and `tasks.stopAll()` are also there.
- Class methods can be tasks; struct methods can't.

### Enums

```cpp
enum State { Idle, Running, Jumping }

State s = State.Running;
print(s);              // Running
int n = int(s);        // 1 (counting from 0)
State t = State(0);    // Idle
```

### Optional values (`?`) and `null`

```cpp
Enemy? target = null;     // may hold an Enemy, or nothing
int? bestScore = null;

if (target != null) {
    target.takeHit(5);
}
int shown = bestScore ?? 0;
```

- Only optional types (with `?`) can be `null`.
- An optional can be used where its plain type is needed. If it's null at that moment, the game stops with `Runtime error in main.frg:12: 'target' is null`. Teach the learner to check with `!= null` first.
- `if (target)` is an error. Write `if (target != null)`.

### Pointers

```cpp
void addOne(Pointer<int> counter) {
    counter.value += 1;       // .value is the variable it points at
}

int score = 10;
addOne(score);                // score is now 11
Pointer<int> p = score;       // points at score
p.value = 50;                 // score is now 50
```

A pointer must point at a variable, field or list item. Don't keep one pointing at a function's local variable after that function ends. Pointers are rarely needed, since classes are already shared, so teach them late.

### Built-in types (always available, no include)

| Type | Fields | Extras |
|---|---|---|
| `vec2` | `x y` | `length()` `normalized()` `dot(v)` `distance(v)` `angle()` `rotated(radians)` `lerp(v, t)`, `vec2.ZERO` `ONE` `UP` `DOWN` `LEFT` `RIGHT` |
| `vec3` | `x y z` | `length()` `normalized()` `dot(v)` `cross(v)` `distance(v)` `lerp(v, t)`, `vec3.ZERO` `ONE` `UP` `FORWARD` `RIGHT` |
| `vec4` | `x y z w` | `length()` `normalized()` `dot(v)` |
| `color` | `r g b a` (0–255) | `color(r, g, b)` or `color(r, g, b, a)` from ints (kept between 0 and 255, so `color(random.range(0, 255), ...)` just works), `fade(alpha)`, `lerp(c, t)`. Named colors: `color.WHITE BLACK BLANK LIGHTGRAY GRAY DARKGRAY RED MAROON ORANGE YELLOW GOLD GREEN LIME DARKGREEN SKYBLUE BLUE DARKBLUE PURPLE VIOLET PINK MAGENTA BROWN BEIGE` |
| `rect` | `x y width height` | `overlaps(other)` `contains(point)` `center()` `position()` `size()` |

Vectors work with math: `pos += velocity * dt;`. Vectors add and subtract with vectors, and multiply or divide by numbers.

### Libraries

Library functions are reached through the library name (`window.open(...)`). Library *types* like `Key` and `Body` are used by plain name. Each file includes what it uses.

| Include | Contents |
|---|---|
| `<stdio>` | `print`, `write`, `input` |
| `<math>` | `math.sqrt pow sin cos tan asin acos atan atan2 exp log floor ceil round abs sign min max clamp lerp`, and `math.PI TAU E`. Angles are in radians. |
| `<random>` | `random.range(1, 6)` (whole numbers, both ends included), `random.range(0.5, 2.0)` (decimals), `random.value()` (0 to 1), `random.chance(0.25)`, `random.seed(n)`, `random.color()` (a random solid color) |
| `<window>` | `window.open(width, height, "title")`, `shouldClose()`, `run()`, `close()`, `deltaTime()` (seconds since the last frame), `time()`, `width()`, `height()`, `fps()`, `setFPS(n)`, `setTitle(t)`, `setBackground(color)`, `toggleFullscreen()`, `screenshot("file.png")` |
| `<input>` | `input.keyDown(Key.SPACE)` (held), `input.keyPressed(...)` (only the frame it was pressed), `input.keyReleased(...)`, `input.mouseDown(MouseButton.LEFT)`, `mousePressed`, `mouseReleased`, `mousePosition()` (a vec2), `mouseX()`, `mouseY()`, `mouseWheel()` |
| `<graphics2d>` | `graphics2d.drawRect(x, y, w, h, color)` or `drawRect(rect, color)`, `drawRectOutline(x, y, w, h, color, thickness)`, `drawCircle(x, y, radius, color)` or `drawCircle(vec2, radius, color)`, `drawCircleOutline(x, y, radius, color)`, `drawLine(from, to, color, thickness)`, `drawTriangle(a, b, c, color)`, `drawText(text, x, y, size, color)`, `textWidth(text, size)`, `drawFPS(x, y)`, `clear(color)`, and a 2D camera (below) |
| `<physics>` | the `Body` class, plus `physics.add(body)`, `remove(body)`, `step(dt)`, `setGravity(vec2)`, `gravity()`, `clear()`, `count()` |
| `<assets>` | `assets.loadTexture("x.png")` gives a `Texture`, `assets.loadFont("x.ttf")` gives a `Font`, `assets.exists(path)` |
| `<audio>` | `audio.loadSound("x.wav")` gives a `Sound`, `audio.loadMusic("x.ogg")` gives a `Music`, `audio.setVolume(v)` |
| `<graphics3d>` | the `Camera` struct and `Model` class, `graphics3d.setCamera(cam)`, `drawCube(pos, size, color)`, `drawCubeOutline`, `drawSphere(center, radius, color)`, `drawSphereOutline`, `drawCylinder(pos, radius, height, color)`, `drawPlane(center, vec2 size, color)`, `drawGrid(slices, spacing)`, `drawLine(from, to, color)`, `toScreen(vec3)`, `loadModel("x.glb")` |
| `<time>` | `time.now()`, `time.sleep(s)`, `time.clock()`, `time.date()`, `year() month() day() hour() minute() second()`, the `Timer` class |
| `<file>` | `file.read(path)`, `write(path, text)`, `append(path, text)`, `exists(path)`, `remove(path)`, `readLines(path)`, `writeLines(path, list)`, `files(folder)` |
| `<scenes>` | the `Scene` class, `scenes.change(scene)`, `update(dt)`, `draw()`, `current()` |
| `<ui>` | `ui.button(rect, "text")` (true when clicked), `ui.checkbox(rect, "text", checked)` (new value), `ui.slider(rect, value, min, max)` (new value), `ui.label(text, x, y, size, color)`, `ui.panel(rect)`, `ui.setTheme(bg, accent, text)` |
| `<ECS>` | the `Entity` and `Component` classes, `ECS.create(name)`, `add(entity)`, `update(dt)`, `draw()`, `find(name)`, `all()`, `withComponent("Type")`, `clear()`, `count()` |
| `<tasks>` | `tasks.update(dt)`, `runAll()`, `count()`, `stopAll()` (only needed without a window) |
| `<OpenGL>` `<DirectX>` `<Vulkan>` | low-level toolkits for writing your own rendering, **instead of** the engine (see "Low-level toolkits" below) |

The details of the newer libraries are in "More libraries" after the Physics section.

**Keys:** `Key.A` through `Key.Z`, `Key.NUM0` through `Key.NUM9`, `SPACE ENTER ESCAPE BACKSPACE TAB LEFT RIGHT UP DOWN LEFT_SHIFT RIGHT_SHIFT LEFT_CONTROL RIGHT_CONTROL LEFT_ALT RIGHT_ALT`, and `F1` through `F12`. **Mouse buttons:** `MouseButton.LEFT RIGHT MIDDLE`.

**Screen coordinates:** (0, 0) is the **top-left** corner. x goes right, and **y goes down**, so jumping means a *negative* y velocity.

**2D camera** (`<graphics2d>`), for levels bigger than the window. `Camera2D` has `vec2 target` (the point in the world to look at), `vec2 offset` (where on the screen it shows, usually the middle of the window), `float zoom` (1 = normal, must be above 0) and `float rotation` (degrees). Everything drawn after `graphics2d.setCamera(camera)` goes through it, until `graphics2d.resetCamera()`, so draw the world first, then reset, then the score and menus:

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));     // target, offset (zoom = 1, rotation = 0)

void update(float dt){
    camera.target = camera.target.lerp(hero.center(), 5 * dt);  // glide toward the player
}

void draw(){
    graphics2d.setCamera(camera);   // the world moves with the camera
    level.draw();
    graphics2d.resetCamera();       // the text stays on the screen
    graphics2d.drawText("Coins: {coins}", 20, 20, 24, color.GOLD);
}
```

`graphics2d.screenToWorld(input.mousePosition())` turns the mouse position into a world position (and `worldToScreen` goes the other way). `Camera2D` is a struct, so call `setCamera` again after changing it (each frame, in `draw`). Physics doesn't know about the camera; only drawing moves. The 3D `Camera` from `<graphics3d>` is separate.

### Physics

A `Body` is a box: `Body(x, y, width, height)`.
- Fields you set: `position`, `size`, `velocity` (all vec2), `isStatic` (true for floors, walls and platforms that never move), and `gravityScale` (1 by default).
- Fields Forge sets during each `physics.step(dt)`: `onGround`, `onCeiling` and `onWall`.
- Methods: `bounds()` (a rect), `center()`, `overlaps(otherBody)`.
- Gravity starts at `vec2(0, 980)`, in pixels per second per second, pointing down.
- Moving bodies stop when they hit static bodies, and push each other apart (so crates stack). A body with `solid = false` passes through everything, which is useful for coins and trigger zones; check those with `a.overlaps(b)`.
- Your own classes can inherit from `Body`: `class Player : Body { ... }`. With no constructor of its own, it uses `Body(x, y, width, height)`.
- **Debug view:** `physics.drawDebug()` (in `draw`) outlines every body: static ones blue, moving ones green, `solid = false` ones purple, and a yellow bar under a moving body that's `onGround`. Great for "why does my player fall through?" moments. The file that calls it needs `#include <graphics2d>` as well (physics itself never draws). With a 2D camera, call it before `resetCamera()`.

```cpp
Body ground = Body(0, 500, 960, 40);
ground.isStatic = true;
physics.add(ground);

Body box = Body(100, 100, 32, 32);
physics.add(box);

// once per frame:
physics.step(dt);
if (box.onGround and input.keyPressed(Key.SPACE)) {
    box.velocity.y = -600;      // up is negative y
}
```

### More libraries

**Images, fonts and sounds** (`<assets>`, `<audio>`). Files go in the game's folder next to `main.frg`. Loading works anywhere, even as a global variable before the window opens.

```cpp
Texture player = assets.loadTexture("player.png");   // .png .jpg .bmp .tga .gif .qoi
Font big = assets.loadFont("title.ttf");             // .ttf .otf
Sound jump = audio.loadSound("jump.wav");            // short sounds: .wav .ogg .mp3 .flac
Music song = audio.loadMusic("song.ogg");            // long music, streamed and looping

player.draw(x, y);                                    // draw(x, y, tint) tints it
player.drawScaled(x, y, 4);
player.drawRotated(vec2(x, y), 45, 1);                // centered at (x, y), turned 45 DEGREES, scale 1
player.drawPart(rect(0, 0, 16, 16), rect(x, y, 64, 64));   // one frame of a sprite sheet, stretched to 64x64
big.drawText("Hello", 20, 20, 48, color.WHITE);       // big.measure(text, size) gives the width
jump.play();
song.play();
```

- `Texture`: `width()`, `height()`, `draw`, `drawScaled`, `drawRotated`, `drawPart`.
- `Sound`: `play()`, `stop()`, `isPlaying()`, `setVolume(v)`, `setPitch(p)`.
- `Music`: `play()`, `stop()`, `pause()`, `resume()`, `isPlaying()`, `setVolume(v)`, `setLooping(b)`, `length()`, `position()`.
- A missing file stops the game with "can't find the file 'x.png'".
- **Sprite-sheet animation:** keep a frame number, and change it every 0.1 seconds or so with a timer variable. Draw `drawPart(rect(frame * 16, 0, 16, 16), rect(x, y, 64, 64))`.

**3D** (`<graphics3d>`). In 3D, **y goes up**.

```cpp
Camera cam = Camera(vec3(8, 6, 8), vec3(0, 0, 0));    // camera position, and the point it looks at
Model ship = graphics3d.loadModel("ship.glb");        // .obj .gltf .glb .iqm .vox .m3d

void draw(){
    cam.orbit(1);                                     // circle around the target by 1 degree
    graphics3d.setCamera(cam);
    graphics3d.drawGrid(20, 1);
    graphics3d.drawCube(vec3(0, 0.5, 0), vec3(1, 1, 1), color.RED);
    ship.draw(vec3(2, 0, 0));                         // also draw(pos, scale, tint), drawRotated(pos, axis, degrees, scale, tint)
    vec2 p = graphics3d.toScreen(vec3(0, 2, 0));
    graphics2d.drawText("label", p.x, p.y, 20, color.WHITE);   // 2D drawing goes on top of the 3D
}
```

`Camera` is a struct with `position`, `target`, `up`, `fov` (45 to start with) and `orbit(degrees)`. There's no lighting yet: shapes are drawn in flat colors.

**Scenes** (`<scenes>`). Each part of a game is a class inheriting from `Scene` that fills in any of `enter()`, `update(float dt)`, `draw()` and `exit()`. `scenes.change(Level())` switches at the start of the next `scenes.update(dt)`. The game calls `scenes.update(dt)` and `scenes.draw()` every frame.

**UI** (`<ui>`). UI is drawn every frame like everything else: `if (ui.button(rect(380, 200, 200, 50), "Play")) { ... }` draws the button and is true on the frame it's clicked. `volume = ui.slider(rect(...), volume, 0, 1);` and `showFps = ui.checkbox(rect(...), "Show FPS", showFps);` give back the new value.

**ECS** (`<ECS>`): entities made of components.

```cpp
class Health : Component {
    int hp = 3;
    void start() { print("{entity().name} is ready"); }   // runs once, before the first update
    void update(float dt) {
        if (hp <= 0) { entity().destroy(); }
    }
}

Entity slime = ECS.create("slime");
slime.add(Health());
Health h = slime.get("Health") as Health;    // find a component by its class name
// every frame: ECS.update(dt); ECS.draw();
```

An `Entity` has `name`, `active`, `components`, `add`, `get("Type")`, `has("Type")`, `remove("Type")` and `destroy()`.

**Timer** (`<time>`): `Timer cooldown = Timer(0.5);`, then `if (cooldown.isDone()) { ...; cooldown.restart(); }`. It also has `left()` and `elapsed()`.

**Files** (`<file>`), useful for save games: `file.write("save.txt", "{score}");` and later `int score = int(file.read("save.txt").trim());`. Check `file.exists(path)` first.

### Making a game: two kinds of game loop

**Your own loop.** `window.shouldClose()` shows the frame you drew and starts the next one:

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

**The engine loop.** `window.run()` calls `update(float dt)` and then `draw()` every frame. Both must be written in **main.frg** with exactly those names and parameters:

```cpp
int main(){
    window.open(800, 450, "My Game");
    window.run();
    return 0;
}

void update(float dt){ /* move things */ }
void draw(){ /* draw things */ }
```

The screen is cleared to the background color before every frame. **Escape** or the window's X closes the game. Multiply speeds by `dt` so movement is the same speed on every computer: `x += 200 * dt;` moves 200 pixels per second.

### Low-level toolkits (advanced, optional)

`<OpenGL>`, `<DirectX>` and `<Vulkan>` let advanced users draw with a real graphics API, writing their own shaders. **Only teach these if the learner asks.** Forge's engine is all anyone needs to make games.

The rules, which the learner chose on purpose:

- A toolkit is **not** part of the engine, and the engine can't be switched over to use one.
- A game that includes a toolkit **can't** include `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`. Doing so is a build error: "<OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)".
- A game can use only **one** toolkit.
- `<stdio>`, `<math>`, `<random>`, `<time>`, `<file>`, `<audio>`, `<physics>`, `<scenes>`, `<ECS>` and tasks still work with a toolkit.

Every toolkit has the same window, keyboard and mouse functions, with its own name in front:

- the window: `openWindow(w, h, title)`, `shouldClose()` (call it at the top of the loop: it shows the last frame), `closeWindow()`, `width()`, `height()`, `deltaTime()`, `time()`, `setTitle(t)`, `setVSync(on)`;
- input: `keyDown(Key.X)`, `keyPressed`, `keyReleased`, `mouseDown(MouseButton.LEFT)`, `mousePressed`, `mouseReleased`, `mousePosition()`, `mouseWheel()`;
- matrices, each a `list<float>` of 16 numbers: `identity()`, `multiply(a, b)`, `translate(vec3)`, `rotate(degrees, vec3 axis)`, `scale(vec3)`, `lookAt(eye, target, up)`, `perspective(fov, aspect, near, far)`, `orthographic(l, r, b, t, n, f)`.

There's no `window.run()` with a toolkit: the game always writes its own `while (!X.shouldClose())` loop. Graphics objects are `int` handles.

**`<OpenGL>`** (OpenGL 3.3) is the one to start with. Its functions are the real OpenGL ones without the `gl` prefix (`OpenGL.clearColor`, `OpenGL.TRIANGLES`), so OpenGL tutorials can be followed. One difference: `vertexAttribPointer(index, size, strideFloats, offsetFloats)` counts in floats, not bytes.

```cpp
#include <OpenGL>

string VERTEX = """
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
out vec3 vColor;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
""";

string FRAGMENT = """
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
""";

int main(){
    OpenGL.openWindow(800, 600, "Triangle");
    int program = OpenGL.buildProgram(VERTEX, FRAGMENT);
    list<float> corners = [
        -0.5, -0.5,   1, 0, 0,
         0.5, -0.5,   0, 1, 0,
         0.0,  0.5,   0, 0, 1
    ];
    int vao = OpenGL.genVertexArray();
    OpenGL.bindVertexArray(vao);
    int vbo = OpenGL.genBuffer();
    OpenGL.bindBuffer(OpenGL.ARRAY_BUFFER, vbo);
    OpenGL.bufferData(OpenGL.ARRAY_BUFFER, corners, OpenGL.STATIC_DRAW);
    OpenGL.vertexAttribPointer(0, 2, 5, 0);
    OpenGL.enableVertexAttribArray(0);
    OpenGL.vertexAttribPointer(1, 3, 5, 2);
    OpenGL.enableVertexAttribArray(1);
    while (!OpenGL.shouldClose()) {
        OpenGL.clearColor(0.1, 0.1, 0.15, 1);
        OpenGL.clear(OpenGL.COLOR_BUFFER_BIT);
        OpenGL.useProgram(program);
        OpenGL.bindVertexArray(vao);
        OpenGL.drawArrays(OpenGL.TRIANGLES, 0, 3);
    }
    OpenGL.closeWindow();
    return 0;
}
```

The other OpenGL functions:

| Group | Functions |
|---|---|
| state | `clear(mask)` `viewport` `enable` `disable` `blendFunc` `depthFunc` `cullFace` `lineWidth` `getError` |
| shaders | `createShader` `shaderSource` `compileShader` `shaderCompiled` `shaderLog` `deleteShader` `createProgram` `attachShader` `linkProgram` `programLinked` `programLog` `useProgram` `deleteProgram` |
| uniforms | `getUniformLocation` `getAttribLocation` `uniform1f`…`uniform4f` `uniform1i` `uniformMatrix4(location, list<float>)` |
| buffers | `genVertexArray` `bindVertexArray` `deleteVertexArray` `genBuffer` `bindBuffer` `deleteBuffer` `bufferData(target, list<float>, usage)` `bufferDataInts(target, list<int>, usage)` `vertexAttribPointer` `enableVertexAttribArray` |
| drawing | `drawArrays(mode, first, count)` `drawElements(mode, count)` |
| textures | `loadTexture(path)` (texture coordinate (0, 0) is the bottom-left) `genTexture` `bindTexture` `deleteTexture` `activeTexture` `texParameteri` `generateMipmap` |

**`<DirectX>`** (Direct3D 11) takes HLSL shader text. In order:

- shaders: `compileVertexShader(hlsl, entry)` and `compilePixelShader(hlsl, entry)`, then `setShaders(vs, ps)`;
- the layout of each corner: `createInputLayout(vs, ["POSITION", "COLOR"], [2, 3])`, then `setInputLayout`;
- buffers: `createVertexBuffer(list<float>)`, `setVertexBuffer(buffer, strideFloats)`, `createIndexBuffer(list<int>)` and `setIndexBuffer`;
- values for the shaders: `createConstantBuffer(floatCount)`, `updateConstantBuffer(buffer, list<float>)` and `setConstantBuffer(slot, buffer)`;
- each frame: `clear(r, g, b, a)`, then `draw(count)` or `drawIndexed(count)`.

Settings: `setDepthTest`, `setBlending`, `setCulling(DirectX.CULL_BACK)`, `setTopology(DirectX.TRIANGLES)`. Textures use `loadTexture` ((0, 0) is the top-left), `setTexture(slot, texture)` and `setTextureFilter(smooth)`. `adapterName()` gives the graphics card's name.

**`<Vulkan>`** takes compiled shaders. The learner writes GLSL files (`#version 450`) in the game folder, like `shape.vert` and `shape.frag`. `frg build` compiles each one to `shape.vert.spv`, and a shader mistake fails the build with the line number. In the game:

- loading: `loadShader("shape.vert.spv")`, then `createPipeline(vs, fs, [3, 3], pushConstantFloats)`. The list gives each vertex input's size in floats.
- settings, which apply to pipelines made after them: `setTopology`, `setDepthTest`, `setCulling`, `setBlending`.
- buffers: `createVertexBuffer(list<float>)` and `createIndexBuffer(list<int>)`.
- each frame: `if (Vulkan.beginFrame(r, g, b, a)) { bindPipeline(p); pushConstants(p, list<float>); bindVertexBuffer(b); bindIndexBuffer(b); draw(n) or drawIndexed(n); endFrame(); }`. `beginFrame` is false when the window is minimized.
- values for the shaders go through **push constants**: a `layout(push_constant) uniform` block, with room for at least 32 floats.
- `deviceName()` gives the graphics card's name.
- Vulkan has no textures yet.

The finished cube examples are in `examples/opengl`, `examples/directx` and `examples/vulkan`.

### Error messages

Mistakes are caught before the game runs. Each message shows the file, the line, an arrow at the problem, and usually a hint:

```
error: 'hp' needs an int, but this is a float
  --> main.frg:4:14
   |
 4 |     int hp = 7.5;
   |              ^^^
  hint: use int(...) to turn it into a whole number (the decimals get cut off)
```

**Warnings** look the same but are yellow and start with `warning:`. They never stop the build: the code works, but probably doesn't do what the learner meant. Warnings only show once there are no errors left (VS Code shows both, with yellow underlines for warnings). The warnings are:

| Warning | What it means | Fix |
|---|---|---|
| `this changes 'e', which is only a copy of the item in the list` | `for (Enemy e in enemies) { e.hp = 0; }` with a **struct**: the loop gets a copy, so the list doesn't change | loop with a number: `for (int i = 0; i < enemies.length; i++) { enemies[i].hp = 0; }` |
| `this divides two whole numbers, so the decimals are cut off` | `float ratio = hits / shots;` with two ints: `7 / 2` is `3`, and only then does it become `3.0` | `float(hits) / shots` gives `3.5` |
| `the variable 'x' is never used` | you made a variable and never used it | remove it, or use it |
| `this code can never run, because it comes after 'return'` | lines after `return`, `break` or `continue` in the same `{ }` | remove them, or move them above |
| `comparing decimal numbers with '==' often gives the wrong answer` | `0.1 + 0.2 == 0.3` is false, because decimals are a tiny bit off | check whether they're close: `math.abs(a - b) < 0.001` |

Literal-only divisions like `float f = 70 / 30;` don't warn, because they already give a decimal answer when the result goes into a float.

Problems that can only happen while running (a list position that doesn't exist, using something that's null, dividing by zero, `int("abc")`) stop the game with a message like:

```
Runtime error in main.frg:10: position 5 is outside the list (it has 3 items, at positions 0 to 2)
```

---

## Part 4: Easy things to get wrong

**Differences from C++, Java, C# and Python:**

| You might expect | In Forge |
|---|---|
| `print(a, b)` | Takes one value: `print("{a} {b}")` |
| `"Score: " + score` | Error: use `"Score: {score}"` |
| `/* comment */` | ` ``` comment ``` ` |
| `if (x) return;` without braces | Braces are always required |
| `if (count)` / `if (obj)` | Must be true/false: `if (count > 0)`, `if (obj != null)` |
| `break` at the end of each `case` | Not allowed. Cases never fall through. |
| Calling a function from another file by its plain name | Must use the file prefix: `player.jump()` |
| `#include "player.frg"` | Files in the same folder are automatic. Folders use `#include <"name">` |
| Declaring functions before using them | Not needed and not allowed: just write the function |
| `new Player()` | `Player()` (no `new`) |
| `Player p;` for a class | Needs a value, or make it optional: `Player? p = null;` |
| `str[0]` | `str.charAt(0)` |
| `len(list)` / `list.size()` / `list.count` | `list.length` |
| `for i in range(10)` | `for (int i = 0; i < 10; i++)` |
| `var x = 5;` / `auto x = 5;` | Every variable states its type: `int x = 5;` |
| `x = 5` (without `;`) | Every statement ends with `;` |
| `float` holding `0.1 + 0.2` | It's a decimal number, so tiny rounding is normal, as in every language |

**Things Forge does not have yet.** Don't teach or use these. Say "not in Forge yet" and work around them:

- `char` (single letters are one-letter strings), fixed-size arrays (use `list`), tuples
- Generics you write yourself (`class Box<T>`), function values, lambdas, callbacks
- Two functions or methods with the same name (only constructors can share a name), operator overloading for your own types
- Interfaces, abstract classes, `protected`, `static` fields or methods in your own types
- `do ... while`, labeled breaks, exceptions (`try` / `catch`), `goto`
- Networking, gamepads/controllers, shaders in the engine, 3D lighting, text input boxes in `<ui>`
- `<network>` is planned but **doesn't exist yet**.

---

## Part 5: The example platformer

This game is in `examples/platformer/`. It's the learner's first goal: they should eventually understand every line. Arrows or A/D move, Space/W/Up jumps, you collect 12 coins, and falling into a pit sends you back to the start. The level is 2400 pixels wide, so a `Camera2D` follows the player smoothly (`followPlayer` in main.frg), and the text is drawn after `resetCamera()` so it stays on the screen.

### main.frg

````cpp
#include <window>
#include <graphics2d>
#include <physics>
#include <math>

```
  A tiny platformer: move with the arrow keys (or A / D), jump with Space (or W / Up).
  Collect all the coins. Don't fall in the pits!

  This game uses the engine loop: window.run() calls update(dt) and draw() every frame.
  The level is wider than the window, so a camera follows the player.
```

// The camera looks at `target` (a point in the level) and shows it at `offset` (the middle of the window).
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));

int main(){
    window.open(960, 540, "Forge Platformer");
    window.setBackground(color(24, 26, 40));
    level.build();
    window.run();
    return 0;
}

void update(float dt){
    player.handleInput();
    physics.step(dt);
    player.checkFall();
    level.collectCoins(player.hero.bounds());
    followPlayer(dt);
}

// The camera glides toward the player, but stops at the ends of the level so it never shows past them.
void followPlayer(float dt){
    float half = window.width() / 2.0;
    float x = math.clamp(player.hero.center().x, half, level.WIDTH - half);
    camera.target = camera.target.lerp(vec2(x, window.height() / 2.0), 5 * dt);
}

void draw(){
    graphics2d.setCamera(camera);    // the level and the player are drawn where the camera looks...
    level.draw();
    player.draw();
    graphics2d.resetCamera();        // ...and the text stays in the same place on the screen

    graphics2d.drawText("Arrows / A D to move, Space to jump", 20, 16, 20, color.LIGHTGRAY);
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);

    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2.0, 200, 48, color.WHITE);
    }
}
````

### player.frg

```cpp
#include <window>
#include <graphics2d>
#include <input>
#include <physics>

const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;
const vec2 START = vec2(80, 380);

// A Player is a physics Body (it inherits position, size, velocity, onGround...)
// with a few extra things of its own.
class Player : Body {
    color tint = color.ORANGE;
    int deaths = 0;

    void respawn() {
        position = START;
        velocity = vec2.ZERO;
        deaths++;
        flash();
    }

    // A task: it runs alongside the game, and wait() pauses just this task, not the game.
    task flash() {
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
}

Player hero = makeHero();

Player makeHero(){
    Player p = Player(START.x, START.y, 36, 36);
    physics.add(p);
    return p;
}

void handleInput(){
    float direction = 0;
    if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) {
        direction -= 1;
    }
    if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) {
        direction += 1;
    }
    hero.velocity.x = direction * MOVE_SPEED;

    bool jumpPressed = input.keyPressed(Key.SPACE) or input.keyPressed(Key.W) or input.keyPressed(Key.UP);
    if (jumpPressed and hero.onGround) {
        hero.velocity.y = -JUMP_SPEED;
    }

    // Letting go of jump early makes a shorter jump.
    bool jumpReleased = input.keyReleased(Key.SPACE) or input.keyReleased(Key.W) or input.keyReleased(Key.UP);
    if (jumpReleased and hero.velocity.y < 0) {
        hero.velocity.y *= 0.5;
    }
}

void checkFall(){
    if (hero.position.y > window.height() + 100) {
        hero.respawn();
    }
}

void draw(){
    graphics2d.drawRect(hero.bounds(), hero.tint);
    // Two eyes that look the way the player is moving.
    float look = hero.velocity.x > 0 ? 4 : hero.velocity.x < 0 ? -4 : 0;
    float eyeY = hero.position.y + 12;
    graphics2d.drawRect(hero.position.x + 8 + look, eyeY, 6, 8, color.BLACK);
    graphics2d.drawRect(hero.position.x + 22 + look, eyeY, 6, 8, color.BLACK);
}
```

### level.frg

```cpp
#include <graphics2d>
#include <physics>

const float WIDTH = 2400;    // the level is wider than the 960-pixel window

list<Body> platforms = [];
list<vec2> coins = [];
int totalCoins = 0;
int collected = 0;

void build(){
    // The ground, with three pits.
    addPlatform(0, 500, 400, 40);
    addPlatform(540, 500, 560, 40);
    addPlatform(1250, 500, 500, 40);
    addPlatform(1880, 500, 520, 40);

    // Platforms to jump between.
    addPlatform(140, 400, 160, 20);
    addPlatform(380, 320, 160, 20);
    addPlatform(620, 240, 160, 20);
    addPlatform(410, 150, 100, 20);
    addPlatform(900, 380, 140, 20);
    addPlatform(1120, 330, 120, 20);
    addPlatform(1400, 380, 160, 20);
    addPlatform(1600, 280, 140, 20);
    addPlatform(1780, 200, 120, 20);
    addPlatform(2050, 380, 160, 20);
    addPlatform(2250, 280, 120, 20);

    // Walls at the two ends of the level.
    addPlatform(-20, 0, 20, 540);
    addPlatform(WIDTH, 0, 20, 540);

    coins = [vec2(220, 370), vec2(460, 290), vec2(700, 210), vec2(460, 120), vec2(970, 350), vec2(1180, 300),
             vec2(1480, 350), vec2(1670, 250), vec2(1840, 170), vec2(2130, 350), vec2(2310, 250), vec2(2350, 470)];
    totalCoins = coins.length;
}

void addPlatform(float x, float y, float width, float height){
    Body b = Body(x, y, width, height);
    b.isStatic = true;
    physics.add(b);
    platforms.add(b);
}

void collectCoins(rect playerBox){
    // Go backwards so removing a coin doesn't skip the next one.
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
        }
    }
}

void draw(){
    for (Body p in platforms) {
        graphics2d.drawRect(p.bounds(), color(70, 86, 120));
        graphics2d.drawRect(p.position.x, p.position.y, p.size.x, 5, color(110, 200, 130));
    }
    for (vec2 c in coins) {
        graphics2d.drawCircle(c, 10, color.GOLD);
        graphics2d.drawCircleOutline(c.x, c.y, 10, color.ORANGE);
    }
}
```

**Good exercises once the learner understands it:** show the death count on screen, add a sixth coin, make a moving platform (change a static body's `position` in `update`), add a timer with `window.time()`, add a second level, or add enemies that send you back to the start when `hero.overlaps(enemy)`.


---

## Part 6: Practice projects

Use these whenever the learner finishes a stage and wants more practice, or asks "what should I make?". Each project fits a stage from the plan of lessons in Part 1. They're more projects to choose from, on top of the one project in that table.

How to run a project with the learner:

1. Pick one that fits their stage and sounds fun to them. Offer two or three and let them choose.
2. Give them the goal and the **first step only**. Let them write it, run it and show you.
3. Then the next step. Give a hint only when they're stuck, starting with the smallest one ("which loop repeats until something happens?") before naming functions. The hints listed here are for you; they're all correct Forge.
4. **Don't write the project for them.** Fix mistakes by pointing at the line and asking what it does.
5. When it works, celebrate it, then offer a "make it yours" idea.

The text below is written to the learner, so you can share parts of it with them directly.

### How to do a project

- **Build it in small steps.** Each project is split into steps. Get each step running before starting the next. A program that works but does less is better than a big one that doesn't run.
- **Run it often.** After every few lines, `frg run` it. If something breaks, it's in the lines you just wrote.
- **Read the error.** Forge's messages say what's wrong and usually how to fix it (look at the `hint:` line).
- **Use `print` to look inside.** If a number isn't what you expect, `print("x = {x}")` shows it. The console window shows `print` output even in window games.
- **Stuck for more than 15 minutes?** Read the hints for that project. They name the functions you'll need and show the trickiest piece.
- **Finished?** Try a "make it yours" idea, or make up your own. Changing a working program is one of the best ways to learn.

The **Stage** says which lesson stage the project fits, from the plan of lessons in Part 1. The projects get harder from top to bottom.

### Level 1: Console programs

These only use the console: no window. They need `#include <stdio>` for `print` and `input`, plus whatever the hints mention.

#### 1. Number guessing game

**Stage 4.** The computer thinks of a number from 1 to 100. The player guesses, and is told "too high" or "too low" until they get it. At the end, show how many guesses it took.

**Practices:** `random.range`, `input`, turning text into a number, `while`, `if` / `else`, counting.

Steps:

1. Pick the secret number and print it (just while testing, so you can check the game works).
2. Ask for one guess and say whether it's too high, too low or right.
3. Put the asking in a loop that stops when the guess is right.
4. Count the guesses and print the count at the end. Remove the line that printed the secret.

Hints:

- `#include <random>`, then `int secret = random.range(1, 100);`. Both 1 and 100 can come up.
- `input` gives back text. Turn it into a number with `int(...)`: `int guess = int(input("Your guess: "));`. Typing something that isn't a number stops the program with an error. To ask again instead, check the text first with `isInt()`: `string typed = input("Your guess: ");`, then `if (typed.isInt()) { int guess = int(typed); ... } else { print("Please type a number"); }`.
- A `bool found = false;` and `while (!found) { ... }` is one way to loop until the guess is right.

Make it yours: allow only 7 guesses. Ask "Play again? (y/n)" at the end. Let the player choose easy (1-50) or hard (1-500).

#### 2. Math quiz

**Stage 5.** Ten questions like `7 + 12 = ?` with random numbers. Count the right answers, then show the score as a percentage.

**Practices:** `for` loops, functions that `return` a value, `switch`, and how division works.

Steps:

1. Write `bool askQuestion()`. It makes two random numbers, asks for their sum, and gives back `true` if the answer was right.
2. Call it 10 times in a `for` loop, counting the `true`s.
3. Print the score, and the percentage.
4. Pick `+`, `-` or `*` at random for each question, with `switch`.

Hints:

- The percentage is the classic division trap. `score / 10 * 100` uses whole numbers, so 7 right gives `0`. Use `float(score) / 10 * 100`. (Forge warns about the whole-number version when it goes into a `float`.) Show it with one decimal: `print("{percent:.1}%");`.
- `random.range(1, 3)` picks which operation, and `switch` on that number chooses what to ask.

Make it yours: time the whole quiz with `<time>` (`time.now()` at the start and end). Make the numbers bigger as the score goes up.

#### 3. Shopping list

**Stage 6.** A menu that repeats until the player quits:

```
1) add an item   2) remove an item   3) show the list   4) quit
```

**Practices:** `list<string>` and its methods, a menu loop, `switch` on text.

Steps:

1. Make an empty `list<string> items = [];` and a loop that shows the menu and reads a choice.
2. Make "add" and "show" work. Show each item with its number: `1. milk`.
3. Make "remove" work. Say so if the item isn't in the list.
4. Make "quit" end the loop.

Hints:

- `switch (choice) { case "1": ... case "4": ... }` works on text.
- `items.remove(name)` gives back `false` if the item wasn't there.
- `for (int i = 0; i < items.length; i++)` gives you the position, so you can print `{i + 1}. {items[i]}`.

Make it yours: use a `map<string, int>` to keep how many of each item. Save the list with `<file>` (`file.writeLines("list.txt", items)`) and load it at the start (`file.readLines`, after checking `file.exists`).

#### 4. High-score table

**Stage 6.** Players type a name and a score. Keep the top 5, best first, and save them so they're still there next time.

**Practices:** a `struct`, lists of structs, putting things in order yourself, `<file>`.

Steps:

1. Make `struct Entry { string name; int score; }` and a `list<Entry>`.
2. Add entries and print them.
3. Keep the list in order: put each new entry **before** the first entry with a lower score (`insert`). If there's none, add it at the end.
4. After adding, if there are more than 5, remove the last one (`pop()`).
5. Save each entry as a line like `Ana,120`, and load them at the start.

Hints:

- `sort()` only sorts lists of numbers or text, which is why step 3 keeps the list in order by hand.
- Loading: `file.readLines("scores.txt")` gives a list of lines. For each line, `line.split(",")` gives `["Ana", "120"]`, and `int(parts[1])` is the score.

Make it yours: show a medal (`*`) next to the top score. Don't allow empty names (`name.trim().isEmpty()`).

#### 5. Text adventure

**Stage 9.** A tiny world of 4 or 5 rooms. The player types `north`, `south`, `look` or `take key`, and has to find a key to open the last door.

**Practices:** `enum`, `switch`, functions, `bool` flags, planning a program before writing it.

Steps:

1. Draw your rooms on paper first, with arrows between them.
2. Make `enum Room { Hall, Kitchen, Garden, Cellar }` and a variable for the room the player is in.
3. Write `void describe(Room room)`, which prints a description of each room using `switch`.
4. Read commands in a loop. Moving changes the room, if there's a way to go in that direction.
5. Add the key: `bool hasKey = false;`. One room has it, and the last door only opens with it.

Hints:

- Write one function per room, like `Room kitchen(string command)`, that gives back the room the player ends up in. Then the main loop stays short.
- `command.trim().lower()` makes `"  North "` into `"north"`.

Make it yours: an inventory `list<string>`, and an `inventory` command. A room that's dark until you find a lamp. Split it into files: `rooms.frg` with the room functions, called as `rooms.kitchen(command)`. That's stage 10.

#### 6. Tic-tac-toe

**Stage 6.** Two players take turns on a 3×3 board in the console, until someone wins or the board is full.

**Practices:** a list as a game board, nested lists, functions that check things.

Steps:

1. Store the board as `list<string> board = [" ", " ", " ", " ", " ", " ", " ", " ", " "];`, positions 0 to 8.
2. Write `void showBoard()`, which prints it as three rows.
3. Take turns: ask for a position (1-9), and put `X` or `O` there if it's free.
4. Write `bool hasWon(string mark)`, which checks every winning line.
5. Stop on a win or a full board.

Hints:

- All 8 winning lines, as positions:

  ```cpp
  list<list<int>> LINES = [[0, 1, 2], [3, 4, 5], [6, 7, 8], [0, 3, 6], [1, 4, 7], [2, 5, 8], [0, 4, 8], [2, 4, 6]];
  ```

  Then `for (list<int> line in LINES)` checks whether `board[line[0]]`, `board[line[1]]` and `board[line[2]]` are all `mark`.
- The board is full when `!board.contains(" ")`.

Make it yours: a computer player. First let it pick a random free square. Then make it smarter: win if it can, otherwise block the other player.

#### 7. Turn-based battle

**Stage 8.** A hero fights a line of monsters, one at a time. Each turn the player picks attack, special or heal, and then the monster hits back.

**Practices:** classes, inheritance, replacing methods, lists of objects, `random`.

Steps:

1. Make `class Fighter` with `name`, `hp`, `maxHp`, `strength`, and `void attack(Fighter target)`.
2. Make a hero and one monster. Loop turns until one has 0 `hp`.
3. Make kinds of monster: `class Goblin : Fighter`, `class Dragon : Fighter`. Each replaces `attack` with its own (the dragon sometimes breathes fire for double damage).
4. Put several monsters in a `list<Fighter>`, and fight them in order.

Hints:

- Random damage: `int damage = random.range(strength - 2, strength + 2);`.
- Keep `hp` from going below 0 with `math.max(0, hp - damage)`.
- A `Goblin` can go into a `list<Fighter>`, and calling `attack` on it runs the goblin's own version.

Make it yours: an `enum Action { Attack, Special, Heal }`. Items the hero might be holding: `Weapon? weapon = null;`. Gold and a shop between fights.

### Level 2: Window games

These open a window: `#include <window>` and `<graphics2d>`, and `<input>` for the keyboard. Remember that **y goes down**: `y = 0` is the top of the window. Use `window.run()` with `update(float dt)` and `draw()`, and multiply speeds by `dt`.

#### 8. Bouncing balls

**Stage 11.** A ball moves around the window and bounces off the edges. Then lots of balls, each with its own color and speed.

**Practices:** `vec2`, moving with `dt`, edges, a list of structs.

Steps:

1. One ball: a `vec2 position` and a `vec2 velocity`, moved every frame and drawn with `drawCircle`.
2. Bounce: when it goes past an edge, flip that part of the velocity.
3. Make `struct Ball { vec2 position; vec2 velocity; color tint; }`, and a list of 20 random balls.
4. Space adds a ball at the mouse.

Hints, the core of one ball:

```cpp
vec2 position = vec2(400, 225);
vec2 velocity = vec2(200, 150);
float RADIUS = 20;

void update(float dt){
    position += velocity * dt;
    if (position.x < RADIUS or position.x > window.width() - RADIUS) {
        velocity.x = -velocity.x;
    }
    if (position.y < RADIUS or position.y > window.height() - RADIUS) {
        velocity.y = -velocity.y;
    }
}

void draw(){
    graphics2d.drawCircle(position, RADIUS, color.ORANGE);
}
```

- A ball in a list is a struct, which is a copy. To change it, change `balls[i].position`, not a copy taken out of the list with `for (Ball b in balls)`.
- A random color: `random.color()`. For brighter colors, pick each part yourself: `color(random.range(50, 255), random.range(50, 255), random.range(50, 255))`.

Make it yours: balls bounce off each other (`a.position.distance(b.position) < 2 * RADIUS`). Gravity pulls them down.

#### 9. Pong

**Stage 12.** Two paddles, one ball. W/S moves the left paddle, Up/Down the right one. A player scores when the ball gets past the other paddle.

**Practices:** `rect`, `overlaps`, keyboard input, keeping score, resetting.

Steps:

1. Draw two paddles (`rect`) and move them with the keys. Keep them inside the window.
2. Add the ball from project 8, bouncing off the top and bottom only.
3. When the ball's rect overlaps a paddle, flip its x velocity.
4. When it leaves on the left or right, score a point and put it back in the middle.
5. Show the score at the top with `drawText`.

Hints:

- The ball as a rect: `rect(position.x - 10, position.y - 10, 20, 20)`, then `ballRect.overlaps(leftPaddle)`.
- After flipping, also move the ball out of the paddle, or it can stick inside and flip every frame.
- Center text: `float x = (window.width() - graphics2d.textWidth(text, 40)) / 2.0;` (the `.0` keeps the half pixel; with `/ 2` Forge warns that the decimals are cut off).

Make it yours: the ball speeds up a little on every hit. A computer player that follows the ball (not too fast, or it can't lose). First to 5 wins.

#### 10. Catch the falling stars

**Stage 13.** Stars fall from the top. The player moves a basket left and right to catch them. A missed star costs a life; three misses and it's game over.

**Practices:** spawning things, removing things from a list, a game-over state.

Steps:

1. The basket: a rect at the bottom, moved with Left/Right.
2. One falling star that starts at a random x at the top.
3. A `list<vec2> stars`, with a new one added every second.
4. Caught stars and missed stars are removed from the list, and change the score or lives.
5. Game over: stop the game and show "Game over - press Enter". Enter starts again.

Hints:

- A timer for spawning: add `dt` to a counter every frame, and when it passes 1 second, spawn a star and take 1 off.

  ```cpp
  float spawnTimer = 0;
  // in update:
  spawnTimer += dt;
  if (spawnTimer >= 1) {
      spawnTimer -= 1;
      stars.add(vec2(random.range(20, window.width() - 20), -20));
  }
  ```

- **Removing while looping:** go through the list **backwards**, so removing an item doesn't skip the next one:

  ```cpp
  for (int i = stars.length - 1; i >= 0; i--) {
      if (stars[i].y > window.height()) {
          stars.removeAt(i);
          lives--;
      }
  }
  ```

- `bool gameOver = false;`. In `update`, when it's true, only check for Enter. In `draw`, show the message.

Make it yours: stars fall faster as the score grows. Rare golden stars worth 5. A sound when one is caught (`<audio>`).

#### 11. Snake

**Stage 13.** The classic: the snake moves on a grid, eats food, grows, and dies if it hits a wall or itself.

**Practices:** grid positions, moving on a fixed beat instead of every frame, using a list as a queue.

Steps:

1. Pick a cell size (20 pixels) and keep the snake as a `list<vec2>` of **grid** positions (like `(5, 7)`), with the head first.
2. Draw each part as a square: `drawRect(part.x * 20, part.y * 20, 20, 20, color.GREEN)`.
3. Move a step every 0.15 seconds (the same timer trick as project 10).
4. The arrow keys change the direction, but not straight back into itself.
5. Food at a random cell. Eating it grows the snake; hitting a wall or itself ends the game.

Hints:

- One step: make the new head, put it at the front, and take the tail off the end.

  ```cpp
  vec2 head = snake[0] + direction;   // direction is vec2(1, 0), vec2(0, -1)...
  snake.insert(0, head);
  snake.pop();                        // skip this line on a step where it ate, and it grows
  ```

- Hit itself: check whether any **other** part equals the new head (`==` compares vectors).

Make it yours: walls in the middle of the field. Speed up every 5 food. A best score saved with `<file>`.

#### 12. Breakout

**Stage 13.** A paddle at the bottom, a ball, and rows of bricks at the top. The ball breaks a brick when it hits it.

**Practices:** building a grid of things with nested loops, collisions from different sides.

Steps:

1. The paddle (mouse or arrow keys) and a ball that bounces off the walls and the paddle. Losing the ball off the bottom costs a life.
2. Bricks: a `list<rect>`, made with two `for` loops, rows and columns.
3. When the ball overlaps a brick, remove the brick and flip the ball's y velocity.
4. Win when the list is empty.

Hints:

- Making the bricks:

  ```cpp
  for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 10; col++) {
          bricks.add(rect(col * 80 + 2, row * 30 + 40, 76, 26));
      }
  }
  ```

- Only break **one** brick per frame (`break` out of the loop after a hit), or the ball can flip twice and carry on through.

Make it yours: a different color and score for each row. Where the ball hits the paddle changes its angle. Power-ups that fall from broken bricks.

#### 13. Clicker game

**Stage 17.** Click a big cookie to earn cookies. Spend them on upgrades: more per click, or helpers that earn cookies every second. Progress is saved.

**Practices:** the mouse, `<ui>` buttons, earning over time with `dt`, saving and loading, scenes.

Steps:

1. A big circle in the middle. Clicking inside it adds 1 cookie. Show the total.
2. A `ui.button` "Better click (cost 10)". Buying it makes clicks worth more, and raises its cost.
3. A "Helper" upgrade: each helper earns 1 cookie per second (`cookies += helpers * dt`).
4. Save the numbers to a file when the game closes, and load them when it starts: `file.writeLines("save.txt", ["{cookies}", "{helpers}"])`.
5. A title scene with "Play" and "Reset save" buttons (`<scenes>`).

Hints:

- Clicked inside the circle: `input.mousePressed(MouseButton.LEFT) and input.mousePosition().distance(center) < radius`.
- Helpers earn fractions of a cookie, so keep `float cookies`, and show `int(cookies)` (or `"{cookies:.1}"` for one decimal).
- Save several numbers as several lines with `file.writeLines`, and read them back in the same order.

Make it yours: the cookie shrinks a little when clicked, then grows back. Achievements at 100, 1,000 and 10,000 cookies.

#### 14. Dodge

**Stage 15.** Blocks fly in from the sides. Survive as long as possible. The difficulty is run by **tasks**, which makes waves, pauses and flashes easy.

**Practices:** `task` functions and `wait`, timing events without freezing the game.

Steps:

1. A player square that moves in all four directions, and a survival timer at the top.
2. A `task spawnWaves()` that loops forever: spawn 5 blocks, `wait(3)`, spawn more, and every few waves make them faster.
3. Getting hit takes a life, and starts a `task flash()` that makes the player blink for a second (and can't be hit while blinking).
4. Game over when the lives run out.

Hints:

- A blink with a task:

  ```cpp
  bool visible = true;

  task flash(){
      for (int i = 0; i < 6; i++) {
          visible = !visible;
          wait(0.15);
      }
      visible = true;
  }
  ```

  Start it with `flash();`. The game keeps running while it waits.

Make it yours: "Wave 3!" text that appears for 2 seconds (another task). A shield pickup.

#### 15. Platformer remix

**Stage 16.** Take the example platformer (Part 5) and make it your own game.

**Practices:** reading and changing a bigger program, `<physics>`, `<assets>`, `<audio>`, `<scenes>`.

Ideas, roughly from easiest to hardest:

1. Change the colors, the jump height and the speed until it feels right to you.
2. Add more platforms and coins, and a door that only opens when all coins are collected.
3. Give the player a sprite and a jump sound.
4. A moving platform: a static `Body` whose position you change every frame, going back and forth.
5. An enemy that walks left and right on a platform. Touching it sends you back to the start.
6. A second level, using `<scenes>`, and a title screen.

### Level 3: Bigger projects

#### 16. Top-down shooter with components

**Stage 19.** A ship in the middle of the screen turns and shoots at asteroids coming from every side. Built from `<ECS>` components.

**Practices:** designing with components, `is` / `as`, angles with `math.sin` / `math.cos`.

Steps:

1. Components: `Movement` (position and velocity) and `Health`, plus one per kind of thing that says how it looks and acts: `Ship`, `Bullet`, `Asteroid`.
2. The ship turns with Left/Right and shoots with Space. A bullet is an entity with a `Movement` and a `Lifetime` (it destroys itself after 1 second).
3. Asteroids are entities made at the edges, moving toward the middle.
4. Collisions: compare the distance between bullets and asteroids. Big asteroids split into two small ones.

Hints:

- A direction from an angle: `vec2(math.cos(angle), math.sin(angle))`, with the angle in radians (`degrees * math.PI / 180`).
- `ECS.withComponent("Asteroid")` gives every asteroid entity, ready to check against the bullets.

#### 17. Solar system in 3D

**Stage 20.** A sun in the middle, planets going round it at different speeds, and a moon going round one planet. The camera can turn around the scene.

**Practices:** `<graphics3d>`, circles with `math.sin` and `math.cos`, `Camera`.

Steps:

1. A camera looking at the middle, a `drawGrid`, and a yellow sphere for the sun.
2. One planet: its position is `vec3(math.cos(t) * distance, 0, math.sin(t) * distance)`, where `t` grows every frame.
3. Several planets, each a struct with its own distance, speed, size and color.
4. A moon: the same circle, but around its planet's position instead of the middle.
5. Left/Right turns the camera: keep a `Camera cam`, call `cam.orbit(degrees)`, then `graphics3d.setCamera(cam)`.

Make it yours: lines showing the orbits (many short `drawLine`s around a circle). Up/Down zooms. Labels with each planet's name, placed with `graphics3d.toScreen`.

#### 18. Your own game

**Any stage.** Pick a simple game you like and make a small version of it. Good first choices: Flappy Bird, Space Invaders, Frogger, Asteroids, Tetris (hard!), a maze game.

Before writing code:

1. Write down, in one sentence, what the player does.
2. List the **smallest** version that's still fun to play for a minute. That's version 1.
3. Write the steps for version 1, like the projects above.

Then build version 1, play it, and only then add more.

---

*Forge v0.6. If the learner reports that something in this file doesn't work, believe the compiler over this file and tell them to report it to whoever maintains Forge.*
