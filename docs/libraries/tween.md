# `<tween>`: Smooth Changes Over Time

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

## Tween

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

## Easing: the shape of the change

`Ease` decides how the value moves between the start and the end:

| `Ease.` | Movement | Good for |
|---|---|---|
| `LINEAR` | the same speed all the way | progress bars, steady movement |
| `IN` | starts slowly, ends fast | things falling, leaving the screen |
| `OUT` (the default) | starts fast, ends slowly | things arriving: menus, pop-ups |
| `IN_OUT` | slow, fast, slow | moving something from one place to another |
| `BOUNCE` | falls in and bounces at the end, like a dropped ball | titles, pickups landing |

`tween.ease(Ease ease, float t)` gives the eased amount for `t` from 0 to 1, if you want the shape without a `Tween`: `tween.ease(Ease.OUT, 0.5)` is `0.875`.

## Positions and colors

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

## Examples

### A title that drops in and bounces

```cpp
Tween titleDrop = Tween(-80, 70, 0.9, Ease.BOUNCE);

void update(float dt){
    titleDrop.update(dt);
}

void draw(){
    graphics2d.drawText("FORGE", 380, titleDrop.value(), 64, color.ORANGE);
}
```

[examples/menu](../../examples/menu/main.frg) does this.

### Something pulsing forever

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

### In a task

A [task](../language/11-tasks.md) can play a tween from start to end on its own:

```cpp
float menuX = -300;

task showMenu(){
    Tween t = Tween(-300, 40, 0.5);
    while (!t.isDone()) {
        t.update(window.deltaTime());
        menuX = t.value();
        wait();                      // the next frame
    }
}
```

## Why it works like this

A tween is a value you update and read, like a [`Timer`](time.md). A tween doesn't run code by itself when it ends: check `isDone()` instead (or keep a [function value](../language/06-functions.md#functions-as-values) and call it when `isDone()` becomes true). That also means it works the same way in every kind of game loop, and in tasks.
