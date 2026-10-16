# 8. Lists and Maps

Games deal with *many* things: enemies, coins, high scores, inventory items. **Lists** hold many values in order, and **maps** look values up by a key.

## Lists

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

### Positions

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

### List methods

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
| `sort()` | — | puts the items in order, smallest first (lists of numbers, text, or a type with a [`less` method](04-operators.md#operators-for-your-own-types)) |
| `reverse()` | — | flips the order |
| `copy()` | a list | a separate copy of the list |
| `removeWhere(f)` | — | removes every item that the [function value](06-functions.md#functions-as-values) `f` says `true` for: `enemies.removeWhere((Enemy e) { return e.hp <= 0; });` |
| `filter(f)` | a list | a new list with only the items `f` says `true` for |
| `any(f)` | `bool` | `true` if `f` says `true` for at least one item |
| `sortBy(f)` | — | sorts the items by the number `f` gives for each, smallest first: `enemies.sortBy((Enemy e) { return e.hp; });` |

`first()`, `last()`, `pop()`, `removeAt()` and `insert()` stop the game with a runtime error if the list is empty or the position is wrong.

"Equal" means `==`: numbers and text by value, structs field by field (or with their own [`equals`](04-operators.md#operators-for-your-own-types)), and classes only when it's the very same object. Lists of function values, and of structs holding one, can't use these three, because function values can't be compared.

### Going through a list

```cpp
for (string name in names) {
    print(name);
}

for (int i = 0; i < names.length; i++) {
    print("{i}: {names[i]}");
}
```

Use the counter version when you need the position, or want to change struct items (`points[i].x += 1`). See [for ... in](05-control-flow.md#for--in-going-through-a-list) for removing items while looping.

### Copying

Assigning a list to another variable, or passing it to a function, makes a **copy**:

```cpp
list<int> a = [1, 2];
list<int> b = a;
b.add(3);
print(a);     // [1, 2] - a didn't change
```

If the items are **class** objects, the copy holds the same objects (the list is copied, the objects aren't).

### Comparing and printing

`a == b` is true if both lists have equal items in the same order. `print([1, 2, 3])` shows `[1, 2, 3]`.

## Maps

A `map<K, V>` stores values (`V`) under keys (`K`). Each key appears at most once.

```cpp
map<string, int> ages = {"Ana": 12, "Ben": 14};
map<int, string> levelNames = {1: "Forest", 2: "Cave"};
map<string, list<int>> scoresByPlayer = {};
```

- Keys can be whole numbers, text, bools or enums.
- `{ key: value, ... }` makes a map. `{}` is an empty map (the type must be written).

### Reading and writing

```cpp
ages["Cy"] = 9;            // adds a new key (or replaces the value if it exists)
print(ages["Ana"]);        // 12
ages["Ana"] += 1;          // 13
```

- **Reading** a key that isn't in the map stops the game: "the key isn't in the map (use .has(key) to check first, or .get(key, backup))".
- **Writing** to a key that isn't there creates it. That includes `+=`: `counts["new"] += 1` creates `"new"` at 0 and then adds 1, which makes counting easy.

### Map methods

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

### Going through a map

```cpp
for (string name in ages.keys()) {
    print("{name} is {ages[name]}");
}
```

Maps always keep their keys in sorted order (numbers smallest first, text alphabetically).

### Example: counting

```cpp
map<string, int> counts = {};
list<string> drops = ["coin", "gem", "coin", "coin"];
for (string d in drops) {
    counts[d] += 1;
}
print(counts);      // {coin: 3, gem: 1}
```

### Copying, comparing and printing

Like lists, maps are **copied** when assigned or passed to functions. `==` compares every key and value. `print` shows `{Ana: 12, Ben: 14}`.

## Lists and maps inside other things

Lists and maps can hold anything, including other lists and maps, structs and classes, and they can be fields of structs and classes:

```cpp
class Inventory {
    map<string, int> items = {};
    void addItem(string name){
        items[name] += 1;
    }
}

list<list<int>> grid = [[0, 0, 0], [0, 1, 0]];
grid[1][1] = 5;
```

## What's next

[Structs and classes](09-structs-and-classes.md): making your own types.
