# 9. Structs and Classes

So far every value has been a single number, some text, or a list. Games need *things*: a player with a name, health and a position, or an enemy with a type and speed. **Structs** and **classes** let you make your own types that bundle values together, along with the functions that work on them.

## A first struct

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

## Fields

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

## Methods

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

### this

Inside a method, **`this`** means "the value this method was called on". You usually don't need it, since `x` already means this point's `x`. It's needed when a parameter has the same name as a field:

```cpp
void setX(float x) {
    this.x = x;          // the field = the parameter
}
```

## Making values: constructors

### Without a constructor

A type with no constructor can be made in two ways:

```cpp
Point a = Point();          // every field at its starting value
Point b = Point(3, 4);      // every field, in the order they're written
Point c = {3, 4};           // the same as Point(3, 4)
```

It's either none of the fields, or all of them; giving only some is an error that lists the fields in order.

### Constructors

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

### Several constructors

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

## Structs vs classes: copying vs sharing

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

### Things to know about structs

- In `for (Point p in points)`, `p` is a copy. Change items with `points[i].x = ...`. (Forge gives a warning if you change `p` inside the loop.)
- `getPoint().x = 5;` is an error, because `getPoint()` gives back a temporary copy that would be thrown away. Store it in a variable first.
- A struct can't contain itself, directly or as `Node?`, because it would be infinitely big. A struct can contain a `list` of itself. If you need "a thing that points to another thing of the same kind", use a class.

### When class objects go away

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

## Inheritance

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
- You can inherit from some built-in classes too, like [`Body`](../libraries/physics.md), [`Scene`](../libraries/scenes.md) and [`Component`](../libraries/ecs.md).

### super

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

### Which constructor is used

- If a class has its own constructors, one of those is used.
- If not, it uses its parent's constructors (or its grandparent's, and so on).
- If none of them have any, it's made with `Type()` or `Type(every field in order)`, where inherited fields come first.

## is and as: what kind of object is it?

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

## private

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

## How objects look when printed

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

## static: shared by the whole type

A normal field belongs to **each object**: every enemy has its own `hp`. A **`static`** field belongs to the **type itself**: there's only one, shared by all of them. A static method is a function that belongs to the type: it's called on the type's name, not on an object.

```cpp
class Enemy {
    static int count = 0;        // one counter for all enemies
    string name;

    Enemy(string name) {
        this.name = name;
        count++;                 // inside the class, no "Enemy." is needed
    }

    static Enemy boss() {        // makes a special enemy
        Enemy b = Enemy("Boss");
        return b;
    }
}

Enemy a = Enemy("Grub");
Enemy b = Enemy.boss();         // a static method: called on the type
print(Enemy.count);             // 2: a static field, reached through the type
Enemy.count = 0;                // and changed the same way
```

Things to know:

- Outside the type, write the type's name first: `Enemy.count`, `Enemy.boss()`. Inside it, the plain name works.
- A static method has no object, so it can't use the normal fields, the normal methods or `this`. Forge says so, and suggests making it a normal method or giving it the object as a parameter.
- A normal method can't be called on the type (`Enemy.attack()`); it needs an object.
- Static fields can have a starting value. Like global variables, static fields of a class type (or a `Function<...>`) need one, or must be optional.
- Static fields aren't part of an object: they're not in its constructor, and not shown when it's printed.
- Structs can have static fields and methods too: `Point.origin`, `Point.distance(a, b)`.
- `private static` works: only the type itself can use it.
- A class that inherits gets its parent's static fields and methods (they're still the same, shared ones). Static methods can't be replaced.

## Interfaces and abstract classes

Sometimes you want to say "every enemy can be attacked" without writing *how* yet. Forge has two ways.

An **interface** is a list of methods, with no code. A class that has the interface promises to write them all:

```cpp
interface Attacker {
    int power();
    void attack(string target);
}

class Trap : Attacker {
    int power() {
        return 7;
    }

    void attack(string target) {
        print("the trap snaps at {target}");
    }
}
```

An **abstract class** is a normal class that can also have `abstract` methods: methods with no code, which every class that inherits from it must write. It can still have fields, constructors and normal methods:

