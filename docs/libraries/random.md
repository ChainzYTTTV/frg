# `<random>`: Random Numbers

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

`random.range` picks the whole-number version when both values are whole numbers, and the decimal version otherwise:

```cpp
int roll = random.range(1, 6);              // 1, 2, 3, 4, 5 or 6
float speed = random.range(50.0, 120.0);    // somewhere from 50 to 120
float alsoDecimal = random.range(0, 2.5);   // one decimal makes it the decimal version
```

If `min` is bigger than `max`, they're swapped.

## Seeds

Every run of the game gets different random numbers. To get the **same** sequence every time (to reproduce a bug, or to make a "daily level"), set a seed first:

```cpp
random.seed(12345);
```

The same seed gives the same sequence of numbers.

## Examples

```cpp
// pick a random item from a list
list<string> loot = ["sword", "shield", "potion"];
string drop = loot[random.range(0, loot.length - 1)];

// 10% chance of a critical hit
int damage = random.chance(0.1) ? 20 : 10;

// spawn somewhere along the top of the screen
vec2 spawn = vec2(random.range(0.0, 960.0), -20);
```
