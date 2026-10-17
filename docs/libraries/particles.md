# `<particles>`: Dust, Sparks and Explosions

```cpp
#include <particles>
```

**Particles** are lots of tiny dots that fly out, fade and disappear: dust when a player lands, sparks when something is hit, smoke, fire, explosions. A few lines make a game feel much more alive.

An `Emitter` makes them. Give it a position, then each frame `update(dt)` it and `draw()` it:

```cpp
Emitter sparks = Emitter(vec2(400, 300));

void update(float dt){
    if (input.mousePressed(MouseButton.LEFT)) {
        sparks.position = input.mousePosition();
        sparks.burst(30);              // 30 particles at once
    }
    sparks.update(dt);
}

void draw(){
    sparks.draw();
}
```

## Emitter

| Making one | |
|---|---|
| `Emitter(vec2 position)` | an emitter at that point. It makes nothing until you `burst` or give it a `rate` |

| Method | What it does |
|---|---|
| `burst(int count)` | makes `count` particles right now (an explosion, a puff of dust) |
| `update(float dt)` | moves every particle, makes new ones if `rate` is more than 0, and removes the ones that are too old. Call it once per frame |
| `draw()` | draws every particle. It's 2D drawing, so it goes through the [2D camera](graphics2d.md#camera-scrolling-and-zooming) like everything else |
| `count()` | how many particles are alive now |
| `clear()` | removes every particle |

These fields can be changed at any time; the new values are used for the particles made after that:

| Field | Type | Start value | Meaning |
|---|---|---|---|
| `position` | `vec2` | (given) | where new particles start |
| `rate` | `float` | `0` | particles per second, for a steady stream (smoke, fire). `0`: only `burst` makes them |
| `lifetime` | `float` | `1` | how many seconds each particle lives |
| `speed` | `float` | `100` | how fast they fly out, in pixels per second |
| `direction` | `float` | `-90` | which way they fly, in degrees: `0` is right, `-90` up, `90` down, `180` left |
| `spread` | `float` | `360` | how wide the spray is, in degrees around `direction`: `360` is every way, `30` a narrow jet |
| `startColor` | `color` | white | the color when a particle is new... |
| `endColor` | `color` | see-through white | ...and when it disappears. Particles blend between the two, so an `endColor` with alpha 0 fades them out |
| `startSize` | `float` | `8` | the size (across, in pixels) when new... |
| `endSize` | `float` | `2` | ...and at the end |
| `gravity` | `vec2` | `vec2(0, 0)` | pulls the particles, in pixels per second per second: `vec2(0, 400)` makes them fall |

To look natural, each particle's speed and lifetime are a little different (speed from 70% to 130%, lifetime from 80% to 120%).

An emitter keeps at most 5000 particles alive; past that, new ones are skipped.

## Recipes

```cpp
// Dust when landing: a small puff that goes up and out, then settles.
dust.lifetime = 0.4;
dust.speed = 90;
dust.direction = -90;
dust.spread = 160;
dust.gravity = vec2(0, 300);
dust.startColor = color(200, 200, 210);
dust.endColor = color(200, 200, 210, 0);
dust.burst(14);

// Fire: a steady stream going up, from yellow to see-through red.
fire.rate = 80;
fire.direction = -90;
fire.spread = 25;
fire.speed = 60;
fire.startColor = color.YELLOW;
fire.endColor = color.RED.fade(0);
fire.startSize = 14;
fire.endSize = 4;

// An explosion: everything at once, in every direction.
boom.speed = 300;
boom.lifetime = 0.7;
boom.startColor = color.ORANGE;
boom.endColor = color.DARKGRAY.fade(0);
boom.startSize = 12;
boom.burst(80);
```

One emitter can be moved around and reused: set its `position`, then `burst`. Particles already flying stay where they are.

[examples/platformer](../../examples/platformer/) uses an emitter for dust when the player lands (in `player.frg`) and one for sparkles when a coin is collected (in `level.frg`).

## Limits

- Particles are colored circles (going from `startColor` to `endColor`): they can't be pictures.
- 2D only, and particles don't collide with anything.