```cpp
abstract class Enemy {
    string name;
    int hp = 10;

    Enemy(string name) {
        this.name = name;
    }

    abstract int reward();          // no code: each kind of enemy says how much it's worth

    void hurt(int amount) {
        hp -= amount;
        print("{name} is worth {reward()} gold");
    }
}

class Goblin : Enemy, Attacker {    // one class first, then any number of interfaces
    Goblin(string name) {
        super(name);
    }

    int reward() {
        return 5;
    }

    int power() {
        return 3;
    }

    void attack(string target) {
        print("{name} pokes {target}");
    }
}
```

Interfaces and abstract classes are used like any class type: in variables, lists, parameters, `is` and `as`:

```cpp
list<Attacker> attackers = [Goblin("Grub"), Trap()];
for (Attacker a in attackers) {
    a.attack("you");                // each one does its own attack
}
if (attackers[0] is Goblin) {
    Goblin g = attackers[0] as Goblin;
}
```

Things to know:

- After the `:` comes at most one class, then any number of interfaces, separated by commas: `class Goblin : Enemy, Attacker, Flyer`. A class can also have only interfaces: `class Trap : Attacker`.
- An interface has only methods ending in `;`: no code, no fields, no constructors, nothing `static` or `private`. An interface can't inherit from anything.
- Only an `abstract class` can have `abstract` methods. A class that inherits from it, or has an interface, must write every one of those methods with exactly the same parameter and return types. If one is missing, Forge tells you which, and what to add.
- An interface or an abstract class can't be made directly: `Attacker()` and `Enemy()` are errors. Make one of the classes that have it.
- `super.reward()` doesn't work when `reward` is abstract in the parent, because there's no code to call.
- Structs can't have interfaces or be abstract, because they're copied rather than shared.
- Printing something through an interface prints the real object, like `Goblin(name: Grub, hp: 10)`.

## Generic types: one type that holds any type

`list<int>` and `list<string>` are the same kind of thing holding different types. Your own structs and classes can work like that too. Put a name for "some type" inside `< >` after the type's name, and use it like a type inside:

```cpp
class Box<T> {
    T item;

    Box(T item) {
        this.item = item;
    }

    T open() {
        return item;
    }
}

Box<int> coins = Box(5);              // here T is int
Box<string> note = Box("hello");      // here T is string
Box<Player> hero = Box(Player());     // here T is Player
print(coins.open() + 1);              // 6
```

A type can have several of them: `struct Pair<A, B> { A first; B second; }`, used as `Pair<string, int> score = Pair("Ana", 30);`.

Things to know:

- `T` can be any type at all: numbers, text, lists, your own structs and classes, even another `Box<int>`.
- Forge works out what `T` is from where the value goes, so write the full type where you make one: `Box<int> coins = Box(5);`. On its own, like `print(Box(5))`, Forge asks you to say what `T` is.
- Inside the type, Forge doesn't know what `T` will be, so a `T` can only be stored, passed on, given back, compared with `==` and `!=`, and printed. You can't use its fields or methods (`item.name`) or do maths with it.
- `T?`, `list<T>` and `map<string, T>` all work inside.
- Because `T` could be a class, which can't be empty, a field of type `T` needs a value from every constructor (`this.item = item;`, not inside an `if` or a loop), or has to be optional (`T? item;`). A `T` local variable needs a starting value.
- Without a constructor, a generic type is made with all its fields: `Pair("Ana", 30)`.
- `Box<int>` and `Box<float>` are different types: one can't be stored in the other.
- For now, a generic type can't inherit, have interfaces, or have `static` fields and methods, and no class can inherit from one. Functions can't be generic (only structs and classes).
- The name inside `< >` can't already be a type's name. `T` is the usual choice, or a short word like `Item`.

## Types from other files

Types follow the [file-name prefix rule](07-files-and-folders.md):

```cpp
entities.Player p = entities.Player("Ana");
list<entities.Player> party = [];
class Boss : enemies.goblin.Goblin { ... }
```

## Rules summary

- Types are written at the top level of a file.
- Field and method names must all be different within a type (and from its parents' fields).
- A field of class type needs a starting value or must be optional.
- Methods can call each other by name, and can use fields by name or through `this`.
- Methods can't have the same name as each other. Only constructors can share a name.
- A class inherits from at most one class, plus any number of interfaces.
- A generic type (`class Box<T>`) is written with every type it holds: `Box<int>`.
- Class objects are cleaned up automatically once nothing uses them anymore. There's no need (and no way) to delete them.

## What's next

[Enums, optionals and pointers](10-enums-optionals-pointers.md).
