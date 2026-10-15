# 10. Enums, Optionals and Pointers

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

### More about optionals

- `Type?` variables made without a value start as `null`.
- `print(null)` and printing a null optional show `null`.
- A plain value converts to an optional automatically: `int? x = 5;`.
- `obj as Type` gives an optional (see [is and as](09-structs-and-classes.md#is-and-as-what-kind-of-object-is-it)).
- Fields of class type are often optional: `Enemy? target;` (otherwise they need a starting value).

## Pointers

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

### Throwing a pointer away: remove()

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

### When to use pointers

Rarely. Class objects are already shared (a function that gets a `Player` can change it), so pointers are only needed to let a function change a caller's **number, bool, string, struct or list** variable. Often it's clearer to return the new value instead:

```cpp
int addOne(int n) {
    return n + 1;
}
score = addOne(score);
```

### Danger: don't keep pointers to things that go away

A pointer to a function's local variable stops being valid when that function ends, and the same goes for a pointer to a list item once the list grows or shrinks. Forge can't always catch this. Using such a pointer gives garbage values or a crash. Keep pointers short-lived: pass them into a function, and don't store them. When you do keep one and you're done with it, `p.remove()` it, so using it by mistake gives a clear error.

## What's next

[Tasks](11-tasks.md): doing things over time without freezing the game.
