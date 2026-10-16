# 10. Enums and Optionals

## Enums

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

vec2 step(Direction d){
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

## Optionals and null

Sometimes a value might be **missing**: an enemy might not have a target yet, or a search might find nothing. Add **`?`** to any type to make it **optional**, meaning it can also hold **`null`** (nothing):

```cpp
Enemy? target = null;
int? bestScore = null;
string? nickname = "Ace";
```

### Only optionals can be null

```cpp
int score = null;        // error: null can only go in an optional type - add ? to the type, like int?
```

This is on purpose: in Forge, a plain `Enemy` **always** holds an enemy. Only types you mark with `?` can be empty, so you always know where to check.

### Checking for null

```cpp
if (target != null) {
    target.takeHit(5);
}

if (bestScore == null) {
    print("No best score yet");
}
```

`if (target)` is an error; compare with `null`.

### Using an optional

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

### Backup values: ?? / otherwise

```cpp
int shown = bestScore ?? 0;                  // bestScore, or 0 if it's null
string name = nickname otherwise "Player";   // the word version
```

If the backup is optional too, the result is optional: `a ?? b ?? 0`.

### Functions that might not find anything

```cpp
int? findIndex(list<string> items, string wanted){
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

### More about optionals

- `Type?` variables made without a value start as `null`.
- `print(null)` and printing a null optional show `null`.
- A plain value converts to an optional automatically: `int? x = 5;`.
- `obj as Type` gives an optional (see [is and as](09-structs-and-classes.md#is-and-as-what-kind-of-object-is-it)).
- Fields of class type are often optional: `Enemy? target;` (otherwise they need a starting value).

## Changing a caller's number: return it

Forge has no pointers. When a function should change a number, a bool, text, a struct or a list for whoever called it, **return the new value**:

```cpp
int addOne(int n){
    return n + 1;
}

score = addOne(score);
```

Class objects are already shared, so a function that gets a `Player` can change it directly. To change several values at once, keep them together in a class:

```cpp
class Stats {
    int hp = 100;
    int coins = 0;
}

void pickUpCoin(Stats s){
    s.coins += 1;           // changes the caller's Stats
}
```

(Forge 0.9.0 and earlier had a `Pointer<T>` type. It was removed, because a pointer to a list item or to a field of an object could quietly change the wrong memory once the list grew or the object was freed.)

## What's next

[Tasks](11-tasks.md): doing things over time without freezing the game.
