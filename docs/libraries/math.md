# `<math>`: Math Functions

```cpp
#include <math>
```

Everything is reached as `math.name`.

## Constants

| Constant | Value | Meaning |
|---|---|---|
| `math.PI` | 3.14159… | half a turn, in radians |
| `math.TAU` | 6.28318… | a full turn, in radians (2π) |
| `math.E` | 2.71828… | the base of natural logarithms |

They take on whichever decimal type they're used as (`float` or `double`).

## Functions

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
- Angles are in **radians**. To convert, `radians = degrees * math.PI / 180`. Note that [`Texture.drawRotated`](assets.md) and [`graphics3d`](graphics3d.md) rotations use **degrees**.
- `sqrt` of a negative number, or `log` of zero or less, gives `nan` (not a number).
- Squaring: `x * x` or `math.pow(x, 2)`. There's no `^` power operator.

## Example: moving toward a target

```cpp
vec2 toTarget = target - position;
float dist = toTarget.length();
if (dist > 1) {
    position += toTarget.normalized() * math.min(speed * dt, dist);
}
```

## Example: bobbing up and down

```cpp
float bob = math.sin(window.time() * 3) * 10;     // between -10 and 10, three times a second-ish
graphics2d.drawCircle(x, y + bob, 8, color.GOLD);
```
