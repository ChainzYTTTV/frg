# The Forge Language (v0.5)

This guide covers everything Forge can do **right now**. Planned features are listed at the end.

**Contents:** [A Forge game](#a-forge-game) · [Variables](#variables-and-types) · [Text](#text) · [Operators](#operators) · [If and loops](#if-loops-and-switch) · [Functions](#functions) · [Files and folders](#multiple-files) · [Lists and maps](#lists-and-maps) · [Structs and classes](#structs-and-classes) · [Enums](#enums) · [Optional values and null](#optional-values-and-null) · [Pointers](#pointers) · [Tasks](#tasks-waiting-without-freezing-the-game) · [Built-in types](#built-in-types-vec2-color-rect) · [Libraries](#libraries) · [Making a game](#making-a-game) · [Errors](#errors)

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

- A type can have **several constructors**, as long as each takes different types of values: `Size(float side)` and `Size(float w, float h)`. Forge picks the one that matches what you give it.
- A type with no constructor is made with `Type()` or `Type(every field, in order)`.
- A class without its own constructors uses the ones it inherits. In a constructor, `super(...)` runs the parent class's constructor.
- `print` shows a struct or class as `Point(x: 3.0, y: 4.0)`. To show it differently, add a method `string toString()`.

### Checking what kind of object something is: `is` and `as`

```cpp
for (Entity e in enemies) {
    if (e is Goblin) {               // true if e is a Goblin (or something that inherits from Goblin)
        Goblin g = e as Goblin;      // as gives a Goblin?, which is null if e isn't a Goblin
        g.cackle();
    }
}
```

### private

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

## Tasks: waiting without freezing the game

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
| `<assets>` | images and fonts: `assets.loadTexture("player.png")`, `assets.loadFont("font.ttf")`, `assets.exists(path)` (see below) |
| `<audio>` | `audio.loadSound("jump.wav")`, `audio.loadMusic("song.ogg")`, `audio.setVolume(0.5)` (see below) |
| `<graphics3d>` | 3D drawing (see below) |
| `<time>` | `time.now()` (seconds since the game started), `time.sleep(seconds)`, `time.clock()` ("14:05:09"), `time.date()` ("2026-10-14"), `time.year()` `month()` `day()` `hour()` `minute()` `second()`, and the `Timer` class |
| `<file>` | `file.read(path)` `write(path, text)` `append(path, text)` `exists(path)` `remove(path)` `readLines(path)` `writeLines(path, lines)` `files(folder)`. Paths are relative to the game's folder. |
| `<scenes>` | the `Scene` class and `scenes.change(scene)` `update(dt)` `draw()` `current()` (see below) |
| `<ui>` | `ui.button(rect, "text")` (true when clicked), `ui.checkbox(rect, "text", checked)` (gives the new value), `ui.slider(rect, value, min, max)` (gives the new value), `ui.label(text, x, y, size, color)`, `ui.panel(rect)`, `ui.setTheme(background, accent, text)` |
| `<ECS>` | the `Entity` and `Component` classes (see below) |
| `<tasks>` | `tasks.update(dt)` `runAll()` `count()` `stopAll()` (see [Tasks](#tasks-waiting-without-freezing-the-game)) |

Screen coordinates start at the **top-left** corner, and **y goes down**.

### Images, fonts and sounds

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
big.drawText("Hello", 20, 20, 48, color.WHITE);
jump.play();
song.play();                                           // loops until song.stop()
```

| | Methods |
|---|---|
| `Texture` | `width()` `height()` `draw` `drawScaled` `drawRotated` `drawPart` |
| `Font` | `drawText(text, x, y, size, color)` `measure(text, size)` |
| `Sound` | `play()` `stop()` `isPlaying()` `setVolume(v)` `setPitch(p)` |
| `Music` | `play()` `stop()` `pause()` `resume()` `isPlaying()` `setVolume(v)` `setLooping(b)` `length()` `position()` |

### 3D

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

### Scenes

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

### Entities and components (ECS)

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

### Timer

```cpp
Timer cooldown = Timer(0.5);           // half a second
if (cooldown.isDone()) {
    shoot();
    cooldown.restart();
}
```

`Timer` also has `left()` and `elapsed()`.

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

Your own classes can inherit from `Body`: `class Player : Body { ... }`. Moving bodies push each other apart, so crates stack. Set `solid = false` on things that should pass through everything, like coins and trigger zones, and check those with `a.overlaps(b)`.

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

See `examples/platformer` for a complete game. The other examples are `shapes` (2D drawing), `sprites` (images and sound), `menu` (scenes and UI) and `world3d` (3D).

When the game is finished, `frg build --release` makes an `.exe` without the console window. `print` output isn't shown in a release build.

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

If a file has several syntax mistakes, Forge reports all of them in one go.

## Coming next

- **Graphics backends:** `#include <OpenGL>`, `<Vulkan>`, `<DirectX>` (everything currently draws through raylib, which uses OpenGL)
- **Language:** generic types you write yourself (`class Box<T>`), functions as values
- **Editor:** autocomplete for your own code (a language server)
