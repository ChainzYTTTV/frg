# Built-in Types: vec2, vec3, vec4, color, rect

These five types are **always available**, with no `#include`. They're [structs](../language/09-structs-and-classes.md), so they're copied when assigned, and their fields can be read and changed directly.

## vec2

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

## vec3

A 3D vector, for [3D graphics](graphics3d.md). In 3D, **y points up**.

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

## vec4

| Fields | `float x`, `y`, `z`, `w` |
|---|---|
| Making one | `vec4(x, y, z, w)`, `vec4()` |
| Methods | `length()`, `normalized()`, `dot(vec4 other)` |
| Constants | `vec4.ZERO`, `vec4.ONE` |

## Vector math

| Expression | Result |
|---|---|
| `a + b`, `a - b` | add or subtract the same kind of vector |
| `v * 2`, `2 * v`, `v / 2` | scale by a number |
| `-v` | flip the direction |
| `a == b`, `a != b` | compare every part |
| `v += w;` `v -= w;` | change in place |
| `v *= 2;` `v /= 2;` | scale in place |

Multiplying two vectors together isn't allowed; use `dot` or `cross`. Word forms work too: `a plus b`, `v times 2`.

## color

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

## rect

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

## Printing

`print(vec2(1, 2))` shows `vec2(1.0, 2.0)`. The others show the same way: `color(255, 0, 0, 255)`, `rect(0.0, 0.0, 10.0, 10.0)`.
