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

Go in this order. Each stage ends with a small project. Move at the learner's pace and skip ahead if a stage is easy for them.

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

In **VS Code** (with the Forge extension): errors are underlined when you save, the **▶ button** at the top right (or **F5**) runs the game, typing `window.` or `color.` shows autocomplete, and snippets like `game`, `class` and `foreach` insert templates.

Forge is **compiled**. The whole program is checked before it runs, so most mistakes are caught before the game starts.

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
string full = "Game" + " Over";         // + joins two pieces of text
```

- Escapes: `\n` new line, `\t` tab, `\"` quote, `\\` backslash, `{{` or `\{` for a literal `{`, and `}}` or `\}` for a literal `}`.
- `"Score: " + 5` is an **error**. Write `"Score: {5}"` instead.
- Text has `.length` (no parentheses) and these methods: `contains(t)`, `indexOf(t)` (-1 if not found), `startsWith(t)`, `endsWith(t)`, `upper()`, `lower()`, `trim()`, `replace(old, new)`, `split(separator)` (gives a list), `substring(start, count)`, `charAt(i)` (gives a one-letter string), `repeat(n)`, `isEmpty()`.
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

- **List methods:** `.length`, `add(x)`, `insert(i, x)`, `removeAt(i)` (gives back the removed item), `remove(x)` (removes the first match and gives true/false), `contains(x)`, `indexOf(x)`, `first()`, `last()`, `pop()` (removes and gives back the last item), `clear()`, `isEmpty()`, `sort()` (numbers or text only), `reverse()`, `copy()`.
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
| `color` | `r g b a` (0–255) | `color(r, g, b)` or `color(r, g, b, a)`, `fade(alpha)`, `lerp(c, t)`. Named colors: `color.WHITE BLACK BLANK LIGHTGRAY GRAY DARKGRAY RED MAROON ORANGE YELLOW GOLD GREEN LIME DARKGREEN SKYBLUE BLUE DARKBLUE PURPLE VIOLET PINK MAGENTA BROWN BEIGE` |
| `rect` | `x y width height` | `overlaps(other)` `contains(point)` `center()` `position()` `size()` |

Vectors work with math: `pos += velocity * dt;`. Vectors add and subtract with vectors, and multiply or divide by numbers.

### Libraries

Library functions are reached through the library name (`window.open(...)`). Library *types* like `Key` and `Body` are used by plain name. Each file includes what it uses.

| Include | Contents |
|---|---|
| `<stdio>` | `print`, `write`, `input` |
| `<math>` | `math.sqrt pow sin cos tan asin acos atan atan2 exp log floor ceil round abs sign min max clamp lerp`, and `math.PI TAU E`. Angles are in radians. |
| `<random>` | `random.range(1, 6)` (whole numbers, both ends included), `random.range(0.5, 2.0)` (decimals), `random.value()` (0 to 1), `random.chance(0.25)`, `random.seed(n)` |
| `<window>` | `window.open(width, height, "title")`, `shouldClose()`, `run()`, `close()`, `deltaTime()` (seconds since the last frame), `time()`, `width()`, `height()`, `fps()`, `setFPS(n)`, `setTitle(t)`, `setBackground(color)`, `toggleFullscreen()`, `screenshot("file.png")` |
| `<input>` | `input.keyDown(Key.SPACE)` (held), `input.keyPressed(...)` (only the frame it was pressed), `input.keyReleased(...)`, `input.mouseDown(MouseButton.LEFT)`, `mousePressed`, `mouseReleased`, `mousePosition()` (a vec2), `mouseX()`, `mouseY()`, `mouseWheel()` |
| `<graphics2d>` | `graphics2d.drawRect(x, y, w, h, color)` or `drawRect(rect, color)`, `drawRectOutline(x, y, w, h, color, thickness)`, `drawCircle(x, y, radius, color)` or `drawCircle(vec2, radius, color)`, `drawCircleOutline(x, y, radius, color)`, `drawLine(from, to, color, thickness)`, `drawTriangle(a, b, c, color)`, `drawText(text, x, y, size, color)`, `textWidth(text, size)`, `drawFPS(x, y)`, `clear(color)` |
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

The details of the newer libraries are in "More libraries" after the Physics section.

**Keys:** `Key.A` through `Key.Z`, `Key.NUM0` through `Key.NUM9`, `SPACE ENTER ESCAPE BACKSPACE TAB LEFT RIGHT UP DOWN LEFT_SHIFT RIGHT_SHIFT LEFT_CONTROL RIGHT_CONTROL LEFT_ALT RIGHT_ALT`, and `F1` through `F12`. **Mouse buttons:** `MouseButton.LEFT RIGHT MIDDLE`.

**Screen coordinates:** (0, 0) is the **top-left** corner. x goes right, and **y goes down**, so jumping means a *negative* y velocity.

### Physics

A `Body` is a box: `Body(x, y, width, height)`.
- Fields you set: `position`, `size`, `velocity` (all vec2), `isStatic` (true for floors, walls and platforms that never move), and `gravityScale` (1 by default).
- Fields Forge sets during each `physics.step(dt)`: `onGround`, `onCeiling` and `onWall`.
- Methods: `bounds()` (a rect), `center()`, `overlaps(otherBody)`.
- Gravity starts at `vec2(0, 980)`, in pixels per second per second, pointing down.
- Moving bodies stop when they hit static bodies, and push each other apart (so crates stack). A body with `solid = false` passes through everything, which is useful for coins and trigger zones; check those with `a.overlaps(b)`.
- Your own classes can inherit from `Body`: `class Player : Body { ... }`. With no constructor of its own, it uses `Body(x, y, width, height)`.

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
- Networking, gamepads/controllers, shaders, 3D lighting, cameras that scroll or zoom in 2D, text input boxes in `<ui>`
- Libraries with these names are planned but **don't exist yet**: `<OpenGL>` `<Vulkan>` `<DirectX>`. They're planned as separate low-level toolkits for advanced users, **not** as part of the built-in engine.

---

## Part 5: The example platformer

This game is in `examples/platformer/`. It's the learner's first goal: they should eventually understand every line. Arrows or A/D move, Space/W/Up jumps, you collect 5 coins, and falling into the pit sends you back to the start.

### main.frg

````cpp
#include <window>
#include <graphics2d>
#include <physics>

```
  A tiny platformer: move with the arrow keys (or A / D), jump with Space (or W / Up).
  Collect all the coins. Don't fall in the pit!

  This game uses the engine loop: window.run() calls update(dt) and draw() every frame.
```

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
}

void draw(){
    level.draw();
    player.draw();

    graphics2d.drawText("Arrows / A D to move, Space to jump", 20, 16, 20, color.LIGHTGRAY);
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);

    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2, 200, 48, color.WHITE);
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

list<Body> platforms = [];
list<vec2> coins = [];
int totalCoins = 0;
int collected = 0;

void build(){
    // The ground has a pit in the middle.
    addPlatform(0, 500, 400, 40);
    addPlatform(540, 500, 420, 40);

    // Platforms to jump between.
    addPlatform(140, 400, 160, 20);
    addPlatform(380, 320, 160, 20);
    addPlatform(620, 240, 160, 20);
    addPlatform(410, 150, 100, 20);

    // Walls at the edges of the screen.
    addPlatform(-20, 0, 20, 540);
    addPlatform(960, 0, 20, 540);

    coins = [vec2(220, 370), vec2(460, 290), vec2(700, 210), vec2(460, 120), vec2(880, 470)];
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

*Forge v0.5. If the learner reports that something in this file doesn't work, believe the compiler over this file and tell them to report it to whoever maintains Forge.*
