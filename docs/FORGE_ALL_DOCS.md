# Forge 0.7.0: the complete documentation

This one file holds everything written about the Forge programming language, so it can be uploaded in one go:

- **Part A** is the teaching brief, written for an AI that teaches Forge to a beginner. It explains how to teach, what Forge has and doesn't have, the example platformer, and practice projects.
- **Parts B to F** are the full documentation, page by page: getting started, every part of the language, every library, the guides, and how Forge works inside.
- **Part G** is the one-page cheat sheet.

Where the parts disagree, the detailed pages (B to F) are more precise, and the compiler is always right. This file is made from the separate pages in Forge's docs folder by docs/make-all-docs.ps1, so don't edit it by hand.

## Contents

- **Part A: The teaching brief (for an AI tutor)**: Teaching Brief: the Forge Programming Language
- **Part B: Start here**: Forge Documentation - Getting Started
- **Part C: The language**: 1. The Basics - 2. Variables and Types - 3. Text - 4. Operators - 5. Making Decisions and Repeating - 6. Functions - 7. Files and Folders - 8. Lists and Maps - 9. Structs and Classes - 10. Enums, Optionals and Pointers - 11. Tasks - 12. Errors - 13. Reserved Words - 14. Coming From Other Languages
- **Part D: The libraries**: Library Overview - Built-in Types: vec2, vec3, vec4, color, rect - <stdio>: Console Input and Output - <math>: Math Functions - <random>: Random Numbers - <window>: The Game Window - <input>: Keyboard, Mouse and Controllers - <graphics2d>: 2D Drawing - <graphics3d>: 3D Drawing - <assets>: Images, Animations and Fonts - <audio>: Sounds and Music - <physics>: Boxes, Gravity and Collisions - <time>: Clock, Date and Timers - <file>: Reading and Writing Text Files - <save>: Remembering Things Between Runs - <tween>: Smooth Changes Over Time - <particles>: Dust, Sparks and Explosions - <tilemap>: Levels Drawn as Text - <scenes>: Menus, Levels and Game Over - <ui>: Buttons, Checkboxes, Sliders and Text Boxes - <ECS>: Entities and Components - <tasks>: Tasks Without a Window - Low-Level Toolkits: OpenGL, DirectX and Vulkan - <OpenGL>: Drawing with OpenGL Yourself - <DirectX>: Drawing with Direct3D 11 Yourself - <Vulkan>: Drawing with Vulkan Yourself
- **Part E: Guides**: The Game Loop - Sprites and Animation - Platformer Walkthrough
- **Part F: Under the hood**: How Forge Works
- **Part G: The cheat sheet (everything on one page)**: Forge Cheat Sheet (v0.7)

---

# Part A: The teaching brief (for an AI tutor)

<!-- docs/TEACH_ME_FORGE.md -->

## Teaching Brief: the Forge Programming Language

> **To the AI reading this:** you're going to teach a beginner a programming language called **Forge**. It's a brand-new language, so you have never seen it before, and nothing about it is on the internet. **This file is the only source of truth.** Read all of it before you start.

---

### Part 1: How to teach

#### Who you're teaching

- They're **new to programming**. They know the very basics (variables, if statements, printing) but not deeper ideas such as memory, generics or design patterns.
- They designed Forge's rules together with the AI that built it, so they have opinions about how it looks. They didn't write the compiler, though, and they're learning to *use* the language.
- Their goal is **making games**. The first real target is a **2D platformer**: a cube that jumps between rectangle platforms. A finished example is in Part 5.
- They use **Windows** and **VS Code**, and Forge is installed. They can run any code you give them, so they're the one running things, not you.

#### How to teach well

1. **Only teach what's in this file.** Never invent a feature, function or library. If Forge has something, it's listed here. If it isn't here, Forge doesn't have it yet. Say so plainly ("Forge can't do that yet") and show the closest way to do it with what *is* here.
2. **Many languages look like Forge, but don't assume they match.** Forge looks like C++ and reads a bit like Python, but its rules differ in places. Part 4 lists the differences you're most likely to get wrong.
3. **One small idea at a time.** Explain the idea in plain words. Then show a short, complete program that uses it, and give a small exercise. Wait for the learner to try it before moving on.
4. **Programs you give must be complete and runnable.** They need `main.frg`, `int main()`, the `#include` lines they use, and `return 0;`. A missing include is the most common beginner error.
5. **Let them run it, then read the output together.** Ask them to paste what they see, including any errors. Forge's error messages are written for beginners (examples in Part 3), so teach them to read the message and the `hint:` line.
6. **Use game examples** (health, scores, enemies, coins, jumping) instead of abstract ones like `foo` and `bar`.
7. **Check understanding.** Every few lessons, ask them to predict what a program prints before they run it, or to fix a broken program.
8. **Be encouraging, but correct mistakes clearly.** If their code is wrong, say what's wrong and why, then let them fix it themselves before showing the answer.

#### The plan of lessons

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
| 17. Menus | `<scenes>`, `<ui>`, `<save>` | A main menu, and a best score that's remembered |
| 18. Checking kinds, hiding things | `is` / `as`, `private`, several constructors | Different enemy types in one list |
| 19. Components | `<ECS>` | Rebuild a small game out of components |
| 20. 3D | `<graphics3d>` | A spinning cube scene |
| 21. (Optional, advanced) Graphics APIs | `<OpenGL>` first, then `<DirectX>` or `<Vulkan>` if they're curious. Only if the learner asks: the engine is all they need for games. | One colored triangle, then a spinning cube |

---

### Part 2: Using Forge

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
| `frg new mygame` | makes a new folder `mygame/` with a starter `main.frg` (a hello world) |
| `frg new mygame --template window` | starts from a working game instead: `--template window` (a ball moved with the arrow keys), `platformer` (physics, jumping, a following camera) or `3d` (a cube and a turning camera). Good starting points once the learner reaches those lessons |
| `frg run` | checks the code, compiles it, and runs the game |
| `frg build` | compiles into `build/<folder name>.exe` without running it |
| `frg check` | only looks for errors |
| `frg package` | the finished game, with every file it needs (but not the `.frg` files or `save.txt`), zipped into `dist/<name>.zip` to share. If the game is in git, add `build/`, `dist/` and `save.txt` to `.gitignore` |
| `frg build --release` | the finished game: no console window, smaller `.exe` |
| `--no-console` / `--console` | add to `run` or `build` to turn the console window off or on |

A picture called `icon.png` in the game's top folder becomes the `.exe`'s icon and the window's icon (square, ideally 256 × 256). Without one, the game just gets the plain Windows icon.

The **console window** (the black window next to the game) shows `print` output and errors. It's on while testing. Turn it off with `--no-console` to see the game as players will, and `--release` turns it off for the finished game. Without a console, runtime errors pop up in a box titled "The game stopped". Text-only programs need the console: without it nothing shows. In VS Code, **Forge: Turn the Console On/Off** switches it for the ▶ button, and **Forge: Build Release** builds the finished game.

In **VS Code** (with the Forge extension): errors are underlined when you save, the **▶ button** at the top right (or **F5**) runs the game, typing `window.` or `color.` shows autocomplete, and snippets like `game`, `class` and `foreach` insert templates.

Forge is **compiled**. The whole program is checked before it runs, so most mistakes are caught before the game starts.

The first build (and the first `--release` build) shows "Preparing Forge's runtime ... (only the first time)" and takes longer: Forge is getting its own code ready once, so later builds of every game are faster. It's stored in a `cache` folder next to `frg.exe`, which Forge keeps under about 400 MB by itself. `frg cache` shows its size and `frg cache clear` deletes it (the next build just prepares it again).

---

### Part 3: The language reference

#### Comments

````cpp
// one line

```
many
lines
```
````

Multi-line comments use three backticks at the start and end. `/* */` is **not** Forge.

#### Variables and types

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

#### Text

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

#### Printing and reading (`#include <stdio>`)

- `print(x)` shows one value and starts a new line, and `print()` prints an empty line.
- `write(x)` shows a value without starting a new line.
- `string answer = input("Your name? ");` waits for the player to type a line.
- `print` takes **one** value. To show several, put them in text: `print("x = {x}, y = {y}")`.

#### Operators

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

#### If, loops, switch

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

do {                             // like while, but runs at least once: the check comes after
    guess = int(input("Guess: "));
} while (guess != secret);       // note the ; at the end
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

#### Functions

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

#### Multiple files

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

#### Lists and maps

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

#### Structs and classes

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
- **When class objects are freed:** automatically, as soon as nothing (no variable, field or list) refers to them any more. There's no `delete`. The one exception: two objects that refer to **each other** (`a.partner = b; b.partner = a;`, or a team with a list of players where each player has a `Team? team` field) keep each other alive, so they're never freed while the game runs, even after the rest of the game has forgotten them. Memory comes back when the game closes. It rarely matters in small games (leftover objects aren't drawn or updated unless they're still in a list); it only adds up if such pairs are made over and over, like for every bullet. To avoid it, set one side to `null` when done: `e.partner = null;` before removing `e`. Explain it only if the learner builds objects that point at each other.

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

#### Tasks: waiting without freezing the game

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

#### Enums

```cpp
enum State { Idle, Running, Jumping }

State s = State.Running;
print(s);              // Running
int n = int(s);        // 1 (counting from 0)
State t = State(0);    // Idle
```

#### Optional values (`?`) and `null`

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

#### Pointers

```cpp
void addOne(Pointer<int> counter) {
    counter.value += 1;       // .value is the variable it points at
}

int score = 10;
addOne(score);                // score is now 11
Pointer<int> p = score;       // points at score
p.value = 50;                 // score is now 50
```

A pointer must point at a variable, field or list item. Don't keep one pointing at a function's local variable after that function ends. `p.remove()` throws a pointer away (the variable it pointed at is untouched): afterwards `p.isRemoved()` is true, and using `p` stops the game with "this pointer was removed (with remove()) ..." until it's pointed at something again with `p = someVariable;`. With a pointer to a list or map, `p.remove(x)` (with a value) is still the list's own remove. Pointers are rarely needed, since classes are already shared, so teach them late.

#### Built-in types (always available, no include)

| Type | Fields | Extras |
|---|---|---|
| `vec2` | `x y` | `length()` `lengthSquared()` `normalized()` `dot(v)` `distance(v)` `angle()` `rotated(radians)` `lerp(v, t)`, `vec2.ZERO` `ONE` `UP` `DOWN` `LEFT` `RIGHT` |
| `vec3` | `x y z` | `length()` `lengthSquared()` `normalized()` `dot(v)` `cross(v)` `distance(v)` `lerp(v, t)`, `vec3.ZERO` `ONE` `UP` `FORWARD` `RIGHT` |
| `vec4` | `x y z w` | `length()` `normalized()` `dot(v)` |
| `color` | `r g b a` (0–255) | `color(r, g, b)` or `color(r, g, b, a)` from ints (kept between 0 and 255, so `color(random.range(0, 255), ...)` just works), `fade(alpha)`, `lerp(c, t)`. Named colors: `color.WHITE BLACK BLANK LIGHTGRAY GRAY DARKGRAY RED MAROON ORANGE YELLOW GOLD GREEN LIME DARKGREEN SKYBLUE BLUE DARKBLUE PURPLE VIOLET PINK MAGENTA BROWN BEIGE` |
| `rect` | `x y width height` | `overlaps(other)` `contains(point)` `center()` `position()` `size()` |

Vectors work with math: `pos += velocity * dt;`. Vectors add and subtract with vectors, and multiply or divide by numbers.

#### Libraries

Library functions are reached through the library name (`window.open(...)`). Library *types* like `Key` and `Body` are used by plain name. Each file includes what it uses.

| Include | Contents |
|---|---|
| `<stdio>` | `print`, `write`, `input` |
| `<math>` | `math.sqrt pow sin cos tan asin acos atan atan2 exp log floor ceil round abs sign min max clamp lerp`, and `math.PI TAU E`. Angles are in radians. |
| `<random>` | `random.range(1, 6)` (whole numbers, both ends included), `random.range(0.5, 2.0)` (decimals), `random.value()` (0 to 1), `random.chance(0.25)`, `random.seed(n)`, `random.color()` (a random solid color) |
| `<window>` | `window.open(width, height, "title")`, `shouldClose()`, `run()`, `close()`, `deltaTime()` (seconds since the last frame), `time()`, `width()`, `height()`, `fps()`, `setFPS(n)`, `setTitle(t)`, `setBackground(color)`, `toggleFullscreen()`, `screenshot("file.png")` |
| `<input>` | `input.keyDown(Key.SPACE)` (held), `input.keyPressed(...)` (only the frame it was pressed), `input.keyReleased(...)`, `input.mouseDown(MouseButton.LEFT)`, `mousePressed`, `mouseReleased`, `mousePosition()` (a vec2), `mouseX()`, `mouseY()`, `mouseWheel()`. Controllers (gamepads): `input.gamepadDown(GamepadButton.A)`, `gamepadPressed`, `gamepadReleased` (Xbox-style names: `A B X Y LEFT_BUMPER RIGHT_BUMPER LEFT_TRIGGER RIGHT_TRIGGER BACK START LEFT_STICK RIGHT_STICK`, and the D-pad `UP DOWN LEFT RIGHT`), `input.leftStick()` / `rightStick()` (a vec2 from -1 to 1; y is down, like the screen; small wobbles read as 0), `leftTrigger()` / `rightTrigger()` (0 to 1), `gamepadConnected()`, `gamepadName()`. Each takes an optional controller number: 0 (the default) is the first one plugged in. Without a controller they all read "not pressed", so a game can check both: `if (input.keyDown(Key.SPACE) or input.gamepadDown(GamepadButton.A))` |
| `<graphics2d>` | `graphics2d.drawRect(x, y, w, h, color)` or `drawRect(rect, color)`, `drawRectOutline(x, y, w, h, color, thickness)`, `drawCircle(x, y, radius, color)` or `drawCircle(vec2, radius, color)`, `drawCircleOutline(x, y, radius, color)`, `drawLine(from, to, color, thickness)`, `drawTriangle(a, b, c, color)`, `drawText(text, x, y, size, color)`, `textWidth(text, size)`, `drawFPS(x, y)`, `clear(color)`, and a 2D camera (below) |
| `<physics>` | the `Body` class, plus `physics.add(body)`, `remove(body)`, `step(dt)`, `setGravity(vec2)`, `gravity()`, `clear()`, `count()` |
| `<assets>` | `assets.loadTexture("x.png")` gives a `Texture`, `assets.loadFont("x.ttf")` gives a `Font` (an optional second number is the size the letters are prepared at: use about the biggest size you draw it at, 64 to start with), `assets.exists(path)` |
| `<audio>` | `audio.loadSound("x.wav")` gives a `Sound`, `audio.loadMusic("x.ogg")` gives a `Music`, `audio.setVolume(v)` |
| `<graphics3d>` | the `Camera` struct and `Model` class, `graphics3d.setCamera(cam)`, `drawCube(pos, size, color)`, `drawCubeOutline`, `drawSphere(center, radius, color)`, `drawSphereOutline`, `drawCylinder(pos, radius, height, color)`, `drawPlane(center, vec2 size, color)`, `drawGrid(slices, spacing)`, `drawLine(from, to, color)`, `toScreen(vec3)`, `loadModel("x.glb")` |
| `<time>` | `time.now()`, `time.sleep(s)`, `time.clock()`, `time.date()`, `year() month() day() hour() minute() second()`, the `Timer` class |
| `<tilemap>` | levels drawn as text, one character per tile: `TileMap level = TileMap("""` ... `""", 32);` (or `tilemap.load("level1.txt", 32)`). The game decides what characters mean. `level.addBodies("#")` makes static physics bodies, merging neighbouring tiles into big boxes (`addBodies("=", true)` makes one-way platforms); `level.find("C")` gives the top-left corner (pixels) of every `C` tile; `get(column, row)` (`" "` outside the map), `set(column, row, "#")`, `columns()`, `rows()`, `width()`, `height()` (pixels), `tileRect(column, row)`, `columnAt(x)`, `rowAt(y)`; `draw("#", color)` fills tiles with a color, `drawTiles(tileset, 16, "#=")` draws them from a tileset image (each character is the next 16x16 picture). Don't name the variable `map` (a type name) |
| `<particles>` | dust, sparks, smoke, explosions: `Emitter sparks = Emitter(vec2(x, y));`, then `sparks.burst(30);` (all at once) or `sparks.rate = 50;` (a steady stream per second), and each frame `sparks.update(dt);` and `sparks.draw();`. Fields to set: `position`, `lifetime` (seconds), `speed`, `direction` (degrees: 0 right, -90 up), `spread` (degrees around it: 360 every way), `startColor`/`endColor` (an end color with alpha 0 fades them out), `startSize`/`endSize`, `gravity` (a vec2). Also `count()` and `clear()`. It draws in 2D, through the 2D camera |
| `<tween>` | smooth changes over time: `Tween slide = Tween(-300, 40, 0.5, Ease.OUT);` (from, to, seconds, easing), then `slide.update(dt);` and use `slide.value()`. Also `progress()` (0 to 1, with `lerp` for positions and colors: `a.lerp(b, t.progress())`), `isDone()`, `restart()`, `reverse()` (go back from where it is), fields `looping` and `pingPong`. `Ease.LINEAR` (steady), `IN` (slow start), `OUT` (slow end, the default), `IN_OUT`, `BOUNCE`. There are no callbacks: check `isDone()` |
| `<save>` | `save.setInt(key, n)` / `getInt(key, backup)`, the same for `Float`, `String` and `Bool`, `has(key)`, `remove(key)`, `clear()`. Kept in `save.txt` in the game's folder |
| `<file>` | `file.read(path)`, `write(path, text)`, `append(path, text)`, `exists(path)`, `remove(path)`, `readLines(path)`, `writeLines(path, list)`, `files(folder)` |
| `<scenes>` | the `Scene` class, `scenes.change(scene)`, `update(dt)`, `draw()`, `current()` |
| `<ui>` | `ui.button(rect, "text")` (true when clicked), `ui.checkbox(rect, "text", checked)` (new value), `ui.slider(rect, value, min, max)` (new value), `ui.textBox(rect, text, maxLength = 0)` (a box to type in: `name = ui.textBox(rect(20, 20, 240, 40), name, 12);` gives the new text; click to type, Backspace deletes, Enter or a click elsewhere finishes), `ui.isTyping()` (true while typing, so the game can skip its key shortcuts), `ui.label(text, x, y, size, color)`, `ui.panel(rect)`, `ui.setTheme(bg, accent, text)` |
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

#### Physics

A `Body` is a box: `Body(x, y, width, height)`.
- Fields you set: `position`, `size`, `velocity` (all vec2), `isStatic` (true for floors, walls and platforms that never move), `oneWay` (with `isStatic`: a one-way platform, which you can jump up through from below and walk through sideways, but land on from above), and `gravityScale` (1 by default).
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

#### More libraries

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
- **Sprite-sheet animation:** `Animation walk = Animation(sheet, 16, 16, 4, 8);` (the sheet, frame width and height, how many frames, frames per second, and optionally the row they start in, from 0). Then `walk.update(dt);` in `update` and `walk.drawScaled(x, y, 4);` (or `draw(x, y)`) in `draw`. Frames are read left to right. `flip(true)` mirrors it to face left (`isFlipped()` tells which way it faces), `play()` / `pause()` / `reset()`, `frame()`, `setFrame(i)`, `frameCount()`, `isPlaying()`, and fields `fps` and `looping` (set `looping = false` for a one-time animation, then check `isDone()`). It's a class, so `current = run;` shares the same animation. Learners can also do it by hand first to understand it: keep a frame number, change it every 0.1 seconds with a timer, and draw `drawPart(rect(frame * 16, 0, 16, 16), rect(x, y, 64, 64))`.

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

**Saving** (`<save>`), the easy way to remember things between runs (high scores, settings, progress). Values are kept by name in `save.txt` in the game's folder, and written straight away:

```cpp
#include <save>

int best = save.getInt("best");           // 0 the very first time (or save.getInt("best", 100) for another backup)
if (score > best) {
    save.setInt("best", score);
}
save.setString("name", "Ana");
save.setFloat("volume", 0.8);
save.setBool("music", true);
```

`getInt` / `getFloat` / `getString` / `getBool` take the key and an optional backup value, given back when the key isn't saved yet. `save.has(key)`, `save.remove(key)` and `save.clear()` (to reset everything) are the rest. Because `save` is a library name, a function or variable can't be called `save` (use `saveGame`).

**Files** (`<file>`), for your own file formats (levels, lists): `file.write("notes.txt", "{score}");` and later `int score = int(file.read("notes.txt").trim());`. Check `file.exists(path)` first.

#### Making a game: two kinds of game loop

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

void update(float dt){
    // move things: dt is the time since the last frame, in seconds
}

void draw(){
    // draw things
}
```

The screen is cleared to the background color before every frame. **Escape** or the window's X closes the game. Multiply speeds by `dt` so movement is the same speed on every computer: `x += 200 * dt;` moves 200 pixels per second.

#### Low-level toolkits (advanced, optional)

`<OpenGL>`, `<DirectX>` and `<Vulkan>` let advanced users draw with a real graphics API, writing their own shaders. **Only teach these if the learner asks.** Forge's engine is all anyone needs to make games.

The rules, which the learner chose on purpose:

- A toolkit is **not** part of the engine, and the engine can't be switched over to use one.
- A game that includes a toolkit **can't** include `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`. Doing so is a build error: "<OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)".
- A game can use only **one** toolkit.
- `<stdio>`, `<math>`, `<random>`, `<time>`, `<file>`, `<save>`, `<audio>`, `<physics>`, `<scenes>`, `<ECS>` and tasks still work with a toolkit.

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

#### Error messages

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

### Part 4: Easy things to get wrong

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
- Labeled breaks, exceptions (`try` / `catch`), `goto`
- Networking, shaders in the engine, 3D lighting
- `<network>` is planned but **doesn't exist yet**.

---

### Part 5: The example platformer

This game is in `examples/platformer/`. It's the learner's first goal: they should eventually understand every line. Arrows or A/D move, Space/W/Up jumps, you collect 12 coins, and falling into a pit sends you back to the start. The level is 2400 pixels wide, so a `Camera2D` follows the player smoothly (`followPlayer` in main.frg). The level itself is drawn as text in level.frg with a `<tilemap>` (`#` ground, `=` one-way planks, `C` coins, `P` the start), and the text is drawn after `resetCamera()` so it stays on the screen.

#### main.frg

````cpp
#include <window>
#include <graphics2d>
#include <physics>
#include <math>

```
  A tiny platformer: move with the arrow keys (or A / D), jump with Space (or W / Up).
  Collect all the coins. Don't fall in the pits!
  The level is drawn as text in level.frg (a <tilemap>).

  This game uses the engine loop: window.run() calls update(dt) and draw() every frame.
  The level is wider than the window, so a camera follows the player.
```

// The camera looks at `target` (a point in the level) and shows it at `offset` (the middle of the window).
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));

int main(){
    window.open(960, 540, "Forge Platformer");
    window.setBackground(color(24, 26, 40));
    level.build();
    player.start = level.start;     // the P in the level
    player.hero.position = level.start;
    window.run();
    return 0;
}

void update(float dt){
    player.handleInput();
    physics.step(dt);
    player.checkLanding(dt);
    player.checkFall();
    level.collectCoins(player.hero.bounds());
    level.updateEffects(dt);
    followPlayer(dt);
}

// The camera glides toward the player, but stops at the ends of the level so it never shows past them.
void followPlayer(float dt){
    float half = window.width() / 2.0;
    float x = math.clamp(player.hero.center().x, half, level.tiles.width() - half);
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

#### player.frg

```cpp
#include <window>
#include <graphics2d>
#include <input>
#include <physics>
#include <particles>

const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;
vec2 start = vec2(80, 380);    // main.frg moves it to the P in the level

// A Player is a physics Body (it inherits position, size, velocity, onGround...)
// with a few extra things of its own.
class Player : Body {
    color tint = color.ORANGE;
    int deaths = 0;

    void respawn() {
        position = start;
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

// A puff of dust when the player lands.
Emitter dust = makeDust();
bool wasOnGround = false;
float fallSpeed = 0;

Emitter makeDust(){
    Emitter e = Emitter(vec2(0, 0));
    e.lifetime = 0.4;
    e.speed = 90;
    e.direction = -90;          // up...
    e.spread = 160;             // ...and out to both sides
    e.gravity = vec2(0, 300);
    e.startColor = color(200, 200, 210);
    e.endColor = color(200, 200, 210, 0);   // fades out
    e.startSize = 9;
    e.endSize = 3;
    return e;
}

Player makeHero(){
    Player p = Player(start.x, start.y, 36, 36);
    physics.add(p);
    return p;
}

void handleInput(){
    fallSpeed = hero.velocity.y;    // remembered before physics.step stops the fall
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

// Called after physics.step: a hard landing makes a puff of dust at the player's feet.
void checkLanding(float dt){
    if (hero.onGround and !wasOnGround and fallSpeed > 250) {
        dust.position = vec2(hero.center().x, hero.position.y + hero.size.y);
        dust.burst(14);
    }
    wasOnGround = hero.onGround;
    dust.update(dt);
}

void checkFall(){
    if (hero.position.y > window.height() + 100) {
        hero.respawn();
    }
}

void draw(){
    dust.draw();
    graphics2d.drawRect(hero.bounds(), hero.tint);
    // Two eyes that look the way the player is moving.
    float look = hero.velocity.x > 0 ? 4 : hero.velocity.x < 0 ? -4 : 0;
    float eyeY = hero.position.y + 12;
    graphics2d.drawRect(hero.position.x + 8 + look, eyeY, 6, 8, color.BLACK);
    graphics2d.drawRect(hero.position.x + 22 + look, eyeY, 6, 8, color.BLACK);
}
```

#### level.frg

```cpp
#include <graphics2d>
#include <physics>
#include <particles>
#include <tilemap>

// The level, drawn with characters. Each character is one 36 x 36 tile:
//   #  ground and walls          =  a platform you can jump up through
//   C  a coin                    P  where the player starts
TileMap tiles = TileMap("""
#                                                          #
#                                                          #
#                                                          #
#                                                          #
#                                             C            #
#                            C               ====          #
#                      C    ####                           #
#         C           ====                C                #
#        ====                            ====              #
#    C              C                  C             C     #
#   ====          =====         C    =====          ====   #
#                                                          #
# P     C                                               C  #
############    ##############     ###########    ##########
############    ##############     ###########    ##########
""", 36);

list<vec2> coins = [];
int totalCoins = 0;
int collected = 0;
vec2 start = vec2(0, 0);

// Golden sparkles where a coin was picked up.
Emitter sparkles = makeSparkles();

Emitter makeSparkles(){
    Emitter e = Emitter(vec2(0, 0));
    e.lifetime = 0.6;
    e.speed = 140;
    e.gravity = vec2(0, 250);
    e.startColor = color.GOLD;
    e.endColor = color(255, 255, 160, 0);
    e.startSize = 7;
    e.endSize = 1;
    return e;
}

void build(){
    // Neighbouring # tiles become a few big static bodies, not hundreds of small ones.
    tiles.addBodies("#");
    tiles.addBodies("=", true);     // true: one-way platforms

    // find gives the top-left corner of each tile; coins are kept as the middle of their tile.
    for (vec2 corner in tiles.find("C")) {
        coins.add(corner + vec2(tiles.tileSize / 2, tiles.tileSize / 2));
    }
    totalCoins = coins.length;
    start = tiles.find("P")[0];
}

void collectCoins(rect playerBox){
    // Go backwards so removing a coin doesn't skip the next one.
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
            sparkles.position = c;
            sparkles.burst(24);
        }
    }
}

void updateEffects(float dt){
    sparkles.update(dt);
}

void draw(){
    tiles.draw("#", color(70, 86, 120));
    // Grass on every # tile that has nothing above it.
    for (int row = 0; row < tiles.rows(); row++) {
        for (int column = 0; column < tiles.columns(); column++) {
            if (tiles.get(column, row) == "#" and tiles.get(column, row - 1) != "#") {
                rect r = tiles.tileRect(column, row);
                graphics2d.drawRect(r.x, r.y, r.width, 5, color(110, 200, 130));
            }
        }
    }
    // One-way platforms are drawn as thin planks.
    for (vec2 p in tiles.find("=")) {
        graphics2d.drawRect(p.x, p.y, tiles.tileSize, 10, color(170, 130, 90));
    }
    for (vec2 c in coins) {
        graphics2d.drawCircle(c, 10, color.GOLD);
        graphics2d.drawCircleOutline(c.x, c.y, 10, color.ORANGE);
    }
    sparkles.draw();
}
```

**Good exercises once the learner understands it:** show the death count on screen, add a coin (type a `C` into the level text in level.frg), make a moving platform (change a static body's `position` in `update`), add a timer with `window.time()`, add a second level, or add enemies that send you back to the start when `hero.overlaps(enemy)`.


---

### Part 6: Practice projects

Use these whenever the learner finishes a stage and wants more practice, or asks "what should I make?". Each project fits a stage from the plan of lessons in Part 1. They're more projects to choose from, on top of the one project in that table.

How to run a project with the learner:

1. Pick one that fits their stage and sounds fun to them. Offer two or three and let them choose.
2. Give them the goal and the **first step only**. Let them write it, run it and show you.
3. Then the next step. Give a hint only when they're stuck, starting with the smallest one ("which loop repeats until something happens?") before naming functions. The hints listed here are for you; they're all correct Forge.
4. **Don't write the project for them.** Fix mistakes by pointing at the line and asking what it does.
5. When it works, celebrate it, then offer a "make it yours" idea.

The text below is written to the learner, so you can share parts of it with them directly.

#### How to do a project

- **Build it in small steps.** Each project is split into steps. Get each step running before starting the next. A program that works but does less is better than a big one that doesn't run.
- **Run it often.** After every few lines, `frg run` it. If something breaks, it's in the lines you just wrote.
- **Read the error.** Forge's messages say what's wrong and usually how to fix it (look at the `hint:` line).
- **Use `print` to look inside.** If a number isn't what you expect, `print("x = {x}")` shows it. The console window shows `print` output even in window games.
- **Stuck for more than 15 minutes?** Read the hints for that project. They name the functions you'll need and show the trickiest piece.
- **Finished?** Try a "make it yours" idea, or make up your own. Changing a working program is one of the best ways to learn.

The **Stage** says which lesson stage the project fits, from the plan of lessons in Part 1. The projects get harder from top to bottom.

#### Level 1: Console programs

These only use the console: no window. They need `#include <stdio>` for `print` and `input`, plus whatever the hints mention.

##### 1. Number guessing game

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
- A `bool found = false;` and `while (!found) { ... }` is one way to loop until the guess is right. Another is `do { ... } while (guess != secret);`, which always asks at least once (make `int guess = 0;` before the `do`, so the condition can see it).

Make it yours: allow only 7 guesses. Ask "Play again? (y/n)" at the end. Let the player choose easy (1-50) or hard (1-500).

##### 2. Math quiz

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

##### 3. Shopping list

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

##### 4. High-score table

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
- Another way to save: the `<save>` library keeps values by name, so there's no file format to write. Save entry `i` as `save.setString("name{i}", e.name)` and `save.setInt("score{i}", e.score)`, plus `save.setInt("count", entries.length)`, and read them back with `save.getString` and `save.getInt`. Doing it by hand with `<file>` first is good practice, though.

Make it yours: show a medal (`*`) next to the top score. Don't allow empty names (`name.trim().isEmpty()`).

##### 5. Text adventure

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

##### 6. Tic-tac-toe

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

##### 7. Turn-based battle

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

#### Level 2: Window games

These open a window: `#include <window>` and `<graphics2d>`, and `<input>` for the keyboard. Remember that **y goes down**: `y = 0` is the top of the window. Use `window.run()` with `update(float dt)` and `draw()`, and multiply speeds by `dt`.

##### 8. Bouncing balls

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

##### 9. Pong

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

##### 10. Catch the falling stars

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

##### 11. Snake

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

##### 12. Breakout

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

Make it yours: a different color and score for each row. Where the ball hits the paddle changes its angle. Bricks that burst into particles (`<particles>`) when broken. Power-ups that fall from broken bricks.

##### 13. Clicker game

**Stage 17.** Click a big cookie to earn cookies. Spend them on upgrades: more per click, or helpers that earn cookies every second. Progress is saved.

**Practices:** the mouse, `<ui>` buttons, earning over time with `dt`, saving and loading, scenes.

Steps:

1. A big circle in the middle. Clicking inside it adds 1 cookie. Show the total.
2. A `ui.button` "Better click (cost 10)". Buying it makes clicks worth more, and raises its cost.
3. A "Helper" upgrade: each helper earns 1 cookie per second (`cookies += helpers * dt`).
4. Save the numbers, and load them when the game starts, with `<save>`: `save.setFloat("cookies", cookies);` and `float cookies = save.getFloat("cookies");`.
5. A title scene with "Play" and "Reset save" buttons (`<scenes>`).

Hints:

- Clicked inside the circle: `input.mousePressed(MouseButton.LEFT) and input.mousePosition().distance(center) < radius`.
- Helpers earn fractions of a cookie, so keep `float cookies`, and show `int(cookies)` (or `"{cookies:.1}"` for one decimal).
- `<save>` writes the file straight away, so saving after every purchase (and every few seconds for the helpers) is fine. Load with a backup value for the very first run: `save.getInt("helpers", 0)`.
- "Reset save" is `save.clear()`, then set the variables back to their starting values.

Make it yours: the cookie shrinks a little when clicked, then grows back. Let the player name their bakery on the title screen with `ui.textBox`, and save the name with `save.setString`. Achievements at 100, 1,000 and 10,000 cookies.

##### 14. Dodge

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

Make it yours: "Wave 3!" text that appears for 2 seconds (another task), sliding in with a `Tween` from `<tween>`. A shield pickup.

##### 15. Platformer remix

**Stage 16.** Take the example platformer (Part 5) and make it your own game.

**Practices:** reading and changing a bigger program, `<physics>`, `<assets>`, `<audio>`, `<scenes>`.

Ideas, roughly from easiest to hardest:

1. Change the colors, the jump height and the speed until it feels right to you.
2. Add more platforms and coins (type them into the level text in `level.frg`), and a door that only opens when all coins are collected (a `D` tile that `set` turns into `" "`).
3. Give the player a sprite and a jump sound.
4. A moving platform: a static `Body` whose position you change every frame, going back and forth.
5. An enemy that walks left and right on a platform. Touching it sends you back to the start.
6. A second level, using `<scenes>`, and a title screen.

#### Level 3: Bigger projects

##### 16. Top-down shooter with components

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

##### 17. Solar system in 3D

**Stage 20.** A sun in the middle, planets going round it at different speeds, and a moon going round one planet. The camera can turn around the scene.

**Practices:** `<graphics3d>`, circles with `math.sin` and `math.cos`, `Camera`.

Steps:

1. A camera looking at the middle, a `drawGrid`, and a yellow sphere for the sun.
2. One planet: its position is `vec3(math.cos(t) * distance, 0, math.sin(t) * distance)`, where `t` grows every frame.
3. Several planets, each a struct with its own distance, speed, size and color.
4. A moon: the same circle, but around its planet's position instead of the middle.
5. Left/Right turns the camera: keep a `Camera cam`, call `cam.orbit(degrees)`, then `graphics3d.setCamera(cam)`.

Make it yours: lines showing the orbits (many short `drawLine`s around a circle). Up/Down zooms. Labels with each planet's name, placed with `graphics3d.toScreen`.

##### 18. Your own game

**Any stage.** Pick a simple game you like and make a small version of it. Good first choices: Flappy Bird, Space Invaders, Frogger, Asteroids, Tetris (hard!), a maze game.

Before writing code:

1. Write down, in one sentence, what the player does.
2. List the **smallest** version that's still fun to play for a minute. That's version 1.
3. Write the steps for version 1, like the projects above.

Then build version 1, play it, and only then add more.

---

*Forge v0.7. If the learner reports that something in this file doesn't work, believe the compiler over this file and tell them to report it to whoever maintains Forge.*

---

# Part B: Start here

<!-- docs/README.md -->

## Forge Documentation

Forge (`.frg`) is a compiled programming language for making 2D and 3D games. It looks like C++, has optional plain-English words for every operator, and its error messages explain what went wrong in everyday language.

These docs are for **anyone using or learning Forge**, including people who have never programmed before. If you're new, read them in order. If you already know another language, skim Getting Started (see “Getting Started”), then read Things that surprise people from other languages (see “14. Coming From Other Languages”).

> Forge was designed by a person and written by an AI (see the main README (the file `README.md` in the Forge folder)). These docs describe **Forge 0.7**. If something here disagrees with what the compiler does, the compiler is right; please report the mismatch.

### Start here

| | |
|---|---|
| Getting Started (see “Getting Started”) | Setup, the `frg` command, VS Code, your first program, and building a finished game |
| Cheat sheet (see “Forge Cheat Sheet (v0.7)”) | The whole language on one page, for looking things up quickly |
| Everything in one file (the file `FORGE_ALL_DOCS.md` in the Forge folder) | The teaching brief and every page below, combined into one file to upload to an AI chat. Rebuild it with `.\docs\make-all-docs.ps1` after changing the docs. |

### The language

Read these in order the first time.

| # | Chapter | What's in it |
|---|---|---|
| 1 | The basics (see “1. The Basics”) | How a program is laid out, `main`, statements, comments, braces, names |
| 2 | Variables and types (see “2. Variables and Types”) | Every type, number sizes, `const`, converting between types, how division works |
| 3 | Text (see “3. Text”) | Strings, putting values inside text, escapes, raw text, every text method |
| 4 | Operators (see “4. Operators”) | Every operator in symbol and word form, and which ones happen first |
| 5 | Making decisions and repeating (see “5. Making Decisions and Repeating”) | `if`, `while`, `for`, `for ... in`, `switch`, `break`, `continue` |
| 6 | Functions (see “6. Functions”) | Writing and calling functions, parameters, default values, `return` |
| 7 | Files and folders (see “7. Files and Folders”) | Splitting a game into files, the file-name prefix rule, folders, `#include` |
| 8 | Lists and maps (see “8. Lists and Maps”) | `list<T>`, `map<K, V>` and every method they have |
| 9 | Structs and classes (see “9. Structs and Classes”) | Your own types: fields, methods, constructors, inheritance, `is`/`as`, `private` |
| 10 | Enums, optionals and pointers (see “10. Enums, Optionals and Pointers”) | `enum`, `Type?` and `null`, `Pointer<T>` |
| 11 | Tasks (see “11. Tasks”) | `task` functions and `wait()`: timed events without freezing the game |
| 12 | Errors (see “12. Errors”) | How to read error messages, and a list of common ones with fixes |
| 13 | Reserved words (see “13. Reserved Words”) | Every keyword, built-in name and word operator |
| 14 | Coming from other languages (see “14. Coming From Other Languages”) | Where Forge differs from C++, C#, Java, JavaScript and Python |

### The libraries

Everything that isn't the core language lives in a library you `#include`. See the library overview (see “Library Overview”) for a one-line summary of each.

| Library | For |
|---|---|
| Built-in types (see “Built-in Types: vec2, vec3, vec4, color, rect”) | `vec2`, `vec3`, `vec4`, `color`, `rect` (always available) |
| `<stdio>` (see “`<stdio>`: Console Input and Output”) | Printing to and reading from the console |
| `<math>` (see “`<math>`: Math Functions”) | Square roots, trigonometry, rounding, min/max/clamp |
| `<random>` (see “`<random>`: Random Numbers”) | Random numbers |
| `<window>` (see “`<window>`: The Game Window”) | Opening the game window and running the game loop |
| `<input>` (see “`<input>`: Keyboard, Mouse and Controllers”) | Keyboard, mouse and controllers (gamepads) |
| `<graphics2d>` (see “`<graphics2d>`: 2D Drawing”) | Drawing shapes and text |
| `<graphics3d>` (see “`<graphics3d>`: 3D Drawing”) | Cameras, 3D shapes and 3D models |
| `<assets>` (see “`<assets>`: Images, Animations and Fonts”) | Loading images and fonts, and sprite-sheet animation |
| `<audio>` (see “`<audio>`: Sounds and Music”) | Sound effects and music |
| `<physics>` (see “`<physics>`: Boxes, Gravity and Collisions”) | Gravity and collisions for boxes |
| `<time>` (see “`<time>`: Clock, Date and Timers”) | The clock, the date, and timers |
| `<save>` (see “`<save>`: Remembering Things Between Runs”) | Remembering values between runs: high scores, settings, progress |
| `<tilemap>` (see “`<tilemap>`: Levels Drawn as Text”) | Levels drawn as text: walls, coins and the start from characters |
| `<particles>` (see “`<particles>`: Dust, Sparks and Explosions”) | Dust, sparks, smoke and explosions |
| `<tween>` (see “`<tween>`: Smooth Changes Over Time”) | Smooth changes over time: sliding, bouncing, fading |
| `<file>` (see “`<file>`: Reading and Writing Text Files”) | Reading and writing text files |
| `<scenes>` (see “`<scenes>`: Menus, Levels and Game Over”) | Switching between menu, levels and game over |
| `<ui>` (see “`<ui>`: Buttons, Checkboxes, Sliders and Text Boxes”) | Buttons, checkboxes, sliders, text boxes |
| `<ECS>` (see “`<ECS>`: Entities and Components”) | Building game objects out of components |
| `<tasks>` (see “`<tasks>`: Tasks Without a Window”) | Running tasks in programs without a window |

#### Low-level toolkits

For advanced users who want to write their own rendering with a real graphics API. These are **separate from the engine**: a game uses either the engine or one toolkit, never both. Start with Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”).

| Toolkit | For |
|---|---|
| `<OpenGL>` (see “`<OpenGL>`: Drawing with OpenGL Yourself”) | OpenGL 3.3, with GLSL shaders |
| `<DirectX>` (see “`<DirectX>`: Drawing with Direct3D 11 Yourself”) | Direct3D 11, with HLSL shaders |
| `<Vulkan>` (see “`<Vulkan>`: Drawing with Vulkan Yourself”) | Vulkan, with GLSL shaders compiled to SPIR-V |

### Guides

| | |
|---|---|
| The game loop (see “The Game Loop”) | How a game runs frame by frame, and `dt` |
| Sprites and animation (see “Sprites and Animation”) | Drawing images, sprite sheets, flipping, animation timing |
| Platformer walkthrough (see “Platformer Walkthrough”) | The example platformer, explained line by line |
| Practice projects (see Part 6 of the teaching brief, in Part A) | 18 projects with steps and hints, from a guessing game to a 3D solar system |

### Under the hood

| | |
|---|---|
| How Forge works (see “How Forge Works”) | What happens between your `.frg` files and a `.exe`, for the curious |

<!-- docs/getting-started.md -->

## Getting Started

This page gets you from nothing to a running game, and covers every `frg` command.

### What you need

| | Why |
|---|---|
| **Windows 10 or 11** | Forge only builds Windows games for now |
| **w64devkit**, with its `bin` folder on your PATH | Forge turns your code into C++ and uses w64devkit's `g++` to make the `.exe` |
| **Forge itself** (`frg.exe`), with its `bin` folder on your PATH | So you can type `frg` in any terminal |
| **VS Code** (optional, recommended) | With the Forge extension: colors, errors as you save, a Run button |

To check that everything is set up, open a terminal and run:

```
frg version
g++ --version
```

The first should print `Forge 0.7.0` and the second should print a GCC version. If `frg` isn't found, add Forge's `bin` folder to your PATH and restart the terminal.

#### Building Forge from source

You only need this if you're changing Forge itself. From the Forge folder:

```
cmake -S . -B build -G Ninja
cmake --build build
```

That produces `bin/frg.exe`, with `bin/runtime/` next to it. The runtime folder must stay next to `frg.exe`.

### Your first program

```
frg new hello
cd hello
frg run
```

`frg new` makes a folder with one file, `main.frg`:

```cpp
#include <stdio>

int main(){
    print("Hello, World!");
    return 0;
}
```

`frg run` checks your code, compiles it and runs it. You should see:

```
   Compiling hello (1 file)
    Finished build/hello.exe in 1.02s
     Running hello.exe
Hello, World!
```

What each line of the program means:

| Line | Meaning |
|---|---|
| `#include <stdio>` | Use the `stdio` library, which is where `print` comes from |
| `int main(){` | The start of `main`, the function every program begins in. `int` means it gives back a whole number when it finishes. |
| `print("Hello, World!");` | Show some text. Every statement ends with `;`. |
| `return 0;` | Finish `main` and report "everything went fine" (0). Returning 1 means "something went wrong". |
| `}` | The end of `main` |

#### Starting from a template

`frg new` can also start you off with a small working game. Add `--template` and one of these:

| Command | You get |
|---|---|
| `frg new mygame` (or `--template console`) | the hello world above: a program that prints text, no window |
| `frg new mygame --template window` | a window with a ball you move with the arrow keys, using `window.run()` with `update(dt)` and `draw()` |
| `frg new mygame --template platformer` | a small platformer: a box that runs and jumps across platforms, with physics, a pit, and a camera that follows it through a level wider than the window |
| `frg new mygame --template 3d` | a 3D scene: a cube and a ball on a grid, and a camera you turn with Left / Right |

Each one is a single `main.frg` with comments, ready to `frg run` and then change into your own game. The game's name (`mygame`) becomes its window title and the name of its `.exe`, so it can only use letters, numbers and `_`, and can't start with a number.

### The `frg` command

Run these in a terminal. `[folder]` is the game's top folder (the one with `main.frg`); leave it out to use the current folder.

| Command | What it does |
|---|---|
| `frg new <name>` | Makes a folder `<name>/` containing a starter `main.frg`. Add `--template window`, `platformer` or `3d` to start from a small game (see “Starting from a template”) |
| `frg run [folder]` | Checks, compiles and runs the game |
| `frg build [folder]` | Checks and compiles into `build/<folder name>.exe`, without running it |
| `frg check [folder]` | Only looks for errors (fast, makes no `.exe`) |
| `frg package [folder]` | Makes the finished game and zips it, ready to share: `dist/<name>.zip` (see Sharing your game (see “Sharing your game”)) |
| `frg cache` | Shows how big the build cache is (why there is one (see “Why the first build is slower”)). `frg cache clear` deletes it |
| `frg version` | Shows the Forge version |
| `frg help` | Shows this list |

#### The console window: on for testing, off for players

A game built with `frg build` or `frg run` opens with a black **console window** next to it. That's where `print` output and error messages appear, which is what you want while you're making the game. Players don't want it, though, so it can be turned off:

| Add this to `build` or `run` | Console | Use it for |
|---|---|---|
| (nothing) | on | testing and debugging |
| `--no-console` | off | trying the game the way players will see it |
| `--release` | off, and the `.exe` is a bit smaller | the finished game you give to people |
| `--release --console` | on | a finished build that still shows what's going on |

```
frg run                  // console on: see print() output and errors
frg run --no-console     // no console
frg build --release      // the finished game, no console
```

Without a console, `print` output isn't shown anywhere. **Runtime errors still are**: they pop up in a small box titled "The game stopped" that says what went wrong, instead of the game just disappearing. A program that never opens a window (a text game) needs its console: build it without one and nothing shows at all. Forge prints a note if you try.

In VS Code, the **Forge: Turn the Console On/Off** command switches the console for the ▶ button, and **Forge: Build Release** builds the finished game.

#### Why the first build is slower

The very first time you build a game (and the first time with `--release` or without a console), Forge shows:

```
   Preparing Forge's runtime for these libraries (only the first time, to make later builds faster)
```

It's getting Forge's own code ready once, so every build after that is quicker, for all your games. It's kept in a `cache` folder next to `frg.exe`. Each kind of game (console, window, release...) gets its own prepared set of about 100 MB, and Forge keeps the whole folder under about **400 MB** by deleting the sets that haven't been used for the longest time. When Forge (or w64devkit) is updated, the old sets are deleted by themselves too.

```
frg cache          // shows where the cache is and how big it is
frg cache clear    // deletes it; the next build prepares it again
```

`--release` builds take a little longer, because they make the game run as fast as possible. Normal builds skip some of that work: they're quicker to make, and the game behaves exactly the same.

When `frg run` starts your game, the game's **working folder** is the game's top folder. That matters for files your game loads or saves; see Files (see “`<file>`: Reading and Writing Text Files”) and Assets (see “`<assets>`: Images, Animations and Fonts”).

If the game crashes or `main` returns something other than 0, `frg run` says so after the game ends, for example `[hello.exe exited with code 1]`.

### A game's folder

```
mygame/
  main.frg          <- required: the game starts in its main()
  player.frg        <- any other .frg files next to main.frg are part of the game automatically
  enemies/          <- subfolders join the game with #include <"enemies">
    goblin.frg
  player.png        <- images, sounds, fonts and models your game loads
  icon.png          <- optional: the game's icon (see below)
  build/            <- made by frg: mygame.exe, plus the C++ Forge generated (you can ignore it)
  dist/             <- made by frg package: the finished game, ready to share
```

- `main.frg` must be in the **top** folder, not in a subfolder, and it must contain `int main()`.
- File and folder names may only use letters, numbers and `_`, and can't start with a number. They also can't be a Forge keyword or library name, so no `map.frg` or `math.frg`.
- See Files and folders (see “7. Files and Folders”) for how files use each other.
- If you keep your game in **git**, don't save the folders Forge makes. Put a file called `.gitignore` in the game's folder with these lines:

  ```
  build/
  dist/
  save.txt
  ```

  (`save.txt` is where `<save>` (see “`<save>`: Remembering Things Between Runs”) keeps your own test saves.)

#### The game's icon

Put a picture called **`icon.png`** in the game's top folder (next to `main.frg`), and `frg build` makes it the icon of the `.exe`, which shows in File Explorer and the taskbar. In window games it's the window's icon too. A square picture works best, ideally 256 × 256 pixels. Without an `icon.png`, the game gets Windows' plain program icon.

If `icon.png` isn't really a PNG picture, Forge gives a warning and builds the game without an icon.

### Sharing your game

The easy way is one command:

```
frg package
```

It makes a finished (`--release`) build, then puts the `.exe` and every file the game needs into `dist/<name>/`, and zips that folder into **`dist/<name>.zip`**. Send that zip to people: they unzip it and double-click the `.exe`.

What goes into the package: everything in the game's folder (images, sounds, fonts, models, level files, `icon.png`, sub-folders...), **except** your `.frg` files, the `build/` and `dist/` folders, hidden files and folders (names starting with `.`, like `.git`), shader sources (`.vert`, `.frag` and the others; their compiled `.spv` files are included), and `save.txt`, so players don't start with your test save.

`frg package --console` keeps the console window in the packaged game, for testers who should see errors.

The zip is made with `tar.exe`, which comes with Windows 10 and newer. If it can't be made, the `dist/<name>/` folder is still there, and you can zip it yourself (right-click it, then **Send to → Compressed (zipped) folder**).

#### Doing it by hand

The `.exe` in `build/` runs on any Windows PC, with nothing else installed. Everything is built into it.

If your game loads images, fonts, sounds or 3D models, copy them next to the `.exe`, or into the folder above it, keeping the same sub-folders as in your game's folder. Forge looks in both places.

Text files read and written with `<file>` (see “`<file>`: Reading and Writing Text Files”) work differently: they're found relative to the folder the game was **started from**. When someone double-clicks the `.exe`, that's the folder the `.exe` is in.

### VS Code

The Forge extension adds:

| Feature | How |
|---|---|
| Syntax colors | Automatic for `.frg` files |
| Errors while you work | Saving a file checks the whole game; mistakes are underlined in red, with the hint attached. The Problems panel (Ctrl+Shift+M) lists them all. |
| Run the game | The ▶ button at the top-right of the editor, or **F5** |
| Build only | **Ctrl+Shift+B** |
| Autocomplete | Type a library name and a dot (`window.`, `graphics2d.`, `color.`, `Key.`) to see what's in it |
| Hover help | Hold the mouse over a library function to see how it's used |
| Snippets | Type `main`, `game`, `gameloop`, `class`, `struct`, `enum`, `func`, `if`, `for`, `foreach`, `while`, `switch`, `task`, `scene`, `component` or `body`, then press Tab |
| Comments | **Ctrl+/** toggles `//` comments; **Shift+Alt+A** wraps a selection in a ` ``` ` comment |

To install or update it, run `.\editors\vscode\package.ps1 -Install` from the Forge folder, then in VS Code run **Developer: Reload Window**.

Settings: `forge.path` is the full path to `frg.exe` if VS Code can't find it. `forge.checkOnSave` turns checking on save on or off.

### Troubleshooting

| Problem | Fix |
|---|---|
| `frg` is not recognized | Forge's `bin` folder isn't on your PATH. Add it, then open a new terminal. |
| `couldn't find g++` | w64devkit's `bin` folder isn't on your PATH. |
| `there's no main.frg in ...` | You're in the wrong folder, or the file is named differently. `main.frg` must be in the game's top folder. |
| `couldn't replace build\game.exe` | The game is still running. Close it and try again. |
| `the C++ compiler rejected Forge's output` | That's a bug in Forge, not in your code. The message shows where the generated C++ is; please report it. |
| `Forge's runtime files are missing` | The `runtime` folder must be next to `frg.exe`. Rebuild Forge, or copy it back. |
| The game can't find `player.png` | Put the file in the game's top folder (next to `main.frg`), and check the spelling and capitals. |
| VS Code shows old red underlines | Save the file again; they update on every save. |

---

# Part C: The language

<!-- docs/language/01-basics.md -->

## 1. The Basics

This chapter covers how a Forge program is put together: where it starts, what a statement is, how comments work, and the rules for names.

### Where a program starts

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
- Other files and folders can be part of the program too; see Files and folders (see “7. Files and Folders”).

### Statements and semicolons

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

### Blocks and braces

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

### Comments

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

### The order things are written in

**Functions, types and global variables can be used before or after the place they're written**, even in other files. Forge reads everything first, then checks it:

```cpp
#include <stdio>

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

### Top level

Outside of any function (the **top level** of a file) you can only write:

- `#include` lines
- functions
- global variables (`int score = 0;`)
- types: `struct`, `class`, `enum`

Any other code, like `print("hi");`, has to go inside a function. Forge says "this code has to go inside a function" if it isn't.

### Names

Names for variables, functions, types and fields:

- can use letters, numbers and `_`
- can't start with a number (`player2` is fine, `2player` isn't)
- are case-sensitive: `score`, `Score` and `SCORE` are three different names
- can't be a reserved word, like `if`, `class`, `int`, `print` or a library name like `window` (see the full list (see “13. Reserved Words”))
- can't be the name of one of your top-level files or folders. If you have `player.frg`, nothing can be called `player`, because `player.something` always means the file.

Common styles (Forge doesn't enforce them, but the docs and examples use them):

| Kind | Style | Example |
|---|---|---|
| variables, functions, fields, methods | `camelCase` | `jumpSpeed`, `takeDamage()` |
| types (struct, class, enum) | `PascalCase` | `Player`, `GameState` |
| constants | `UPPER_CASE` | `MAX_HP`, `GRAVITY` |
| files and folders | `lower_case` | `player.frg`, `enemies/` |

### Expressions vs statements

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

### What's next

Variables and types (see “2. Variables and Types”): storing numbers, text and true/false values.

<!-- docs/language/02-variables-and-types.md -->

## 2. Variables and Types

A **variable** is a named box that holds a value. Every variable has a **type**, which says what kind of value it holds. The type is written before the name.

```cpp
int hp = 15;
float speed = 2.5;
double price = 19.99;
bool alive = true;
string name = "Hero";
```

Once a variable has a type, it keeps it: you can't put text into an `int`.

### Making and changing variables

```cpp
int score = 0;      // make a variable with a starting value
int lives;          // without a value, it starts at zero (see below)

score = 100;        // change it (no type this time)
score += 50;        // add 50 to it (see Operators for all the ways)
```

Writing the type a second time makes a **new** variable. That's an error if one with the same name already exists in the same place:

```cpp
int score = 0;
int score = 5;      // error: there's already a variable called 'score' here
score = 5;          // this is what was meant
```

### Starting values

A variable made without a value starts at the "zero" of its type:

| Type | Starts as |
|---|---|
| any number | `0` (or `0.0`) |
| `bool` | `false` |
| `string` | `""` (empty text) |
| `list`, `map` | empty |
| struct | every field at its starting value |
| optional (`Type?`) | `null` |
| `task` | a finished task |

**Classes and pointers have no zero**, so a variable of a class type must be given a value: `Enemy e = Enemy();`. If it should start empty, make it optional instead: `Enemy? e = null;`. See Structs and classes (see “9. Structs and Classes”).

### Where variables live

| Where it's made | Name | Who can use it | How long it lasts |
|---|---|---|---|
| Inside a function or block | **local** | only code after it, inside the same `{ }` | until the `}` |
| At the top level of a file | **global** | every function in the file, and other files (`player.score`) | the whole program |
| A function's parameters | **parameter** | that function | until the function returns |

```cpp
int highScore = 0;          // global

void play(){
    int score = 50;         // local to play
    if (score > highScore) {
        int bonus = 10;     // local to this if block
        highScore = score + bonus;
    }
    print(bonus);           // error: bonus only exists inside the if block
}
```

Global variables are created before `main` starts, in the order they're written.

### Constants

`const` makes a variable that can never be set to a new value. It must be given one straight away:

```cpp
const int MAX_HP = 100;
const float GRAVITY = 980;

MAX_HP = 50;    // error: 'MAX_HP' is const, so it can't be changed
```

`const` only stops the **variable** from being set again. If it holds a list or an object, the contents can still change: `const list<int> scores = []; scores.add(5);` is allowed.

### The types

#### Whole numbers

| Type | Smallest | Biggest | Notes |
|---|---|---|---|
| `int8` | -128 | 127 | |
| `int16` | -32,768 | 32,767 | |
| `int` (same as `int32`) | -2,147,483,648 | 2,147,483,647 | **the everyday whole number** |
| `int64` | about -9.2 × 10¹⁸ | about 9.2 × 10¹⁸ | |
| `int128` | about -1.7 × 10³⁸ | about 1.7 × 10³⁸ | |
| `uint8` | 0 | 255 | `u` = unsigned: no negative numbers |
| `uint16` | 0 | 65,535 | |
| `uint` (same as `uint32`) | 0 | 4,294,967,295 | |
| `uint64` | 0 | about 1.8 × 10¹⁹ | |
| `uint128` | 0 | about 3.4 × 10³⁸ | |

Use `int` unless you have a reason not to. Use the smaller types when you have huge numbers of values (each `uint8` is 1 byte instead of 4), and the bigger ones when values can grow past 2 billion.

Whole numbers can be written in several ways:

```cpp
int a = 1000000;
int b = 1_000_000;      // underscores are ignored, for readability
int c = 0xFF;           // hexadecimal (base 16): 255
int d = 0b1010;         // binary (base 2): 10
```

**Wrapping around:** when a whole number goes past its biggest or smallest value, it wraps around to the other end, like a car's odometer:

```cpp
uint8 x = 250;
x += 10;            // 260 doesn't fit in a uint8, so it wraps around to 4
int8 y = 127;
y++;                // wraps around to -128
```

Writing a number that doesn't fit is an error: `uint8 z = 300;` → "this number doesn't fit in a uint8". So is a negative number in an unsigned type: `uint n = -1;`.

#### Decimal numbers

| Type | Precision | Use for |
|---|---|---|
| `float` | about 7 digits | **game math**: positions, speeds, angles, timers |
| `double` | about 15 digits | when you need more accuracy (money totals, very large worlds) |

```cpp
float speed = 2.5;
double distance = 1234567.891;
float tiny = 0.000001;
float big = 1.5e6;      // 1.5 × 10⁶ = 1,500,000
```

Decimal numbers are stored in binary, so some values can't be stored exactly, the same as in every language. `0.1 + 0.2` as a `double` prints `0.30000000000000004`. Never use `==` to check whether two calculated decimal numbers are equal; check whether they're close instead: `math.abs(a - b) < 0.0001`.

When printed, a whole-valued decimal number always shows `.0`, so you can tell it apart from an `int`: `print(5.0)` shows `5.0`, while `print(5)` shows `5`.

#### bool

`true` or `false`. Comparisons give back a `bool`:

```cpp
bool alive = hp > 0;
bool canJump = onGround and not paused;
```

Conditions in `if` and `while` must be `bool`. Writing `if (hp)` is an error; write `if (hp > 0)` or `if (hp != 0)`.

#### string

Text. See Text (see “3. Text”) for everything you can do with it.

```cpp
string name = "Hero";
string greeting = "Hi {name}!";
```

#### Other types

| Type | Chapter |
|---|---|
| `list<T>`, `map<K, V>` | Lists and maps (see “8. Lists and Maps”) |
| your own `struct`, `class`, `enum` | Structs and classes (see “9. Structs and Classes”), Enums (see “10. Enums, Optionals and Pointers”) |
| `Type?` (optional), `Pointer<T>` | Enums, optionals and pointers (see “10. Enums, Optionals and Pointers”) |
| `task` | Tasks (see “11. Tasks”) |
| `vec2`, `vec3`, `vec4`, `color`, `rect` | Built-in types (see “Built-in Types: vec2, vec3, vec4, color, rect”) |

### Converting between types

#### Automatic conversions

Forge converts a value automatically **only when nothing can be lost**:

| From | To | Automatic? |
|---|---|---|
| a smaller whole number type | a bigger one (`int8` → `int`) | yes |
| any whole number | `float` or `double` | yes |
| `float` | `double` | yes |
| a decimal number | a whole number | **no**: decimals would be lost |
| a bigger whole number type | a smaller one (`int` → `int8`) | **no**: it might not fit |
| signed (`int`) | unsigned (`uint`) | **no**: negatives don't fit |
| a number | `string` | **no** |
| `string` | a number | **no** |

```cpp
int whole = 7;
float f = whole;        // fine: 7.0
int back = f;           // error: 'back' needs an int, but this is a float
```

#### Converting yourself

Write the type's name like a function:

| Conversion | Example | Result | Notes |
|---|---|---|---|
| decimal → whole | `int(3.9)` | `3` | The decimals are cut off, not rounded. Use `math.round` first to round. |
| whole → decimal | `float(7)` | `7.0` | |
| bigger → smaller | `int8(300)` | `44` | Wraps around if it doesn't fit |
| text → number | `int("42")`, `float("2.5")` | `42`, `2.5` | Spaces around are ignored. If the text isn't a number, the game stops with a runtime error. |
| anything → text | `string(42)`, `string(true)` | `"42"`, `"true"` | Works for every type, the same as `"{x}"` |
| enum → whole | `int(State.Running)` | its position, from 0 | |
| whole → enum | `State(1)` | the value at that position | |

### Numbers in calculations

When two numbers of different types are combined, the result has the "bigger" type:

| Combination | Result |
|---|---|
| `int` with `float` | `float` |
| `float` with `double` | `double` |
| `int8` with `int` | `int` |
| `int` with `uint` | `int64` (big enough for both) |

#### How division works

**Dividing two whole numbers gives a whole number.** The answer is cut toward zero, and the remainder is thrown away:

| Expression | Result | Why |
|---|---|---|
| `7 / 2` | `3` | 3.5 cut to 3 |
| `-7 / 2` | `-3` | -3.5 cut toward zero |
| `70 / 30` | `2` | 2.33 cut to 2 |
| `7 % 2` | `1` | the remainder |
| `-7 % 3` | `-1` | the remainder takes the sign of the left side |
| `7.0 / 2` | `3.5` | one side is a decimal, so it's decimal division |
| `float(7) / 2` | `3.5` | same |

So to get a decimal answer from two whole numbers, make one of them a decimal first:

```cpp
int coins = 70;
int players = 30;
float each = float(coins) / players;    // 2.3333333
float wrong = coins / players;          // 2.0: the division happened first, with whole numbers
```

Forge gives a **warning** for the `wrong` line (see Warnings (see “12. Errors” › “Warnings”)): the game still builds, but the hint shows the fix, `float(coins) / players`.

**One special case: numbers written straight into the code** (like `70`, as opposed to a variable) take on the type of wherever the result goes. So this gives a decimal answer:

```cpp
float result = 70 / 30;     // 2.3333333: both numbers are written out, and the result goes into a float
print(70 / 30);             // 2: nothing says "float" here, so it's whole-number division
```

The same rule makes `int8 x = 100;` work without converting: `100` takes on the type `int8`. It also means `x * 1000` (with `x` an `int8`) is calculated as an `int8` and wraps around. Convert first if you need a bigger result: `int(x) * 1000`.

**Dividing by zero** with whole numbers stops the game with "Runtime error in main.frg:12: division by zero". With decimal numbers, it gives `inf` (infinity) or `nan` (not a number) instead.

### What's next

Text (see “3. Text”): strings, and putting values inside them.

<!-- docs/language/03-text.md -->

## 3. Text

Text is called a **string** in programming: a "string" of characters. In Forge, the type is `string`, and text values are written between double quotes.

```cpp
string name = "Hero";
string empty = "";
```

Single quotes don't work: `'hi'` is an error. Text also has to start and end on the same line.

### Putting values inside text

Anything inside `{ }` in a string is worked out and turned into text. This is called **interpolation**:

```cpp
int hp = 15;
int maxHp = 20;
print("HP: {hp}/{maxHp}");                   // HP: 15/20
print("Half: {maxHp / 2}");                  // Half: 10
print("Alive? {hp > 0}");                     // Alive? true
print("Name: {name.upper()}");               // Name: HERO
print("Has an e? {name.contains("e")}");     // quotes inside { } are fine
```

Any expression works inside `{ }`, and any type can be shown: numbers, bools, lists, maps, structs, classes, enums and optionals. See How values look as text (see “How values look as text”) below.

#### Choosing how many decimals

Put `:.` and a number from 0 to 9 at the end, inside the `{ }`, to show a decimal number with exactly that many decimals. It rounds to the nearest:

```cpp
float speed = 3.14159;
print("Speed: {speed:.2}");              // Speed: 3.14
print("{speed:.0} m/s");                 // 3 m/s
print("{float(hits) / shots * 100:.1}%");  // 87.5%
```

It only works on `float` and `double`. For a whole number, turn it into one first: `{float(score):.2}`. The ternary still works as usual inside `{ }`: in `{fast ? 1 : 2}` the `:` belongs to the ternary, because a number never starts with a dot.

#### Joining text with +

`+` joins two strings:

```cpp
string full = "Game" + " " + "Over";     // "Game Over"
full += "!";                             // "Game Over!"
```

`+` only works between two strings. `"Score: " + 5` is an error, because Forge won't guess how you want the number written. Use `{ }` instead: `"Score: {5}"`.

### Escapes: special characters

A backslash `\` inside a string starts an **escape**:

| Write | You get |
|---|---|
| `\n` | a new line |
| `\t` | a tab |
| `\"` | a double quote `"` |
| `\\` | a backslash `\` |
| `\{` or `{{` | a `{` (instead of starting a `{value}`) |
| `\}` or `}}` | a `}` |
| `\'` | a single quote |
| `\r` | a carriage return (rarely needed) |

```cpp
print("Line one\nLine two");
print("She said \"hi\"");
print("C:\\Games\\forge");          // C:\Games\forge
print("Use {{braces}} like this");   // Use {braces} like this
```

Any other letter after `\` is an error.

### Raw text

Normal text has to start and end on the same line. For longer text, write it between **three** double quotes, `"""`. This is called **raw text**:

```cpp
string HELP = """
Arrow keys: move
Space: jump
Escape: quit
""";
print(HELP);
```

Raw text is used exactly as written:

- **It can go over many lines.** A line break straight after the opening `"""` is left out, so the text above starts with `Arrow keys`. The line break before the closing `"""` is kept.
- **Escapes don't work.** `\n` stays as a backslash and an `n`. `{ }` doesn't put values in: `"""{score}"""` is the text `{score}`.
- **It can contain `"` and `\`.** The only thing it can't contain is `"""`, which ends it.

That makes raw text the way to write things like shader code for `<OpenGL>` (see “`<OpenGL>`: Drawing with OpenGL Yourself”) and `<DirectX>` (see “`<DirectX>`: Drawing with Direct3D 11 Yourself”), maps made of characters, or Windows paths:

```cpp
string LEVEL = """
##########
#..P....$#
##########
""";
string folder = """C:\Games\forge""";
```

To put values into longer text, add them on with `+`: `"""Score: """ + "{score}"`.

### Comparing text

| | Example | |
|---|---|---|
| Equal | `name == "Hero"` (or `name equals "Hero"`) | Capitals matter: `"Hero" == "hero"` is false |
| Not equal | `name != "Hero"` | |
| Alphabetical order | `"apple" < "banana"` | Compares character by character; capital letters come before lowercase ones |

To compare without caring about capitals: `a.lower() == b.lower()`.

### Text methods

A **method** is a function that belongs to a value, called with a dot: `name.upper()`. Text methods never change the original string; they give back a new one.

| Method | Gives back | What it does | Example | Result |
|---|---|---|---|---|
| `.length` | `int` | How many characters (bytes) there are. No `()`. | `"Hero".length` | `4` |
| `isEmpty()` | `bool` | True if it's `""` | `"".isEmpty()` | `true` |
| `isInt()` | `bool` | True if `int(...)` would work on it | `" 42 ".isInt()` | `true` |
| `isFloat()` | `bool` | True if `float(...)` would work on it (whole numbers count) | `"2.5".isFloat()` | `true` |
| `upper()` | `string` | All capitals | `"Hero".upper()` | `"HERO"` |
| `lower()` | `string` | All lowercase | `"Hero".lower()` | `"hero"` |
| `trim()` | `string` | Removes spaces, tabs and new lines from both ends | `"  hi  ".trim()` | `"hi"` |
| `contains(t)` | `bool` | True if `t` appears anywhere | `"Hello".contains("ell")` | `true` |
| `startsWith(t)` | `bool` | True if it begins with `t` | `"Hello".startsWith("He")` | `true` |
| `endsWith(t)` | `bool` | True if it ends with `t` | `"file.png".endsWith(".png")` | `true` |
| `indexOf(t)` | `int` | The position where `t` first appears (from 0), or `-1` if it doesn't | `"Hello".indexOf("l")` | `2` |
| `replace(a, b)` | `string` | Replaces every `a` with `b` | `"a-b-c".replace("-", "+")` | `"a+b+c"` |
| `split(sep)` | `list<string>` | Cuts it into pieces at every `sep` | `"a,b,c".split(",")` | `["a", "b", "c"]` |
| `substring(start, count)` | `string` | `count` characters starting at position `start` | `"Forge".substring(1, 3)` | `"org"` |
| `charAt(i)` | `string` | The character at position `i`, as one-letter text | `"Forge".charAt(0)` | `"F"` |
| `repeat(n)` | `string` | The text `n` times over | `"ab".repeat(3)` | `"ababab"` |

Notes:

- Positions start at **0**: in `"Forge"`, `F` is at 0 and `e` is at 4.
- `substring` and `charAt` stop the game with a runtime error if the positions are outside the text.
- `split("")` (an empty separator) splits into single characters.
- Text can't be indexed with `[ ]`. Use `charAt(i)`.
- Forge has no `char` type. A single character is a one-letter `string`.
- `.length` counts bytes. Letters with accents and emoji take more than one byte each, so `"é".length` is 2.

#### Example: reading "key=value" lines

```cpp
string line = "speed=320";
list<string> parts = line.split("=");
string key = parts[0];                  // "speed"
float value = float(parts[1]);          // 320.0
```

### Turning text into numbers and back

```cpp
int n = int("42");          // text → whole number
float f = float("2.5");     // text → decimal number
string s = string(99);      // anything → text
string t = "{99}";          // the same thing
```

If the text isn't a valid number (`int("abc")`), the game stops with: `Runtime error in main.frg:7: couldn't turn "abc" into an int`. Spaces around the number are fine.

To avoid that, check first with `isInt()` or `isFloat()`:

```cpp
string typed = input("How many players? ");
if (typed.isInt()) {
    int players = int(typed);
    print("{players} players");
} else {
    print("'{typed}' isn't a whole number");
}
```

### How values look as text

This is what `print(x)` and `"{x}"` produce for each kind of value:

| Value | Shows as |
|---|---|
| whole number | `42` |
| decimal number | `2.5`, `5.0`, `1e+20`, `inf`, `nan` |
| bool | `true` / `false` |
| string | the text itself, with no quotes |
| list | `[1, 2, 3]` |
| map | `{Ana: 12, Ben: 14}` (keys in sorted order) |
| enum | the value's name: `Running` |
| struct or class | `Point(x: 1.0, y: 2.0)`. Classes inside a class show as `<Name>` to keep it short. |
| null | `null` |
| `vec2` / `color` / `rect` | `vec2(1.0, 2.0)`, `color(255, 0, 0, 255)`, `rect(0.0, 0.0, 10.0, 10.0)` |
| task | `task (running)` / `task (done)` |

A struct or class can choose how it's shown by having a method `string toString()`:

```cpp
class Player {
    string name;
    int hp = 100;
    string toString() {
        return "{name} ({hp} HP)";
    }
}
print(Player("Ana", 100));     // Ana (100 HP)
```

### Reading what the player types

In a console program (no window), `input` from `<stdio>` waits for the player to type a line and press Enter:

```cpp
string name = input("What's your name? ");
int age = int(input("How old are you? "));
print("Hi {name}, next year you'll be {age + 1}");
```

If the player might type something that isn't a number, check it with `isInt()` before using `int(...)` (see Turning text into numbers and back (see “Turning text into numbers and back”)).

### What's next

Operators (see “4. Operators”): every way to combine values, in symbols and in words.

<!-- docs/language/04-operators.md -->

## 4. Operators

An **operator** combines or changes values: `+` adds, `>` compares, `and` checks two conditions.

**Every operator in Forge has two spellings: a symbol and a word.** They mean exactly the same thing and can be mixed freely, so use whichever reads better to you:

```cpp
if (hp <= 0 && !invincible) { ... }
if (hp less or equal 0 and not invincible) { ... }     // the same thing
```

### Math

| Symbol | Word | Meaning | Example | Result |
|---|---|---|---|---|
| `a + b` | `a plus b` | add | `7 plus 3` | `10` |
| `a - b` | `a minus b` | subtract | `7 minus 3` | `4` |
| `a * b` | `a times b` | multiply | `7 times 3` | `21` |
| `a / b` | `a div b` | divide | `7 div 2` | `3` (whole numbers, see below) |
| `a % b` | `a mod b` | remainder after dividing | `7 mod 3` | `1` |
| `-a` | `negative a` | flip the sign | `negative 5` | `-5` |

- **Division of two whole numbers gives a whole number**, cut toward zero: `7 / 2` is `3`. Make one side a decimal to get a decimal answer: `7.0 / 2` is `3.5`. See How division works (see “2. Variables and Types” › “How division works”) for the full rules, including the one special case.
- `%` / `mod` gives the remainder: `10 mod 3` is `1`. It's handy for "every Nth time" (`if (frame mod 10 == 0)`) and for wrapping around (`index = (index + 1) mod count`). It works on decimal numbers too: `7.5 mod 2` is `1.5`.
- Dividing a whole number by zero (with `/` or `%`) stops the game with a runtime error.
- `+` also joins two strings: `"Game" + "Over"`.
- `+` and `-` work on vectors (`vec2 + vec2`), and `*` and `/` multiply or divide a vector by a number. See Built-in types (see “Built-in Types: vec2, vec3, vec4, color, rect”).
- **There is no power operator.** `^` means something else in Forge (see Bits (see “Bits”)). To square or cube a number, use `x * x`, `x * x * x`, or `math.pow(x, 2)`.

### Comparing

Comparisons give back `true` or `false`.

| Symbol | Word | Meaning |
|---|---|---|
| `a == b` | `a equals b` | equal |
| `a != b` | `a not equals b` | not equal |
| `a > b` | `a greater b` | greater than |
| `a < b` | `a less b` | less than |
| `a >= b` | `a greater or equal b` | greater than or equal to |
| `a <= b` | `a less or equal b` | less than or equal to |

```cpp
if (score greater or equal 100) { print("You win!"); }
if (name equals "Ana") { ... }
if (lives not equals 0) { ... }
```

What can be compared:

| Values | `==` `!=` | `<` `>` `<=` `>=` |
|---|---|---|
| numbers (any mix of number types) | yes | yes |
| strings | yes | yes (alphabetical order) |
| bools | yes | no |
| enums of the same type | yes | no |
| structs of the same type | yes: every field must match | no |
| classes | yes: true only if both are **the same object** | no |
| lists, maps | yes: same items in the same order | no |
| an optional and `null` | yes: `target == null` | no |

`=` (one equals sign) **sets** a variable; `==` (two) **compares**. Writing `if (x = 5)` is an error that tells you to use `==`.

### Logic

| Symbol | Word | Meaning |
|---|---|---|
| `a && b` | `a and b` | true if **both** are true |
| `a \|\| b` | `a or b` | true if **at least one** is true |
| `!a` | `not a` | flips true and false |

Both sides must be `bool`. `if (hp and alive)` is an error if `hp` is a number; write `if (hp > 0 and alive)`.

**`and` and `or` stop early:** in `a and b`, if `a` is false, `b` isn't even worked out (the answer is already false). That makes this safe:

```cpp
if (target != null and target.hp > 0) { ... }   // target.hp is only read when target isn't null
```

**`not` vs `!`:** they mean the same thing, but `not` is weaker, like in Python. `not a == b` means `not (a == b)`, while `!a == b` means `(!a) == b`. When in doubt, add parentheses.

### Choosing between two values

| Symbol | Word | Meaning |
|---|---|---|
| `condition ? a : b` | `a if condition else b` | `a` when the condition is true, otherwise `b` |
| `maybe ?? backup` | `maybe otherwise backup` | `maybe`, unless it's `null`, in which case `backup` |

```cpp
string label = hp > 50 ? "healthy" : "hurt";
string label2 = "healthy" if hp > 50 else "hurt";      // the same thing (Python style)

int? best = loadBestScore();
int shown = best ?? 0;
int shown2 = best otherwise 0;                          // the same thing
```

Both choices must be the same type, or types that fit together (like `int` and `float`, giving a `float`). For `if ... else`, the word `if` must be on the same line as the value before it. See Optionals (see “10. Enums, Optionals and Pointers”) for more about `null`.

### Changing variables

| Symbol | Word | Meaning |
|---|---|---|
| `x = 5;` | `set x to 5;` | set `x` to 5 |
| `x += 5;` | `increase x by 5;` | add 5 to `x` |
| `x -= 5;` | `decrease x by 5;` | subtract 5 from `x` |
| `x *= 2;` | `multiply x by 2;` | multiply `x` by 2 |
| `x /= 2;` | `divide x by 2;` | divide `x` by 2 (whole-number division if `x` is a whole number) |
| `x %= 3;` | `set x to x mod 3;` | the remainder (no shorter word form) |
| `x++;` | `increase x;` | add 1 |
| `x--;` | `decrease x;` | subtract 1 |

These are statements, so they each go on their own line. They work on anything that can be changed: variables, fields, list items and map entries.

```cpp
set player.hp to 100;
increase scores[0] by 10;
decrease lives;
for (int i = 0; i less 10; increase i) { ... }
```

- `+=` works on strings too: `message += "!";`
- On a vector, `+=` and `-=` take a vector, and `*=` and `/=` take a number: `position += velocity * dt;`
- `++`/`--` (and `increase x;`/`decrease x;`) only work on numbers, and only as a statement on their own. There's no `y = x++;`.

### Bits

These work on the individual binary bits of whole numbers. Most games don't need them.

| Symbol | Word | Meaning | Example | Result |
|---|---|---|---|---|
| `a & b` | `a bitand b` | bits that are 1 in both | `6 & 3` | `2` |
| `a \| b` | `a bitor b` | bits that are 1 in either | `6 \| 3` | `7` |
| `a ^ b` | `a bitxor b` | bits that are 1 in exactly one | `6 ^ 3` | `5` |
| `~a` | `bitnot a` | flip every bit | `~0` | `-1` |
| `a << n` | `a shiftleft n` | move bits left (multiply by 2ⁿ) | `1 << 4` | `16` |
| `a >> n` | `a shiftright n` | move bits right (divide by 2ⁿ) | `256 >> 2` | `64` |

Writing `&` or `|` with two bools is an error that asks "did you mean 'and'?".

### Order of operations

When an expression has several operators, some happen before others, as in school math: `2 + 3 * 4` is `14`, because `*` happens first. From **first** to **last**:

| Order | Operators (symbol / word) |
|---|---|
| 1 (first) | `f()` calls, `a.b` fields, `a[i]` indexing |
| 2 | `-a` / `negative`, `!a` / `not` (written directly before a value), `~a` / `bitnot` |
| 3 | `*` `/` `%` (times, div, mod) |
| 4 | `+` `-` (plus, minus) |
| 5 | `<<` `>>` (shiftleft, shiftright) |
| 6 | `<` `>` `<=` `>=` (less, greater, ...), `is`, `as` |
| 7 | `==` `!=` (equals, not equals) |
| 8 | `&` (bitand) |
| 9 | `^` (bitxor) |
| 10 | `\|` (bitor) |
| 11 | `not` (written before a whole condition) |
| 12 | `&&` / `and` |
| 13 | `\|\|` / `or` |
| 14 | `??` / `otherwise` |
| 15 (last) | `? :` / `if ... else` |

Operators on the same row are worked out left to right: `10 - 3 - 2` is `5`.

**Parentheses always win.** If you're unsure, add them: `(a + b) * c`. One common trap: the bit operators come *after* `==`, so `x & 1 == 0` means `x & (1 == 0)`, which is an error. Write `(x & 1) == 0`.

### The word versions and names

Most word operators only count as operators when they sit **between two values**, which is a place where a plain name can never be. So you can still use these words as names:

```cpp
int times = 3;
int set = 2;
print(times times set);     // 6
```

The statement words (`set`, `increase`, `decrease`, `multiply`, `divide`) only count at the start of a statement that has the matching `to` or `by`, so a function called `set(...)` still works. The only words fully reserved are `negative` and `bitnot`, plus `and`, `or`, `not`, `is` and `as`. See Reserved words (see “13. Reserved Words”).

### What's next

Making decisions and repeating (see “5. Making Decisions and Repeating”): `if`, loops and `switch`.

<!-- docs/language/05-control-flow.md -->

## 5. Making Decisions and Repeating

Programs normally run top to bottom. **Control flow** changes that: running code only sometimes (`if`, `switch`) or many times (`while`, `for`).

### if / else

```cpp
if (hp <= 0) {
    print("Game over");
}
```

The code inside `{ }` runs only if the **condition** in `( )` is true. Add `else` for when it's false, and `else if` to check more conditions in order:

```cpp
if (hp <= 0) {
    print("Game over");
} else if (hp < 10) {
    print("Low health!");
} else {
    print("Fine");
}
```

Only the **first** matching branch runs. If `hp` is 0, you see "Game over" but not "Low health!", even though 0 is also less than 10.

Rules:

- The parentheses around the condition are required.
- The braces are required, even for one line.
- The condition must be a `bool`. `if (hp)` is an error; write `if (hp > 0)`. For optionals, write `if (target != null)`.
- `=` in a condition is an error, because it sets a variable. Use `==` to compare.
- `else` has to come right after the `}` of an `if`.

### while

Repeats as long as the condition is true. It checks the condition before every round, including the first:

```cpp
int countdown = 3;
while (countdown > 0) {
    print(countdown);
    countdown--;
}
print("Go!");
```

If the condition never becomes false, the loop runs forever. That's sometimes on purpose: `while (true) { ... }` with a `break` or `return` inside.

### do ... while

Like `while`, but the condition is checked **after** each round, so the code always runs at least once:

```cpp
string answer = "";
do {
    answer = input("Type yes or no: ");
} while (answer != "yes" and answer != "no");
```

That's handy when you need to do something before you can check it, like asking a question and then checking the answer. Notice the `;` after the `)`: a `do ... while` ends with one.

`break` and `continue` work the same as in `while` (`continue` jumps to the check). Variables made inside the `{ }` are gone by the time the condition is checked, so make them before the `do` if the condition needs them, like `answer` above.

### for

`for` packs a counter into one line:

```cpp
for (int i = 0; i < 5; i++) {
    print(i);          // 0, 1, 2, 3, 4
}
```

| Part | Runs | Here |
|---|---|---|
| start | once, before anything | `int i = 0` |
| condition | before every round; stops when it's false | `i < 5` |
| step | after every round | `i++` |

The counter (`i`) only exists inside the loop. Any of the three parts can be left empty: `for (;;)` loops forever. The word forms work here too: `for (int i = 0; i less 5; increase i)`.

Counting down, or in steps:

```cpp
for (int i = 10; i > 0; i--) { ... }          // 10, 9, ... 1
for (int i = 0; i <= 100; i += 10) { ... }    // 0, 10, 20, ... 100
```

### for ... in: going through a list

```cpp
list<string> names = ["Ana", "Bo", "Cy"];
for (string name in names) {
    print("Hi {name}");
}
```

The variable (`name`) gets each item in turn, from first to last. Its type must match the list's item type, or be a type the items convert to automatically.

- **Structs are copied** into the loop variable, so changing it doesn't change the list. To change items, loop with a counter and use `list[i]`:

  ```cpp
  for (int i = 0; i < points.length; i++) {
      points[i].x += 1;
  }
  ```

- **Classes are shared**, so changes through the loop variable *do* change the objects: `for (Enemy e in enemies) { e.hp -= 1; }` works.
- Adding to the list during the loop is safe; new items are visited too. Removing items during a `for ... in` loop skips items. To remove while looping, go **backwards** with a counter:

  ```cpp
  for (int i = enemies.length - 1; i >= 0; i--) {
      if (enemies[i].hp <= 0) {
          enemies.removeAt(i);
      }
  }
  ```

- To go through a map, loop over its keys: `for (string key in ages.keys()) { print("{key}: {ages[key]}"); }`

### break and continue

- `break` leaves the loop straight away.
- `continue` skips the rest of this round and goes on to the next.

```cpp
for (int i = 0; i < 10; i++) {
    if (i == 2) {
        continue;      // skip 2
    }
    if (i == 5) {
        break;         // stop at 5
    }
    print(i);          // 0, 1, 3, 4
}
```

Both only work inside loops. `break` affects only the loop it's directly in, not any loop around that one.

### switch

`switch` compares one value against a list of cases:

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

- The value can be a whole number, text, a bool or an enum.
- A `case` can list several values, separated by commas.
- `default` runs if no case matches. It's optional, and there can be only one.
- **Each case ends where the next one starts.** Unlike C, C++ and Java, cases never "fall through" into the next one, so **`break` isn't needed**. In fact `break` directly inside a switch is an error, because it would be confusing: in Forge, `break` only leaves loops.
- A case can contain any statements, including loops and other switches.

```cpp
string command = input("> ");
switch (command) {
    case "quit", "exit":
        running = false;
    case "help":
        showHelp();
    default:
        print("Unknown command");
}
```

### return

`return` leaves the current function immediately. See Functions (see “6. Functions”). Inside a loop, it also leaves the loop.

```cpp
int findFirstNegative(list<int> nums) {
    for (int i = 0; i < nums.length; i++) {
        if (nums[i] < 0) {
            return i;      // found it: stop looking
        }
    }
    return -1;             // not found
}
```

### What's next

Functions (see “6. Functions”): naming a piece of code so you can use it again.

<!-- docs/language/06-functions.md -->

## 6. Functions

A **function** is a named piece of code you can run ("call") whenever you need it. Functions keep programs short (write something once, use it many times) and readable (a good name explains what the code does).

### Writing a function

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

### void: functions that give nothing back

```cpp
void greet(string name){
    print("Hi {name}!");
}

greet("Ana");
```

`void` means "nothing". A `void` function can use `return;` (with no value) to stop early. Using its result, as in `int x = greet("Ana");`, is an error: "this doesn't give back a value".

### return

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

### Parameters

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

#### What a function can change

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

To change a number or a list for the caller, either return the new value, or pass a `Pointer`. See Pointers (see “10. Enums, Optionals and Pointers” › “Pointers”).

#### Default values

A parameter can have a value to use when the caller leaves it out:

```cpp
void shout(string text, int times = 3){
    print(text.repeat(times));
}

shout("hey");         // heyheyhey
shout("hey", 1);      // hey
```

Once a parameter has a default, every parameter after it needs one too.

### Order doesn't matter

A function can be called from anywhere in the game, above or below where it's written, and from other files (with the file's name in front; see Files and folders (see “7. Files and Folders”)). Forge reads all the code before checking any of it.

### Functions calling themselves

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

### Rules

- Each function name can be used only once per file. Two functions with the same name in one file is an error, even with different parameters.
- A function's name can't be a reserved word (see “13. Reserved Words”), a built-in function name (`print`, `wait`), or the name of one of your top-level files.
- Functions are always written at the top level of a file (or inside a struct or class, where they're called methods). You can't write a function inside another function.
- Functions can't be stored in variables or passed to other functions.

### Methods

Functions written inside a `struct` or `class` are called **methods**, and they're called on a value: `player.jump()`. See Structs and classes (see “9. Structs and Classes”).

### Tasks

A function whose return type is `task` can pause itself with `wait(seconds)` while the rest of the game keeps running. See Tasks (see “11. Tasks”).

### What's next

Files and folders (see “7. Files and Folders”): splitting a game across several files.

<!-- docs/language/07-files-and-folders.md -->

## 7. Files and Folders

Small programs fit in `main.frg`. Bigger games are easier to work with when they're split up: one file for the player, one for enemies, one for the level. Forge makes this simple.

### Files next to main.frg

**Every `.frg` file in the same folder as `main.frg` is part of the game automatically.** There's no need to list or include them.

```
mygame/
  main.frg
  player.frg
  level.frg
```

### The file-name prefix rule

To use something from another file, **start with that file's name and a dot**. This works for functions, global variables and types:

```cpp
// player.frg
int score = 0;

void jump(float height){
    print("jumping {height}");
}

class Player {
    int hp = 100;
}
```

```cpp
// main.frg
#include <stdio>

int main(){
    player.jump(2.5);                         // a function from player.frg
    player.score += 100;                      // a global variable from player.frg
    player.Player p = player.Player();        // a type from player.frg
    print(player.score);
    return 0;
}
```

- Things in the **same** file are used without a prefix.
- The prefix is **always required** for other files, even when there's no confusion. That way, `player.jump()` always tells you exactly where `jump` comes from.
- Forgetting the prefix gives a helpful error: "unknown name 'jump' - 'jump' is in player.frg - use player.jump".
- Because the file name is used like this, nothing else can share that name. With a `player.frg` in the top folder, a variable called `player` is an error.
- Each file has its own names, so two files can both have a function called `update`. They're `player.update()` and `enemy.update()`.

### Folders

A folder's files only join the game when a file **in the folder above it** includes the folder by name, in quotes:

```
mygame/
  main.frg               #include <"enemies">
  enemies/
    goblin.frg           #include <"bosses">
    orc.frg
    bosses/
      dragon.frg
```

```cpp
// main.frg
#include <"enemies">
```

```cpp
// enemies/goblin.frg
#include <"bosses">      // bosses/ is next to goblin.frg, so goblin.frg (or orc.frg) includes it
```

You reach things in folders by their **full path from the top folder**, from anywhere in the game:

```cpp
enemies.goblin.spawn();
enemies.bosses.dragon.roar();
enemies.orc.Orc o = enemies.orc.Orc();
```

The full path is used even between files in the same folder: inside `goblin.frg`, `orc.frg`'s functions are `enemies.orc.something()`.

Rules:

- A folder only needs to be included once, by any one file next to it.
- Include one folder at a time: `#include <"enemies/bosses">` doesn't work. Files inside `enemies` include `bosses` themselves.
- Using a folder that isn't included is an error that tells you which `#include` to add.
- A folder and a file in the same place can't have the same name (`enemy.frg` and `enemy/`).
- File and folder names may only use letters, numbers and `_`, can't start with a number, and can't be a keyword, type name or library name (so not `map.frg`, `class.frg`, `window.frg` or `math.frg`).

### Libraries

Libraries are included with angle brackets and **no** quotes:

```cpp
#include <stdio>
#include <window>
```

**Each file includes the libraries it uses.** If `player.frg` calls `print`, then `player.frg` needs `#include <stdio>`, even if `main.frg` has it too. Forgetting gives: "'print' is part of the stdio library, which this file doesn't include - add #include <stdio> at the top of player.frg".

Library functions keep the library's name: `window.open(...)`, `math.sqrt(2)`. The exceptions are `<stdio>`'s functions (`print`, `write`, `input`), which are used on their own. Types from a library (`Key`, `Body`, `Texture`, ...) are used by their plain name once the library is included.

See the library list (see “Library Overview”).

### private: keeping things inside a file

Put `private` in front of a function, global variable or type to make it usable **only inside its own file**:

```cpp
// player.frg
private int jumpsUsed = 0;          // only player.frg can see this

private void resetJumps(){          // ...or call this
    jumpsUsed = 0;
}

void land(){                        // other files can call player.land()
    resetJumps();
}
```

From another file, `player.jumpsUsed` is an error: "'jumpsUsed' is private to player.frg". This is useful for helper functions and values that other files shouldn't touch. `public` means the opposite (usable everywhere), which is also what you get with nothing written.

### Where things run

- Global variables in every file are created before `main` starts, in file order (main.frg first, then the others alphabetically, then folders as they're included).
- A global variable's starting value can call functions, even from other files. However, a global's starting value shouldn't depend on another file's global, because that one might not be created yet.

### What's next

Lists and maps (see “8. Lists and Maps”): holding many values in one variable.

<!-- docs/language/08-lists-and-maps.md -->

## 8. Lists and Maps

Games deal with *many* things: enemies, coins, high scores, inventory items. **Lists** hold many values in order, and **maps** look values up by a key.

### Lists

A `list<T>` holds any number of values of type `T`, in order.

```cpp
list<int> scores = [10, 25, 7];
list<string> names = ["Ana", "Bo"];
list<Enemy> enemies = [];          // empty
list<list<int>> grid = [[1, 2], [3, 4]];
```

- The type in `< >` is the type of every item. All items must be that type, or convert to it automatically.
- `[ ]` with values makes a list. If the type can't be worked out (an empty `[]` with no type written), you must write it: `list<int> xs = [];`.
- Without a type written, `[1, 2.5]` is a `list<float>`, because the numbers adapt to each other.

#### Positions

Items are numbered from **0**: the first item is at 0, and the last is at `length - 1`.

```cpp
list<string> names = ["Ana", "Bo", "Cy"];
print(names[0]);           // Ana
print(names[2]);           // Cy
names[1] = "Ben";          // change an item
scores[0] += 5;            // change an item in place
```

Using a position that isn't in the list stops the game:

```
Runtime error in main.frg:9: position 5 is outside the list (it has 3 items, at positions 0 to 2)
```

#### List methods

| Method | Gives back | What it does |
|---|---|---|
| `.length` | `int` | how many items (no `()`) |
| `add(x)` | — | puts `x` at the end |
| `addAll(other)` | — | puts every item of the list `other` at the end, in order: `[1, 2]` then `addAll([3, 4])` gives `[1, 2, 3, 4]` |
| `insert(i, x)` | — | puts `x` at position `i`, moving later items along. `i` can be `length`, which is the same as `add`. |
| `removeAt(i)` | the item | takes out the item at position `i` and gives it back |
| `remove(x)` | `bool` | takes out the first item equal to `x`; true if one was found |
| `contains(x)` | `bool` | true if any item equals `x` |
| `indexOf(x)` | `int` | the position of the first item equal to `x`, or `-1` |
| `first()` | the item | the first item |
| `last()` | the item | the last item |
| `pop()` | the item | takes out the last item and gives it back |
| `clear()` | — | removes everything |
| `isEmpty()` | `bool` | true if there are no items |
| `sort()` | — | puts the items in order, smallest first (lists of numbers or text only) |
| `reverse()` | — | flips the order |
| `copy()` | a list | a separate copy of the list |

`first()`, `last()`, `pop()`, `removeAt()` and `insert()` stop the game with a runtime error if the list is empty or the position is wrong.

"Equal" means `==`: numbers and text by value, structs field by field, and classes only when it's the very same object.

#### Going through a list

```cpp
for (string name in names) {
    print(name);
}

for (int i = 0; i < names.length; i++) {
    print("{i}: {names[i]}");
}
```

Use the counter version when you need the position, or want to change struct items (`points[i].x += 1`). See for ... in (see “5. Making Decisions and Repeating” › “for ... in: going through a list”) for removing items while looping.

#### Copying

Assigning a list to another variable, or passing it to a function, makes a **copy**:

```cpp
list<int> a = [1, 2];
list<int> b = a;
b.add(3);
print(a);     // [1, 2] - a didn't change
```

If the items are **class** objects, the copy holds the same objects (the list is copied, the objects aren't).

#### Comparing and printing

`a == b` is true if both lists have equal items in the same order. `print([1, 2, 3])` shows `[1, 2, 3]`.

### Maps

A `map<K, V>` stores values (`V`) under keys (`K`). Each key appears at most once.

```cpp
map<string, int> ages = {"Ana": 12, "Ben": 14};
map<int, string> levelNames = {1: "Forest", 2: "Cave"};
map<string, list<int>> scoresByPlayer = {};
```

- Keys can be whole numbers, text, bools or enums.
- `{ key: value, ... }` makes a map. `{}` is an empty map (the type must be written).

#### Reading and writing

```cpp
ages["Cy"] = 9;            // adds a new key (or replaces the value if it exists)
print(ages["Ana"]);        // 12
ages["Ana"] += 1;          // 13
```

- **Reading** a key that isn't in the map stops the game: "the key isn't in the map (use .has(key) to check first, or .get(key, backup))".
- **Writing** to a key that isn't there creates it. That includes `+=`: `counts["new"] += 1` creates `"new"` at 0 and then adds 1, which makes counting easy.

#### Map methods

| Method | Gives back | What it does |
|---|---|---|
| `.length` | `int` | how many keys |
| `has(key)` | `bool` | true if the key is in the map |
| `get(key, backup)` | the value | the value for `key`, or `backup` if it isn't there |
| `remove(key)` | `bool` | takes out the key and its value; true if it was there |
| `keys()` | `list<K>` | all the keys, in sorted order |
| `values()` | `list<V>` | all the values, in the same order as `keys()` |
| `clear()` | — | removes everything |
| `isEmpty()` | `bool` | true if there are no keys |

#### Going through a map

```cpp
for (string name in ages.keys()) {
    print("{name} is {ages[name]}");
}
```

Maps always keep their keys in sorted order (numbers smallest first, text alphabetically).

#### Example: counting

```cpp
map<string, int> counts = {};
list<string> drops = ["coin", "gem", "coin", "coin"];
for (string d in drops) {
    counts[d] += 1;
}
print(counts);      // {coin: 3, gem: 1}
```

#### Copying, comparing and printing

Like lists, maps are **copied** when assigned or passed to functions. `==` compares every key and value. `print` shows `{Ana: 12, Ben: 14}`.

### Lists and maps inside other things

Lists and maps can hold anything, including other lists and maps, structs and classes, and they can be fields of structs and classes:

```cpp
class Inventory {
    map<string, int> items = {};
    void addItem(string name) {
        items[name] += 1;
    }
}

list<list<int>> grid = [[0, 0, 0], [0, 1, 0]];
grid[1][1] = 5;
```

### What's next

Structs and classes (see “9. Structs and Classes”): making your own types.

<!-- docs/language/09-structs-and-classes.md -->

## 9. Structs and Classes

So far every value has been a single number, some text, or a list. Games need *things*: a player with a name, health and a position, or an enemy with a type and speed. **Structs** and **classes** let you make your own types that bundle values together, along with the functions that work on them.

### A first struct

```cpp
struct Point {
    float x;
    float y;
}

Point p = Point(3, 4);
print(p.x);              // 3.0
p.y = 10;
print(p);                // Point(x: 3.0, y: 10.0)
```

- `x` and `y` are **fields**: variables that belong to each `Point`.
- `p.x` reads or changes a field.
- Types are written at the top level of a file, never inside a function.

### Fields

```cpp
class Enemy {
    string name;
    int hp = 100;              // a starting value
    float speed = 2.5;
    list<string> loot = [];
    Enemy? target;             // optional, starts as null
}
```

- A field without a starting value starts at its type's zero (`0`, `false`, `""`, empty list).
- **A field whose type is a class must have a starting value or be optional**, because a class has no zero: `Weapon weapon = Weapon();` or `Weapon? weapon;`.
- A field's starting value can't use other fields. It's worked out before the object exists.

### Methods

Functions written inside a type are **methods**. They're called on a value with a dot, and they can use that value's fields directly:

```cpp
struct Point {
    float x;
    float y;

    float length() {
        return math.sqrt(x * x + y * y);
    }

    void moveBy(float dx, float dy) {
        x += dx;
        y += dy;
    }
}

Point p = Point(3, 4);
print(p.length());       // 5.0
p.moveBy(1, 1);
```

#### this

Inside a method, **`this`** means "the value this method was called on". You usually don't need it, since `x` already means this point's `x`. It's needed when a parameter has the same name as a field:

```cpp
void setX(float x) {
    this.x = x;          // the field = the parameter
}
```

### Making values: constructors

#### Without a constructor

A type with no constructor can be made in two ways:

```cpp
Point a = Point();          // every field at its starting value
Point b = Point(3, 4);      // every field, in the order they're written
Point c = {3, 4};           // the same as Point(3, 4)
```

It's either none of the fields, or all of them; giving only some is an error that lists the fields in order.

#### Constructors

A **constructor** is a special method with the type's name and no return type. It runs when a value is made, after the fields get their starting values:

```cpp
class Enemy {
    string name;
    int hp = 100;

    Enemy(string name) {
        this.name = name;
    }
}

Enemy e = Enemy("Grok");
```

Once a type has a constructor, values must be made through a constructor (`Enemy()` with nothing is an error unless there's a constructor that takes nothing).

#### Several constructors

A type can have several constructors, as long as each takes **different types** of values. Forge picks the one that fits what you give it:

```cpp
struct Size {
    float w;
    float h;

    Size(float side) {          // a square
        w = side;
        h = side;
    }

    Size(float w, float h) {
        this.w = w;
        this.h = h;
    }
}

Size square = Size(3);          // uses the first
Size wide = Size(4, 2);         // uses the second
```

Two constructors that take the same types are an error. Constructor parameters can have default values, like any function's.

### Structs vs classes: copying vs sharing

Structs and classes look almost the same. The one big difference is **what happens when you copy one**:

| | struct | class |
|---|---|---|
| `b = a;` | `b` is a **separate copy** | `b` is **another name for the same object** |
| passed to a function | the function gets a copy | the function gets the same object |
| `a == b` | true if every field is equal | true only if they're the very same object |
| inheritance | no | yes |
| can be `null` | only as `Type?` | only as `Type?` |
| methods can be tasks | no | yes |

```cpp
struct PointS { float x; }
class PointC { float x; }

PointS s1 = PointS(1);
PointS s2 = s1;
s2.x = 99;
print(s1.x);       // 1.0 - s1 is untouched

PointC c1 = PointC(1);
PointC c2 = c1;
c2.x = 99;
print(c1.x);       // 99.0 - same object
```

**Rule of thumb:** use a **struct** for small pieces of data that are just values, like a position, a size, a color or a score entry. Use a **class** for game objects that have an identity and get changed from many places: the player, enemies, items, the level.

#### Things to know about structs

- In `for (Point p in points)`, `p` is a copy. Change items with `points[i].x = ...`. (Forge gives a warning if you change `p` inside the loop.)
- `getPoint().x = 5;` is an error, because `getPoint()` gives back a temporary copy that would be thrown away. Store it in a variable first.
- A struct can't contain itself, directly or as `Node?`, because it would be infinitely big. A struct can contain a `list` of itself. If you need "a thing that points to another thing of the same kind", use a class.

#### When class objects go away

You never delete objects yourself in Forge. A class object stays alive as long as **something still refers to it**: a variable, a field, or a list. When the last one lets go (the variable ends, the field is set to something else, the item is removed from the list), the object is freed straight away.

There's one case where that never happens: **two objects that refer to each other**.

```cpp
class Enemy {
    Enemy? partner = null;
}

void makePair(){
    Enemy a = Enemy();
    Enemy b = Enemy();
    a.partner = b;
    b.partner = a;
}   // a and b end here, but each one is still referred to by the other's partner field
```

After `makePair` ends, nothing in your game can reach `a` or `b` any more, but they keep each other alive, so they're never freed while the game runs. (The same goes for a longer loop: A refers to B, B to C, C back to A.) All the memory is given back when the game closes.

**When does it matter?** Rarely, in small games. A few objects left over use a tiny bit of memory and nothing else: they aren't drawn or updated unless they're still in one of your lists. It only adds up if a loop like this is made **over and over**, for example for every bullet, every frame, in a game that runs for a long time.

**How to avoid it:** when you're done with one of the objects, set one side of the link to `null`:

```cpp
void removeEnemy(Enemy e){
    e.partner = null;      // break the loop first
    enemies.remove(e);
}
```

A common example is a parent and its children, like a team and its players. Let the team hold a `list<Player>`, and give each `Player` a `Team? team` field. When a player leaves the team, set its `team = null` as well as removing it from the list.

### Inheritance

A class can **inherit** from another class with `:`. It gets all the parent's fields and methods, and can add its own:

```cpp
class Entity {
    string name;
    int hp = 100;

    Entity(string name) {
        this.name = name;
    }

    void takeHit(int amount) {
        hp -= amount;
    }

    string describe() {
        return "an entity";
    }
}

class Goblin : Entity {
    int gold = 5;              // extra field

    string describe() {        // replaces Entity's version
        return "a goblin carrying {gold} gold";
    }
}

Goblin g = Goblin("Grok");     // Goblin has no constructor, so it uses Entity's
g.takeHit(10);                 // inherited from Entity
print(g.describe());           // a goblin carrying 5 gold
```

- **A child can go anywhere its parent can.** An `Entity` variable, parameter or list item can hold a `Goblin`:

  ```cpp
  list<Entity> everyone = [Goblin("Grok"), Entity("Blob")];
  for (Entity e in everyone) {
      print(e.describe());       // each one uses its own version
  }
  ```

- **Replacing a method:** write a method with the same name, the same parameter types and the same return type. Calls use the real object's version, even through an `Entity` variable. A replacement that doesn't match exactly is an error that shows what it should look like.
- A child can't add a field with the same name as one of its parent's fields.
- Only classes inherit; `struct A : B` is an error. A class inherits from one parent, and that parent can have its own parent.
- You can inherit from some built-in classes too, like `Body` (see “`<physics>`: Boxes, Gravity and Collisions”), `Scene` (see “`<scenes>`: Menus, Levels and Game Over”) and `Component` (see “`<ECS>`: Entities and Components”).

#### super

**`super`** reaches the parent class's version of things:

```cpp
class Player : Entity {
    int score = 0;

    Player(string name) {
        super(name);                   // run Entity's constructor
        hp = 150;
    }

    string describe() {
        return "the player, " + super.describe();    // Entity's describe()
    }
}
```

- `super(...)` runs a parent constructor. It can only be used inside a constructor.
- `super.method()` calls the parent's version of a method, even if this class replaced it.

#### Which constructor is used

- If a class has its own constructors, one of those is used.
- If not, it uses its parent's constructors (or its grandparent's, and so on).
- If none of them have any, it's made with `Type()` or `Type(every field in order)`, where inherited fields come first.

### is and as: what kind of object is it?

When a variable's type is a parent class, it might hold any of its children. **`is`** checks which:

```cpp
for (Entity e in everyone) {
    if (e is Goblin) {
        print("{e.name} is a goblin");
    }
}
```

**`as`** converts to the child type. It gives back an **optional**, which is `null` if the object isn't one:

```cpp
Goblin? g = e as Goblin;
if (g != null) {
    print(g.gold);
}

// or, once you've checked with `is`:
if (e is Goblin) {
    Goblin g2 = e as Goblin;     // fine: as gives Goblin?, which is used as a Goblin
    print(g2.gold);
}
```

- `e is Goblin` is also true for classes that inherit from `Goblin`.
- `is` and `as` only work with classes, and only between related classes. `cat is Car` is an error: "a Cat can never be a Car".
- If `e` is `null`, `e is X` is false and `e as X` is `null`.

### private

`private` in front of a field or method makes it usable **only inside the class's own methods** (and the methods of classes that inherit from it):

```cpp
class BankAccount {
    private int balance = 0;

    void deposit(int amount) {
        if (amount > 0) {
            balance += amount;
        }
    }

    int getBalance() {
        return balance;
    }
}

BankAccount acc = BankAccount();
acc.deposit(50);
print(acc.getBalance());     // 50
acc.balance = 1000000;       // error: 'balance' is private to BankAccount
```

Private things stop other code from changing values in ways that break the rules (here, no negative deposits). Constructors can be private too. `public` (the default) can be written to be explicit.

### How objects look when printed

`print(obj)` and `"{obj}"` show the type and every field:

```
Point(x: 3.0, y: 4.0)
Enemy(name: Grok, hp: 100, speed: 2.5, loot: [], target: null)
```

Inside a class's output, other class objects show as `<Name>` (so two objects pointing at each other don't print forever).

To choose what's shown, add a method `string toString()`:

```cpp
class Enemy {
    string name;
    int hp = 100;

    string toString() {
        return "{name} ({hp} HP)";
    }
}
```

### Types from other files

Types follow the file-name prefix rule (see “7. Files and Folders”):

```cpp
entities.Player p = entities.Player("Ana");
list<entities.Player> party = [];
class Boss : enemies.goblin.Goblin { ... }
```

### Rules summary

- Types are written at the top level of a file.
- Field and method names must all be different within a type (and from its parents' fields).
- A field of class type needs a starting value or must be optional.
- Methods can call each other by name, and can use fields by name or through `this`.
- Methods can't have the same name as each other. Only constructors can share a name.
- Class objects are cleaned up automatically once nothing uses them anymore. There's no need (and no way) to delete them.

### What's next

Enums, optionals and pointers (see “10. Enums, Optionals and Pointers”).

<!-- docs/language/10-enums-optionals-pointers.md -->

## 10. Enums, Optionals and Pointers

### Enums

An **enum** is a type with a fixed list of named values. Use one whenever something is "one of these few choices": a game state, a direction, an enemy kind.

```cpp
enum State { Idle, Running, Jumping }

State s = State.Idle;
if (s == State.Idle) {
    s = State.Running;
}
print(s);                  // Running
```

- Values are written `TypeName.Value`.
- Enums can be compared with `==` and `!=`, used in `switch`, and used as map keys. They can't be compared with `<` or `>`.
- `print` shows the value's name.
- Each value has a number, its position counting from 0: `int(State.Running)` is `1`, and `State(2)` is `Jumping`. A number with no matching value prints as `State(?)`.
- An enum needs at least one value, and each value can appear only once. A trailing comma is allowed.
- A variable of an enum type made without a value starts as the first value.

```cpp
enum Direction { Up, Down, Left, Right }

vec2 step(Direction d) {
    switch (d) {
        case Direction.Up:
            return vec2(0, -1);
        case Direction.Down:
            return vec2(0, 1);
        case Direction.Left:
            return vec2(-1, 0);
        default:
            return vec2(1, 0);
    }
}
```

### Optionals and null

Sometimes a value might be **missing**: an enemy might not have a target yet, or a search might find nothing. Add **`?`** to any type to make it **optional**, meaning it can also hold **`null`** (nothing):

```cpp
Enemy? target = null;
int? bestScore = null;
string? nickname = "Ace";
```

#### Only optionals can be null

```cpp
int score = null;        // error: null can only go in an optional type - add ? to the type, like int?
```

This is on purpose: in Forge, a plain `Enemy` **always** holds an enemy. Only types you mark with `?` can be empty, so you always know where to check.

#### Checking for null

```cpp
if (target != null) {
    target.takeHit(5);
}

if (bestScore == null) {
    print("No best score yet");
}
```

`if (target)` is an error; compare with `null`.

#### Using an optional

An optional can be used anywhere its plain type is needed: reading a field, calling a method, doing math, passing it to a function. **If it's `null` at that moment, the game stops** with a message naming the exact place:

```
Runtime error in main.frg:14: 'target' is null
```

So check with `!= null` first, unless you're sure it can't be null.

```cpp
int? maybe = 5;
int sure = maybe;          // fine: maybe isn't null right now
int doubled = maybe * 2;   // fine
```

#### Backup values: ?? / otherwise

```cpp
int shown = bestScore ?? 0;                  // bestScore, or 0 if it's null
string name = nickname otherwise "Player";   // the word version
```

If the backup is optional too, the result is optional: `a ?? b ?? 0`.

#### Functions that might not find anything

```cpp
int? findIndex(list<string> items, string wanted) {
    for (int i = 0; i < items.length; i++) {
        if (items[i] == wanted) {
            return i;
        }
    }
    return null;
}

int? where = findIndex(names, "Bo");
if (where != null) {
    print("found at {where}");
}
```

#### More about optionals

- `Type?` variables made without a value start as `null`.
- `print(null)` and printing a null optional show `null`.
- A plain value converts to an optional automatically: `int? x = 5;`.
- `obj as Type` gives an optional (see is and as (see “9. Structs and Classes” › “is and as: what kind of object is it?”)).
- Fields of class type are often optional: `Enemy? target;` (otherwise they need a starting value).

### Pointers

A `Pointer<T>` points at a variable, so you can read and change that variable from somewhere else. `.value` is the variable it points at:

```cpp
void addOne(Pointer<int> counter) {
    counter.value += 1;
}

int score = 10;
addOne(score);              // score is now 11

Pointer<int> p = score;     // p points at score
p.value = 50;               // score is now 50
print(score);               // 50
```

- Making a pointer from a variable is automatic: write the variable, and Forge points at it.
- A pointer can point at a variable, a field (`player.hp`) or a list item (`scores[0]`), but not at a calculated value like `score + 1`.
- For a pointer to a struct, fields can be used directly: `Pointer<Point> pp = p; pp.x = 5;`.
- A `Pointer<T>` must start pointing at something. `Pointer<T>?` can be `null`.

#### Throwing a pointer away: remove()

`p.remove()` makes the pointer point at nothing. The thing it pointed at isn't changed; only the pointer lets go of it. After that, `p.isRemoved()` is `true`, and using `p` stops the game with a clear message instead of doing something strange:

```cpp
int score = 10;
Pointer<int> p = score;
p.remove();
print(p.isRemoved());    // true
print(score);            // 10 - score itself is fine
p.value = 5;             // stops the game: "this pointer was removed (with remove()), so it doesn't point at anything any more"
p = score;               // pointing it at something again makes it usable again
```

| | What it does |
|---|---|
| `p.remove()` | makes `p` point at nothing |
| `p.isRemoved()` | `true` after `remove()`, until `p` is pointed at something again |

If a pointer points at a list or a map, `p.remove(x)` **with** a value is still the list's (or map's) own `remove`. Only `p.remove()` with nothing in the brackets is the pointer's.

#### When to use pointers

Rarely. Class objects are already shared (a function that gets a `Player` can change it), so pointers are only needed to let a function change a caller's **number, bool, string, struct or list** variable. Often it's clearer to return the new value instead:

```cpp
int addOne(int n) {
    return n + 1;
}
score = addOne(score);
```

#### Danger: don't keep pointers to things that go away

A pointer to a function's local variable stops being valid when that function ends, and the same goes for a pointer to a list item once the list grows or shrinks. Forge can't always catch this. Using such a pointer gives garbage values or a crash. Keep pointers short-lived: pass them into a function, and don't store them. When you do keep one and you're done with it, `p.remove()` it, so using it by mistake gives a clear error.

### What's next

Tasks (see “11. Tasks”): doing things over time without freezing the game.

<!-- docs/language/11-tasks.md -->

## 11. Tasks

Games are full of things that happen **over time**: flash for half a second, wait 3 seconds and then spawn a wave, count down 3, 2, 1. Writing these with timers and frame counters gets messy fast. **Tasks** let you write them top to bottom, as if the game paused, without actually pausing the game.

### A first task

```cpp
task flash() {
    tint = color.WHITE;
    wait(0.2);              // this task pauses for 0.2 seconds...
    tint = color.ORANGE;    // ...then carries on from here
}
```

A function whose return type is **`task`** can call **`wait(seconds)`**. Only that task pauses; every other part of the game (drawing, input, other tasks) keeps running.

**Calling a task starts it.** It runs straight away until its first `wait`, then the call returns and your code carries on. The task picks up where it left off once the time has passed:

```cpp
void respawn() {
    position = START;
    flash();                // starts flashing, and returns immediately
    print("respawned");     // runs right away, while the flash is still going
}
```

### wait

| Call | Pauses the task until |
|---|---|
| `wait(seconds)` | that much game time has passed (`wait(0.5)` is half a second) |
| `wait()` | the next frame |
| `wait(otherTask)` | `otherTask` has finished |

`wait` can only be used inside a `task` function; anywhere else it's an error that suggests changing the function's return type to `task`.

### Keeping hold of a task

Calling a task gives back a `task` value. Keep it if you want to check on it or stop it:

```cpp
task t = countdown();

if (t.isDone()) { ... }     // has it finished?
t.stop();                   // stop it early: it never continues
```

| Method | What it does |
|---|---|
| `isDone()` | `true` once the task has finished (or was stopped) |
| `stop()` | stops the task; it won't continue from its `wait` |

A `task` variable made without a value counts as already done. `print(t)` shows `task (running)` or `task (done)`.

### Examples

#### Countdown

```cpp
task countdown() {
    for (int i = 3; i > 0; i--) {
        message = "{i}";
        wait(1);
    }
    message = "GO!";
    wait(1);
    message = "";
}
```

#### Spawning waves forever

```cpp
task spawnWaves() {
    int wave = 1;
    while (true) {
        for (int i = 0; i < wave * 3; i++) {
            spawnEnemy();
            wait(0.4);
        }
        wave++;
        wait(10);
    }
}
```

A task that never ends is fine; it just keeps going alongside the game. Keep the `task` value if you might want to `stop()` it later.

#### One task waiting for another

```cpp
task bossIntro() {
    task text = showText("A wild boss appears!");
    wait(text);                 // carry on when showText has finished
    bossActive = true;
}
```

#### Tasks in a class

Methods of a **class** can be tasks. The object stays alive while its task runs, even if nothing else holds on to it:

```cpp
class Player : Body {
    color tint = color.ORANGE;

    task flash() {
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
}
```

Struct methods can't be tasks, because a struct gets copied around, so there's no single "it" to come back to.

### Rules

- `return;` ends a task early. A task can't return a value.
- A task's parameters are copies, kept for as long as the task runs.
- Tasks run on the same "thread" as the rest of the game. Nothing runs at exactly the same moment, so there's no need to worry about two things changing a variable at once. A task's code just slots in between frames.

### When tasks run

**With a window** (`<window>`): at the start of every frame, any task whose wait is over continues, before your `update` / `mainLoop` code runs. Time is measured in game frames' time (`window.deltaTime()`).

**Without a window** (console programs), nothing moves tasks forward automatically. Include `<tasks>` and either:

- call `tasks.update(dt)` yourself, which moves time forward by `dt` seconds and continues any task that's due; or
- call `tasks.runAll()`, which really waits, in real time, until every task has finished.

```cpp
#include <stdio>
#include <tasks>

task hello() {
    print("Hello...");
    wait(1);
    print("...world!");
}

int main(){
    hello();
    tasks.runAll();          // waits one second, then prints "...world!"
    return 0;
}
```

| `<tasks>` function | What it does |
|---|---|
| `tasks.update(float dt)` | moves task time forward by `dt` seconds |
| `tasks.runAll()` | keeps going (really waiting) until no tasks are left |
| `tasks.count()` | how many tasks are waiting |
| `tasks.stopAll()` | stops every waiting task |

### Tasks vs timers

For simple "has enough time passed?" checks, a `Timer` (see “`<time>`: Clock, Date and Timers” › “Timer”) can be simpler (`if (cooldown.isDone()) { ... }`). Use a task when there's a **sequence**: do this, wait, do that, wait, do something else.

### What's next

Errors (see “12. Errors”): reading Forge's error messages.

<!-- docs/language/12-errors.md -->

## 12. Errors

Everyone's code has mistakes. Forge's job is to find them early and explain them plainly. There are two kinds:

| Kind | When | Example |
|---|---|---|
| **Compile errors** | before the game runs, while Forge checks your code | a missing `;`, a misspelled name, a float put into an int |
| **Runtime errors** | while the game is running | a list position that doesn't exist, using something that's null |

Most mistakes are compile errors, so they're caught before you ever see the game.

Forge also gives **warnings**: code that works, but probably doesn't do what you meant. A warning never stops the game from building. See Warnings (see “Warnings”) below.

### Reading a compile error

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

### Common compile errors

#### Writing code

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
| `unknown escape '\x'` | a `\` followed by an unknown letter | see escapes (see “3. Text” › “Escapes: special characters”); write `\\` for a backslash |
| `this { is never closed` (inside text) | a `{` in a string starts a `{value}` | write `{{` or `\{` for a literal `{` |
| `Forge doesn't need function declarations` | `void jump();` without a body | just write the function with its `{ }` body, anywhere |
| `make one variable per line` | `int a, b;` | `int a; int b;` |

#### Names

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

#### Types

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

#### Functions and types

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

#### Libraries and files

| Error | What it means | Fix |
|---|---|---|
| `there's no library called <stdoi>` + did you mean | a typo in an include | `#include <stdio>` |
| `the <network> library isn't built yet` | it's planned, but doesn't exist yet | |
| `<OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)` | the low-level toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”) replace the engine; they can't be mixed with it | use the engine **or** the toolkit, not both |
| `a game can only use one of <OpenGL>, <DirectX> and <Vulkan>` | | pick one toolkit |
| `the shader shape.frag has errors` | a `<Vulkan>` shader file has a mistake; the lines below say where | fix the shader |
| `there's no folder called 'x' next to this file` | `#include <"x">`, but no such folder | check the name and where the folder is |
| `the file name 'my game.frg' can't be used` | file names can only have letters, numbers and `_` | rename the file |

### Warnings

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

### Runtime errors

These stop the game with a message, the file and the line:

```
Runtime error in main.frg:12: position 5 is outside the list (it has 3 items, at positions 0 to 2)
```

`frg run` then shows `[game.exe exited with code 1]`.

| Message | Cause | Prevent it with |
|---|---|---|
| `position N is outside the list (...)` | `list[N]` where `N` isn't between 0 and `length - 1` | check `i < list.length`; loop with `for ... in` |
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

Crashes that don't give a message (`[game.exe crashed (code 0xC0000005)]`) usually mean a pointer (see “10. Enums, Optionals and Pointers” › “Pointers”) was used after the thing it pointed at went away, or a function called itself forever.

### The C++ compiler rejected Forge's output

If you ever see:

```
error: the C++ compiler rejected Forge's output - this is a bug in Forge, not in your code
```

then your code passed all of Forge's checks, but Forge produced something invalid. That's Forge's fault. The message shows where the generated C++ file is; please report it, with the code that caused it.

### What's next

Reserved words (see “13. Reserved Words”).

<!-- docs/language/13-reserved-words.md -->

## 13. Reserved Words

These words have special meanings, so they can't be used as names for variables, functions, types, fields, files or folders.

### Keywords

| Group | Words |
|---|---|
| Control flow | `if` `else` `while` `do` `for` `in` `switch` `case` `default` `return` `break` `continue` |
| Values | `true` `false` `null` `this` `super` |
| Declarations | `struct` `class` `enum` `const` `void` |
| Modifiers | `private` `public` `static` (built-in libraries only) |
| Logic | `and` `or` `not` |
| Classes | `is` `as` |
| Word operators | `negative` `bitnot` |
| Saved for the future | `new` `delete` |

### Built-in type names

`void` `bool` `string` `task` `int` `int8` `int16` `int32` `int64` `int128` `uint` `uint8` `uint16` `uint32` `uint64` `uint128` `float` `double` `list` `map` `Pointer` `vec2` `vec3` `vec4` `color` `rect`

### Built-in functions

`print` `write` `input` (from `<stdio>`), and `wait` (inside tasks).

### Library names

`stdio` `math` `random` `window` `input` `graphics2d` `graphics3d` `assets` `audio` `physics` `time` `file` `save` `tween` `particles` `tilemap` `scenes` `ui` `ECS` `tasks` `OpenGL` `DirectX` `Vulkan`, plus `network`, which is planned.

Types that come from a library (`Key`, `MouseButton`, `GamepadButton`, `Body`, `Texture`, `Animation`, `Font`, `Sound`, `Music`, `Camera`, `Camera2D`, `Model`, `Timer`, `Tween`, `Ease`, `Emitter`, `TileMap`, `Scene`, `Entity`, `Component`) can't be reused as names of your own types **in a file that includes that library**.

### Your own top-level files and folders

If the game has `player.frg` or an `enemies/` folder in its top folder, then `player` and `enemies` can't be used as names anywhere in the game, because `player.something` always means the file.

### Word operators that are *not* reserved

These words are only operators in specific places, so they can still be used as names:

| Words | Count as operators when |
|---|---|
| `plus` `minus` `times` `div` `mod` `equals` `greater` `less` `otherwise` `bitand` `bitor` `bitxor` `shiftleft` `shiftright` | they come right after a value: `a plus b` |
| `equal` | right after `greater or` / `less or` |
| `set` ... `to`, `increase` / `decrease` ... `by`, `multiply` / `divide` ... `by` | at the start of a statement with the matching `to` / `by` later on |
| `increase x;` / `decrease x;` | at the start of a statement that ends right after the name |

For example, this is fine, though not recommended:

```cpp
int times = 3;
int set = 2;
print(times times set);    // 6
```

See Operators (see “4. Operators”) for what they all do.

### Names Forge uses internally

Forge's generated code adds `_` to the end of every name you write, and uses `$` in its own names, so nothing you name can clash with C++ or with Forge's runtime. You never see these unless you open the generated C++.

<!-- docs/language/14-coming-from-other-languages.md -->

## 14. Coming From Other Languages

If you already know C++, C#, Java, JavaScript or Python, Forge will feel familiar. This page lists where it's **different**, so your habits don't trip you up.

### Overall

| | Forge |
|---|---|
| Looks like | C++ / C#: braces, semicolons, typed variables (`int x = 5;`) |
| Compiled? | Yes. Forge is checked fully before running, then compiled to native code through C++. |
| Memory | Automatic: class objects are reference-counted (freed when nothing uses them). No `new`, `delete`, garbage collector pauses or manual freeing. |
| Entry point | `int main()` in `main.frg`, returning 0 for success |
| Type inference | None. Every variable states its type (no `var` or `auto`). |
| Declaration order | Doesn't matter for functions, types and globals, even across files. No forward declarations or headers. |

### Habits that need changing

| You might write | In Forge | Why |
|---|---|---|
| `print(a, b)` (Python) | `print("{a} {b}")` | `print` takes one value. Put several values in one string. |
| `"Score: " + score` | `"Score: {score}"` | `+` only joins two strings; Forge doesn't guess number formatting. |
| `f"{x}"` (Python), `` `${x}` `` (JS), `$"{x}"` (C#) | `"{x}"` | every string can hold `{values}` |
| `/* comment */` | ` ``` comment ``` ` | |
| `if (x) return;` | `if (x) { return; }` | braces are always required |
| `if (count)`, `if (obj)`, `while (n)` | `if (count > 0)`, `if (obj != null)` | conditions must be `bool`; there's no "truthy" |
| `x = y = 0;`, `if ((n = next()) > 0)` | separate statements | assignment is a statement, not a value |
| `y = x++;` | `x++;` then `y = x;` | `++`/`--` are statements only |
| `case 1: ...; break;` | `case 1: ...` | switch cases never fall through; `break` in a switch is an error |
| `new Player()` | `Player()` | there's no `new` |
| `Player p;` (C++/C#) | `Player p = Player();` or `Player? p = null;` | class variables must hold an object, or be optional |
| `#include "player.h"` / `import player` | nothing (same folder); `#include <"folder">` for folders | every `.frg` file next to `main.frg` is included automatically |
| calling `jump()` from another file | `player.jump()` | other files' things always need the file-name prefix |
| `void jump();` declarations | just write the function | no forward declarations or headers |
| `arr.size()`, `len(arr)`, `arr.Count` | `arr.length` (no parentheses) | |
| `str[0]` | `str.charAt(0)` | text can't be indexed |
| `'c'` (a char) | `"c"` | there's no `char` type; single quotes are an error |
| `for i in range(10)` (Python) | `for (int i = 0; i < 10; i++)` | |
| `for (auto& x : list)` (C++) | `for (int x in list)` | the item type is written out |
| `x ** 2`, `x ^ 2`, `pow(x, 2)` | `x * x` or `math.pow(x, 2)` | no power operator; `^` is bitwise xor |
| `7 / 2 == 3.5` (Python 3) | `7 / 2` is `3` | whole-number division, like C, C++, Java and C# |
| `null` for any object (Java/C#) | only for `Type?` | plain types can never be null |
| `x?.y` (C#/JS) | `if (x != null) { x.y }` | an optional is used directly; it stops the game if it's null |
| `(Goblin)e` / `e as Goblin` (C#) | `e as Goblin` gives `Goblin?` | |
| `protected` | `private` | `private` already includes classes that inherit |
| `static` members | top-level functions and globals in a file | `static` isn't available for your own types |
| overloading `void f(int)` / `void f(float)` | different names | only constructors can share a name |
| `class B extends A` / `class B : public A` | `class B : A` | |
| `@Override` / `override` | nothing | a method with the same name and types replaces the parent's |
| `base.method()` / `super.method()` | `super.method()` | |
| `try` / `catch` | not available | runtime errors stop the game with a message |
| lambdas, function pointers, callbacks | not available | |
| templates / generics you write yourself | not available | the built-in `list<T>`, `map<K, V>` and `Pointer<T>` are generic |
| threads | not available | use tasks (see “11. Tasks”) for things that happen over time |

### Things Forge adds

- **Word operators**: every symbol has a word form: `a plus b`, `x greater or equal y`, `set x to 5;`, `increase score by 10;`. See Operators (see “4. Operators”).
- **Tasks**: `task` functions with `wait(seconds)`, like coroutines or `async`, but simpler. See Tasks (see “11. Tasks”).
- **A file is a namespace**: `player.frg`'s contents are reached as `player.x`, and folders nest: `enemies.goblin.spawn()`.
- **Literals adapt to their destination**: `int8 x = 100;` needs no cast, and `float r = 7 / 2;` is `3.5` (because both numbers are written out). See How division works (see “2. Variables and Types” › “How division works”).
- **Runtime checks** on list positions, map keys, null and division by zero, with the file and line in the message.
- **Game types built in**: `vec2`, `vec3`, `color`, `rect`, with math operators.

### How Forge compares, roughly

| | Forge | C++ | C# | Python |
|---|---|---|---|---|
| Typing | static, explicit | static | static | dynamic |
| Speed | close to C++ | fast | fast | slower |
| Memory | automatic (reference counting) | manual / RAII | garbage collected | garbage collected |
| Null safety | only `Type?` can be null | no | partial (nullable types) | no |
| Game libraries | built in | external | external (Unity, MonoGame) | external (pygame) |

---

# Part D: The libraries

<!-- docs/libraries/README.md -->

## Library Overview

The core language (variables, functions, types, lists...) needs no includes. Everything else lives in a **library** that a file brings in with `#include <name>`:

```cpp
#include <window>
#include <graphics2d>
```

How libraries work:

- **Each file includes the libraries it uses.** If two files both draw, both include `<graphics2d>`.
- **Functions** are reached through the library's name: `window.open(...)`, `math.sqrt(2)`. The `<stdio>` functions (`print`, `write`, `input`) are the exception; they're used on their own.
- **Types** from a library are used by their plain name: `Key.SPACE`, `Body`, `Texture`, `Timer`.
- Using a library without including it is an error that says exactly which `#include` to add.

| Library | One-line summary | Needs a window? |
|---|---|---|
| (built in) `vec2` `vec3` `vec4` `color` `rect` (see “Built-in Types: vec2, vec3, vec4, color, rect”) | Vectors, colors and rectangles, always available | no |
| `<stdio>` (see “`<stdio>`: Console Input and Output”) | `print`, `write`, `input` for the console | no |
| `<math>` (see “`<math>`: Math Functions”) | `sqrt`, `sin`, `cos`, `pow`, `floor`, `abs`, `min`, `max`, `clamp`, `lerp`, `PI` | no |
| `<random>` (see “`<random>`: Random Numbers”) | Random whole and decimal numbers, chances | no |
| `<window>` (see “`<window>`: The Game Window”) | Opens the game window, runs the game loop, frame timing | **is** the window |
| `<input>` (see “`<input>`: Keyboard, Mouse and Controllers”) | Keyboard, mouse and controllers (gamepads) | yes |
| `<graphics2d>` (see “`<graphics2d>`: 2D Drawing”) | Rectangles, circles, lines, triangles, text | yes |
| `<graphics3d>` (see “`<graphics3d>`: 3D Drawing”) | 3D camera, cubes, spheres, grids, 3D models | yes |
| `<assets>` (see “`<assets>`: Images, Animations and Fonts”) | Loading images (`Texture`) and fonts (`Font`), and sprite-sheet animation (`Animation`) | to draw them |
| `<audio>` (see “`<audio>`: Sounds and Music”) | Sound effects (`Sound`) and music (`Music`) | no |
| `<physics>` (see “`<physics>`: Boxes, Gravity and Collisions”) | Box bodies with gravity and collisions | no |
| `<time>` (see “`<time>`: Clock, Date and Timers”) | The clock and date, `sleep`, and the `Timer` class | no |
| `<save>` (see “`<save>`: Remembering Things Between Runs”) | Remembering values (high scores, settings) between runs | no |
| `<tilemap>` (see “`<tilemap>`: Levels Drawn as Text”) | Levels drawn as text, turned into physics bodies and drawn as tiles (`TileMap`) | to draw them |
| `<particles>` (see “`<particles>`: Dust, Sparks and Explosions”) | Dust, sparks, smoke and explosions (`Emitter`) | yes |
| `<tween>` (see “`<tween>`: Smooth Changes Over Time”) | Smooth changes over time: sliding, bouncing, fading, with easing | no |
| `<file>` (see “`<file>`: Reading and Writing Text Files”) | Reading and writing text files | no |
| `<scenes>` (see “`<scenes>`: Menus, Levels and Game Over”) | Splitting a game into scenes (menu, level...) | no |
| `<ui>` (see “`<ui>`: Buttons, Checkboxes, Sliders and Text Boxes”) | Buttons, checkboxes, sliders, text boxes, labels, panels | yes |
| `<ECS>` (see “`<ECS>`: Entities and Components”) | Entities built from components | no |
| `<tasks>` (see “`<tasks>`: Tasks Without a Window”) | Running tasks (see “11. Tasks”) in programs without a window | no |

"Needs a window" means the library only works after `window.open(...)`. Calling a drawing function earlier stops the game with "the window isn't open yet".

### The built-in engine

`<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>`, `<audio>` and `<ui>` are Forge's built-in game engine. They're built on [raylib](https://www.raylib.com/), which Forge includes, and they all work together: one window, one game loop, and 2D and 3D drawing mixed in one frame.

### Low-level toolkits: OpenGL, DirectX and Vulkan

For advanced users who want to write their own rendering, there are three **separate** toolkits:

| Toolkit | For |
|---|---|
| `<OpenGL>` (see “`<OpenGL>`: Drawing with OpenGL Yourself”) | Drawing with OpenGL 3.3 and GLSL shaders |
| `<DirectX>` (see “`<DirectX>`: Drawing with Direct3D 11 Yourself”) | Drawing with Direct3D 11 and HLSL shaders |
| `<Vulkan>` (see “`<Vulkan>`: Drawing with Vulkan Yourself”) | Drawing with Vulkan and SPIR-V shaders (compiled from GLSL by `frg build`) |

They are **not** part of the engine, and the engine can't be switched over to use them. A game that includes one makes its own window and does all its own drawing; it can't use `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`, and it can use only one toolkit. Libraries that don't draw (`<math>`, `<audio>`, `<physics>`...) still work. Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”) explains it all, and lists what the three have in common.

<!-- docs/libraries/core-types.md -->

## Built-in Types: vec2, vec3, vec4, color, rect

These five types are **always available**, with no `#include`. They're structs (see “9. Structs and Classes”), so they're copied when assigned, and their fields can be read and changed directly.

### vec2

A 2D vector: a position, a direction, a velocity or a size on the screen.

```cpp
vec2 position = vec2(100, 200);
vec2 velocity = vec2(50, 0);
position += velocity * dt;
print(position.x);
```

| Fields | |
|---|---|
| `float x` | left/right (screen x grows to the right) |
| `float y` | up/down (screen y grows **downward**) |

| Making one | |
|---|---|
| `vec2(float x, float y)` | |
| `vec2()` | `(0, 0)` |

| Method | Gives back | What it does |
|---|---|---|
| `length()` | `float` | how long the vector is: √(x² + y²) |
| `lengthSquared()` | `float` | x² + y² (faster; fine for comparing lengths) |
| `normalized()` | `vec2` | the same direction with length 1 (`(0, 0)` stays `(0, 0)`) |
| `dot(vec2 other)` | `float` | the dot product: x·x' + y·y' |
| `distance(vec2 other)` | `float` | how far apart two points are |
| `angle()` | `float` | the vector's angle in **radians**, from the +x direction |
| `rotated(float radians)` | `vec2` | the vector turned by an angle |
| `lerp(vec2 target, float t)` | `vec2` | part of the way to `target`: `t = 0` gives this vector, `t = 1` gives `target`, `t = 0.5` the halfway point |

| Constant | Value |
|---|---|
| `vec2.ZERO` | `(0, 0)` |
| `vec2.ONE` | `(1, 1)` |
| `vec2.UP` | `(0, -1)`: up on the **screen** |
| `vec2.DOWN` | `(0, 1)` |
| `vec2.LEFT` | `(-1, 0)` |
| `vec2.RIGHT` | `(1, 0)` |

### vec3

A 3D vector, for 3D graphics (see “`<graphics3d>`: 3D Drawing”). In 3D, **y points up**.

| Fields | `float x`, `float y`, `float z` |
|---|---|
| Making one | `vec3(float x, float y, float z)`, `vec3()` = `(0, 0, 0)` |

| Method | Gives back | What it does |
|---|---|---|
| `length()` | `float` | length |
| `lengthSquared()` | `float` | length squared |
| `normalized()` | `vec3` | the same direction with length 1 |
| `dot(vec3 other)` | `float` | dot product |
| `cross(vec3 other)` | `vec3` | cross product: a vector at right angles to both |
| `distance(vec3 other)` | `float` | distance between two points |
| `lerp(vec3 target, float t)` | `vec3` | part of the way to `target` |

| Constant | Value |
|---|---|
| `vec3.ZERO` | `(0, 0, 0)` |
| `vec3.ONE` | `(1, 1, 1)` |
| `vec3.UP` | `(0, 1, 0)` |
| `vec3.FORWARD` | `(0, 0, -1)` |
| `vec3.RIGHT` | `(1, 0, 0)` |

### vec4

| Fields | `float x`, `y`, `z`, `w` |
|---|---|
| Making one | `vec4(x, y, z, w)`, `vec4()` |
| Methods | `length()`, `normalized()`, `dot(vec4 other)` |
| Constants | `vec4.ZERO`, `vec4.ONE` |

### Vector math

| Expression | Result |
|---|---|
| `a + b`, `a - b` | add or subtract the same kind of vector |
| `v * 2`, `2 * v`, `v / 2` | scale by a number |
| `-v` | flip the direction |
| `a == b`, `a != b` | compare every part |
| `v += w;` `v -= w;` | change in place |
| `v *= 2;` `v /= 2;` | scale in place |

Multiplying two vectors together isn't allowed; use `dot` or `cross`. Word forms work too: `a plus b`, `v times 2`.

### color

A color with red, green, blue and alpha (opacity) parts, each from 0 to 255.

```cpp
color sky = color(100, 180, 255);
color seeThrough = color(255, 0, 0, 128);   // half transparent red
graphics2d.drawRect(0, 0, 100, 100, color.ORANGE);
```

| Fields | |
|---|---|
| `uint8 r`, `uint8 g`, `uint8 b` | red, green, blue (0–255) |
| `uint8 a` | alpha: 255 is solid, 0 is invisible |

| Making one | |
|---|---|
| `color(r, g, b)` | solid (`a` = 255) |
| `color(r, g, b, a)` | |

The parts are `int`s. Anything below 0 becomes 0, and anything above 255 becomes 255, so `color(random.range(0, 255), 100, 300)` works and is the same as `color(..., 100, 255)`.
| `color()` | solid black |

| Method | What it does |
|---|---|
| `fade(float alpha)` | the same color with a new opacity, from 0.0 (invisible) to 1.0 (solid) |
| `lerp(color target, float t)` | a mix between this color and `target` (`t = 0.5` is halfway) |

Named colors (`color.RED`, etc.):

| Name | r, g, b | Name | r, g, b |
|---|---|---|---|
| `WHITE` | 255, 255, 255 | `GREEN` | 0, 228, 48 |
| `BLACK` | 0, 0, 0 | `LIME` | 0, 158, 47 |
| `BLANK` | invisible (alpha 0) | `DARKGREEN` | 0, 117, 44 |
| `LIGHTGRAY` | 200, 200, 200 | `SKYBLUE` | 102, 191, 255 |
| `GRAY` | 130, 130, 130 | `BLUE` | 0, 121, 241 |
| `DARKGRAY` | 80, 80, 80 | `DARKBLUE` | 0, 82, 172 |
| `RED` | 230, 41, 55 | `PURPLE` | 200, 122, 255 |
| `MAROON` | 190, 33, 55 | `VIOLET` | 135, 60, 190 |
| `ORANGE` | 255, 161, 0 | `PINK` | 255, 109, 194 |
| `YELLOW` | 253, 249, 0 | `MAGENTA` | 255, 0, 255 |
| `GOLD` | 255, 203, 0 | `BROWN` | 127, 106, 79 |
| | | `BEIGE` | 211, 176, 131 |

Methods work on named colors too: `color.SKYBLUE.lerp(color.PURPLE, 0.5)`.

### rect

An axis-aligned rectangle: its top-left corner, width and height.

```cpp
rect player = rect(100, 300, 32, 48);
rect coin = rect(120, 310, 16, 16);
if (player.overlaps(coin)) {
    print("got it!");
}
```

| Fields | `float x`, `float y` (top-left corner), `float width`, `float height` |
|---|---|
| Making one | `rect(x, y, width, height)`, `rect()` |

| Method | Gives back | What it does |
|---|---|---|
| `overlaps(rect other)` | `bool` | true if the two rectangles overlap (just touching edges doesn't count) |
| `contains(vec2 point)` | `bool` | true if the point is inside |
| `center()` | `vec2` | the middle point |
| `position()` | `vec2` | the top-left corner |
| `size()` | `vec2` | `(width, height)` |

### Printing

`print(vec2(1, 2))` shows `vec2(1.0, 2.0)`. The others show the same way: `color(255, 0, 0, 255)`, `rect(0.0, 0.0, 10.0, 10.0)`.

<!-- docs/libraries/stdio.md -->

## `<stdio>`: Console Input and Output

```cpp
#include <stdio>
```

"stdio" stands for **st**an**d**ard **i**nput and **o**utput: the text console. Unlike other libraries, its functions are used without a prefix: `print(...)`, not `stdio.print(...)`.

With a window open, `print` output still goes to the console window behind the game. That's handy for debugging. In a `--release` or `--no-console` build there's no console, so the output isn't visible. Runtime errors still are: they pop up in a box.

### print

```cpp
void print(value)
void print()
```

Shows one value, then starts a new line. `print()` with nothing just starts a new line. Any type can be printed; see how values look as text (see “3. Text” › “How values look as text”).

```cpp
print("Hello!");
print(42);
print([1, 2, 3]);
print("x = {x}, y = {y}");     // to show several values, put them in one string
```

`print(a, b)` is an error: `print` takes one value.

### write

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

### input

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
- `input` is for console programs. In a windowed game, use `<input>` (see “`<input>`: Keyboard, Mouse and Controllers”) for the keyboard. With both included, `input(...)` is this function and `input.keyDown(...)` is the library.

<!-- docs/libraries/math.md -->

## `<math>`: Math Functions

```cpp
#include <math>
```

Everything is reached as `math.name`.

### Constants

| Constant | Value | Meaning |
|---|---|---|
| `math.PI` | 3.14159… | half a turn, in radians |
| `math.TAU` | 6.28318… | a full turn, in radians (2π) |
| `math.E` | 2.71828… | the base of natural logarithms |

They take on whichever decimal type they're used as (`float` or `double`).

### Functions

"Number" means any number type. Functions that give a `float` give a `double` instead if you pass a `double`.

| Function | Gives back | What it does | Example |
|---|---|---|---|
| `math.sqrt(x)` | float | square root | `math.sqrt(16)` → `4.0` |
| `math.pow(x, p)` | float | x to the power p | `math.pow(2, 10)` → `1024.0` |
| `math.abs(x)` | same type as x | distance from zero | `math.abs(-3)` → `3` |
| `math.sign(x)` | same type as x | -1, 0 or 1 | `math.sign(-7)` → `-1` |
| `math.min(a, b)` | the common type | the smaller one | `math.min(3, 8)` → `3` |
| `math.max(a, b)` | the common type | the bigger one | `math.max(3, 8)` → `8` |
| `math.clamp(x, low, high)` | the common type | x, kept between low and high | `math.clamp(15, 0, 10)` → `10` |
| `math.lerp(a, b, t)` | float | from a (t = 0) to b (t = 1) | `math.lerp(0, 100, 0.25)` → `25.0` |
| `math.floor(x)` | float | round down | `math.floor(2.7)` → `2.0` |
| `math.ceil(x)` | float | round up | `math.ceil(2.1)` → `3.0` |
| `math.round(x)` | float | round to the nearest whole number (halves go away from zero) | `math.round(2.5)` → `3.0` |
| `math.sin(r)` | float | sine of an angle in radians | `math.sin(math.PI / 2)` → `1.0` |
| `math.cos(r)` | float | cosine | |
| `math.tan(r)` | float | tangent | |
| `math.asin(x)` | float | inverse sine, in radians | |
| `math.acos(x)` | float | inverse cosine, in radians | |
| `math.atan(x)` | float | inverse tangent, in radians | |
| `math.atan2(y, x)` | float | the angle of the point (x, y), in radians. Note: y comes first. | |
| `math.exp(x)` | float | e to the power x | |
| `math.log(x)` | float | natural logarithm | |

Notes:

- **`floor`, `ceil` and `round` give back a decimal number.** Wrap them in `int(...)` for a whole number: `int(math.round(2.6))` is `3`.
- `min`, `max`, `clamp` and `abs` keep whole numbers whole: `math.max(3, 8)` is the `int` `8`. Mixing types gives the bigger type: `math.min(2.5, 1)` is the `float` `1.0`.
- Angles are in **radians**. To convert, `radians = degrees * math.PI / 180`. Note that `Texture.drawRotated` (see “`<assets>`: Images, Animations and Fonts”) and `graphics3d` (see “`<graphics3d>`: 3D Drawing”) rotations use **degrees**.
- `sqrt` of a negative number, or `log` of zero or less, gives `nan` (not a number).
- Squaring: `x * x` or `math.pow(x, 2)`. There's no `^` power operator.

### Example: moving toward a target

```cpp
vec2 toTarget = target - position;
float dist = toTarget.length();
if (dist > 1) {
    position += toTarget.normalized() * math.min(speed * dt, dist);
}
```

### Example: bobbing up and down

```cpp
float bob = math.sin(window.time() * 3) * 10;     // between -10 and 10, three times a second-ish
graphics2d.drawCircle(x, y + bob, 8, color.GOLD);
```

<!-- docs/libraries/random.md -->

## `<random>`: Random Numbers

```cpp
#include <random>
```

| Function | Gives back | What it does |
|---|---|---|
| `random.range(int min, int max)` | `int` | a random whole number from `min` to `max`, **both included** |
| `random.range(float min, float max)` | `float` | a random decimal number from `min` up to (just under) `max` |
| `random.value()` | `float` | a random decimal from 0 up to (just under) 1 |
| `random.chance(float probability)` | `bool` | true with that probability: `random.chance(0.25)` is true about 1 time in 4 |
| `random.seed(int value)` | — | restarts the random numbers from a fixed starting point |
| `random.color()` | `color` | a random solid color (each of red, green and blue from 0 to 255) |

`random.range` picks the whole-number version when both values are whole numbers, and the decimal version otherwise:

```cpp
int roll = random.range(1, 6);              // 1, 2, 3, 4, 5 or 6
float speed = random.range(50.0, 120.0);    // somewhere from 50 to 120
float alsoDecimal = random.range(0, 2.5);   // one decimal makes it the decimal version
```

If `min` is bigger than `max`, they're swapped.

### Seeds

Every run of the game gets different random numbers. To get the **same** sequence every time (to reproduce a bug, or to make a "daily level"), set a seed first:

```cpp
random.seed(12345);
```

The same seed gives the same sequence of numbers.

### Examples

```cpp
// pick a random item from a list
list<string> loot = ["sword", "shield", "potion"];
string drop = loot[random.range(0, loot.length - 1)];

// 10% chance of a critical hit
int damage = random.chance(0.1) ? 20 : 10;

// spawn somewhere along the top of the screen
vec2 spawn = vec2(random.range(0.0, 960.0), -20);
```

<!-- docs/libraries/window.md -->

## `<window>`: The Game Window

```cpp
#include <window>
```

Opens the window your game is drawn in, runs the game loop, and keeps track of time between frames. See the game loop guide (see “The Game Loop”) for the ideas behind it.

### A minimal game

```cpp
#include <window>
#include <graphics2d>

int main(){
    window.open(800, 450, "My Game");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    graphics2d.drawCircle(400, 225, 50, color.RED);
}
```

### Functions

#### Opening and closing

| Function | What it does |
|---|---|
| `window.open(int width, int height, string title)` | Opens the window, `width` × `height` pixels. Frames are limited to 60 per second to start with. Calling it again while it's open does nothing. |
| `window.close()` | Closes the window. It's also closed automatically when `main` ends. |

#### The game loop: two ways

**Your own loop:**

```cpp
while (!window.shouldClose()) {
    // update and draw one frame here
}
window.close();
```

| Function | What it does |
|---|---|
| `window.shouldClose()` | Shows the frame drawn since the last call, then starts the next frame. Gives back `true` when the player closes the window (the X button or Escape), so the loop ends. |

**The engine loop:**

```cpp
window.run();
```

| Function | What it does |
|---|---|
| `window.run()` | Runs the game until the window closes. Every frame, it calls **`update(float dt)`** and then **`draw()`**, and then shows the frame. Both functions must be written in **main.frg** with exactly those names and parameters, or it's a compile error. When the window closes, `run()` closes it and returns. |

Both loops do the same things each frame:

1. Clear the screen to the background color.
2. Continue any tasks (see “11. Tasks”) whose `wait` is over.
3. Your code: update, then draw.
4. Show the frame, read the keyboard and mouse, and wait if needed to keep to the frame rate.

#### Time

| Function | Gives back | What it does |
|---|---|---|
| `window.deltaTime()` | `float` | seconds since the last frame, usually about 0.0167 at 60 FPS. This is the same `dt` that `update(dt)` gets. |
| `window.time()` | `double` | seconds since the window opened |
| `window.fps()` | `int` | frames per second right now |
| `window.setFPS(int fps)` | — | the most frames per second to draw (60 to start with); 0 means no limit |

**Multiply movement by `dt`** so the game runs at the same speed on fast and slow computers: `x += 200 * dt;` moves 200 pixels per **second**, however many frames that takes.

#### Size and look

| Function | What it does |
|---|---|
| `window.width()` | the window's width in pixels (`int`) |
| `window.height()` | the window's height in pixels (`int`) |
| `window.setTitle(string title)` | changes the title bar text |
| `window.setBackground(color c)` | the color the screen is cleared to every frame (dark blue-gray to start with) |
| `window.toggleFullscreen()` | switches between fullscreen and windowed |
| `window.screenshot(string fileName)` | saves what's on screen as a `.png` in the game's folder |

### Notes

- Drawing can happen any time after `open()`; a frame starts by itself if one isn't running.
- Calling drawing functions before `window.open(...)` stops the game with "the window isn't open yet".
- Escape closes the window by default.
- If the game's folder has an `icon.png`, it becomes the window's icon (and the `.exe`'s icon). See The game's icon (see “Getting Started” › “The game's icon”).
- `<input>` (see “`<input>`: Keyboard, Mouse and Controllers”), `<graphics2d>` (see “`<graphics2d>`: 2D Drawing”), `<graphics3d>` (see “`<graphics3d>`: 3D Drawing”) and `<ui>` (see “`<ui>`: Buttons, Checkboxes, Sliders and Text Boxes”) all work with this window.

### Testing games automatically

Games read three environment variables, so they can be tested without anyone at the keyboard (Forge's own tests use this):

| Variable | Effect |
|---|---|
| `FRG_AUTOTEST_FRAMES=90` | stop after 90 frames |
| `FRG_AUTOTEST_SHOT=shot.png` | take this screenshot just before stopping |
| `FRG_AUTOTEST_INPUT=RIGHT:10-60,SPACE:30-31` | pretend these keys are held during those frames |

<!-- docs/libraries/input.md -->

## `<input>`: Keyboard, Mouse and Controllers

```cpp
#include <input>
```

Reads the keyboard and mouse in a windowed game. Input is updated once per frame (when the frame is shown), so check it every frame in your `update` / loop.

### Keyboard

| Function | Gives back | True when |
|---|---|---|
| `input.keyDown(Key key)` | `bool` | the key is **held down** (true every frame while held) |
| `input.keyPressed(Key key)` | `bool` | the key was **just pressed** (true for one frame only) |
| `input.keyReleased(Key key)` | `bool` | the key was **just let go** (true for one frame only) |

Use `keyDown` for things that continue while held (walking), and `keyPressed` for things that happen once per press (jumping, shooting, menus):

```cpp
if (input.keyDown(Key.RIGHT)) {
    x += speed * dt;
}
if (input.keyPressed(Key.SPACE) and onGround) {
    velocityY = -600;
}
```

#### Keys

| Group | Values |
|---|---|
| Letters | `Key.A` … `Key.Z` |
| Number row | `Key.NUM0` … `Key.NUM9` |
| Arrows | `Key.LEFT` `Key.RIGHT` `Key.UP` `Key.DOWN` |
| Common | `Key.SPACE` `Key.ENTER` `Key.ESCAPE` `Key.BACKSPACE` `Key.TAB` |
| Modifiers | `Key.LEFT_SHIFT` `Key.RIGHT_SHIFT` `Key.LEFT_CONTROL` `Key.RIGHT_CONTROL` `Key.LEFT_ALT` `Key.RIGHT_ALT` |
| Function keys | `Key.F1` … `Key.F12` |

`Key` is an enum (see “10. Enums, Optionals and Pointers” › “Enums”), so keys can be stored in variables and lists: `Key jumpKey = Key.SPACE;`.

Escape also closes the window by default.

### Mouse

| Function | Gives back | What it is |
|---|---|---|
| `input.mousePosition()` | `vec2` | the mouse's position in the window (top-left is `(0, 0)`) |
| `input.mouseX()` | `float` | just the x |
| `input.mouseY()` | `float` | just the y |
| `input.mouseDown(MouseButton b)` | `bool` | the button is held down |
| `input.mousePressed(MouseButton b)` | `bool` | the button was just pressed (one frame) |
| `input.mouseReleased(MouseButton b)` | `bool` | the button was just let go (one frame) |
| `input.mouseWheel()` | `float` | how far the wheel moved this frame (positive = away from you) |

Buttons: `MouseButton.LEFT`, `MouseButton.RIGHT`, `MouseButton.MIDDLE`.

```cpp
if (input.mousePressed(MouseButton.LEFT)) {
    vec2 m = input.mousePosition();
    if (rect(100, 100, 200, 50).contains(m)) {
        print("clicked the box");
    }
}
```

### Controllers (gamepads)

Xbox-style controllers (and most others) plugged in by USB or Bluetooth work too. Buttons are named like on an Xbox controller:

| `GamepadButton.` | Button |
|---|---|
| `A` `B` `X` `Y` | the four buttons on the right (on a PlayStation controller: cross, circle, square, triangle) |
| `UP` `DOWN` `LEFT` `RIGHT` | the D-pad (the cross on the left) |
| `LEFT_BUMPER` `RIGHT_BUMPER` | the shoulder buttons |
| `LEFT_TRIGGER` `RIGHT_TRIGGER` | the triggers, as buttons (pressed or not). For how far, use `leftTrigger()` |
| `BACK` `START` | the two small buttons in the middle |
| `LEFT_STICK` `RIGHT_STICK` | pressing a stick down like a button |

| Function | Gives back | What it does |
|---|---|---|
| `input.gamepadDown(GamepadButton button, int gamepad = 0)` | `bool` | true **while** the button is held |
| `input.gamepadPressed(GamepadButton button, int gamepad = 0)` | `bool` | true only in the frame it was pressed |
| `input.gamepadReleased(GamepadButton button, int gamepad = 0)` | `bool` | true only in the frame it was let go |
| `input.leftStick(int gamepad = 0)` | `vec2` | the left stick: x from -1 (left) to 1 (right), y from -1 (up) to 1 (down) |
| `input.rightStick(int gamepad = 0)` | `vec2` | the right stick, the same way |
| `input.leftTrigger(int gamepad = 0)` | `float` | how far the left trigger is pressed, from 0 to 1 |
| `input.rightTrigger(int gamepad = 0)` | `float` | the same for the right trigger |
| `input.gamepadConnected(int gamepad = 0)` | `bool` | true if that controller is plugged in |
| `input.gamepadName(int gamepad = 0)` | `string` | the controller's name, or `""` |

`gamepad` picks the controller: `0` (the default) is the first one plugged in, `1` the second, and so on, for local multiplayer.

Things to know:

- Sticks never rest exactly in the middle, so small movements (less than a fifth of the way) read as exactly `(0, 0)`. That's called a **dead zone**. Past it, the value still goes all the way to 1.
- The stick's `y` points **down**, like the screen, so `position += input.leftStick() * speed * dt;` moves the right way.
- Without a controller, every function just says "not pressed" (`false`, `vec2(0, 0)`, `0`), so a game can always check the keyboard and the controller together:

```cpp
bool jump = input.keyPressed(Key.SPACE) or input.gamepadPressed(GamepadButton.A);

vec2 move = input.leftStick();
if (input.keyDown(Key.LEFT) or input.gamepadDown(GamepadButton.LEFT)) {
    move.x = -1;
}
if (input.keyDown(Key.RIGHT) or input.gamepadDown(GamepadButton.RIGHT)) {
    move.x = 1;
}
player.velocity.x = move.x * MOVE_SPEED;
```

### input vs input()

`<stdio>` has a function `input(...)` for typing into the console, and this library is also called `input`. With both included, `input.keyDown(...)` is this library and `input("Name? ")` is the console function.

### Examples

#### Eight-way movement

```cpp
vec2 move = vec2.ZERO;
if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) { move.x -= 1; }
if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) { move.x += 1; }
if (input.keyDown(Key.UP) or input.keyDown(Key.W)) { move.y -= 1; }
if (input.keyDown(Key.DOWN) or input.keyDown(Key.S)) { move.y += 1; }
position += move.normalized() * speed * dt;   // normalized: diagonals aren't faster
```

#### Toggling a pause menu

```cpp
if (input.keyPressed(Key.P)) {
    paused = not paused;
}
```

<!-- docs/libraries/graphics2d.md -->

## `<graphics2d>`: 2D Drawing

```cpp
#include <graphics2d>
```

Draws shapes and text in the window (see “`<window>`: The Game Window”). Everything is redrawn every frame: the screen is cleared before each frame, so draw everything you want to see, every frame.

### Coordinates

- `(0, 0)` is the **top-left** corner of the window.
- **x** grows to the right, and **y** grows **downward**.
- Sizes and positions are in pixels. Decimal positions are fine.
- Things drawn later appear **on top** of things drawn earlier.

### Functions

Several functions have two versions: one taking separate numbers, and one taking a `vec2` or `rect` (see “Built-in Types: vec2, vec3, vec4, color, rect”). Use whichever is handier.

#### Rectangles

| Function | Draws |
|---|---|
| `graphics2d.drawRect(float x, float y, float width, float height, color c)` | a filled rectangle; (x, y) is the top-left corner |
| `graphics2d.drawRect(rect r, color c)` | the same, from a `rect` |
| `graphics2d.drawRectOutline(float x, float y, float width, float height, color c, float thickness = 1)` | the outline of a rectangle |
| `graphics2d.drawRectOutline(rect r, color c, float thickness = 1)` | the same, from a `rect` |

#### Circles

| Function | Draws |
|---|---|
| `graphics2d.drawCircle(float x, float y, float radius, color c)` | a filled circle, centered on (x, y) |
| `graphics2d.drawCircle(vec2 center, float radius, color c)` | the same, from a `vec2` |
| `graphics2d.drawCircleOutline(float x, float y, float radius, color c)` | a 1-pixel circle outline |

#### Lines and triangles

| Function | Draws |
|---|---|
| `graphics2d.drawLine(float x1, float y1, float x2, float y2, color c, float thickness = 1)` | a line between two points |
| `graphics2d.drawLine(vec2 from, vec2 to, color c, float thickness = 1)` | the same, from `vec2`s |
| `graphics2d.drawTriangle(vec2 a, vec2 b, vec2 c, color fill)` | a filled triangle (the corners can be in any order) |

#### Text

| Function | Gives back | What it does |
|---|---|---|
| `graphics2d.drawText(string text, float x, float y, int size, color c)` | — | draws text with its top-left corner at (x, y). `size` is the height in pixels. |
| `graphics2d.textWidth(string text, int size)` | `int` | how wide that text would be, in pixels |
| `graphics2d.drawFPS(float x, float y)` | — | draws the current frames per second |

This uses the built-in pixel font. For other fonts, see `Font` (see “`<assets>`: Images, Animations and Fonts” › “Font”).

Centering text:

```cpp
string title = "GAME OVER";
int w = graphics2d.textWidth(title, 48);
graphics2d.drawText(title, (window.width() - w) / 2.0, 200, 48, color.WHITE);
```

#### Other

| Function | What it does |
|---|---|
| `graphics2d.clear(color c)` | fills the whole screen with a color (the window already clears to its background color each frame) |

### Camera: scrolling and zooming

When a level is bigger than the window, use a **camera**. Everything drawn after `graphics2d.setCamera(...)` is moved (and zoomed, and turned) so that the camera's `target`, a point in your level, shows at its `offset`, a point on the screen. `graphics2d.resetCamera()` goes back to drawing straight on the screen, which is what you want for the score and menus.

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));   // target, offset

void update(float dt){
    // glide toward the player: a bit of the way there each frame
    camera.target = camera.target.lerp(hero.center(), 5 * dt);
}

void draw(){
    graphics2d.setCamera(camera);      // the world...
    drawLevel();
    graphics2d.drawRect(hero.bounds(), color.ORANGE);
    graphics2d.resetCamera();          // ...and then the text, which stays put
    graphics2d.drawText("Coins: {coins}", 20, 20, 24, color.GOLD);
}
```

| `Camera2D` field | What it is |
|---|---|
| `vec2 target` | the point in the world the camera looks at |
| `vec2 offset` | where on the screen `target` shows. Use the middle of the window (`vec2(window.width() / 2.0, window.height() / 2.0)`) to keep the target centered |
| `float zoom` | `1` is normal size, `2` makes everything twice as big, `0.5` half. Must be more than 0 |
| `float rotation` | turns the view, in degrees |

Make one with `Camera2D(target, offset)` or `Camera2D(target, offset, zoom, rotation)`.

| Function | What it does |
|---|---|
| `graphics2d.setCamera(Camera2D camera)` | draw through this camera from now on. It stays set (even in later frames) until `resetCamera` |
| `graphics2d.resetCamera()` | stop using the camera: draw straight on the screen again |
| `graphics2d.camera()` | the camera that was set (a copy) |
| `graphics2d.screenToWorld(vec2 point)` | where a point on the screen is in the world. Use it for the mouse: `graphics2d.screenToWorld(input.mousePosition())` |
| `graphics2d.worldToScreen(vec2 point)` | where a point in the world shows on the screen |

Things to know:

- `Camera2D` is a struct, so `setCamera` takes a copy. After changing `camera.target`, call `setCamera(camera)` again (usually at the start of `draw()`, every frame).
- To stop the camera from showing past the edges of the level, keep the target away from them with `math.clamp`: `math.clamp(x, window.width() / 2.0, LEVEL_WIDTH - window.width() / 2.0)`.
- Physics doesn't know about the camera. Bodies keep their world positions; only the drawing moves.
- The 3D camera (`Camera`, in `<graphics3d>`) is a different thing. A game can use both: 3D drawing ignores the 2D camera, and the 2D camera comes back at the next 2D drawing.

### Colors and transparency

Every function takes a `color` (see “Built-in Types: vec2, vec3, vec4, color, rect” › “color”). Make one with `color(r, g, b)`, use a named one like `color.RED`, or make it see-through with `color(255, 0, 0, 100)` or `color.RED.fade(0.4)`.

### Images

To draw pictures from files, see `<assets>` (see “`<assets>`: Images, Animations and Fonts”) (`Texture`). For sprite sheets and animation, see the sprites guide (see “Sprites and Animation”).

### Mixing with 3D

2D drawing can be mixed with `<graphics3d>` (see “`<graphics3d>`: 3D Drawing”) in the same frame. 2D drawn after 3D appears on top of it, which is how you'd draw a HUD over a 3D scene.

### Example: a health bar

```cpp
void drawHealthBar(float x, float y, int hp, int maxHp){
    float width = 200;
    float filled = width * hp / maxHp;
    graphics2d.drawRect(x, y, width, 20, color.DARKGRAY);
    graphics2d.drawRect(x, y, filled, 20, hp > maxHp / 4 ? color.GREEN : color.RED);
    graphics2d.drawRectOutline(x, y, width, 20, color.WHITE, 2);
    graphics2d.drawText("{hp}/{maxHp}", x + 8, y + 2, 16, color.WHITE);
}
```

<!-- docs/libraries/graphics3d.md -->

## `<graphics3d>`: 3D Drawing

```cpp
#include <graphics3d>
```

Draws 3D shapes and models through a camera. See examples/world3d (the file `examples/world3d/main.frg` in the Forge folder) for a working scene.

### 3D coordinates

- Positions are `vec3` (see “Built-in Types: vec2, vec3, vec4, color, rect” › “vec3”): `(x, y, z)`.
- **y points up**, unlike the screen in 2D. The ground is usually y = 0.
- Units are whatever you decide (1 unit = 1 meter is common). Shapes are sized in the same units.

### The camera

What you see depends on the camera: where it is, and what it's looking at.

```cpp
Camera cam = Camera(vec3(8, 6, 8), vec3(0, 0, 0));   // standing at (8, 6, 8), looking at the middle
graphics3d.setCamera(cam);
```

`Camera` is a struct:

| Field | Type | Meaning |
|---|---|---|
| `position` | `vec3` | where the camera is |
| `target` | `vec3` | the point it looks at |
| `up` | `vec3` | which way is up for the camera, `(0, 1, 0)` to start with |
| `fov` | `float` | field of view in degrees (how wide it sees), 45 to start with |

| Making one | |
|---|---|
| `Camera(vec3 position, vec3 target)` | |
| `Camera()` | at `(0, 5, 10)`, looking at `(0, 0, 0)` |

| Method | What it does |
|---|---|
| `orbit(float degrees)` | moves the camera around its target (around the up axis), keeping its distance and height |

| Function | What it does |
|---|---|
| `graphics3d.setCamera(Camera camera)` | uses this camera for the 3D drawing that comes after |
| `graphics3d.camera()` | gives back the camera being used |

Because `Camera` is a struct, changing your variable doesn't change the camera being used. Call `setCamera` again after changing it, usually every frame:

```cpp
void update(float dt){
    cam.orbit(30 * dt);           // 30 degrees per second
    graphics3d.setCamera(cam);
}
```

### Shapes

| Function | Draws |
|---|---|
| `graphics3d.drawCube(vec3 position, vec3 size, color c)` | a solid box **centered** on position |
| `graphics3d.drawCubeOutline(vec3 position, vec3 size, color c)` | the edges of a box |
| `graphics3d.drawSphere(vec3 center, float radius, color c)` | a solid ball |
| `graphics3d.drawSphereOutline(vec3 center, float radius, color c)` | a wireframe ball |
| `graphics3d.drawCylinder(vec3 position, float radius, float height, color c)` | a cylinder standing on `position`, going up by `height` |
| `graphics3d.drawPlane(vec3 center, vec2 size, color c)` | a flat rectangle on the ground (x-z plane) |
| `graphics3d.drawGrid(int slices, float spacing)` | grid lines on the ground around the middle, handy for seeing where things are |
| `graphics3d.drawLine(vec3 from, vec3 to, color c)` | a line |

There's no lighting yet: shapes are drawn in flat colors. Outlines (`drawCubeOutline`) help show their shape.

### Models

Load 3D models made in programs like Blender:

```cpp
Model ship = graphics3d.loadModel("ship.glb");
```

| Function | What it does |
|---|---|
| `graphics3d.loadModel(string path)` | loads a model file from the game folder. Supported: `.obj`, `.gltf`, `.glb`, `.iqm`, `.vox`, `.m3d`. |

| `Model` method | What it does |
|---|---|
| `draw(vec3 position, float scale = 1, color tint = color.WHITE)` | draws the model there |
| `drawRotated(vec3 position, vec3 axis, float degrees, float scale = 1, color tint = color.WHITE)` | draws it turned `degrees` around `axis` (for example `vec3(0, 1, 0)` spins around the up axis) |

Like textures (see “`<assets>`: Images, Animations and Fonts”), the file is found when it's loaded (a missing file is reported straight away), but it's only sent to the graphics card when it's first drawn, so loading as a global variable works. Models that include their own textures and colors (`.glb`, `.gltf`) are drawn with them; `tint` multiplies them.

### 3D points on the 2D screen

| Function | Gives back | What it does |
|---|---|---|
| `graphics3d.toScreen(vec3 point)` | `vec2` | where a 3D point appears in the window, using the current camera |

Use it to draw 2D labels, health bars or markers over 3D things:

```cpp
vec2 p = graphics3d.toScreen(enemyPosition + vec3(0, 2, 0));
graphics2d.drawText("Enemy", p.x - 30, p.y, 20, color.WHITE);
```

### Mixing 2D and 3D

Every 3D drawing call switches into 3D mode by itself, and every 2D call (see “`<graphics2d>`: 2D Drawing”) switches back. You can mix them freely in one frame; 2D drawn after 3D appears on top. The usual order is: 3D world first, then the 2D HUD.

### Example

```cpp
#include <window>
#include <graphics3d>
#include <math>

Camera cam = Camera(vec3(6, 5, 6), vec3(0, 0, 0));
float t = 0;

int main(){
    window.open(960, 540, "Bouncing ball");
    window.run();
    return 0;
}

void update(float dt){
    t += dt;
    cam.orbit(20 * dt);
    graphics3d.setCamera(cam);
}

void draw(){
    graphics3d.drawGrid(10, 1);
    graphics3d.drawCube(vec3(0, 1, 0), vec3(2, 2, 2), color.SKYBLUE);
    graphics3d.drawCubeOutline(vec3(0, 1, 0), vec3(2, 2, 2), color.WHITE);
    graphics3d.drawSphere(vec3(0, 3 + math.abs(math.sin(t * 3)), 0), 0.5, color.ORANGE);
}
```

<!-- docs/libraries/assets.md -->

## `<assets>`: Images, Animations and Fonts

```cpp
#include <assets>
```

Loads pictures (`Texture`) and fonts (`Font`) from files, and draws them. `Animation` plays the frames of a sprite sheet. See the sprites guide (see “Sprites and Animation”) for a walk-through.

### Where files go

Put asset files in the game's top folder (next to `main.frg`), or in a subfolder: `assets.loadTexture("art/player.png")`. When loading, Forge looks:

1. relative to the folder the game was started from (with `frg run`, that's the game's top folder),
2. next to the `.exe`,
3. in the folder above the `.exe`.

So the game finds its files whether it's started with `frg run` or by double-clicking `build/game.exe`. A missing file stops the game with "can't find the file 'player.png' - put it in the game's folder (next to main.frg), or check the spelling". Capitals matter for the error message, though Windows itself ignores them.

### Loading

| Function | Gives back | What it does |
|---|---|---|
| `assets.loadTexture(string path)` | `Texture` | loads an image: `.png`, `.jpg`, `.bmp`, `.tga`, `.gif` (first frame) or `.qoi` |
| `assets.loadFont(string path, int quality = 64)` | `Font` | loads a font: `.ttf` or `.otf`. `quality` is the size the letters are prepared at; use about the biggest size you'll draw it at. |
| `assets.exists(string path)` | `bool` | true if the file can be found (using the same search) |

**Load once, draw many times.** Load assets once, as global variables or at the start of the game, never inside `draw()`, which runs 60 times a second:

```cpp
Texture player = assets.loadTexture("player.png");    // good: loaded once
```

Loading works even **before** `window.open(...)`. The file is read straight away (so a missing file is reported straight away), and sent to the graphics card the first time it's drawn.

### Texture

A loaded image.

| Method | What it does |
|---|---|
| `width()` | the image's width in pixels (`int`) |
| `height()` | the image's height in pixels (`int`) |
| `draw(float x, float y, color tint = color.WHITE)` | draws it at its normal size with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | draws it `scale` times as big (2 = double size) |
| `drawRotated(vec2 center, float degrees, float scale = 1, color tint = color.WHITE)` | draws it **centered** on `center`, turned `degrees` clockwise |
| `drawPart(rect source, rect destination, color tint = color.WHITE)` | draws the `source` part of the image into the `destination` rectangle on screen, stretched to fit. This is how sprite sheets are drawn. |

**Tint** multiplies the image's colors: `color.WHITE` leaves it as it is, `color.RED` makes it reddish, and `color.WHITE.fade(0.5)` makes it half see-through.

**Flipping:** give `drawPart` a source with a negative width to flip the image left-to-right: `rect(0, 0, -16, 16)`. A negative height flips it upside down.

```cpp
Texture hero = assets.loadTexture("hero.png");

void draw(){
    hero.draw(100, 100);
    hero.drawScaled(200, 100, 4);                    // pixel art, 4x bigger
    hero.drawRotated(vec2(400, 200), angle, 2);
    hero.drawPart(rect(0, 0, 16, 16), rect(500, 100, 64, 64));
}
```

Pixel art stays sharp when scaled up; there's no blurring.

### Animation

Plays the frames of a **sprite sheet** (one image holding every frame of an animation, side by side) one after the other. The sprites and animation guide (see “Sprites and Animation” › “Animation”) explains it step by step.

```cpp
Texture sheet = assets.loadTexture("hero.png");
Animation walk = Animation(sheet, 16, 16, 4, 8);     // 16x16 frames, 4 of them, 8 per second

void update(float dt){
    walk.update(dt);
}

void draw(){
    walk.drawScaled(100, 100, 4);
}
```

| Making one | |
|---|---|
| `Animation(Texture sheet, int frameWidth, int frameHeight, int frames, float fps)` | frames starting at the top-left of the sheet, read left to right (carrying on in the next row if needed) |
| `Animation(sheet, frameWidth, frameHeight, frames, fps, int row)` | frames starting at the beginning of row `row` (0 is the top row), for sheets with one row per animation |

| Method or field | What it does |
|---|---|
| `update(float dt)` | moves the animation on by `dt` seconds. Call it once per frame |
| `draw(float x, float y, color tint = color.WHITE)` | draws the current frame at its real size, with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | the same, `scale` times bigger |
| `play()` / `pause()` | starts and stops it. A paused animation stays on its frame |
| `reset()` | back to the first frame, playing |
| `flip(bool flipped)` | `flip(true)` draws it mirrored left-to-right, `flip(false)` normally |
| `isFlipped()`, `isPlaying()` | what was chosen |
| `frame()`, `setFrame(int frame)`, `frameCount()` | the frame being shown (from 0), jumping to one, and how many there are |
| `float fps` | frames per second; change it any time (0 stops it) |
| `bool looping` | `true` (the start): after the last frame it starts again. `false`: it stops on the last frame |
| `isDone()` | `true` when an animation that doesn't loop has reached its last frame |

If the sheet doesn't have room for the frames you asked for, the game stops with a message saying how big the sheet really is. `Animation` is a class, so `Animation current = walk;` makes `current` mean the very same animation, not a copy.

### Font

A loaded font.

| Method | Gives back | What it does |
|---|---|---|
| `drawText(string text, float x, float y, float size, color c)` | — | draws text with its top-left corner at (x, y). `size` is the height in pixels. |
| `measure(string text, float size)` | `float` | how wide the text would be, in pixels |

```cpp
Font title = assets.loadFont("fonts/Title.ttf", 96);

void draw(){
    string text = "Forge Quest";
    float w = title.measure(text, 72);
    title.drawText(text, (window.width() - w) / 2.0, 80, 72, color.GOLD);
}
```

`Font()` (made directly, not loaded) is the built-in pixel font.

Fonts are prepared for drawing the first time they're used, which needs the window to be open. `measure` also needs the window.

### Memory

Textures and fonts are freed automatically when nothing uses them anymore, and everything is cleaned up when the window closes. There's no need to unload anything.

<!-- docs/libraries/audio.md -->

## `<audio>`: Sounds and Music

```cpp
#include <audio>
```

Plays sound effects and music. Audio files go in the game folder and are found the same way as assets (see “`<assets>`: Images, Animations and Fonts” › “Where files go”).

### Sound vs Music

| | `Sound` | `Music` |
|---|---|---|
| For | short effects: jump, hit, coin | long tracks: background music |
| Loaded | completely into memory | streamed from the file a bit at a time while it plays |
| Formats | `.wav`, `.ogg`, `.mp3`, `.flac`, `.qoi` | `.ogg`, `.mp3`, `.wav`, `.flac`, `.xm`, `.mod` |
| Loops by default | no | yes |

### Loading

| Function | Gives back |
|---|---|
| `audio.loadSound(string path)` | `Sound` |
| `audio.loadMusic(string path)` | `Music` |

Load once (as global variables, or at the start of the game), not every time a sound plays:

```cpp
Sound jump = audio.loadSound("sounds/jump.wav");
Music theme = audio.loadMusic("music/theme.ogg");
```

### Sound

| Method | What it does |
|---|---|
| `play()` | plays the sound from the start. Playing it again while it's playing restarts it. |
| `stop()` | stops it |
| `isPlaying()` | `true` while it's playing |
| `setVolume(float volume)` | 0.0 (silent) to 1.0 (full volume) |
| `setPitch(float pitch)` | 1.0 is normal; 2.0 is an octave higher and twice as fast, 0.5 an octave lower |

Tip: a slightly random pitch makes repeated sounds less tiring to hear: `jump.setPitch(random.range(0.9, 1.1)); jump.play();`

### Music

| Method | What it does |
|---|---|
| `play()` | starts from the beginning |
| `stop()` | stops it (the next `play()` starts from the beginning) |
| `pause()` | pauses it |
| `resume()` | carries on from where it was paused |
| `isPlaying()` | `true` while it's playing |
| `setVolume(float volume)` | 0.0 to 1.0 |
| `setLooping(bool looping)` | whether it starts over when it ends (on to start with) |
| `length()` | the track's length in seconds (`float`) |
| `position()` | how far through it is, in seconds (`float`) |

Music has to be fed to the speakers a little at a time. **With a window open, Forge does this every frame by itself.** In a program without a window, call `audio.update()` often (many times a second), or the music stutters and stops.

### Volume

| Function | What it does |
|---|---|
| `audio.setVolume(float volume)` | the volume of everything, 0.0 to 1.0 |
| `audio.update()` | keeps music playing in programs without a window |

### No sound device

If the computer has no working sound device, Forge prints `[audio] no sound device found - sounds will be silent` once. Everything else keeps working; sounds just don't make any noise.

### Example

```cpp
#include <window>
#include <input>
#include <audio>

Sound coin = audio.loadSound("coin.wav");
Music song = audio.loadMusic("song.ogg");

int main(){
    window.open(800, 450, "Audio");
    song.setVolume(0.6);
    song.play();
    window.run();
    return 0;
}

void update(float dt){
    if (input.keyPressed(Key.SPACE)) {
        coin.play();
    }
    if (input.keyPressed(Key.M)) {
        if (song.isPlaying()) {
            song.pause();
        } else {
            song.resume();
        }
    }
}

void draw(){
}
```

<!-- docs/libraries/physics.md -->

## `<physics>`: Boxes, Gravity and Collisions

```cpp
#include <physics>
```

Simple 2D physics made for platformers. Every **body** is an axis-aligned box (never rotated). Each step, moving bodies fall with gravity, move by their velocity, and get pushed out of anything solid they run into.

It works in screen coordinates (see “`<graphics2d>`: 2D Drawing” › “Coordinates”): y grows **downward**, so gravity pulls toward +y and jumping means a **negative** y velocity.

This library doesn't draw anything: draw bodies yourself, for example with `graphics2d.drawRect(body.bounds(), color.ORANGE)`. It doesn't need a window either.

### Body

```cpp
Body crate = Body(100, 50, 32, 32);    // x, y (top-left corner), width, height
physics.add(crate);
```

| Making one | |
|---|---|
| `Body(float x, float y, float width, float height)` | a moving, solid box |

| Field | Type | Meaning |
|---|---|---|
| `position` | `vec2` | top-left corner |
| `size` | `vec2` | width and height |
| `velocity` | `vec2` | pixels per second |
| `isStatic` | `bool` | `true` for things that never move: floors, walls, platforms. Static bodies ignore gravity and velocity. (`false` to start with.) |
| `solid` | `bool` | `false` for things that shouldn't push or be pushed, like coins and trigger zones. (`true` to start with.) |
| `oneWay` | `bool` | for static bodies: `true` makes a **one-way platform** that you can jump up through from below and walk through sideways, but land on from above. (`false` to start with.) |
| `gravityScale` | `float` | how strongly gravity pulls this body: 1 is normal, 0 floats, 2 is twice as heavy |
| `onGround` | `bool` | **set by Forge:** true if it landed on something during the last step |
| `onCeiling` | `bool` | **set by Forge:** true if it bumped its head |
| `onWall` | `bool` | **set by Forge:** true if it hit something sideways |

| Method | Gives back | What it is |
|---|---|---|
| `bounds()` | `rect` | the body's rectangle, handy for drawing it |
| `center()` | `vec2` | the middle point |
| `overlaps(Body other)` | `bool` | true if the two boxes overlap right now |

### The world

| Function | What it does |
|---|---|
| `physics.add(Body body)` | adds a body to the world. Adding it twice does nothing. |
| `physics.remove(Body body)` | takes it out of the world |
| `physics.step(float dt)` | moves everything forward by `dt` seconds. Call it **once per frame**. |
| `physics.setGravity(vec2 gravity)` | gravity, in pixels per second per second. Starts at `vec2(0, 980)`. |
| `physics.gravity()` | the current gravity |
| `physics.clear()` | removes every body |
| `physics.count()` | how many bodies are in the world |

Bodies only move during `physics.step`, and only if they've been added.

### Seeing the bodies: the debug view

When something collides in a way you don't expect, draw what physics sees:

```cpp
#include <graphics2d>    // physics.drawDebug() draws with graphics2d, so the file needs both
#include <physics>

void draw(){
    level.draw();
    player.draw();
    physics.drawDebug();     // on top of everything else
}
```

`physics.drawDebug()` draws the outline of every body in the world:

| Outline | Body |
|---|---|
| blue | static (`isStatic = true`): floors, walls, platforms |
| light blue, with a thick top edge | a one-way platform (`oneWay = true`) |
| green | moving |
| purple | not solid (`solid = false`): coins, triggers |
| a yellow bar along the bottom | a moving body that's standing on something (`onGround` is true) |

It's drawn like anything else in `<graphics2d>`, so with a 2D camera (see “`<graphics2d>`: 2D Drawing” › “Camera: scrolling and zooming”), call it before `resetCamera()` so the outlines line up with the level. Take the line out (or put it behind a key, like `if (showDebug) { ... }`) when you're done.

The physics library itself never draws (it doesn't use raylib), which is why the file calling `drawDebug` needs `#include <graphics2d>`. Without it, Forge says so.

### What happens in a step

For each moving (not static) body:

1. `onGround`, `onCeiling` and `onWall` are reset to false.
2. Gravity is added to its velocity: `velocity += gravity × gravityScale × dt`.
3. It moves sideways by `velocity.x × dt`. If that runs it into a static solid body, it's pushed back out, `velocity.x` becomes 0, and `onWall` becomes true.
4. It moves up or down by `velocity.y × dt`. If that runs it into a static solid body, it's pushed out, `velocity.y` becomes 0, and `onGround` (when moving down) or `onCeiling` (when moving up) becomes true. A one-way platform (`oneWay = true`) only counts here, and only when the body is falling onto it from above; in step 3 it never counts.
5. Fast bodies move in several small steps, so they can't pass straight through thin platforms.

Then moving solid bodies that overlap each other are pushed apart, in whichever direction they overlap least. A body resting on the ground isn't pushed down into it; the one on top is pushed up instead (and gets `onGround`), so things stack.

To keep things stable after a pause, `dt` is capped at 0.05 seconds per step.

### Patterns

#### Walking and jumping

```cpp
void update(float dt){
    float dir = 0;
    if (input.keyDown(Key.LEFT)) { dir -= 1; }
    if (input.keyDown(Key.RIGHT)) { dir += 1; }
    player.velocity.x = dir * 300;                    // set x speed directly, every frame

    if (input.keyPressed(Key.SPACE) and player.onGround) {
        player.velocity.y = -600;                     // up is negative
    }
    physics.step(dt);
}
```

Read `onGround` **before** calling `physics.step` in the same frame: it describes the previous step.

#### One-way platforms

Platforms you can jump up through from below, and then stand on, like in many classic platformers:

```cpp
Body ledge = Body(300, 320, 160, 12);
ledge.isStatic = true;
ledge.oneWay = true;
physics.add(ledge);
```

A body only lands on it when it's **falling** and was **above** the platform's top just before. From below and from the sides, it passes straight through. `oneWay` only matters for static bodies. In the debug view (see “Seeing the bodies: the debug view”), one-way platforms are light blue with a thick top edge.

#### Platforms and walls

To build a whole level of walls and platforms, drawing it as text with `<tilemap>` (see “`<tilemap>`: Levels Drawn as Text”) is usually easier: `level.addBodies("#")` makes the static bodies for you.

```cpp
void addPlatform(float x, float y, float w, float h){
    Body b = Body(x, y, w, h);
    b.isStatic = true;
    physics.add(b);
}
```

#### Collectibles and triggers

```cpp
Body coin = Body(300, 200, 16, 16);
coin.isStatic = true;
coin.solid = false;          // the player passes through it
physics.add(coin);

// every frame:
if (player.overlaps(coin)) {
    physics.remove(coin);
    score++;
}
```

#### Your own body classes

Classes can inherit from `Body` to add their own fields and methods. They use `Body`'s constructor if they don't have one:

```cpp
class Player : Body {
    int lives = 3;
    void respawn() {
        position = vec2(80, 300);
        velocity = vec2.ZERO;
    }
}

Player hero = Player(80, 300, 32, 32);
```

### Limits

- Boxes only: no circles, slopes or rotation.
- Static bodies don't move by themselves, but you can change their `position` yourself (moving platforms). Bodies standing on one aren't carried along automatically.
- There's no bounciness or friction: set `velocity` yourself for those effects.

<!-- docs/libraries/time.md -->

## `<time>`: Clock, Date and Timers

```cpp
#include <time>
```

### Functions

| Function | Gives back | What it is |
|---|---|---|
| `time.now()` | `double` | seconds since the game started (real time) |
| `time.sleep(float seconds)` | — | stops **everything** for that long |
| `time.clock()` | `string` | the time of day, like `"14:05:09"` |
| `time.date()` | `string` | today's date, like `"2026-10-14"` |
| `time.year()` | `int` | for example `2026` |
| `time.month()` | `int` | 1 (January) to 12 |
| `time.day()` | `int` | day of the month, 1–31 |
| `time.hour()` | `int` | 0–23 |
| `time.minute()` | `int` | 0–59 |
| `time.second()` | `int` | 0–59 |

The clock and date are the computer's local time.

**Don't use `time.sleep` in a windowed game.** It freezes the whole game: nothing draws and input stops. To wait inside game logic, use a task (see “11. Tasks”) with `wait(seconds)`, or a `Timer`.

### Timer

A countdown, for cooldowns, spawn intervals, power-up durations and so on.

```cpp
Timer cooldown = Timer(0.5);        // half a second

void update(float dt){
    if (input.keyDown(Key.SPACE) and cooldown.isDone()) {
        shoot();
        cooldown.restart();
    }
}
```

| Making one | |
|---|---|
| `Timer(float seconds)` | starts counting straight away |

| Method | Gives back | What it does |
|---|---|---|
| `isDone()` | `bool` | true once the time has run out |
| `left()` | `float` | seconds remaining (0 when done) |
| `elapsed()` | `float` | seconds since it started or was restarted |
| `restart()` | — | starts counting again from now |

A Timer measures **real** time, so it keeps counting while the game is paused. For game-time effects that should pause with the game, use a task (see “11. Tasks”) or count `dt` yourself.

Showing a countdown:

```cpp
Timer roundTimer = Timer(60);
graphics2d.drawText("{int(roundTimer.left())}", 20, 20, 32, color.WHITE);
```

<!-- docs/libraries/file.md -->

## `<file>`: Reading and Writing Text Files

```cpp
#include <file>
```

Reads and writes text files: level data, lists, your own file formats. (To simply remember a high score or settings between runs, `<save>` (see “`<save>`: Remembering Things Between Runs”) is easier.)

### Where files go

Paths are relative to the folder the game was **started from**:

- with `frg run`, that's the game's top folder (next to `main.frg`);
- when someone double-clicks the `.exe`, it's the folder the `.exe` is in.

Subfolders are fine (`"saves/slot1.txt"`). `write`, `append` and `writeLines` create missing folders by themselves.

### Functions

| Function | Gives back | What it does |
|---|---|---|
| `file.read(string path)` | `string` | the whole file as text. If the file doesn't exist, the game stops with a runtime error, so check with `exists` first. |
| `file.write(string path, string text)` | `bool` | replaces the file's contents with `text` (creating the file if needed). True if it worked. |
| `file.append(string path, string text)` | `bool` | adds `text` to the end of the file (creating it if needed) |
| `file.exists(string path)` | `bool` | true if there's a file there |
| `file.remove(string path)` | `bool` | deletes the file; true if there was one |
| `file.readLines(string path)` | `list<string>` | the file split into lines (without the line breaks) |
| `file.writeLines(string path, list<string> lines)` | `bool` | writes each item as a line |
| `file.files(string folder)` | `list<string>` | the names of the files in a folder (not subfolders), in alphabetical order; empty if the folder doesn't exist |

Windows line endings (`\r\n`) are turned into plain `\n` when reading.

### Saving and loading a high score

```cpp
#include <file>

int loadHighScore(){
    if (!file.exists("highscore.txt")) {
        return 0;
    }
    return int(file.read("highscore.txt").trim());
}

void saveHighScore(int score){
    file.write("highscore.txt", "{score}");
}
```

### Saving several values: one per line as key=value

For saving settings and progress, the `<save>` (see “`<save>`: Remembering Things Between Runs”) library does all of this for you (`save.setInt("coins", coins)`, `save.getInt("coins")`). This section shows how to do it by hand with `<file>`, for when you want your own format.

```cpp
void saveGame(){
    list<string> lines = [
        "level={level}",
        "coins={coins}",
        "name={playerName}"
    ];
    file.writeLines("save.txt", lines);
}

void loadGame(){
    if (!file.exists("save.txt")) {
        return;
    }
    for (string line in file.readLines("save.txt")) {
        list<string> parts = line.split("=");
        if (parts.length != 2) {
            continue;
        }
        switch (parts[0]) {
            case "level":
                level = int(parts[1]);
            case "coins":
                coins = int(parts[1]);
            case "name":
                playerName = parts[1];
        }
    }
}
```

### Notes

- Files are text only (no images or raw bytes).
- `int(text)` stops the game if the text isn't a number. If players might edit your save files, check values before converting.

<!-- docs/libraries/save.md -->

## `<save>`: Remembering Things Between Runs

```cpp
#include <save>
```

Keeps values (a high score, the music volume, which level the player reached) after the game closes, so they're still there next time. Each value has a name, called its **key**.

```cpp
#include <stdio>
#include <save>

int main(){
    int runs = save.getInt("runs") + 1;     // 0 the first time, so this is 1
    save.setInt("runs", runs);
    print("You've started this game {runs} times");
    return 0;
}
```

Run it a few times and the number goes up.

### Functions

| Function | What it does |
|---|---|
| `save.setInt(string key, int value)` | saves a whole number |
| `save.getInt(string key, int backup = 0)` | the saved whole number, or `backup` if there isn't one |
| `save.setFloat(string key, float value)` | saves a decimal number |
| `save.getFloat(string key, float backup = 0)` | the saved decimal number, or `backup` |
| `save.setString(string key, string value)` | saves text |
| `save.getString(string key, string backup = "")` | the saved text, or `backup` |
| `save.setBool(string key, bool value)` | saves `true` or `false` |
| `save.getBool(string key, bool backup = false)` | the saved bool, or `backup` |
| `save.has(string key)` | `true` if something is saved under that key |
| `save.remove(string key)` | forgets one value |
| `save.clear()` | forgets everything (for a "Reset save" button) |

The **backup** is what you get when nothing is saved yet, like the very first time the game runs. It's also what you get if the value was saved as a different kind: `getInt` on a key that holds `"hello"` gives the backup.

Saving the same key again replaces the old value.

### Where it's saved

Everything goes into one file, **`save.txt`, in the game's folder**: the folder with `main.frg` while you're making the game, or the folder with the `.exe` in a finished game. Each `set...`, `remove` and `clear` writes the file **straight away**, so nothing is lost if the game crashes or is closed suddenly. (It's a small file, so this is fast.)

The file is plain text, one `key=value` per line, so you can open it to see what's saved:

```
# Forge save file: one key=value per line
best=120
music=true
name=Ana
volume=0.75
```

To start over while testing, delete `save.txt` (or call `save.clear()`).

### Example: a best score

```cpp
#include <save>

int best = save.getInt("best");

void gameOver(int score){
    if (score > best) {
        best = score;
        save.setInt("best", best);
    }
}
```

### Example: settings

```cpp
#include <save>
#include <audio>

void loadSettings(){
    audio.setVolume(save.getFloat("volume", 0.8));    // 0.8 until the player changes it
}

void setVolume(float v){
    audio.setVolume(v);
    save.setFloat("volume", v);
}
```

### Things to know

- Keys can be any text except `""`. Saving with an empty key stops the game with "the key can't be empty text".
- `save` is a library name, so your own functions and variables can't be called `save`. Use something like `saveGame`.
- For your own file formats (a level made of characters, a list of lines), use `<file>` (see “`<file>`: Reading and Writing Text Files”) instead.

<!-- docs/libraries/tween.md -->

## `<tween>`: Smooth Changes Over Time

```cpp
#include <tween>
```

A **tween** changes a number smoothly from one value to another over some time: a menu sliding in, a title dropping and bouncing, a coin fading out. ("Tween" comes from "in-between": it works out every value between the start and the end.)

```cpp
Tween slideIn = Tween(-300, 40, 0.5, Ease.OUT);   // from -300 to 40, in half a second

void update(float dt){
    slideIn.update(dt);
}

void draw(){
    ui.panel(rect(slideIn.value(), 100, 280, 300));  // the panel slides in from the left
}
```

Every frame: `update(dt)` moves it on, then `value()` is where it is now.

### Tween

| Making one | |
|---|---|
| `Tween(float from, float to, float seconds, Ease ease = Ease.OUT)` | goes from `from` to `to` in `seconds` |

| Method or field | What it does |
|---|---|
| `update(float dt)` | moves it on by `dt` seconds. Call it once per frame |
| `value()` | the number now: `from` at the start, `to` at the end |
| `progress()` | how far along it is, from 0 to 1 (with the easing). Use it with `lerp`, below |
| `isDone()` | `true` once it has reached the end |
| `restart()` | back to the start |
| `reverse()` | goes back from where it is now to where it started, like a menu sliding back out |
| `bool looping` | `true`: when it reaches the end, it starts again (forever) |
| `bool pingPong` | `true`: when it reaches the end, it goes back again. With `looping` too, it goes back and forth forever |

A time of `0` jumps straight to the end. A negative time stops the game with an error.

### Easing: the shape of the change

`Ease` decides how the value moves between the start and the end:

| `Ease.` | Movement | Good for |
|---|---|---|
| `LINEAR` | the same speed all the way | progress bars, steady movement |
| `IN` | starts slowly, ends fast | things falling, leaving the screen |
| `OUT` (the default) | starts fast, ends slowly | things arriving: menus, pop-ups |
| `IN_OUT` | slow, fast, slow | moving something from one place to another |
| `BOUNCE` | falls in and bounces at the end, like a dropped ball | titles, pickups landing |

`tween.ease(Ease ease, float t)` gives the eased amount for `t` from 0 to 1, if you want the shape without a `Tween`: `tween.ease(Ease.OUT, 0.5)` is `0.875`.

### Positions and colors

A tween gives one number. To move a `vec2` or change a `color`, tween from 0 to 1 and use `progress()` with `lerp`:

```cpp
Tween fade = Tween(0, 1, 2, Ease.LINEAR);
vec2 start = vec2(100, 100);
vec2 end = vec2(500, 300);

void update(float dt){
    fade.update(dt);
}

void draw(){
    vec2 p = start.lerp(end, fade.progress());
    color c = color.WHITE.lerp(color.BLANK, fade.progress());    // fades out
    graphics2d.drawCircle(p, 20, c);
}
```

### Examples

#### A title that drops in and bounces

```cpp
Tween titleDrop = Tween(-80, 70, 0.9, Ease.BOUNCE);

void update(float dt){
    titleDrop.update(dt);
}

void draw(){
    graphics2d.drawText("FORGE", 380, titleDrop.value(), 64, color.ORANGE);
}
```

examples/menu (the file `examples/menu/main.frg` in the Forge folder) does this.

#### Something pulsing forever

```cpp
Tween pulse = Tween(1, 1.3, 0.4, Ease.IN_OUT);

int main(){
    pulse.looping = true;
    pulse.pingPong = true;
    ...
}

void draw(){
    coin.drawScaled(x, y, 2 * pulse.value());
}
```

#### In a task

A task (see “11. Tasks”) can play a tween from start to end on its own:

```cpp
float menuX = -300;

task showMenu() {
    Tween t = Tween(-300, 40, 0.5);
    while (!t.isDone()) {
        t.update(window.deltaTime());
        menuX = t.value();
        wait();                      // the next frame
    }
}
```

### Why it works like this

A tween is a value you update and read, like a `Timer` (see “`<time>`: Clock, Date and Timers”). Forge doesn't have functions as values yet, so a tween can't run code by itself when it ends: check `isDone()` instead. That also means it works the same way in every kind of game loop, and in tasks.

<!-- docs/libraries/particles.md -->

## `<particles>`: Dust, Sparks and Explosions

```cpp
#include <particles>
```

**Particles** are lots of tiny dots that fly out, fade and disappear: dust when a player lands, sparks when something is hit, smoke, fire, explosions. A few lines make a game feel much more alive.

An `Emitter` makes them. Give it a position, then each frame `update(dt)` it and `draw()` it:

```cpp
Emitter sparks = Emitter(vec2(400, 300));

void update(float dt){
    if (input.mousePressed(MouseButton.LEFT)) {
        sparks.position = input.mousePosition();
        sparks.burst(30);              // 30 particles at once
    }
    sparks.update(dt);
}

void draw(){
    sparks.draw();
}
```

### Emitter

| Making one | |
|---|---|
| `Emitter(vec2 position)` | an emitter at that point. It makes nothing until you `burst` or give it a `rate` |

| Method | What it does |
|---|---|
| `burst(int count)` | makes `count` particles right now (an explosion, a puff of dust) |
| `update(float dt)` | moves every particle, makes new ones if `rate` is more than 0, and removes the ones that are too old. Call it once per frame |
| `draw()` | draws every particle. It's 2D drawing, so it goes through the 2D camera (see “`<graphics2d>`: 2D Drawing” › “Camera: scrolling and zooming”) like everything else |
| `count()` | how many particles are alive now |
| `clear()` | removes every particle |

These fields can be changed at any time; the new values are used for the particles made after that:

| Field | Type | Start value | Meaning |
|---|---|---|---|
| `position` | `vec2` | (given) | where new particles start |
| `rate` | `float` | `0` | particles per second, for a steady stream (smoke, fire). `0`: only `burst` makes them |
| `lifetime` | `float` | `1` | how many seconds each particle lives |
| `speed` | `float` | `100` | how fast they fly out, in pixels per second |
| `direction` | `float` | `-90` | which way they fly, in degrees: `0` is right, `-90` up, `90` down, `180` left |
| `spread` | `float` | `360` | how wide the spray is, in degrees around `direction`: `360` is every way, `30` a narrow jet |
| `startColor` | `color` | white | the color when a particle is new... |
| `endColor` | `color` | see-through white | ...and when it disappears. Particles blend between the two, so an `endColor` with alpha 0 fades them out |
| `startSize` | `float` | `8` | the size (across, in pixels) when new... |
| `endSize` | `float` | `2` | ...and at the end |
| `gravity` | `vec2` | `vec2(0, 0)` | pulls the particles, in pixels per second per second: `vec2(0, 400)` makes them fall |

To look natural, each particle's speed and lifetime are a little different (speed from 70% to 130%, lifetime from 80% to 120%).

An emitter keeps at most 5000 particles alive; past that, new ones are skipped.

### Recipes

```cpp
// Dust when landing: a small puff that goes up and out, then settles.
dust.lifetime = 0.4;
dust.speed = 90;
dust.direction = -90;
dust.spread = 160;
dust.gravity = vec2(0, 300);
dust.startColor = color(200, 200, 210);
dust.endColor = color(200, 200, 210, 0);
dust.burst(14);

// Fire: a steady stream going up, from yellow to see-through red.
fire.rate = 80;
fire.direction = -90;
fire.spread = 25;
fire.speed = 60;
fire.startColor = color.YELLOW;
fire.endColor = color.RED.fade(0);
fire.startSize = 14;
fire.endSize = 4;

// An explosion: everything at once, in every direction.
boom.speed = 300;
boom.lifetime = 0.7;
boom.startColor = color.ORANGE;
boom.endColor = color.DARKGRAY.fade(0);
boom.startSize = 12;
boom.burst(80);
```

One emitter can be moved around and reused: set its `position`, then `burst`. Particles already flying stay where they are.

examples/platformer (the file `examples/platformer/` in the Forge folder) uses an emitter for dust when the player lands (in `player.frg`) and one for sparkles when a coin is collected (in `level.frg`).

<!-- docs/libraries/tilemap.md -->

## `<tilemap>`: Levels Drawn as Text

```cpp
#include <tilemap>
```

Instead of placing every platform with numbers, **draw the level with characters**. Each character is one square **tile** of the level:

```cpp
TileMap level = TileMap("""
##################
#                #
#   C      ===   #
#  ===           #
# P       C      #
##################
""", 32);
```

What each character means is up to your game. Here `#` is a wall, `=` a platform, `C` a coin and `P` where the player starts, and a space is nothing. The `32` is the size of a tile in pixels, so this level is 18 × 32 = 576 pixels wide.

Then use the map to make the level:

```cpp
void build(){
    level.addBodies("#");                    // walls become static physics bodies
    level.addBodies("=", true);              // one-way platforms
    for (vec2 corner in level.find("C")) {   // coins
        coins.add(corner + vec2(16, 16));    // the middle of the tile
    }
    hero.position = level.find("P")[0];      // the player's start
}

void draw(){
    level.draw("#", color.DARKGRAY);         // walls as gray squares
    level.draw("=", color.BROWN);
}
```

### Making one

| | |
|---|---|
| `TileMap(string text, float tileSize)` | a map from text: one line per row of tiles. `"""raw text"""` is the easiest way to write it in your code |
| `tilemap.load(string path, float tileSize)` | a map from a text file in the game's folder, like `tilemap.load("level1.txt", 32)` |

The top-left character is column 0, row 0. Lines can have different lengths: short lines are filled with spaces. Blank lines at the end are left out. Characters are single letters and symbols (not `é` or emoji).

Keeping each level in its own file makes it easy to have many levels: `level1.txt`, `level2.txt`...

### Reading and changing tiles

| Method | Gives back | What it does |
|---|---|---|
| `columns()`, `rows()` | `int` | the size of the map in tiles |
| `width()`, `height()` | `float` | the size of the map in pixels |
| `float tileSize` | | the size of one tile, in pixels (a field) |
| `get(int column, int row)` | `string` | the tile there, like `"#"`. Outside the map it's `" "`, so checking neighbours at the edges is safe |
| `set(int column, int row, string tile)` | — | changes a tile, like `level.set(5, 2, " ")` to remove it. Outside the map, the game stops with an error |
| `find(string tiles)` | `list<vec2>` | the **top-left corner**, in pixels, of every tile that's one of these characters, row by row. `find("C")` finds the coins; `find("CD")` finds both `C` and `D` |
| `tileRect(int column, int row)` | `rect` | the rectangle of a tile, in pixels |
| `columnAt(float x)`, `rowAt(float y)` | `int` | which column and row a point in the level is in: `level.get(level.columnAt(p.x), level.rowAt(p.y))` is the tile under `p` |

### Walls and physics

| Method | Gives back | What it does |
|---|---|---|
| `addBodies(string tiles, bool oneWay = false)` | `list<Body>` | makes static physics (see “`<physics>`: Boxes, Gravity and Collisions”) bodies for those tiles, adds them to the physics world, and gives them back. With `true`, they're one-way platforms |
| `solidAreas(string tiles)` | `list<rect>` | the rectangles `addBodies` would use, if you want to do something else with them |

Neighbouring tiles are **merged** into bigger rectangles: a row of 30 wall tiles becomes one body, and a solid block becomes one box. That keeps the number of bodies small, which keeps physics fast, and stops players from catching on the seams between tiles.

Because `addBodies` uses `Body`, a file that includes `<tilemap>` can use the bodies it gives back without including `<physics>` itself, but needs `#include <physics>` to call `physics.step` and to write the type `Body`.

### Drawing

| Method | What it does |
|---|---|
| `draw(string tiles, color c)` | fills every tile that's one of these characters with a color |
| `drawTiles(Texture tileset, int sourceSize, string tiles)` | draws those tiles with pictures from a **tileset** image |

A tileset is one image with many tile pictures side by side, each `sourceSize` pixels square. The first character in `tiles` uses the first picture (top-left), the second character the next picture to its right, and so on (then the next row):

```cpp
Texture tileset = assets.loadTexture("tiles.png");    // 16 x 16 pictures: grass, dirt, plank

void draw(){
    level.drawTiles(tileset, 16, "#.=");     // # is picture 0, . is picture 1, = is picture 2
}
```

Pictures are stretched to the map's `tileSize`, and stay sharp (good for pixel art).

Drawing goes through the 2D camera (see “`<graphics2d>`: 2D Drawing” › “Camera: scrolling and zooming”), so a level wider than the window scrolls like everything else. To stop the camera at the level's edges, use `level.width()`:

```cpp
float half = window.width() / 2.0;
camera.target.x = math.clamp(hero.center().x, half, level.width() - half);
```

For your own extra drawing, go through the tiles with two loops:

```cpp
for (int row = 0; row < level.rows(); row++) {
    for (int column = 0; column < level.columns(); column++) {
        if (level.get(column, row) == "#" and level.get(column, row - 1) != "#") {
            rect r = level.tileRect(column, row);
            graphics2d.drawRect(r.x, r.y, r.width, 5, color.GREEN);    // grass on top
        }
    }
}
```

### Things to know

- A variable can't be called `map` (that's the type `map<K, V>`), so call yours `level`, `tiles` or `world`.
- examples/platformer (the file `examples/platformer/level.frg` in the Forge folder) builds its whole level from a tile map. The walkthrough (see “Platformer Walkthrough” › “level.frg”) explains it.

<!-- docs/libraries/scenes.md -->

## `<scenes>`: Menus, Levels and Game Over

```cpp
#include <scenes>
```

Most games have several **scenes**: a title menu, the levels, a pause screen, a game-over screen. Each needs its own update and draw code. This library keeps them apart, and switches between them cleanly.

### Scene

Each scene is a class that inherits from `Scene` and fills in any of these methods (they all do nothing to start with):

| Method | Runs |
|---|---|
| `void enter()` | once, when the scene starts |
| `void update(float dt)` | every frame, from `scenes.update(dt)` |
| `void draw()` | every frame, from `scenes.draw()` |
| `void exit()` | once, when switching to another scene |

```cpp
class Menu : Scene {
    void update(float dt) {
        if (input.keyPressed(Key.ENTER)) {
            scenes.change(Level(1));
        }
    }
    void draw() {
        graphics2d.drawText("Press Enter to play", 300, 200, 30, color.WHITE);
    }
}

class Level : Scene {
    int number;
    Level(int number) {
        this.number = number;
    }
    void enter() {
        print("starting level {number}");
    }
    void draw() {
        graphics2d.drawText("Level {number}", 20, 20, 30, color.WHITE);
    }
}
```

Scenes are ordinary classes, so they can have fields, constructors and helper methods, and they keep their state for as long as they're running.

### Functions

| Function | What it does |
|---|---|
| `scenes.change(Scene next)` | switches to `next` at the start of the next `scenes.update`. The very first scene starts straight away. |
| `scenes.update(float dt)` | does any pending switch (the old scene's `exit()`, then the new one's `enter()`), then runs the current scene's `update(dt)` |
| `scenes.draw()` | runs the current scene's `draw()` |
| `scenes.current()` | the running scene (`Scene?`, null before the first one) |

### Wiring it up

This uses the `Menu` and `Level` classes from above.

```cpp
#include <window>
#include <graphics2d>
#include <input>
#include <scenes>
#include <stdio>

int main(){
    window.open(960, 540, "Scenes");
    scenes.change(Menu());
    window.run();
    return 0;
}

void update(float dt){
    scenes.update(dt);
}

void draw(){
    scenes.draw();
}
```

Because the switch waits for the next `scenes.update`, it's safe to call `scenes.change` from inside a scene's own `update` or `draw`: the current frame finishes normally first.

### Checking which scene is running

```cpp
if (scenes.current() is Level) {
    Level lvl = scenes.current() as Level;
    print(lvl.number);
}
```

See examples/menu (the file `examples/menu/main.frg` in the Forge folder) for a menu with buttons, using `<ui>` (see “`<ui>`: Buttons, Checkboxes, Sliders and Text Boxes”).

<!-- docs/libraries/ui.md -->

## `<ui>`: Buttons, Checkboxes, Sliders and Text Boxes

```cpp
#include <ui>
```

Simple interface controls for menus and settings screens. They're **immediate mode**: you call them every frame in your draw code, and each one both draws itself and tells you what the player did with it.

```cpp
if (ui.button(rect(380, 200, 200, 50), "Play")) {
    scenes.change(Level());        // runs on the frame the button is clicked
}
```

There are no objects to create or remember. Whatever the control controls lives in your own variables.

### Functions

| Function | Gives back | What it does |
|---|---|---|
| `ui.button(rect area, string label)` | `bool` | draws a button; **true on the frame it's clicked** (the mouse button released over it) |
| `ui.checkbox(rect area, string label, bool checked)` | `bool` | draws a box with a label to its right; gives back the **new** checked value |
| `ui.slider(rect area, float value, float min, float max)` | `float` | draws a slider; gives back the **new** value (drag it with the mouse) |
| `ui.textBox(rect area, string text, int maxLength = 0)` | `string` | draws a box to type in; gives back the **new** text. `maxLength` is the most letters it takes (0 means no limit) |
| `ui.isTyping()` | `bool` | true while the player is typing in a text box |
| `ui.label(string text, float x, float y, int size = 20, color c = color.WHITE)` | — | draws text |
| `ui.panel(rect area)` | — | draws a rounded box to put other controls on |
| `ui.setTheme(color background, color accent, color text)` | — | the colors of every control |

#### Using checkbox and slider

They give back the new value, so store it back into your variable every frame:

```cpp
bool showFps = true;
float volume = 0.7;

void draw(){
    showFps = ui.checkbox(rect(360, 265, 30, 30), "Show FPS", showFps);
    volume = ui.slider(rect(360, 340, 240, 24), volume, 0, 1);
    ui.label("Volume: {int(volume * 100)}%", 360, 310);
}
```

For a checkbox, the `rect` is the clickable area; the box is a square as tall as the rect, with the label to its right.

#### Typing text: textBox

A text box works the same way: give it the text, and store what it gives back.

```cpp
string playerName = "Player";

void draw(){
    ui.label("Your name:", 360, 170);
    playerName = ui.textBox(rect(360, 195, 240, 40), playerName, 12);    // at most 12 letters
}
```

- **Click** the box to start typing in it. Its border lights up and a cursor blinks at the end.
- Type to add letters. **Backspace** deletes the last one (hold it to keep deleting).
- **Enter**, or a click anywhere else, finishes typing.
- With a `maxLength`, extra letters are simply ignored.
- Only one box takes the typing at a time. Each box is known by its `rect`, so two boxes need different rectangles.
- Text that's too long for the box shows its end, where the typing happens.

While the player types, the keys still reach the rest of the game. If the game uses keys for other things (like R to restart), check `ui.isTyping()` first:

```cpp
if (input.keyPressed(Key.R) and !ui.isTyping()) {
    restart();
}
```

### Look

- Buttons light up when the mouse is over them, and get darker while pressed.
- Text size follows the control's height.
- The default theme is a dark blue background, a light blue accent and white text. Change it with `ui.setTheme(...)`, for example `ui.setTheme(color(40, 30, 30), color.ORANGE, color.WHITE)`.

### Notes

- Controls need the window to be open, and must be drawn every frame to stay visible and clickable.
- Draw controls after the game world, so they appear on top.

See examples/menu (the file `examples/menu/main.frg` in the Forge folder) for a full menu screen.

<!-- docs/libraries/ecs.md -->

## `<ECS>`: Entities and Components

```cpp
#include <ECS>
```

An alternative way to organize a game. Instead of one big class per kind of thing (a `Player` class, a `Goblin` class...), every game object is an **Entity**: a named container. You give it **Components**: small, reusable pieces of behavior like `Health`, `Mover` or `Sprite`. Mixing components makes different things: a goblin might be `Health` + `Mover` + `ChasePlayer`, and a crate just `Health`.

(ECS stands for Entity Component System. Forge's version is the friendly "game objects with components" style, as in Unity, rather than a data-oriented engine.)

### Components

A component is a class that inherits from `Component`, filling in any of these methods:

| Method | Runs |
|---|---|
| `void start()` | once, just before the component's first `update` |
| `void update(float dt)` | every time `ECS.update(dt)` is called |
| `void draw()` | every time `ECS.draw()` is called |

| Method you can call | Gives back | What it does |
|---|---|---|
| `entity()` | `Entity?` | the entity this component belongs to (null if it hasn't been added to one) |

```cpp
class Health : Component {
    int hp = 3;

    void update(float dt) {
        if (hp <= 0) {
            entity().destroy();
        }
    }
}

class Mover : Component {
    vec2 position = vec2.ZERO;
    vec2 velocity = vec2.ZERO;

    void update(float dt) {
        position += velocity * dt;
    }

    void draw() {
        graphics2d.drawCircle(position, 10, color.GOLD);
    }
}
```

### Entity

| Making one | |
|---|---|
| `ECS.create(string name)` | makes an entity **and adds it to the world** (usually what you want) |
| `Entity(string name)` | makes an entity that isn't in the world yet; add it later with `ECS.add(e)` |

| Field | Type | Meaning |
|---|---|---|
| `name` | `string` | its name, used by `ECS.find` |
| `active` | `bool` | when false, its components don't update or draw (true to start with) |
| `components` | `list<Component>` | its components, in the order they were added |

| Method | Gives back | What it does |
|---|---|---|
| `add(Component c)` | — | attaches a component |
| `get(string type)` | `Component?` | the first component of that class (or of a class that inherits from it), or null |
| `has(string type)` | `bool` | true if it has one |
| `remove(string type)` | `bool` | removes the first component of that class |
| `destroy()` | — | removes the entity from the world at the end of the current `ECS.update` |

Getting a component as its real class uses `as` (see “9. Structs and Classes” › “is and as: what kind of object is it?”):

```cpp
Health h = slime.get("Health") as Health;
h.hp -= 1;
```

The type name is written as text because Forge has no way to pass a type itself. A typo just means nothing is found (`get` gives `null`).

### The world

| Function | What it does |
|---|---|
| `ECS.create(string name)` | makes and adds an entity |
| `ECS.add(Entity e)` | adds an entity made with `Entity(...)` |
| `ECS.update(float dt)` | runs `start()` (the first time) and `update(dt)` on every component of every active entity, then removes destroyed entities |
| `ECS.draw()` | runs `draw()` on every component of every active entity |
| `ECS.find(string name)` | the first entity with that name (`Entity?`) |
| `ECS.all()` | a list of every entity |
| `ECS.withComponent(string type)` | a list of every entity that has that component |
| `ECS.clear()` | removes every entity |
| `ECS.count()` | how many entities there are |

It's safe for a component to create or destroy entities (including its own) during `update`.

### Putting it together

This uses the `Mover` and `Health` components from above, written in the same file.

```cpp
#include <window>
#include <graphics2d>
#include <ECS>

int main(){
    window.open(800, 450, "ECS");

    Entity ball = ECS.create("ball");
    Mover m = Mover();
    m.position = vec2(100, 200);
    m.velocity = vec2(120, 0);
    ball.add(m);
    ball.add(Health());

    window.run();
    return 0;
}

void update(float dt){
    ECS.update(dt);
}

void draw(){
    ECS.draw();
}
```

### Components talking to each other

A component can find its siblings through its entity:

```cpp
class Poison : Component {
    void update(float dt) {
        Health? h = entity().get("Health") as Health;
        if (h != null) {
            h.hp -= 1;
        }
    }
}
```

<!-- docs/libraries/tasks.md -->

## `<tasks>`: Tasks Without a Window

```cpp
#include <tasks>
```

Tasks (see “11. Tasks”) (functions returning `task` that call `wait`) are part of the core language. With a window open, they move forward by themselves every frame. **This library is only needed in programs without a window**, where something has to move task time forward.

| Function | What it does |
|---|---|
| `tasks.update(float dt)` | moves task time forward by `dt` seconds, and continues every task whose wait is over |
| `tasks.runAll()` | keeps going, really waiting in real time, until there are no tasks left |
| `tasks.count()` | how many tasks are waiting |
| `tasks.stopAll()` | stops every waiting task |

### Running tasks in a console program

```cpp
#include <stdio>
#include <tasks>

task countdown() {
    for (int i = 3; i > 0; i--) {
        print(i);
        wait(1);
    }
    print("Liftoff!");
}

int main(){
    countdown();
    tasks.runAll();       // takes about 3 seconds
    return 0;
}
```

### Stepping by hand

`tasks.update(dt)` doesn't wait in real time; it just moves task time forward. That makes it useful for simulations and tests:

```cpp
countdown();
tasks.update(1);      // prints 2
tasks.update(1);      // prints 1
tasks.update(1);      // prints Liftoff!
```

Notes:

- `tasks.runAll()` returns straight away if there are no tasks, and stops if the only tasks left are waiting on each other.
- A task that loops forever (`while (true) { wait(1); }`) makes `runAll` run forever. Stop it first, or use `update`.
- In a windowed game, don't call `tasks.update` yourself; the window already does, and calling it too would move time forward twice.

<!-- docs/libraries/toolkits.md -->

## Low-Level Toolkits: OpenGL, DirectX and Vulkan

Forge has three extra libraries for people who want to write their **own** rendering code with a real graphics API:

| Toolkit | Graphics API | Shaders are written in | Page |
|---|---|---|---|
| `<OpenGL>` | OpenGL 3.3 (core profile) | GLSL, as text in the program | OpenGL (see “`<OpenGL>`: Drawing with OpenGL Yourself”) |
| `<DirectX>` | Direct3D 11 | HLSL, as text in the program | DirectX (see “`<DirectX>`: Drawing with Direct3D 11 Yourself”) |
| `<Vulkan>` | Vulkan 1.0 | GLSL files, compiled to SPIR-V by `frg build` | Vulkan (see “`<Vulkan>`: Drawing with Vulkan Yourself”) |

> **New to Forge or to making games? You don't need any of these.** Forge's built-in engine (`<window>`, `<graphics2d>`, `<graphics3d>`...) already draws 2D and 3D for you. The toolkits are for learning how graphics cards work, or for drawing things the engine can't.

### Toolkits are not part of the engine

The most important thing to know: **a toolkit is a separate way of making a game, not a setting of the engine.**

- The engine is **not** built on top of these toolkits, and you can't switch the engine over to use one of them. The engine always draws with its own code.
- A game that includes a toolkit **can't use the engine at all**: no `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`. The toolkit opens its own window, and your code does all the drawing: shaders, buffers, matrices, everything.
- A game can use **only one** toolkit.

Both rules are checked when the game is built:

```cpp
#include <window>
#include <OpenGL>     // error: <OpenGL> is a separate toolkit, so it can't be used together with Forge's engine (<window>)
```

```cpp
#include <OpenGL>
#include <Vulkan>     // error: a game can only use one of <OpenGL>, <DirectX> and <Vulkan>
```

The rule is per **game**, not per file: if any file of the game includes `<window>`, no file can include a toolkit.

#### What still works

Libraries that don't draw work the same with a toolkit as with the engine:

| Works with a toolkit | Doesn't (it's part of the engine) |
|---|---|
| `<stdio>`, `<math>`, `<random>`, `<time>`, `<file>` | `<window>`: use the toolkit's own `openWindow` and `shouldClose` |
| `<audio>`: sounds, and music that keeps playing by itself | `<input>`: the toolkit has its own `keyDown`, `mousePosition`... |
| `<physics>`, `<ECS>`, `<scenes>` | `<graphics2d>`, `<graphics3d>`: you draw with shaders instead |
| tasks (see “11. Tasks”) (`wait` moves forward every frame by itself) | `<assets>`: `<OpenGL>` and `<DirectX>` have their own `loadTexture` |
| the core types (`vec2`, `vec3`, `color`...) | `<ui>` |

### What every toolkit has

All three toolkits share the same window, keyboard and mouse functions. They're written here once, with `OpenGL.` in front; use `DirectX.` or `Vulkan.` instead with those toolkits.

#### The window

| Function | What it does |
|---|---|
| `openWindow(int width, int height, string title)` | opens the window and starts the graphics API. Call it once, before anything else. |
| `shouldClose()` | call once per frame, at the top of the game loop. It shows the frame you just drew, reads the keyboard and mouse, and gives back `true` when the player closes the window (or presses **Escape**). |
| `closeWindow()` | frees everything and closes the window. It also happens by itself when the program ends. |
| `width()`, `height()` | the window's size in pixels (`int`). The window can't be resized. |
| `deltaTime()` | seconds since the last frame (`float`), for moving things smoothly |
| `time()` | seconds since the window opened (`double`) |
| `setTitle(string title)` | changes the window's title |
| `setVSync(bool on)` | `true` (the start) waits for the screen before showing each frame, so the game runs at the screen's speed (often 60 frames a second) without tearing. `false` draws as fast as it can. |

The game loop always looks like this:

```cpp
OpenGL.openWindow(800, 600, "My game");
while (!OpenGL.shouldClose()) {
    // update the game, then draw the frame
}
OpenGL.closeWindow();
```

There's no `window.run()` with `update` and `draw` functions here: toolkit games always write their own loop.

#### Keyboard and mouse

These work the same as in `<input>` (see “`<input>`: Keyboard, Mouse and Controllers”) and use the same `Key` and `MouseButton` names (`Key.SPACE`, `Key.A`, `MouseButton.LEFT`...):

| Function | Gives back `true`... |
|---|---|
| `keyDown(Key key)` | while the key is held |
| `keyPressed(Key key)` | only on the frame the key went down |
| `keyReleased(Key key)` | only on the frame the key came up |
| `mouseDown(MouseButton button)`, `mousePressed(...)`, `mouseReleased(...)` | the same, for mouse buttons |

| Function | Gives back |
|---|---|
| `mousePosition()` | where the mouse is, in pixels from the window's top-left corner (`vec2`) |
| `mouseWheel()` | how far the wheel turned this frame (`float`, positive is away from you) |

#### Matrices

Graphics APIs move and turn things with 4×4 **matrices**. Every toolkit has helpers that make them as a `list<float>` of 16 numbers, column by column, which is the order shaders expect:

| Function | Makes a matrix that... |
|---|---|
| `identity()` | does nothing (the starting point) |
| `translate(vec3 offset)` | moves things by `offset` |
| `rotate(float degrees, vec3 axis)` | turns things around `axis` |
| `scale(vec3 amount)` | stretches things (`vec3(2, 2, 2)` is twice as big) |
| `multiply(list<float> a, list<float> b)` | does `b` first, then `a` |
| `lookAt(vec3 eye, vec3 target, vec3 up)` | a camera at `eye` looking at `target` (the "view" matrix) |
| `perspective(float fovDegrees, float aspect, float near, float far)` | 3D with things further away looking smaller (the "projection" matrix). `aspect` is width / height. |
| `orthographic(float left, float right, float bottom, float top, float near, float far)` | flat projection with no perspective, for 2D |

The helpers are the same in each toolkit, except that each one's `perspective` and `orthographic` fit that API's screen (DirectX and Vulkan put depth from 0 to 1, and Vulkan's screen y points down). So always use the helpers from the toolkit you're using. Then the same numbers give the same picture in all three.

Putting them together, the usual order is `projection × view × model`:

```cpp
list<float> model = OpenGL.multiply(OpenGL.translate(vec3(0, 1, 0)), OpenGL.rotate(angle, vec3.UP));
list<float> view = OpenGL.lookAt(vec3(0, 2, 6), vec3(0, 0, 0), vec3.UP);
list<float> projection = OpenGL.perspective(45, float(OpenGL.width()) / OpenGL.height(), 0.1, 100);
list<float> all = OpenGL.multiply(projection, OpenGL.multiply(view, model));
```

#### Handles

Graphics objects (shaders, buffers, textures, pipelines) are given back as an `int` **handle**: a number that stands for the object. Keep it in a variable and pass it to the functions that use the object. A handle of `0` means "nothing".

#### Files

Files the toolkit loads (images, Vulkan's `.spv` shaders) are found the same way as the engine's assets (see “`<assets>`: Images, Animations and Fonts” › “Where files go”): put them in the game's folder, next to `main.frg`.

#### When something goes wrong

Mistakes the toolkit can spot stop the game with a clear message, like the rest of Forge:

```
Runtime error in OpenGL.buildProgram(): the vertex shader has an error:
0(4) : error C1008: undefined variable "positon"
```

Some mistakes can't be spotted, though, because graphics APIs trust you to get the details right. The usual sign is a black or empty window. Check, in this order:

1. the vertex data and the sizes you gave for each corner;
2. that the matrices are multiplied in the right order;
3. that the shader's inputs match the buffer's layout.

### Which one should I pick?

- **OpenGL** is the easiest to start with and has by far the most tutorials online (search for "learnopengl"). Its functions are almost exactly the real OpenGL ones, so tutorials translate line by line.
- **DirectX** (Direct3D 11) is what most Windows games used for years. Its ideas are close to OpenGL's, with more set up at the start.
- **Vulkan** is the most modern and the most work: you describe a whole drawing setup (a *pipeline*) at the start, and record each frame's commands. Forge does the longest part, the setup, for you.

Examples of each, all drawing the same spinning cube: `examples/opengl` (the file `examples/opengl/main.frg` in the Forge folder), `examples/directx` (the file `examples/directx/main.frg` in the Forge folder), `examples/vulkan` (the file `examples/vulkan/main.frg` in the Forge folder).

<!-- docs/libraries/opengl.md -->

## `<OpenGL>`: Drawing with OpenGL Yourself

```cpp
#include <OpenGL>
```

A low-level toolkit for drawing with **OpenGL 3.3** (core profile) directly. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

The functions here are thin wrappers around the real OpenGL functions, with the `gl` taken off the front: `glClearColor(...)` is `OpenGL.clearColor(...)`, and `GL_TRIANGLES` is `OpenGL.TRIANGLES`. So OpenGL tutorials (like learnopengl.com) can be followed almost line by line. The differences are:

- object handles are `int`s;
- data is a `list<float>` (or a `list<int>`), with sizes counted in floats instead of bytes;
- shader code is a `string`, usually raw text (see “3. Text” › “Raw text”) between `"""` marks.

### A whole program

The smallest OpenGL game: one colored triangle.

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

    // Each corner: x y, then r g b.
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
    OpenGL.vertexAttribPointer(0, 2, 5, 0);    // position: 2 floats, every 5 floats, starting at 0
    OpenGL.enableVertexAttribArray(0);
    OpenGL.vertexAttribPointer(1, 3, 5, 2);    // color: 3 floats, starting at float 2
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

What happens, step by step:

1. `openWindow` opens the window and starts OpenGL.
2. `buildProgram` compiles the two shaders and links them into a **program**: the code the graphics card runs for every corner (the vertex shader) and every pixel (the fragment shader).
3. The corners go into a **buffer** in the graphics card's memory. A **vertex array** remembers how that buffer is laid out: which floats are the position and which are the color.
4. Every frame: clear the screen, choose the program and the vertex array, and draw. `shouldClose()` at the top of the loop then shows the finished frame.

The bigger example in `examples/opengl` (the file `examples/opengl/main.frg` in the Forge folder) draws a spinning 3D cube and a textured square.

### The window

| Function | What it does |
|---|---|
| `OpenGL.version()` | the OpenGL version and graphics card, as text (like `"3.3.0 Core Profile ..."`) |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see What every toolkit has (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “What every toolkit has”).

### Drawing settings

| Function | What it does | Real OpenGL |
|---|---|---|
| `clearColor(float r, float g, float b, float a)` | the color `clear` fills the screen with (0 to 1 each) | `glClearColor` |
| `clear(int mask)` | clears the screen. Pass `COLOR_BUFFER_BIT`, and in 3D also `DEPTH_BUFFER_BIT`: `OpenGL.COLOR_BUFFER_BIT bitor OpenGL.DEPTH_BUFFER_BIT` | `glClear` |
| `viewport(int x, int y, int width, int height)` | which part of the window to draw into (the whole window to start with) | `glViewport` |
| `enable(int what)`, `disable(int what)` | turns on/off `DEPTH_TEST` (near things hide far things), `BLEND` (see-through colors) or `CULL_FACE` (skip triangles facing away) | `glEnable`, `glDisable` |
| `blendFunc(int source, int destination)` | how see-through colors mix. The usual: `blendFunc(OpenGL.SRC_ALPHA, OpenGL.ONE_MINUS_SRC_ALPHA)` | `glBlendFunc` |
| `depthFunc(int function)` | when a pixel passes the depth test: `LESS` (the start), `LEQUAL`, `ALWAYS` | `glDepthFunc` |
| `cullFace(int face)` | which side `CULL_FACE` skips: `BACK` (the start), `FRONT`, `FRONT_AND_BACK` | `glCullFace` |
| `lineWidth(float width)` | how thick `LINES` are, in pixels (many cards only allow 1) | `glLineWidth` |
| `getError()` | the oldest error OpenGL noticed, or `NO_ERROR` (0). Useful when the screen stays black. | `glGetError` |

A triangle counts as facing you when its corners go **counter-clockwise** on screen.

### Shaders and programs

The easy way: `buildProgram` does all the steps at once, and stops the game with OpenGL's message if a shader has a mistake.

| Function | What it does |
|---|---|
| `buildProgram(string vertexSource, string fragmentSource)` | compiles both shaders and links them. Gives back the program's handle. |
| `useProgram(int program)` | draws with this program from now on |
| `deleteProgram(int program)` | frees it |

Shaders are written in GLSL and start with `#version 330 core`.

The step-by-step way, which works exactly like real OpenGL:

| Function | What it does |
|---|---|
| `createShader(int type)` | a new shader: `VERTEX_SHADER` or `FRAGMENT_SHADER` |
| `shaderSource(int shader, string source)` | gives it its code |
| `compileShader(int shader)` | compiles it |
| `shaderCompiled(int shader)` | `true` if it compiled |
| `shaderLog(int shader)` | the compiler's messages |
| `deleteShader(int shader)` | frees it (fine once it's linked into a program) |
| `createProgram()` | a new, empty program |
| `attachShader(int program, int shader)` | adds a compiled shader to the program |
| `linkProgram(int program)` | links the program's shaders together |
| `programLinked(int program)` | `true` if it linked |
| `programLog(int program)` | the linker's messages |

#### Uniforms

A **uniform** is a value the program sends to the shaders, which stays the same for a whole draw (a matrix, a color, a time). First look up where it is, once, then set it whenever it changes. Uniforms belong to the program in use, so call `useProgram` first.

| Function | What it does |
|---|---|
| `getUniformLocation(int program, string name)` | where the uniform called `name` is, or `-1` if the shaders don't use it |
| `getAttribLocation(int program, string name)` | the location number of a vertex shader input |
| `uniform1f(int location, float x)` ... `uniform4f(int location, float x, float y, float z, float w)` | sets a `float`, `vec2`, `vec3` or `vec4` uniform |
| `uniform1i(int location, int x)` | sets an `int` or `bool` uniform, or which texture unit a `sampler2D` reads |
| `uniformMatrix4(int location, list<float> matrix)` | sets a `mat4` uniform from 16 numbers, like the matrix helpers (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “Matrices”) make |

```cpp
int projectionLoc = OpenGL.getUniformLocation(program, "projection");
// ... every frame:
OpenGL.useProgram(program);
OpenGL.uniformMatrix4(projectionLoc, OpenGL.perspective(45, float(OpenGL.width()) / OpenGL.height(), 0.1, 100));
```

### Buffers and vertex arrays

| Function | What it does |
|---|---|
| `genVertexArray()` | a new vertex array: it remembers the buffers and layout set up while it's bound |
| `bindVertexArray(int vertexArray)` | uses this vertex array (for setting up, and for drawing) |
| `deleteVertexArray(int vertexArray)` | frees it |
| `genBuffer()` | a new buffer |
| `bindBuffer(int target, int buffer)` | uses this buffer as `ARRAY_BUFFER` (corners) or `ELEMENT_ARRAY_BUFFER` (corner numbers) |
| `deleteBuffer(int buffer)` | frees it |
| `bufferData(int target, list<float> data, int usage)` | copies floats into the bound buffer. `usage` is a hint: `STATIC_DRAW` (set once), `DYNAMIC_DRAW` (changed often), `STREAM_DRAW` (changed every frame) |
| `bufferDataInts(int target, list<int> data, int usage)` | the same with whole numbers: for `ELEMENT_ARRAY_BUFFER` |
| `vertexAttribPointer(int index, int size, int strideFloats, int offsetFloats)` | says where shader input number `index` is in the bound `ARRAY_BUFFER`: `size` floats, one every `strideFloats` floats, starting `offsetFloats` in |
| `enableVertexAttribArray(int index)` | turns that input on |

`vertexAttribPointer` counts in **floats**, where real OpenGL counts in bytes. For corners laid out as `x y z r g b` (6 floats each), position is `(0, 3, 6, 0)` and color is `(1, 3, 6, 3)`.

### Drawing

| Function | What it does |
|---|---|
| `drawArrays(int mode, int first, int count)` | draws `count` corners from the bound vertex array, starting at corner `first` |
| `drawElements(int mode, int count)` | draws using the corner numbers in the vertex array's `ELEMENT_ARRAY_BUFFER`, so corners shared by several triangles are only stored once |

`mode` is what the corners make: `TRIANGLES` (every 3 corners), `TRIANGLE_STRIP`, `TRIANGLE_FAN`, `LINES` (every 2), `LINE_STRIP`, `LINE_LOOP` or `POINTS`.

### Textures

| Function | What it does |
|---|---|
| `loadTexture(string path)` | loads a `.png` (or `.jpg`, `.bmp`...) from the game's folder into a new texture and gives back its handle |
| `genTexture()` | a new, empty texture |
| `bindTexture(int target, int texture)` | uses this texture: the target is `TEXTURE_2D` |
| `activeTexture(int unit)` | which texture unit `bindTexture` sets: `TEXTURE0` (the start) to `TEXTURE3` |
| `texParameteri(int target, int name, int value)` | a texture setting, see below |
| `generateMipmap(int target)` | makes the smaller copies used when the texture is drawn small |
| `deleteTexture(int texture)` | frees it |

`loadTexture` sets the texture up so that:

- texture coordinate `(0, 0)` is the image's **bottom-left** corner, as usual in OpenGL;
- it's smooth when stretched;
- it repeats past the edges.

For sharp pixel art, change the stretching setting after loading:

```cpp
int sheet = OpenGL.loadTexture("player.png");
OpenGL.bindTexture(OpenGL.TEXTURE_2D, sheet);
OpenGL.texParameteri(OpenGL.TEXTURE_2D, OpenGL.TEXTURE_MAG_FILTER, OpenGL.NEAREST);
```

| Setting (`name`) | Values |
|---|---|
| `TEXTURE_MAG_FILTER` (drawn bigger than the image) | `NEAREST` (sharp pixels), `LINEAR` (smooth) |
| `TEXTURE_MIN_FILTER` (drawn smaller) | `NEAREST`, `LINEAR`, `NEAREST_MIPMAP_NEAREST`, `LINEAR_MIPMAP_LINEAR` |
| `TEXTURE_WRAP_S`, `TEXTURE_WRAP_T` (past the edges, across and up) | `REPEAT`, `CLAMP_TO_EDGE` |

In the fragment shader, read the texture with a `uniform sampler2D` and `texture(...)`. A `sampler2D` reads texture unit 0 unless you set it with `uniform1i`.

### Constants

| Group | Names |
|---|---|
| `clear` | `COLOR_BUFFER_BIT`, `DEPTH_BUFFER_BIT`, `STENCIL_BUFFER_BIT` |
| draw modes | `POINTS`, `LINES`, `LINE_LOOP`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP`, `TRIANGLE_FAN` |
| buffers | `ARRAY_BUFFER`, `ELEMENT_ARRAY_BUFFER`, `STATIC_DRAW`, `DYNAMIC_DRAW`, `STREAM_DRAW` |
| shaders | `VERTEX_SHADER`, `FRAGMENT_SHADER` |
| `enable`/`disable` | `DEPTH_TEST`, `BLEND`, `CULL_FACE` |
| blending | `ZERO`, `ONE`, `SRC_ALPHA`, `ONE_MINUS_SRC_ALPHA` |
| depth | `LESS`, `LEQUAL`, `ALWAYS` |
| culling | `FRONT`, `BACK`, `FRONT_AND_BACK` |
| textures | `TEXTURE_2D`, `TEXTURE0`-`TEXTURE3`, `TEXTURE_MIN_FILTER`, `TEXTURE_MAG_FILTER`, `TEXTURE_WRAP_S`, `TEXTURE_WRAP_T`, `NEAREST`, `LINEAR`, `NEAREST_MIPMAP_NEAREST`, `LINEAR_MIPMAP_LINEAR`, `REPEAT`, `CLAMP_TO_EDGE` |
| errors | `NO_ERROR`, `INVALID_ENUM`, `INVALID_VALUE`, `INVALID_OPERATION`, `OUT_OF_MEMORY` |

They have the same numbers as in real OpenGL, so a value from a tutorial can be used as a plain number too.

### Not included

This toolkit covers the parts of OpenGL that most games and tutorials use. It doesn't have:

- framebuffers (drawing into a texture);
- uniform buffers, instancing, or geometry shaders;
- 3D textures and cube maps.

These may come later.

<!-- docs/libraries/directx.md -->

## `<DirectX>`: Drawing with Direct3D 11 Yourself

```cpp
#include <DirectX>
```

A low-level toolkit for drawing with **Direct3D 11**, the graphics API built into Windows. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

The functions are close to Direct3D 11's own, with simpler arguments:

- objects are `int` handles;
- data is a `list<float>` or a `list<int>`, with sizes counted in floats;
- shaders are HLSL code in a `string`, usually raw text (see “3. Text” › “Raw text”) between `"""` marks.

If the computer has no suitable graphics card, Direct3D's software renderer (WARP) is used instead. It's slower, but everything still works.

### A whole program

One colored triangle:

```cpp
#include <DirectX>

string SHADER = """
struct VertexIn {
    float2 position : POSITION;
    float3 color : COLOR;
};

struct PixelIn {
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PixelIn vsMain(VertexIn v) {
    PixelIn o;
    o.position = float4(v.position, 0.0, 1.0);
    o.color = v.color;
    return o;
}

float4 psMain(PixelIn p) : SV_TARGET {
    return float4(p.color, 1.0);
}
""";

int main(){
    DirectX.openWindow(800, 600, "Triangle");
    int vs = DirectX.compileVertexShader(SHADER, "vsMain");
    int ps = DirectX.compilePixelShader(SHADER, "psMain");
    int layout = DirectX.createInputLayout(vs, ["POSITION", "COLOR"], [2, 3]);

    // Each corner: x y, then r g b.
    int corners = DirectX.createVertexBuffer([
        -0.5, -0.5,   1, 0, 0,
         0.5, -0.5,   0, 1, 0,
         0.0,  0.5,   0, 0, 1
    ]);

    DirectX.setShaders(vs, ps);
    DirectX.setInputLayout(layout);
    DirectX.setVertexBuffer(corners, 5);
    while (!DirectX.shouldClose()) {
        DirectX.clear(0.1, 0.1, 0.15, 1);
        DirectX.draw(3);
    }
    DirectX.closeWindow();
    return 0;
}
```

What happens, step by step:

1. `openWindow` opens the window and starts Direct3D.
2. The two shaders are compiled from the same HLSL text, each from its own function. The **vertex shader** runs for every corner, the **pixel shader** for every pixel.
3. The **input layout** tells Direct3D what each corner in the buffer holds: a 2-float `POSITION`, then a 3-float `COLOR`. The names match the `: POSITION` and `: COLOR` marks in the shader's `VertexIn`.
4. The corners go into a **vertex buffer** in the graphics card's memory.
5. Every frame: clear the screen and draw. `shouldClose()` at the top of the loop then shows the finished frame.

The settings made with `set...` stay until they're changed, so things that never change can be set once, before the loop.

The bigger example in `examples/directx` (the file `examples/directx/main.frg` in the Forge folder) draws a spinning 3D cube and a textured square.

### The window

| Function | What it does |
|---|---|
| `DirectX.adapterName()` | the graphics card's name, like `"Radeon RX 570 Series"` |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see What every toolkit has (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “What every toolkit has”).

### Drawing settings

| Function | What it does | To start with |
|---|---|---|
| `clear(float r, float g, float b, float a)` | fills the screen with a color (0 to 1 each) and resets the depth buffer. Call it at the start of each frame. | |
| `setDepthTest(bool on)` | whether near things hide far things | on |
| `setBlending(bool on)` | whether see-through colors (alpha below 1) mix with what's behind them | off |
| `setCulling(int mode)` | skip triangles facing away (`CULL_BACK`) or toward you (`CULL_FRONT`), or neither (`CULL_NONE`) | `CULL_NONE` |
| `setTopology(int topology)` | what the corners make: `TRIANGLES` (every 3), `TRIANGLE_STRIP`, `LINES` (every 2), `LINE_STRIP`, `POINTS` | `TRIANGLES` |

A triangle counts as facing you when its corners go **counter-clockwise** on screen, the same as in OpenGL.

### Shaders

| Function | What it does |
|---|---|
| `compileVertexShader(string hlsl, string entry = "main")` | compiles the function called `entry` as a vertex shader (shader model 5.0). Gives back its handle. |
| `compilePixelShader(string hlsl, string entry = "main")` | the same, as a pixel shader |
| `setShaders(int vertexShader, int pixelShader)` | draws with these two from now on |

If the HLSL has a mistake, the game stops with Direct3D's message, which says the line and what's wrong.

#### Input layouts

| Function | What it does |
|---|---|
| `createInputLayout(int vertexShader, list<string> names, list<int> sizes)` | describes one corner: for each of the vertex shader's inputs, in order, its **semantic name** and how many floats it takes (1-4). Gives back its handle. |
| `setInputLayout(int layout)` | uses this layout from now on |

The names are the marks after the inputs in the shader: `POSITION`, `COLOR`, `TEXCOORD`, `NORMAL`... If a name is used twice, the second is number 1 (`TEXCOORD1` in the shader), the third number 2, and so on. If the names or sizes don't match the vertex shader, `createInputLayout` stops the game with an error.

### Buffers

| Function | What it does |
|---|---|
| `createVertexBuffer(list<float> data)` | a buffer of corners. It can't be changed later. |
| `createIndexBuffer(list<int> indices)` | a buffer of corner numbers, for `drawIndexed`. It can't be changed later. |
| `createConstantBuffer(int floatCount)` | a buffer for values sent to the shaders (matrices, colors, time), which can be changed every frame |
| `updateConstantBuffer(int buffer, list<float> data)` | replaces a constant buffer's values |
| `setVertexBuffer(int buffer, int strideFloats)` | draws corners from this buffer, each `strideFloats` floats long |
| `setIndexBuffer(int buffer)` | uses these corner numbers for `drawIndexed` |
| `setConstantBuffer(int slot, int buffer)` | gives this buffer to both shaders as `register(b<slot>)` |

#### Constant buffers

A constant buffer matches a `cbuffer` in the shader. The floats go in the same order as the `cbuffer`'s variables:

```cpp
string SHADER = """
cbuffer Transform : register(b0) {
    float4x4 world;     // floats 0-15
    float4 tint;        // floats 16-19
};
...
""";

int transform = DirectX.createConstantBuffer(20);
DirectX.setConstantBuffer(0, transform);
// ... every frame:
list<float> data = DirectX.rotate(angle, vec3.UP);
data.addAll([1, 0.5, 0.5, 1]);
DirectX.updateConstantBuffer(transform, data);
```

Things to know:

- **Use `mul(matrix, vector)` in the shader**, like `mul(world, float4(position, 1.0))`. The matrix helpers (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “Matrices”) make matrices in the order that this expects.
- **Mind HLSL's packing.** HLSL packs a `cbuffer` into blocks of 4 floats, and one variable can't be split across two blocks. A `float3` followed by a `float2` leaves a gap. The simplest fix is to only use `float4` and `float4x4`, or to put single `float`s at the end.
- **Sizes are rounded up.** A constant buffer's size is rounded up to a multiple of 4 floats. Sending fewer floats than it holds is fine: the rest are 0.

### Drawing

| Function | What it does |
|---|---|
| `draw(int vertexCount, int startVertex = 0)` | draws `vertexCount` corners from the vertex buffer |
| `drawIndexed(int indexCount, int startIndex = 0)` | draws using the index buffer's corner numbers, so corners shared by several triangles are only stored once |

### Textures

| Function | What it does |
|---|---|
| `loadTexture(string path)` | loads a `.png` (or `.jpg`, `.bmp`...) from the game's folder and gives back its handle |
| `setTexture(int slot, int texture)` | gives the texture to the pixel shader as `register(t<slot>)` |
| `setTextureFilter(bool smooth)` | `true` (the start) blends pixels when the texture is stretched; `false` keeps them sharp, for pixel art |

Texture coordinate `(0, 0)` is the image's **top-left** corner. The texture repeats past its edges. In the shader, read it like this:

```
Texture2D tex : register(t0);
SamplerState samp : register(s0);
// ...
float4 c = tex.Sample(samp, uv);
```

### Matrices

The helpers are listed in Matrices (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “Matrices”). DirectX's `perspective` and `orthographic` put depth from 0 to 1, as Direct3D expects.

### Constants

| Group | Names |
|---|---|
| topology | `POINTS`, `LINES`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP` |
| culling | `CULL_NONE`, `CULL_FRONT`, `CULL_BACK` |

### Not included

This toolkit covers what most games and tutorials need. It doesn't have:

- render targets (drawing into a texture);
- geometry, hull or compute shaders;
- instancing;
- vertex buffers that change after they're made.

These may come later.

<!-- docs/libraries/vulkan.md -->

## `<Vulkan>`: Drawing with Vulkan Yourself

```cpp
#include <Vulkan>
```

A low-level toolkit for drawing with **Vulkan**, the most modern of the three graphics APIs. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

### What Forge does, and what you do

Using Vulkan by hand takes hundreds of lines before the first triangle appears, and almost all of them are the same in every program. So `Vulkan.openWindow` does that part for you. It sets up:

- the Vulkan instance and the window's surface;
- the graphics card (a separate card is picked over one built into the processor);
- the **swapchain**: the window's images, which are drawn into in turn;
- a depth buffer;
- the render pass, framebuffers and command buffers;
- the syncing that lets the processor prepare one frame while the graphics card draws the last.

What's left is the part that's different in every game, and that's yours:

- **shaders**, written in GLSL;
- **pipelines**: a pair of shaders, plus how corners are laid out and drawn;
- **buffers** of corners and corner numbers;
- each frame's drawing commands, recorded between `beginFrame` and `endFrame`.

Games built with `<Vulkan>` need a Vulkan driver to run. Graphics drivers from the last several years include one. Building a game needs nothing extra: Forge comes with everything it needs.

### Shaders are files

Unlike `<OpenGL>` and `<DirectX>`, Vulkan doesn't take shader code as text. It takes **SPIR-V**, a compiled form. So Vulkan shaders go in their own files in the game's folder, named by what they are:

| File ending | Shader |
|---|---|
| `.vert` | vertex shader (runs for every corner) |
| `.frag` | fragment shader (runs for every pixel) |
| `.comp`, `.geom`, `.tesc`, `.tese` | compute, geometry and tessellation shaders (compiled, but this toolkit only draws with vertex and fragment shaders) |

When a game uses `<Vulkan>`, **`frg build` compiles every shader file into a `.spv` file next to it**: `shape.vert` becomes `shape.vert.spv`. The game then loads the `.spv`:

```
   Compiling mygame (1 file)
      Shader shape.frag
      Shader shape.vert
    Finished build/mygame.exe in 3.10s
```

Only shaders that changed since the last build are compiled again. If a shader has a mistake, the build stops and shows where:

```
error: the shader shape.frag has errors

    shape.frag:4: 'shade' : undeclared identifier
Build failed: a shader has errors.
```

Shaders are GLSL starting with `#version 450`. A few things are different from OpenGL's GLSL:

- every input and output needs a `layout(location = N)`;
- values from the program come in a `push_constant` block (see Push constants (see “Push constants”));
- there's no `gl_FragColor`: the fragment shader writes to its own `out vec4`.

The `.spv` files are made from the shader files, so they don't need to be kept or shared: the next build makes them again.

### A whole program

One colored triangle. Three files go in the game's folder.

`triangle.vert`:

```glsl
#version 450
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 vColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
```

`triangle.frag`:

```glsl
#version 450
layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vColor, 1.0);
}
```

`main.frg`:

```cpp
#include <Vulkan>

int main(){
    Vulkan.openWindow(800, 600, "Triangle");

    int vertexShader = Vulkan.loadShader("triangle.vert.spv");
    int fragmentShader = Vulkan.loadShader("triangle.frag.spv");
    // Each corner: 2 floats of position (location 0), then 3 of color (location 1).
    int pipeline = Vulkan.createPipeline(vertexShader, fragmentShader, [2, 3]);

    int corners = Vulkan.createVertexBuffer([
        -0.5,  0.5,   1, 0, 0,
         0.5,  0.5,   0, 1, 0,
         0.0, -0.5,   0, 0, 1
    ]);

    while (!Vulkan.shouldClose()) {
        if (Vulkan.beginFrame(0.1, 0.1, 0.15, 1)) {
            Vulkan.bindPipeline(pipeline);
            Vulkan.bindVertexBuffer(corners);
            Vulkan.draw(3);
            Vulkan.endFrame();
        }
    }
    Vulkan.closeWindow();
    return 0;
}
```

Notice the corners: without a matrix, Vulkan's screen has **y pointing down**, so `-0.5` is near the top. Once you use the matrix helpers (see “Matrices”), y points up again, like in OpenGL.

The bigger example in `examples/vulkan` (the file `examples/vulkan/main.frg` in the Forge folder) draws a spinning 3D cube using a matrix.

### The window

| Function | What it does |
|---|---|
| `Vulkan.deviceName()` | the graphics card's name, like `"Radeon RX 570 Series"` |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see What every toolkit has (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “What every toolkit has”). `setVSync(false)` takes effect from the next frame.

### Pipelines

In Vulkan, nearly every drawing setting is fixed when a **pipeline** is made, instead of being changed while drawing. Different settings (say, lines instead of triangles, or no depth test for a 2D overlay) mean different pipelines.

| Function | What it does |
|---|---|
| `loadShader(string spvPath)` | loads a compiled shader (`.spv`) from the game's folder and gives back its handle |
| `createPipeline(int vertexShader, int fragmentShader, list<int> attributeSizes, int pushConstantFloats = 0)` | makes a pipeline and gives back its handle |

`attributeSizes` describes one corner in the vertex buffer: how many floats each of the vertex shader's inputs takes (1-4), in order of their `location`. `[3, 3]` means `location = 0` is 3 floats and `location = 1` is the next 3. A pipeline whose shader makes its own corners (with no inputs) uses `[]`.

`pushConstantFloats` is how many floats `pushConstants` (see “Push constants”) can send to this pipeline's shaders.

#### Settings for the next pipelines

These change the pipelines made **after** them. They don't change pipelines that already exist:

| Function | What it does | To start with |
|---|---|---|
| `setTopology(int topology)` | what the corners make: `TRIANGLES` (every 3), `TRIANGLE_STRIP`, `TRIANGLE_FAN`, `LINES` (every 2), `LINE_STRIP`, `POINTS` | `TRIANGLES` |
| `setDepthTest(bool on)` | whether near things hide far things | on |
| `setCulling(int mode)` | skip triangles facing away (`CULL_BACK`) or toward you (`CULL_FRONT`), or neither (`CULL_NONE`) | `CULL_NONE` |
| `setBlending(bool on)` | whether see-through colors (alpha below 1) mix with what's behind them | off |

A triangle counts as facing you when its corners go **counter-clockwise** on screen, the same as in OpenGL and DirectX.

```cpp
Vulkan.setCulling(Vulkan.CULL_BACK);
int solid = Vulkan.createPipeline(vs, fs, [3, 3], 16);

Vulkan.setTopology(Vulkan.LINES);
Vulkan.setCulling(Vulkan.CULL_NONE);
int wireframe = Vulkan.createPipeline(vs, fs, [3, 3], 16);
```

### Buffers

| Function | What it does |
|---|---|
| `createVertexBuffer(list<float> data)` | a buffer of corners, laid out as the pipeline's `attributeSizes` says |
| `createIndexBuffer(list<int> indices)` | a buffer of corner numbers, for `drawIndexed` |

Buffers can't be changed after they're made. Send values that change every frame (matrices, colors, time) with push constants instead.

### Drawing a frame

Each frame is recorded between `beginFrame` and `endFrame`:

| Function | What it does |
|---|---|
| `beginFrame(float r, float g, float b, float a)` | starts the frame, cleared to this color (0 to 1 each), and resets the depth buffer. Gives back `false` when there's nothing to draw into this time (say, the window is minimized): then skip the drawing, and don't call `endFrame`. |
| `bindPipeline(int pipeline)` | draws with this pipeline from now on |
| `bindVertexBuffer(int buffer)` | draws corners from this buffer |
| `bindIndexBuffer(int buffer)` | uses these corner numbers for `drawIndexed` |
| `pushConstants(int pipeline, list<float> data)` | sends floats to the shaders, for the draws after it |
| `draw(int vertexCount, int firstVertex = 0)` | draws `vertexCount` corners from the vertex buffer |
| `drawIndexed(int indexCount, int firstIndex = 0)` | draws using the index buffer's corner numbers |
| `endFrame()` | finishes the frame and sends it to the graphics card. It appears in the window at the next `shouldClose()`. |

So the game loop is always shaped like this:

```cpp
while (!Vulkan.shouldClose()) {
    // update the game...
    if (Vulkan.beginFrame(0, 0, 0, 1)) {
        // bind, push and draw...
        Vulkan.endFrame();
    }
}
```

The drawing functions only work between `beginFrame` and `endFrame`: anywhere else they stop the game with an error saying so. Things bound during a frame stay bound until the end of that frame, and each frame starts with nothing bound.

#### Push constants

**Push constants** are the quick way to send a few values to the shaders for each draw: a matrix, a color, a time. In the shader they're a `push_constant` block:

```glsl
layout(push_constant) uniform Push {
    mat4 transform;     // floats 0-15
    vec4 tint;          // floats 16-19
} push;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);
}
```

In the game, make the pipeline with room for that many floats, then send them before each draw. Both shaders can read them:

```cpp
int pipeline = Vulkan.createPipeline(vs, fs, [3, 3], 20);
// ... in the frame:
list<float> data = Vulkan.multiply(camera, model);
data.addAll([1, 1, 1, 1]);
Vulkan.pushConstants(pipeline, data);
Vulkan.drawIndexed(36);
```

Every graphics card allows at least **32 floats** (two matrices) of push constants. Asking for more than the card allows stops `createPipeline` with an error that says the card's limit.

### Matrices

The helpers are listed in Matrices (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan” › “Matrices”). Vulkan's `perspective` and `orthographic` put depth from 0 to 1. They also flip y, so that up is up on screen, like in OpenGL. The usual way is to multiply `projection × view × model` in the game and push the result as one `mat4`:

```cpp
list<float> camera = Vulkan.multiply(Vulkan.perspective(45, float(Vulkan.width()) / Vulkan.height(), 0.1, 100),
                                     Vulkan.lookAt(vec3(0, 2, 6), vec3(0, 0, 0), vec3.UP));
Vulkan.pushConstants(pipeline, Vulkan.multiply(camera, Vulkan.rotate(angle, vec3.UP)));
```

### Constants

| Group | Names |
|---|---|
| topology | `POINTS`, `LINES`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP`, `TRIANGLE_FAN` |
| culling | `CULL_NONE`, `CULL_FRONT`, `CULL_BACK` |

### Not included

This first version of the toolkit covers drawing shapes with your own shaders. It doesn't have:

- **textures** (images) or uniform buffers: they need *descriptor sets*, which aren't here yet. Colors come from the corners and from push constants.
- buffers that change after they're made;
- compute shaders, or drawing into images other than the window;
- multisampling (smoothed edges).

These may come later.

---

# Part E: Guides

<!-- docs/guides/game-loop.md -->

## The Game Loop

A console program runs from top to bottom and ends. A game is different: it runs **in a loop**, many times a second, until the player quits. Each time around is a **frame**.

### What happens in a frame

Every frame, a game does three things:

1. **Read input**: which keys are held, where the mouse is.
2. **Update**: move things, check collisions, change scores, all by a small step of time.
3. **Draw**: clear the screen and draw everything where it is now.

At 60 frames per second, this happens 60 times every second. Each frame is a still picture, and showing them quickly makes the motion.

### The two ways to write it in Forge

#### 1. The engine loop: window.run()

Forge runs the loop and calls two functions you write in `main.frg`:

```cpp
#include <window>
#include <graphics2d>
#include <input>

float x = 100;

int main(){
    window.open(800, 450, "Engine loop");
    window.run();               // only returns when the window closes
    return 0;
}

void update(float dt){          // called first, every frame
    if (input.keyDown(Key.RIGHT)) {
        x += 200 * dt;
    }
}

void draw(){                    // called second, every frame
    graphics2d.drawRect(x, 200, 40, 40, color.ORANGE);
}
```

Keeping `update` (changing things) separate from `draw` (showing things) keeps code tidy. **This is the recommended style.**

#### 2. Your own loop

You write the loop yourself. `window.shouldClose()` ends the previous frame (showing it), starts a new one, and gives back `true` when it's time to quit:

```cpp
int main(){
    window.open(800, 450, "Own loop");
    while (!window.shouldClose()) {
        mainLoop();
    }
    window.close();
    return 0;
}

void mainLoop(){
    float dt = window.deltaTime();
    if (input.keyDown(Key.RIGHT)) {
        x += 200 * dt;
    }
    graphics2d.drawRect(x, 200, 40, 40, color.ORANGE);
}
```

This gives you more control, for example over the order of things, or skipping updates while paused.

#### What Forge does around your code

| Step | Engine loop | Own loop |
|---|---|---|
| clear the screen to the background color | before `update` | at the start of each frame (inside `shouldClose`) |
| continue tasks whose `wait` is over | before `update` | at the start of each frame |
| your code | `update(dt)`, then `draw()` | the loop body |
| show the frame, read new input, keep to 60 FPS | after `draw` | at the next `shouldClose()` |

### dt: delta time

`dt` (or `window.deltaTime()`) is **the number of seconds since the last frame**: about 0.0167 at 60 frames per second.

**Always multiply movement by `dt`:**

```cpp
x += 200 * dt;      // 200 pixels per SECOND, on any computer
x += 3;             // 3 pixels per FRAME: twice as fast at 120 FPS, half as fast when the game lags
```

The same goes for anything that changes over time: velocity, rotation, timers, fading.

```cpp
velocity.y += gravity * dt;        // speed changes by gravity per second
position += velocity * dt;          // position changes by velocity per second
timer -= dt;                        // counts down in seconds
angle += 90 * dt;                   // 90 degrees per second
```

### Game state

Values that must last from frame to frame (positions, scores, lists of enemies) need to be **global variables**, or fields of objects that live in globals. A local variable in `update` starts fresh every frame:

```cpp
float x = 100;        // global: keeps its value between frames

void update(float dt){
    float speed = 200;    // local: made fresh every frame (fine for things that don't change)
    x += speed * dt;
}
```

### Organizing a bigger game

- Split things into files: `player.frg`, `enemies.frg`, `level.frg`, each with its own `update` and `draw` functions, called from `main.frg`'s: `player.update(dt);`, `player.draw();`.
- Use scenes (see “`<scenes>`: Menus, Levels and Game Over”) for menus, levels and game-over screens.
- Use tasks (see “11. Tasks”) for things that happen over several seconds.
- Use ECS (see “`<ECS>`: Entities and Components”) when many different kinds of objects share behavior.

See the platformer walkthrough (see “Platformer Walkthrough”) for a complete, small example.

<!-- docs/guides/sprites-and-animation.md -->

## Sprites and Animation

A **sprite** is a 2D image for a game object. This guide covers drawing sprites, sprite sheets, animation, and flipping. The full reference is in `<assets>` (see “`<assets>`: Images, Animations and Fonts”).

### Drawing one image

```cpp
#include <window>
#include <assets>

Texture ship = assets.loadTexture("ship.png");

int main(){
    window.open(800, 450, "Sprites");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    ship.draw(100, 100);                   // normal size, top-left at (100, 100)
    ship.drawScaled(300, 100, 3);          // three times bigger
}
```

Put `ship.png` in the game's folder, next to `main.frg`.

### Pixel art

Small pixel-art images (16×16, 32×32) are usually drawn several times bigger. Forge keeps them sharp, without blurring. Pick a whole-number scale (2, 3, 4...) so every pixel stays the same size.

### Sprite sheets

A **sprite sheet** is one image holding many pictures (**frames**) side by side, like a strip of film:

```
+------+------+------+------+
|  0   |  1   |  2   |  3   |      blob.png: 4 frames, each 16x16, so 64x16 overall
+------+------+------+------+
```

`drawPart(source, destination)` draws just one part of an image:

- `source` is a `rect` **inside the image**: which frame.
- `destination` is a `rect` **on the screen**: where to draw it, and how big.

```cpp
int frame = 2;
rect source = rect(frame * 16, 0, 16, 16);          // the 3rd frame
rect destination = rect(x, y, 64, 64);              // drawn 4x bigger
blob.drawPart(source, destination);
```

For a sheet with several rows (one row per animation), also pick the row: `rect(frame * 16, row * 16, 16, 16)`.

### Animation

To animate, show the frames one after the other. The `Animation` class from `<assets>` does that for you:

```cpp
#include <assets>

Texture blob = assets.loadTexture("blob.png");
Animation wobble = Animation(blob, 16, 16, 4, 8);    // the sheet, frame width, frame height, how many frames, frames per second

void update(float dt){
    wobble.update(dt);                 // moves on to the next frame when it's time
}

void draw(){
    wobble.drawScaled(x, y, 4);        // the current frame, 4 times bigger
}
```

Frames are read from the sheet **left to right**. If there are more frames than fit in a row, they carry on at the start of the next row.

| Making one | |
|---|---|
| `Animation(Texture sheet, int frameWidth, int frameHeight, int frames, float fps)` | frames starting at the top-left of the sheet |
| `Animation(sheet, frameWidth, frameHeight, frames, fps, int row)` | frames starting at the beginning of row `row` (0 is the top row), for sheets with one row per animation |

| Method or field | What it does |
|---|---|
| `update(float dt)` | moves the animation on by `dt` seconds. Call it once per frame |
| `draw(float x, float y, color tint = color.WHITE)` | draws the current frame at its real size, with its top-left corner at (x, y) |
| `drawScaled(float x, float y, float scale, color tint = color.WHITE)` | the same, `scale` times bigger |
| `play()` / `pause()` | starts and stops it. A paused animation stays on its frame |
| `reset()` | back to the first frame, playing |
| `flip(bool flipped)` | `flip(true)` draws it mirrored left-to-right, `flip(false)` normally (see below) |
| `isFlipped()`, `isPlaying()` | what was chosen |
| `frame()`, `setFrame(int frame)`, `frameCount()` | the frame being shown (from 0), jumping to one, and how many there are |
| `float fps` | frames per second; change it any time (0 stops it) |
| `bool looping` | `true` (the start): after the last frame it starts again. `false`: it stops on the last frame |
| `isDone()` | `true` when an animation that doesn't loop has reached its last frame |

If the sheet doesn't have room for the frames you asked for, the game stops with a message saying how big the sheet really is.

#### Several animations

Make one `Animation` per action, and keep the one being shown in a variable. With a sheet that has the idle frames in row 0 and the running frames in row 1:

```cpp
Texture sheet = assets.loadTexture("hero.png");
Animation idle = Animation(sheet, 16, 16, 2, 3);        // row 0: 2 frames, slow
Animation run = Animation(sheet, 16, 16, 6, 12, 1);     // row 1: 6 frames, fast
Animation current = idle;

void update(float dt){
    Animation wanted = moving ? run : idle;
    if (wanted != current) {
        current = wanted;
        current.reset();       // start the new animation from its first frame
    }
    current.update(dt);
}

void draw(){
    current.drawScaled(x, y, 4);
}
```

`Animation` is a class, so `current = run` makes `current` mean the very same animation as `run` (not a copy).

A one-time animation, like an explosion: set `looping = false`, and remove the explosion when `isDone()` is true.

#### How it works

There's no magic inside: an animation counts time with `dt` and picks a part of the sheet with `drawPart`. Doing it by hand looks like this, which is useful if you ever need something `Animation` can't do:

```cpp
int frame = 0;
float frameTimer = 0;
const float FRAME_TIME = 0.1;      // 10 frames per second
const int FRAME_COUNT = 4;

void update(float dt){
    frameTimer += dt;
    if (frameTimer >= FRAME_TIME) {
        frameTimer -= FRAME_TIME;
        frame = (frame + 1) mod FRAME_COUNT;       // 0, 1, 2, 3, 0, 1, ...
    }
}

void draw(){
    blob.drawPart(rect(frame * 16, 0, 16, 16), rect(x, y, 64, 64));
}
```

### Facing left and right

Draw sprites facing right, then flip them when moving left:

```cpp
if (velocity.x < 0) {
    walk.flip(true);
}
if (velocity.x > 0) {
    walk.flip(false);
}
```

(Only change it when the player is actually moving, so they keep facing the same way when they stop.)

Without an `Animation`, a **negative width** in the source rectangle of `drawPart` flips the image the same way:

```cpp
bool facingLeft = velocity.x < 0;
float w = facingLeft ? -16 : 16;
sheet.drawPart(rect(frame * 16, 0, w, 16), rect(x, y, 64, 64));
```

### Rotating

`drawRotated(center, degrees, scale)` draws the image **centered** on a point and turned. Positive degrees turn clockwise:

```cpp
float angle = 0;

void update(float dt){
    angle += 180 * dt;         // half a turn per second
}

void draw(){
    ship.drawRotated(vec2(400, 225), angle, 2);
}
```

To point at something (like the mouse), work out the angle with `math.atan2` and convert radians to degrees:

```cpp
vec2 toMouse = input.mousePosition() - shipPosition;
float degrees = math.atan2(toMouse.y, toMouse.x) * 180 / math.PI;
```

### Tinting and fading

The `tint` color multiplies the image's colors:

```cpp
ship.draw(x, y, color.RED);                    // reddish (e.g. when hit)
ship.draw(x, y, color.WHITE.fade(0.5));        // half see-through
```

A hit flash with a task (see “11. Tasks”):

```cpp
color shipTint = color.WHITE;

task hitFlash() {
    shipTint = color.RED;
    wait(0.1);
    shipTint = color.WHITE;
}
```

### A working example

examples/sprites (the file `examples/sprites/main.frg` in the Forge folder) loads a 4-frame sprite sheet, animates it with an `Animation`, moves it with the arrow keys (flipping it to face the way it's going), and plays a sound when it hops.

<!-- docs/guides/platformer-walkthrough.md -->

## Platformer Walkthrough

This guide goes through examples/platformer (the file `examples/platformer/` in the Forge folder) line by line. It's a small but complete game: a cube that runs and jumps across platforms, collects twelve coins, and falls back to the start if it drops into a pit. The level is wider than the window, so a **camera** follows the player.

Run it from the Forge folder with `frg run examples/platformer`. The controls are the arrow keys or A/D to move, and Space, W or Up to jump.

The game is split into three files:

| File | Job |
|---|---|
| `main.frg` | opens the window and runs the loop, calling the other two files |
| `player.frg` | the player: input, jumping, falling into a pit, drawing |
| `level.frg` | the level, drawn as text, and the coins |

### main.frg

```cpp
#include <window>
#include <graphics2d>
#include <physics>
#include <math>
```

Each file includes only the libraries it uses itself. `main.frg` opens the window, moves the camera, draws the HUD text, and steps the physics. (`<math>` is for `math.clamp`, below.)

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));
```

The 2D camera (see “`<graphics2d>`: 2D Drawing” › “Camera: scrolling and zooming”). It looks at its `target` (a point in the level) and shows that point at its `offset` (a point on the screen: here the middle of the 960×540 window). It starts out looking at the middle of the first screen.

```cpp
int main(){
    window.open(960, 540, "Forge Platformer");
    window.setBackground(color(24, 26, 40));
    level.build();
    player.start = level.start;     // the P in the level
    player.hero.position = level.start;
    window.run();
    return 0;
}
```

- `window.open` makes a 960×540 window.
- `level.build()` calls `build` in `level.frg`, which turns the level's text into physics bodies and coins. The file-name prefix is required, because it's in another file. Then the player is moved to the `P` in the level.
- `window.run()` starts the engine loop (see “The Game Loop”): from now on Forge calls `update(dt)` and `draw()` every frame, until the window closes.

```cpp
void update(float dt){
    player.handleInput();
    physics.step(dt);
    player.checkFall();
    level.collectCoins(player.hero.bounds());
    followPlayer(dt);
}
```

This order matters:

1. **Input first**: read the keys and set the player's velocity.
2. **Then physics**: move everything by its velocity, with gravity and collisions.
3. **Then the results**: did the player fall into a pit? Did they touch a coin?
4. **Last, the camera**, so it follows where the player is now.

`player.hero` is the global variable `hero` from `player.frg`, and `.bounds()` is its rectangle.

```cpp
void followPlayer(float dt){
    float half = window.width() / 2.0;
    float x = math.clamp(player.hero.center().x, half, level.tiles.width() - half);
    camera.target = camera.target.lerp(vec2(x, window.height() / 2.0), 5 * dt);
}
```

The camera follows the player smoothly:

- `x` is where the camera *wants* to look: the middle of the player. `math.clamp` keeps it at least half a window away from both ends of the level, so the camera never shows the empty space past the edges.
- The camera only follows sideways: its `y` stays in the middle of the window, because the level is exactly one window tall.
- `lerp` moves the target **part of the way** there each frame (`5 * dt` is about 8% at 60 frames per second). Far away, that's a big step; close by, a small one. That's what makes it glide instead of jerk. A bigger number than 5 follows more tightly.

```cpp
void draw(){
    graphics2d.setCamera(camera);
    level.draw();
    player.draw();
    graphics2d.resetCamera();
    ...
}
```

`setCamera` makes everything drawn after it go through the camera: the level and the player are drawn at their **world** positions (a platform at x = 2000 really is at 2000), and the camera decides which part shows. `resetCamera` switches back to plain **screen** positions, so the text stays in the same place however far the player runs.

The level is drawn first, then the player on top of it, then the text on top of everything. Later drawing covers earlier drawing.

```cpp
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);
```

This lines text up against the right edge: measure it, then start it `width` pixels from the right.

```cpp
    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2.0, 200, 48, color.WHITE);
    }
```

When the list of coins is empty, a centered win message is drawn: start half its width left of the middle.

### player.frg

```cpp
const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;
vec2 start = vec2(80, 380);
```

`start` is where the player starts, and where it goes back to after falling. It's a variable, not a constant, because `main.frg` moves it to the `P` in the level. Constants keep the "tuning numbers" in one place. Try changing them: a higher `JUMP_SPEED` means higher jumps.

```cpp
class Player : Body {
    color tint = color.ORANGE;
    int deaths = 0;
    ...
}
```

`Player` **inherits** from the physics `Body`, so a player *is* a body: it has `position`, `size`, `velocity`, `onGround` and the rest. It adds its own `tint` and `deaths`.

```cpp
    void respawn() {
        position = start;
        velocity = vec2.ZERO;
        deaths++;
        flash();
    }
```

Inside a method, `position` and `velocity` are the player's own fields (inherited from `Body`). `flash()` starts a task.

```cpp
    task flash() {
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
```

A task (see “11. Tasks”): it switches the color, waits 0.08 seconds, switches back, and so on, while the game keeps running. Without tasks, this would need a timer variable and some state-tracking.

```cpp
Player hero = makeHero();

Player makeHero(){
    Player p = Player(start.x, start.y, 36, 36);
    physics.add(p);
    return p;
}
```

A global variable's starting value can call a function, even one written below it. `Player(...)` uses `Body`'s constructor (x, y, width, height), since `Player` doesn't have its own. `physics.add` puts it in the physics world so `physics.step` moves it.

```cpp
void handleInput(){
    float direction = 0;
    if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) {
        direction -= 1;
    }
    if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) {
        direction += 1;
    }
    hero.velocity.x = direction * MOVE_SPEED;
```

`direction` ends up -1 (left), 1 (right) or 0 (neither, or both). Setting `velocity.x` directly makes movement instant and responsive, with no sliding.

```cpp
    bool jumpPressed = input.keyPressed(Key.SPACE) or input.keyPressed(Key.W) or input.keyPressed(Key.UP);
    if (jumpPressed and hero.onGround) {
        hero.velocity.y = -JUMP_SPEED;
    }
```

`keyPressed` (not `keyDown`) means one jump per press. `onGround` was set by the previous physics step, so you can only jump while standing on something. The velocity is **negative** because up is negative y on the screen.

```cpp
    bool jumpReleased = input.keyReleased(Key.SPACE) or input.keyReleased(Key.W) or input.keyReleased(Key.UP);
    if (jumpReleased and hero.velocity.y < 0) {
        hero.velocity.y *= 0.5;
    }
}
```

**Variable jump height**: letting go of the button while still rising halves the upward speed. A tap gives a short hop, and holding gives a full jump. Small touches like this make a platformer feel good.

```cpp
void checkFall(){
    if (hero.position.y > window.height() + 100) {
        hero.respawn();
    }
}
```

If the player falls more than 100 pixels below the bottom of the window, it's sent back to the start.

```cpp
void draw(){
    graphics2d.drawRect(hero.bounds(), hero.tint);
    float look = hero.velocity.x > 0 ? 4 : hero.velocity.x < 0 ? -4 : 0;
    float eyeY = hero.position.y + 12;
    graphics2d.drawRect(hero.position.x + 8 + look, eyeY, 6, 8, color.BLACK);
    graphics2d.drawRect(hero.position.x + 22 + look, eyeY, 6, 8, color.BLACK);
}
```

The body is a colored rectangle. The eyes are two small black rectangles, shifted 4 pixels in the direction of movement, so the cube "looks" where it's going. The two `? :` choose 4, -4 or 0.

### level.frg

The level is **drawn as text**, with a tile map (see “`<tilemap>`: Levels Drawn as Text”): each character is one 36 × 36 square of the level.

```cpp
TileMap tiles = TileMap("""
#                                                          #
#                                                          #
#                                                          #
#                                                          #
#                                             C            #
#                            C               ====          #
#                      C    ####                           #
#         C           ====                C                #
#        ====                            ====              #
#    C              C                  C             C     #
#   ====          =====         C    =====          ====   #
#                                                          #
# P     C                                               C  #
############    ##############     ###########    ##########
############    ##############     ###########    ##########
""", 36);
```

| Character | Means |
|---|---|
| `#` | ground and walls |
| `=` | a one-way platform: you can jump up through it and land on top |
| `C` | a coin |
| `P` | where the player starts |
| (a space) | nothing |

The meaning isn't built into Forge: it's what `build` and `draw` below do with each character. To change the level, change the text. The 60 columns × 36 pixels make it 2160 pixels wide, and the 15 rows make it exactly one window (540 pixels) tall.

```cpp
void build(){
    tiles.addBodies("#");
    tiles.addBodies("=", true);
    for (vec2 corner in tiles.find("C")) {
        coins.add(corner + vec2(tiles.tileSize / 2, tiles.tileSize / 2));
    }
    totalCoins = coins.length;
    start = tiles.find("P")[0];
}
```

- `addBodies("#")` makes static physics bodies for the `#` tiles. Neighbouring tiles are **merged**: a row of 12 ground tiles becomes one long body, not 12 small ones. `addBodies("=", true)` does the same for the planks, and `true` makes them one-way (see “`<physics>`: Boxes, Gravity and Collisions” › “One-way platforms”).
- `find("C")` gives the top-left corner of every `C` tile. Each coin is kept as the middle of its tile, half a tile further right and down.
- `find("P")[0]` is the first (and only) `P`: the player's start.

```cpp
void collectCoins(rect playerBox){
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
            ...
        }
    }
}
```

Each coin is checked as a 20×20 box around its center. The loop goes **backwards**: removing item `i` moves every later item down one place, and going backwards means those have already been checked.

```cpp
void draw(){
    tiles.draw("#", color(70, 86, 120));
    for (int row = 0; row < tiles.rows(); row++) {
        for (int column = 0; column < tiles.columns(); column++) {
            if (tiles.get(column, row) == "#" and tiles.get(column, row - 1) != "#") {
                rect r = tiles.tileRect(column, row);
                graphics2d.drawRect(r.x, r.y, r.width, 5, color(110, 200, 130));
            }
        }
    }
    for (vec2 p in tiles.find("=")) {
        graphics2d.drawRect(p.x, p.y, tiles.tileSize, 10, color(170, 130, 90));
    }
    ...
}
```

- `tiles.draw("#", color)` fills every `#` tile with one color.
- The two loops go through every tile and add a strip of green **grass** on top of each `#` that has nothing above it. `get(column, row - 1)` is the tile above; outside the map, `get` gives `" "`, so the top row works too.
- The one-way platforms are drawn as thin wooden planks.
- Then the coins: a gold circle with an orange outline each.

With an image of tiles instead of colors, `tiles.drawTiles(tileset, 16, "#=")` draws each `#` as the tileset's first 16 × 16 picture and each `=` as the second.

### Particles: dust and sparkles

Two particle emitters (see “`<particles>`: Dust, Sparks and Explosions”) make the game feel more alive:

- **Dust when landing** (`player.frg`). `handleInput` remembers `fallSpeed = hero.velocity.y` *before* `physics.step`, because landing sets the velocity to 0. After the step, `checkLanding(dt)` sees that the player is on the ground now but wasn't last frame, and if it was falling fast enough, puts the `dust` emitter at the player's feet and calls `dust.burst(14)`.

```cpp
void checkLanding(float dt){
    if (hero.onGround and !wasOnGround and fallSpeed > 250) {
        dust.position = vec2(hero.center().x, hero.position.y + hero.size.y);
        dust.burst(14);
    }
    wasOnGround = hero.onGround;
    dust.update(dt);
}
```

- **Sparkles when collecting a coin** (`level.frg`). `collectCoins` moves the `sparkles` emitter to the coin and bursts 24 golden particles that fall and fade. `updateEffects(dt)` updates them, and `draw` draws them.

Both emitters are made by small functions (`makeDust`, `makeSparkles`) that set their colors, sizes, speed and gravity. They're drawn between `setCamera` and `resetCamera`, so they stay in the right place in the level.

### Things to try

1. Show the death count on screen (`player.hero.deaths`).
2. Add another coin and a platform to reach it, by typing a `C` and some `=` into the level text. Then make the level longer: add more columns to every row.
3. Make the jump higher, or gravity weaker (`physics.setGravity(vec2(0, 700))` in `main`).
4. Add a timer: `window.time()` at the start, and show how long the run took when all coins are collected.
5. A moving platform: keep a `Body` in a variable and change its `position.x` in `update` using `math.sin(window.time())`.
6. Swap the rectangle for a sprite (see Sprites and animation (see “Sprites and Animation”)) and add a jump sound (see `<audio>` (see “`<audio>`: Sounds and Music”)).
7. Add a second level and a title screen with `<scenes>` (see “`<scenes>`: Menus, Levels and Game Over”).
8. Zoom the camera in a little (`camera.zoom = 1.5;` in `main`), or let it follow the player up and down too.

---

# Part F: Under the hood

<!-- docs/how-forge-works.md -->

## How Forge Works

You don't need any of this to *use* Forge. It's for the curious: what actually happens between your `.frg` files and a running `.exe`, and how to find your way around Forge's own code.

### The big picture

```
your .frg files
      │
      ▼
 1. read files ──── project.cpp: finds main.frg, the files next to it, and #included folders
      │
      ▼
 2. lex ─────────── lexer.cpp:   turns text into tokens (words, numbers, symbols)
      │
      ▼
 3. parse ───────── parser.cpp:  turns tokens into a syntax tree (functions, statements, expressions)
      │
      ▼
 4. check ───────── checker.cpp: works out types and names, and reports errors
      │
      ▼
 5. generate ────── codegen.cpp: writes the program out as C++ (build/forge_generated.cpp)
      │
      ▼
 6. compile ─────── g++ turns that C++, plus Forge's runtime, into build/<game>.exe
```

Steps 1–5 are Forge itself (`frg.exe`, written in C++). Step 6 uses the g++ compiler from w64devkit.

### 1. Reading the project

`frg` starts at `main.frg`, reads every `.frg` file in the same folder, and then follows `#include <"folder">` lines into subfolders. Each file becomes a **module** named after the file, which is why other files reach it as `player.something`. Folders become nested names (`enemies.goblin`).

### 2. Lexing

The **lexer** reads characters and groups them into **tokens**: `int`, `score`, `=`, `10`, `;`. It skips spaces and comments, and keeps track of each token's line and column for error messages. Text with `{interpolation}` is kept whole, and the parts inside the braces are lexed separately later.

### 3. Parsing

The **parser** reads the tokens and builds a **syntax tree**: a structure that captures meaning, like "a variable declaration, type `int`, name `score`, value: the number 10". Expressions follow the order of operations, so `2 + 3 * 4` becomes "add 2 to (multiply 3 by 4)".

The parser recognizes both spellings of every operator (`+` and `plus`) and produces the same tree for both. When it finds a syntax error, it reports it, skips to the end of that statement, and carries on, so several syntax errors can be reported at once.

### 4. Checking

The **checker** is where most of Forge's friendliness lives. It runs in two passes:

- **Pass 1** learns every type, function and global variable in every file. That's why things can be used before they're written, even across files.
- **Pass 2** goes through every function body, working out the type of every expression and making sure everything fits: you can't put a `float` into an `int`, conditions must be `bool`, and every path of a function returns.

Each error comes with a location and, often, a hint. The checker also gives **warnings** for code that works but probably isn't what was meant (a changed copy in a `for ... in` loop, `int / int` going into a float, unused variables, code after `return`, `==` on decimals). Warnings are collected while checking and printed afterwards, in yellow; they're only shown to people when there are no errors, and they never stop the build. "Did you mean...?" suggestions come from comparing the unknown name with known names and picking the closest one.

Built-in types and libraries (`vec2`, `window`, `physics`...) are written in **Forge syntax** inside the compiler (in `prelude.cpp`). The checker reads them like any other code, which is why they follow the same rules and give the same kind of error messages.

### 5. Generating C++

The **code generator** writes the checked program as C++:

| Forge | C++ |
|---|---|
| each file | a C++ namespace: `player.frg` → `m_player` |
| every name you choose | the name plus `_`, so `score` → `score_`. That way nothing can clash with C++ keywords. |
| `struct` | a C++ `struct` (copied by value) |
| `class` | a C++ `struct` shared through `std::shared_ptr` (reference counted) |
| `list<T>`, `map<K, V>` | `std::vector`, `std::map` |
| `T?` | `std::optional<T>` (or a null-able pointer for classes) |
| `task` functions | C++20 coroutines (`co_await`) |
| `"HP: {hp}"` | string concatenation |
| `list[i]` | a checked access that stops with a clear message if `i` is out of range |

Forge's own helper names contain `$`, which Forge names can't, so they never clash with yours either. You can read the result in `build/forge_generated.cpp`.

### 6. Compiling

`frg` runs g++ on the generated file:

```
g++ -std=gnu++20 -O1 -fwrapv -w -I<runtime> -I<cache> forge_generated.cpp -o game.exe -static
```

- If the game's folder has `icon.png`, Forge wraps the PNG in an `.ico` file (an `.ico` can simply hold a PNG), writes a one-line resource script `GLFW_ICON ICON "icon.ico"`, and runs **windres** (from w64devkit) to turn it into `build/icon_res.o`, which is linked in. That gives the `.exe` its icon; the name `GLFW_ICON` is the one raylib's window library looks for, and the window also loads `icon.png` with raylib's `SetWindowIcon`.
- `-O1` optimizes a little, and compiles fast. `--release` builds use `-O2`, which optimizes more but takes longer.
- **Precompiled headers.** Most of a build's time goes into reading Forge's runtime headers (and raylib's). So the generated file starts with `#include "forge_pch.hpp"`, and Forge compiles that header once into `forge_pch.hpp.gch` in a `cache` folder next to `frg.exe` (or in the game's `build/` folder if that one can't be written). There are two versions: one with the core and the libraries that don't use raylib, and one that adds every raylib library, so all games share them. The toolkits' own `.cpp` files are cached as `.o` files the same way. The cache folder's name comes from everything that affects the result (Forge's version, the runtime files' dates, which g++ is used, and the flags), so it's never stale. Each cache folder also has a `stamp.txt` naming the Forge and g++ that made it; when a new folder is made, folders from an older Forge or g++ are deleted, so the cache doesn't grow after updates. Using a folder touches its `stamp.txt`, and after each build the folders used least recently are deleted until the cache is under about 400 MB (the ones that build used always stay). `frg cache` shows the size, and `frg cache clear` deletes it all. If a cached file is unusable anyway, g++ just reads the real headers, so the cache only changes speed, never behavior.
- `-fwrapv` makes whole numbers wrap around predictably when they overflow.
- `-static` puts everything into the `.exe`, so it runs anywhere without extra files.
- Games that use the engine libraries also link **raylib** (`-lraylib -lopengl32 -lgdi32 -lwinmm`).
- Without a console (`--release` or `--no-console`), it adds `-mwindows` and defines `FRG_NO_CONSOLE`, which makes runtime errors show in a message box. `--release` also adds `-s` (a smaller file).

### The runtime

The **runtime** (`runtime/*.hpp`) is the C++ code every game is built with:

| File | Contains |
|---|---|
| `forge.hpp` | printing, text conversion, lists and maps, optionals, tasks, the core types (`vec2`, `color`...), random |
| `forge_window.hpp` | `<window>`, `<input>`, `<graphics2d>` on top of raylib |
| `forge_graphics3d.hpp`, `forge_assets.hpp`, `forge_audio.hpp`, `forge_ui.hpp`, `forge_particles.hpp` | the other engine libraries |
| `forge_physics.hpp`, `forge_scenes.hpp`, `forge_ecs.hpp`, `forge_time.hpp`, `forge_file.hpp`, `forge_save.hpp`, `forge_tween.hpp` | libraries that don't need raylib |
| `forge_physics_draw.hpp` | `physics.drawDebug()`, which draws with raylib. Forge adds it only to games that use both `<physics>` and `<graphics2d>`, so `<physics>` itself never needs raylib |
| `raylib/` | raylib 5.5 (by Ramon Santamaria and contributors, zlib license) |

### Forge's source code

```
src/
  main.cpp          the frg command: new / build / run / check
  common.hpp        source locations and error reporting
  diagnostics.cpp   prints error messages (and --machine output for editors)
  lexer.cpp         text → tokens
  parser.cpp        tokens → syntax tree
  ast.hpp           what the syntax tree is made of
  project.cpp       loading files and folders
  types.cpp         Forge's types (int, list<T>, classes...)
  prelude.cpp       built-in types and libraries, in Forge syntax
  builtins.cpp      print, math and other functions with special rules
  checker.cpp       type checking and error messages
  checker_util.hpp  small helpers shared by the checker and code generator
  codegen.cpp       syntax tree → C++
runtime/            the C++ runtime (see above)
tests/              .test files and the test runner
editors/vscode/     the VS Code extension
examples/           example games
docs/               these docs
```

### Tests

`tests/cases/*.test` files each hold a small program and what should happen:

```
=== main.frg
#include <stdio>
int main(){ print(1 + 1); return 0; }
=== output
2
```

The other kinds are `=== error` (the build must fail with these messages) and `=== build` (it must compile; it isn't run, for games with windows). `=== new` (with a template name) runs `frg new --template` and builds the result. `=== package` runs `frg package` and lists files that must (or, with `!`, must not) be there afterwards. `=== runtime error` runs the program and expects it to stop with these messages. `=== warning` can be added to any test: each of its lines must appear in the warnings. `=== no warnings` (empty) means there must be none. Run them all with `.\tests\run_tests.ps1` (in PowerShell 7).

### How the VS Code extension finds errors

When you save, the extension runs `frg check <game folder> --machine`. In machine mode, each error is printed as one tab-separated line (`E`, file, line, column, length, message, hint), which the extension turns into red underlines. Warnings are the same, but start with `W` and become yellow underlines.

### Adding a library

1. Write the Forge declarations (functions, and classes with their methods) in `src/prelude.cpp`, as a new entry in `preludeLibs()`.
2. Write the C++ in a new `runtime/forge_<name>.hpp`, in `namespace frg::<name>`. Every function's name is the Forge name plus `_`; classes are `struct Name_` with a static `frg$new(...)`.
3. Rebuild Forge, and add tests and docs.

---

# Part G: The cheat sheet (everything on one page)

<!-- docs/LANGUAGE.md -->

## Forge Cheat Sheet (v0.7)

The whole language on one page, for looking things up quickly. For full explanations, examples and every detail, see the complete documentation (see “Forge Documentation”). Planned features are listed at the end.

**Contents:** A Forge game (see “A Forge game”) · Variables (see “Variables and types”) · Text (see “Text”) · Operators (see “Operators”) · If and loops (see “If, loops and switch”) · Functions (see “Functions”) · Files and folders (see “Multiple files”) · Lists and maps (see “Lists and maps”) · Structs and classes (see “Structs and classes”) · Enums (see “Enums”) · Optional values and null (see “Optional values and null”) · Pointers (see “Pointers”) · Tasks (see “Tasks: waiting without freezing the game”) · Built-in types (see “Built-in types: vec2, color, rect”) · Libraries (see “Libraries”) · Making a game (see “Making a game”) · Errors (see “Errors”)

### A Forge game

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
| `frg new mygame` | makes a folder `mygame/` with a starter `main.frg` (a hello world) |
| `frg new mygame --template window` | starts from a small game instead: `window` (a ball to move), `platformer`, or `3d` |
| `frg build` | compiles the game into `build/<folder name>.exe` |
| `frg run` | builds, then runs the game |
| `frg check` | only looks for errors |
| `frg cache` / `frg cache clear` | show how big the build cache is (kept under about 400 MB) / delete it |
| `frg package` | makes the finished game and zips it into `dist/<name>.zip`, ready to share |
| `--release` (after build or run) | the finished game: no console window, smaller `.exe` |
| `--no-console` / `--console` | turn the console window off or on, whatever else is chosen |

### Comments

````cpp
// a single-line comment

```
a multi-line comment
can go over many lines
```
````

### Variables and types

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

#### Converting between types

Forge converts automatically when nothing can be lost. For example, an `int` fits in a `float`. Otherwise you convert yourself:

```cpp
int whole = int(3.9);         // 3 (the decimals are cut off)
float f = float(7) / 2;       // 3.5
int n = int("42");            // text to number
string s = string(99);        // anything to text
```

Watch out: dividing two whole numbers gives a whole number. `7 / 2` is `3`, and `int a = 70; int b = 30; float r = a / b;` gives `2.0`. Use `7.0 / 2` or `float(a) / b` to get a decimal. One special case: when both sides are numbers written straight into the code, they adapt to where the result goes, so `float r = 70 / 30;` is `2.3333333` while `print(70 / 30)` is `2`. (Full rules (see “2. Variables and Types” › “How division works”).)

Whole numbers **wrap around** when they go past their limit. A `uint8` holding `250` becomes `4` after `+= 10`.

### Text

Put any value inside `{ }` in a string to show it. This works for numbers, lists, structs and anything else:

```cpp
print("HP: {hp}/{MAX_HP}");
print("Is it there? {name.contains("ero")}");
print("Speed: {speed:.2}");      // a float or double with 2 decimals: 3.14
```

| Write this | To get |
|---|---|
| `\n` `\t` | a new line, a tab |
| `\"` `\\` | a quote, a backslash |
| `{{` and `}}`, or `\{` and `\}` | a literal `{` or `}` |

**Raw text** goes between three quotes and can cover many lines. It's used exactly as written: escapes and `{ }` don't work in it. A line break straight after the opening `"""` is left out. (Details (see “3. Text” › “Raw text”).)

```cpp
string SHADER = """
#version 330 core
void main() { }
""";
```

Text has `.length` plus these methods: `contains(t)` `indexOf(t)` `startsWith(t)` `endsWith(t)` `upper()` `lower()` `trim()` `replace(a, b)` `split(sep)` `substring(start, count)` `charAt(i)` `repeat(n)` `isEmpty()` `isInt()` `isFloat()` (true if `int(...)` / `float(...)` would work on it).

### Operators

Every operator has a symbol **and** a word. They mean exactly the same and can be mixed. (Full details (see “4. Operators”).)

| Symbol | Word | Meaning |
|---|---|---|
| `a + b` `a - b` `a * b` `a / b` `a % b` | `a plus b` `a minus b` `a times b` `a div b` `a mod b` | math (`%` / `mod` is the remainder) |
| `-a` | `negative a` | flip the sign |
| `a == b` `a != b` | `a equals b` `a not equals b` | equal, not equal |
| `a > b` `a < b` | `a greater b` `a less b` | greater than, less than |
| `a >= b` `a <= b` | `a greater or equal b` `a less or equal b` | |
| `&&` `\|\|` `!` | `and` `or` `not` | logic |
| `c ? a : b` | `a if c else b` | `a` if `c` is true, otherwise `b` |
| `a ?? b` | `a otherwise b` | `a`, or `b` if `a` is null |
| `&` `\|` `^` `~` `<<` `>>` | `bitand` `bitor` `bitxor` `bitnot` `shiftleft` `shiftright` | bits |
| `x = 5;` | `set x to 5;` | |
| `x += 5;` `x -= 5;` | `increase x by 5;` `decrease x by 5;` | |
| `x *= 2;` `x /= 2;` | `multiply x by 2;` `divide x by 2;` | |
| `x++;` `x--;` | `increase x;` `decrease x;` | |

```cpp
if (alive and not paused) { ... }
if (hp less or equal 0) { set lives to lives minus 1; }
string label = hp > 50 ? "healthy" : "hurt";
string label2 = "healthy" if hp greater 50 else "hurt";
```

There's no power operator: `^` is bitwise xor. Use `x * x` or `math.pow(x, 2)`.

### If, loops and switch

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

do {                             // like while, but runs at least once: the check comes after
    guess = int(input("Guess: "));
} while (guess != secret);       // note the ; at the end
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

### Functions

```cpp
int add(int a, int b){
    return a + b;
}

void greet(string who, string greeting = "Hi"){   // greeting has a default value
    print("{greeting} {who}!");
}
```

**Order doesn't matter.** A function can be used above or below the place it's written. Forge reads every file before it runs anything.

### Multiple files

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

#### Folders

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

### Lists and maps

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
| `list<T>` | `.length` `add(x)` `addAll(otherList)` `insert(i, x)` `removeAt(i)` `remove(x)` `contains(x)` `indexOf(x)` `first()` `last()` `pop()` `clear()` `isEmpty()` `sort()` `reverse()` `copy()` |
| `map<K, V>` | `.length` `has(key)` `get(key, backup)` `remove(key)` `keys()` `values()` `clear()` `isEmpty()` |

Positions in a list start at 0. Using a position that isn't in the list, or a key that isn't in the map, stops the game with a clear message.

### Structs and classes

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
- A class object is freed as soon as nothing refers to it any more. Two objects that refer to **each other** (`a.partner = b; b.partner = a;`) are never freed while the game runs. That rarely matters in small games, but if you make such pairs over and over, set one side to `null` when you're done with them (`a.partner = null;`). See When class objects go away (see “9. Structs and Classes” › “When class objects go away”).

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

- A type can have **several constructors**, as long as each takes different types of values: `Size(float side)` and `Size(float w, float h)`. Forge picks the one that matches what you give it.
- A type with no constructor is made with `Type()` or `Type(every field, in order)`.
- A class without its own constructors uses the ones it inherits. In a constructor, `super(...)` runs the parent class's constructor.
- `print` shows a struct or class as `Point(x: 3.0, y: 4.0)`. To show it differently, add a method `string toString()`.

#### Checking what kind of object something is: `is` and `as`

```cpp
for (Entity e in enemies) {
    if (e is Goblin) {               // true if e is a Goblin (or something that inherits from Goblin)
        Goblin g = e as Goblin;      // as gives a Goblin?, which is null if e isn't a Goblin
        g.cackle();
    }
}
```

#### private

`private` hides something so only certain code can use it:

```cpp
class Safe {
    private int code = 1234;          // only Safe's own methods (and classes that inherit from Safe) can use it
    bool tryOpen(int guess) {
        return guess == code;
    }
}

private int attempts = 0;             // at the top of a file: only this file can use it
```

`public` is also allowed, and means the same as writing nothing.

### Enums

An enum is a type with a fixed set of named values:

```cpp
enum State { Idle, Running, Jumping }

State s = State.Running;
print(s);              // Running
int n = int(s);        // 1
State t = State(0);    // Idle
```

### Optional values and null

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

### Pointers

A `Pointer<T>` points at a variable, so you can change that variable from somewhere else. `.value` reads or changes the variable it points at:

```cpp
void addOne(Pointer<int> counter) {
    counter.value += 1;
}

int score = 10;
addOne(score);        // score is now 11
```

Don't keep a pointer to a function's local variable after that function ends. `p.remove()` makes a pointer point at nothing: `p.isRemoved()` is then true, and using `p` stops the game with a clear message ("this pointer was removed ...") until it's pointed at something again (`p = score;`).

### Tasks: waiting without freezing the game

A function whose return type is **`task`** can call **`wait(seconds)`**. Only that task pauses; the rest of the game keeps running.

```cpp
task flash() {
    for (int i = 0; i < 5; i++) {
        tint = color.WHITE;
        wait(0.1);                    // this task pauses for 0.1 seconds
        tint = color.ORANGE;
        wait(0.1);
    }
}

flash();                              // starts it: it runs until its first wait, then carries on by itself
task t = flash();                     // keep it to check t.isDone() or call t.stop()
```

- `wait()` with nothing waits until the next frame. `wait(otherTask)` waits until another task finishes.
- `return;` ends a task early.
- With a window, waiting tasks wake up by themselves every frame. Without a window, `#include <tasks>` and call `tasks.update(dt)` yourself, or `tasks.runAll()` to wait until every task is done. `tasks.count()` and `tasks.stopAll()` are also there.
- Struct methods can't be tasks, but class methods can.

### Built-in types: vec2, color, rect

These are always available:

| Type | Fields | Useful extras |
|---|---|---|
| `vec2` | `x` `y` | `length()` `normalized()` `dot(v)` `distance(v)` `angle()` `rotated(r)` `lerp(v, t)`, `vec2.ZERO` `ONE` `UP` `DOWN` `LEFT` `RIGHT` |
| `vec3` | `x` `y` `z` | `length()` `normalized()` `dot(v)` `cross(v)` `distance(v)` `lerp(v, t)`, `vec3.ZERO` `ONE` `UP` `FORWARD` `RIGHT` |
| `vec4` | `x` `y` `z` `w` | `length()` `normalized()` `dot(v)` |
| `color` | `r` `g` `b` `a` (0 to 255) | `color(r, g, b)` or `color(r, g, b, a)` (ints; below 0 becomes 0, above 255 becomes 255), `fade(alpha)` `lerp(c, t)`, and `color.WHITE` `BLACK` `RED` `GREEN` `BLUE` `YELLOW` `ORANGE` `PURPLE` `PINK` `GOLD` `SKYBLUE` `GRAY` `LIGHTGRAY` `DARKGRAY` `BROWN` `MAGENTA` `LIME` `MAROON` `VIOLET` `BEIGE` `DARKGREEN` `DARKBLUE` `BLANK` |
| `rect` | `x` `y` `width` `height` | `overlaps(r)` `contains(point)` `center()` `position()` `size()` |

Vectors work with math: `pos += velocity * dt;`

### Libraries

Each file includes the libraries it uses. Functions are reached through the library name, like `window.open(...)`. Types from a library, like `Key` or `Body`, are used by their plain name.

| Include | Gives you |
|---|---|
| `<stdio>` | `print(x)` (adds a new line), `print()`, `write(x)` (no new line), `input()` / `input("prompt")` |
| `<math>` | `math.sqrt` `pow` `sin` `cos` `tan` `asin` `acos` `atan` `atan2` `exp` `log` `floor` `ceil` `round` `abs` `sign` `min` `max` `clamp` `lerp`, plus `math.PI` `TAU` `E` |
| `<random>` | `random.range(1, 6)` (whole numbers, both ends included), `random.range(0.5, 2.0)`, `random.value()` (0 to 1), `random.chance(0.25)`, `random.seed(n)`, `random.color()` |
| `<window>` | `window.open(width, height, "title")` `shouldClose()` `run()` `close()` `deltaTime()` `time()` `width()` `height()` `fps()` `setFPS(n)` `setTitle(t)` `setBackground(color)` `toggleFullscreen()` `screenshot("file.png")` |
| `<input>` | `input.keyDown(Key.SPACE)` (held), `keyPressed` (just pressed this frame), `keyReleased`, `mouseDown(MouseButton.LEFT)` `mousePressed` `mouseReleased` `mousePosition()` `mouseX()` `mouseY()` `mouseWheel()`. Keys: `Key.A`…`Key.Z`, `Key.NUM0`…`Key.NUM9`, `SPACE` `ENTER` `ESCAPE` `BACKSPACE` `TAB` `LEFT` `RIGHT` `UP` `DOWN` `LEFT_SHIFT` `RIGHT_SHIFT` `LEFT_CONTROL` `RIGHT_CONTROL` `LEFT_ALT` `RIGHT_ALT` `F1`…`F12`. Controllers: `input.gamepadDown(GamepadButton.A)` (also `gamepadPressed`, `gamepadReleased`; buttons `A` `B` `X` `Y` `LEFT_BUMPER` `RIGHT_BUMPER` `LEFT_TRIGGER` `RIGHT_TRIGGER` `BACK` `START` `LEFT_STICK` `RIGHT_STICK` `UP` `DOWN` `LEFT` `RIGHT`), `leftStick()` `rightStick()` (vec2, -1 to 1), `leftTrigger()` `rightTrigger()` (0 to 1), `gamepadConnected()`, `gamepadName()`. Each takes an optional controller number (0 = the first) |
| `<graphics2d>` | `graphics2d.drawRect(x, y, w, h, color)` or `drawRect(rect, color)`, `drawRectOutline(..., color, thickness)`, `drawCircle(x, y, radius, color)` or `drawCircle(vec2, radius, color)`, `drawCircleOutline`, `drawLine(from, to, color, thickness)`, `drawTriangle(a, b, c, color)`, `drawText(text, x, y, size, color)`, `textWidth(text, size)`, `drawFPS(x, y)`, `clear(color)`, and a 2D camera: `setCamera(Camera2D(target, offset, zoom, rotation))`, `resetCamera()`, `screenToWorld(p)`, `worldToScreen(p)` |
| `<physics>` | the `Body` class and `physics.add(body)` `remove(body)` `step(dt)` `setGravity(vec2)` `gravity()` `clear()` `count()` (see below) |
| `<assets>` | images and fonts: `assets.loadTexture("player.png")`, `assets.loadFont("font.ttf")`, `assets.exists(path)` (see below) |
| `<audio>` | `audio.loadSound("jump.wav")`, `audio.loadMusic("song.ogg")`, `audio.setVolume(0.5)` (see below) |
| `<graphics3d>` | 3D drawing (see below) |
| `<time>` | `time.now()` (seconds since the game started), `time.sleep(seconds)`, `time.clock()` ("14:05:09"), `time.date()` ("2026-10-14"), `time.year()` `month()` `day()` `hour()` `minute()` `second()`, and the `Timer` class |
| `<tilemap>` | `TileMap level = TileMap("""...""", 32);` (text, tile size) or `tilemap.load("level1.txt", 32)`: one character per tile. `level.addBodies("#")` (merged static bodies; `addBodies("=", true)` for one-way), `level.find("C")` (top-left corners), `get(col, row)` `set(col, row, "#")` `columns()` `rows()` `width()` `height()` `tileRect(col, row)` `columnAt(x)` `rowAt(y)` `solidAreas(tiles)`, `draw("#", color)`, `drawTiles(tileset, 16, "#=")` |
| `<particles>` | `Emitter e = Emitter(vec2(x, y));`, then `e.burst(30)` (all at once) or `e.rate = 50` (per second); `e.update(dt)` and `e.draw()` each frame. Fields: `position` `rate` `lifetime` `speed` `direction` (degrees, -90 is up) `spread` (degrees, 360 is every way) `startColor` `endColor` `startSize` `endSize` `gravity`; `count()` `clear()` |
| `<tween>` | `Tween t = Tween(from, to, seconds, Ease.OUT);` then `t.update(dt)` and `t.value()` each frame; `progress()` (0 to 1, for `lerp`), `isDone()`, `restart()`, `reverse()`, fields `looping` `pingPong`. Easing: `Ease.LINEAR` `IN` `OUT` `IN_OUT` `BOUNCE`; `tween.ease(Ease.OUT, t)` |
| `<save>` | `save.setInt(key, n)` and `save.getInt(key, backup)`, also `setFloat`/`getFloat`, `setString`/`getString`, `setBool`/`getBool`, and `has(key)` `remove(key)` `clear()`. Kept in `save.txt` in the game's folder, written straight away. |
| `<file>` | `file.read(path)` `write(path, text)` `append(path, text)` `exists(path)` `remove(path)` `readLines(path)` `writeLines(path, lines)` `files(folder)`. Paths are relative to the game's folder. |
| `<scenes>` | the `Scene` class and `scenes.change(scene)` `update(dt)` `draw()` `current()` (see below) |
| `<ui>` | `ui.button(rect, "text")` (true when clicked), `ui.checkbox(rect, "text", checked)` (gives the new value), `ui.slider(rect, value, min, max)` (gives the new value), `ui.textBox(rect, text, maxLength = 0)` (a box to type in; gives the new text), `ui.isTyping()`, `ui.label(text, x, y, size, color)`, `ui.panel(rect)`, `ui.setTheme(background, accent, text)` |
| `<ECS>` | the `Entity` and `Component` classes (see below) |
| `<tasks>` | `tasks.update(dt)` `runAll()` `count()` `stopAll()` (see Tasks (see “Tasks: waiting without freezing the game”)) |

Screen coordinates start at the **top-left** corner, and **y goes down**.

#### Low-level toolkits

`<OpenGL>`, `<DirectX>` and `<Vulkan>` are for writing your own rendering with a real graphics API. They're **separate from the engine**: a game that includes one can't use `<window>`, `<input>`, `<graphics2d>`, `<graphics3d>`, `<assets>` or `<ui>`, and can only use one toolkit. Libraries that don't draw (`<math>`, `<audio>`, `<physics>`, `<time>`...) still work.

```cpp
#include <OpenGL>

int main(){
    OpenGL.openWindow(800, 600, "My renderer");
    while (!OpenGL.shouldClose()) {
        OpenGL.clearColor(0.1, 0.1, 0.15, 1);
        OpenGL.clear(OpenGL.COLOR_BUFFER_BIT);
        // shaders, buffers and draw calls...
    }
    return 0;
}
```

All three have the same window, keyboard and mouse functions (`openWindow` `shouldClose` `closeWindow` `width` `height` `deltaTime` `time` `setTitle` `setVSync` `keyDown` `keyPressed` `keyReleased` `mouseDown` `mousePressed` `mouseReleased` `mousePosition` `mouseWheel`) and matrix helpers (`identity` `multiply` `perspective` `orthographic` `lookAt` `translate` `rotate` `scale`). See Low-Level Toolkits (see “Low-Level Toolkits: OpenGL, DirectX and Vulkan”), and the pages for OpenGL (see “`<OpenGL>`: Drawing with OpenGL Yourself”), DirectX (see “`<DirectX>`: Drawing with Direct3D 11 Yourself”) and Vulkan (see “`<Vulkan>`: Drawing with Vulkan Yourself”).

#### Images, fonts and sounds

Put the files in your game's folder (next to `main.frg`). Loading works anywhere, even before the window opens.

```cpp
Texture player = assets.loadTexture("player.png");    // .png .jpg .bmp .tga .gif .qoi
Font big = assets.loadFont("title.ttf");              // .ttf .otf
Sound jump = audio.loadSound("jump.wav");             // short sounds: .wav .ogg .mp3 .flac
Music song = audio.loadMusic("song.ogg");             // long music, streamed while it plays

player.draw(x, y);                                     // also: draw(x, y, tint)
player.drawScaled(x, y, 4);                            // 4 times as big
player.drawRotated(vec2(x, y), 45, 1);                 // centered at (x, y), turned 45 degrees
player.drawPart(rect(0, 0, 16, 16), rect(x, y, 64, 64));   // one frame of a sprite sheet, stretched
Animation walk = Animation(sheet, 16, 16, 4, 8);       // a sprite sheet's frames: frame width, height, how many, per second
walk.update(dt);                                       // in update
walk.drawScaled(x, y, 4);                              // in draw; also draw(x, y), flip(true), play(), pause(), reset()
big.drawText("Hello", 20, 20, 48, color.WHITE);
jump.play();
song.play();                                           // loops until song.stop()
```

| | Methods |
|---|---|
| `Texture` | `width()` `height()` `draw` `drawScaled` `drawRotated` `drawPart` |
| `Animation` | `Animation(sheet, frameWidth, frameHeight, frames, fps, row = 0)`, `update(dt)` `draw(x, y)` `drawScaled(x, y, scale)` `flip(bool)` `play()` `pause()` `reset()` `isPlaying()` `isDone()` `frame()` `setFrame(i)` `frameCount()`, fields `fps` `looping` |
| `Font` | `drawText(text, x, y, size, color)` `measure(text, size)` |
| `Sound` | `play()` `stop()` `isPlaying()` `setVolume(v)` `setPitch(p)` |
| `Music` | `play()` `stop()` `pause()` `resume()` `isPlaying()` `setVolume(v)` `setLooping(b)` `length()` `position()` |

#### 3D

```cpp
#include <graphics3d>

Camera cam = Camera(vec3(8, 6, 8), vec3(0, 0, 0));     // where it is, what it looks at
Model ship = graphics3d.loadModel("ship.glb");         // .obj .gltf .glb .iqm .vox .m3d

void draw(){
    cam.orbit(1);                                      // circle around the target, 1 degree
    graphics3d.setCamera(cam);
    graphics3d.drawGrid(20, 1);
    graphics3d.drawCube(vec3(0, 0.5, 0), vec3(1, 1, 1), color.RED);
    ship.draw(vec3(2, 0, 0));
    vec2 p = graphics3d.toScreen(vec3(0, 2, 0));       // where a 3D point is on screen
    graphics2d.drawText("hi", p.x, p.y, 20, color.WHITE);   // 2D draws on top of 3D
}
```

Shapes: `drawCube` `drawCubeOutline` `drawSphere(center, radius, color)` `drawSphereOutline` `drawCylinder(position, radius, height, color)` `drawPlane(center, vec2 size, color)` `drawGrid(slices, spacing)` `drawLine(from, to, color)`. `Camera` has `position`, `target`, `up`, `fov` and `orbit(degrees)`. `Model` has `draw(position, scale, tint)` and `drawRotated(position, axis, degrees, scale, tint)`. In 3D, **y goes up**.

#### Scenes

Each part of the game (menu, level, game over) is a class that inherits from `Scene` and fills in any of `enter()`, `update(float dt)`, `draw()` and `exit()`:

```cpp
class Menu : Scene {
    void draw() {
        if (ui.button(rect(380, 200, 200, 50), "Play")) {
            scenes.change(Level());
        }
    }
}

// in main:  scenes.change(Menu());
// every frame:  scenes.update(dt);  scenes.draw();
```

The switch happens at the start of the next `scenes.update`, so the old scene's `exit()` and the new one's `enter()` run then.

#### Entities and components (ECS)

```cpp
class Health : Component {
    int hp = 3;
    void update(float dt) {
        if (hp <= 0) {
            entity().destroy();
        }
    }
}

Entity slime = ECS.create("slime");
slime.add(Health());
Health h = slime.get("Health") as Health;    // find a component by its class name

// every frame:  ECS.update(dt);  ECS.draw();
```

A `Component` has `start()` (runs once, before its first update), `update(dt)`, `draw()` and `entity()`. An `Entity` has `name`, `active`, `components`, `add(c)`, `get("Type")`, `has("Type")`, `remove("Type")` and `destroy()`. The world has `ECS.create(name)` `add(entity)` `update(dt)` `draw()` `find(name)` `all()` `withComponent("Type")` `clear()` `count()`.

#### Timer

```cpp
Timer cooldown = Timer(0.5);           // half a second
if (cooldown.isDone()) {
    shoot();
    cooldown.restart();
}
```

`Timer` also has `left()` and `elapsed()`.

#### 2D camera

For a level bigger than the window. Draw the world through the camera, then reset it for the text:

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));   // target (a world point), offset (where it shows)
camera.target = camera.target.lerp(hero.center(), 5 * dt);   // in update: glide toward the player
graphics2d.setCamera(camera);                                 // in draw: the world...
level.draw();
graphics2d.resetCamera();                                     // ...then the score, which stays put
```

`Camera2D` also has `zoom` (1 = normal) and `rotation` (degrees). `graphics2d.screenToWorld(input.mousePosition())` gives the mouse's position in the world; `worldToScreen` goes the other way. See graphics2d (see “`<graphics2d>`: 2D Drawing” › “Camera: scrolling and zooming”).

#### Physics

A `Body` is a box: `Body(x, y, width, height)`. It has `position`, `size`, `velocity`, `isStatic`, `solid`, `oneWay`, `gravityScale`, plus `onGround`, `onCeiling` and `onWall`, which say what it touched during the last step. It also has `bounds()`, `center()` and `overlaps(other)`.

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

Your own classes can inherit from `Body`: `class Player : Body { ... }`. Moving bodies push each other apart, so crates stack. Set `solid = false` on things that should pass through everything, like coins and trigger zones, and check those with `a.overlaps(b)`. Set `oneWay = true` on a static platform to make it one-way: you can jump up through it and land on top.

`physics.drawDebug()` draws every body's outline (static blue, moving green, not solid purple, and a yellow bar under a body that's `onGround`), to see what physics sees. It draws with `<graphics2d>`, so that file needs `#include <graphics2d>` too.

### Making a game

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

See `examples/platformer` for a complete game. The other examples are `shapes` (2D drawing), `sprites` (images and sound), `menu` (scenes and UI) and `world3d` (3D).

An `icon.png` in the game's folder becomes the icon of the `.exe` and of the window.

When the game is finished, `frg build --release` makes an `.exe` without the console window. Without a console, `print` output isn't shown, and runtime errors pop up in a box instead. `--no-console` turns the console off without making a release build, and `--console` keeps it on even in one. (Details (see “Getting Started” › “The console window: on for testing, off for players”).)

### Errors

Forge explains what went wrong, where it happened, and usually how to fix it:

```
error: 'hp' needs an int, but this is a float
  --> main.frg:4:14
   |
 4 |     int hp = 7.5;
   |              ^^^
  hint: use int(...) to turn it into a whole number (the decimals get cut off)
```

**Warnings** (yellow, `warning:`) point at code that works but probably isn't what you meant. They never stop the build, and appear once there are no errors:

| Warning | What it means | Fix |
|---|---|---|
| `this changes 'e', which is only a copy of the item in the list` | `for (Enemy e in enemies) { e.hp = 0; }` with a **struct**: the loop gets a copy, so the list doesn't change | loop with a number: `for (int i = 0; i < enemies.length; i++) { enemies[i].hp = 0; }` |
| `this divides two whole numbers, so the decimals are cut off` | `float ratio = hits / shots;` with two ints: `7 / 2` is `3`, and only then does it become `3.0` | `float(hits) / shots` gives `3.5` |
| `the variable 'x' is never used` | you made a variable and never used it | remove it, or use it |
| `this code can never run, because it comes after 'return'` | lines after `return`, `break` or `continue` in the same `{ }` | remove them, or move them above |
| `comparing decimal numbers with '==' often gives the wrong answer` | `0.1 + 0.2 == 0.3` is false, because decimals are a tiny bit off | check whether they're close: `math.abs(a - b) < 0.001` |

Problems that can only be found while the game runs stop it with the file and line:

```
Runtime error in main.frg:10: position 5 is outside the list (it has 3 items, at positions 0 to 2)
```

If a file has several syntax mistakes, Forge reports all of them in one go.

### Coming next

- **Networking:** a `<network>` library for multiplayer games
- **Language:** generic types you write yourself (`class Box<T>`), functions as values
- **Editor:** autocomplete for your own code (a language server)
