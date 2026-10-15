# 5. Making Decisions and Repeating

Programs normally run top to bottom. **Control flow** changes that: running code only sometimes (`if`, `switch`) or many times (`while`, `for`).

## if / else

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

## while

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

## do ... while

Like `while`, but the condition is checked **after** each round, so the code always runs at least once:

```cpp
string answer = "";
do {
    answer = input("Type yes or no: ");
} while (answer != "yes" and answer != "no");
```

That's handy when you need to do something before you can check it, like asking a question and then checking the answer. Notice the `;` after the `)`: a `do ... while` ends with one.

`break` and `continue` work the same as in `while` (`continue` jumps to the check). Variables made inside the `{ }` are gone by the time the condition is checked, so make them before the `do` if the condition needs them, like `answer` above.

## Counting: for (int i in 0..10)

The easiest way to count is a **range**: a start, two dots, and an end.

```cpp
for (int i in 1..5) {
    print(i);          // 1, 2, 3, 4, 5
}
```

**Both ends are included**: `0..10` counts 0, 1, 2 ... 10, which is eleven numbers. The start and end can be any whole numbers, including variables and sums: `for (int i in 1..level * 2)`.

- It always counts **up**, one at a time. If the start is bigger than the end (`5..4`), the loop doesn't run at all. To count down or in steps, use the `for` loop below.
- Lists start at position 0, so the last position is `length - 1`. To go through every position: `for (int i in 0..names.length - 1)`. If you write `0..names.length`, Forge warns you, because that goes one past the end.
- `break` and `continue` work as usual. Changing `i` inside the loop doesn't change how it counts.

## for

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

## for ... in: going through a list

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

## break and continue

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

## switch

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

## return

`return` leaves the current function immediately. See [Functions](06-functions.md). Inside a loop, it also leaves the loop.

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

## What's next

[Functions](06-functions.md): naming a piece of code so you can use it again.
