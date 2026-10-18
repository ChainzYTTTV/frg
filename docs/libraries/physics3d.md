# `<physics3d>`: Boxes and Balls in 3D

```cpp
#include <physics3d>
```

Simple 3D physics: boxes and balls that fall, move, bump into things, climb steps and bounce, and rays to find what's in a direction (for shooting and seeing). It's the 3D version of [`<physics>`](physics.md), and works in the same world as [`<graphics3d>`](graphics3d.md): **y is up**, sizes are in meters (or whatever unit you pick), and looking along +z, +x is on the left.

This library doesn't draw anything: draw bodies yourself, for example `graphics3d.drawCube(box.position, box.size, color.ORANGE)`. It doesn't need a window either. The [example](../../examples/physics3d/main.frg) is a small playground: a player that walks, jumps and climbs stairs, crates, thrown balls and a ray.

## Body3D

A body is a box that never turns, or a ball. **`position` is its middle** (the same as `drawCube`).

| Making one | |
|---|---|
| `Body3D(vec3 position, vec3 size)` | a moving, solid box, `size` wide, tall and deep, with its middle at `position` |

| Field | Type | Meaning |
|---|---|---|
| `position` | `vec3` | the middle |
| `size` | `vec3` | width (x), height (y) and depth (z) |
| `velocity` | `vec3` | meters per second |
| `isStatic` | `bool` | `true` for things that never move: the ground, walls, platforms, steps. Static bodies ignore gravity and velocity. (`false` to start with.) |
| `solid` | `bool` | `false` for things that shouldn't push or be pushed, like pickups and trigger zones. (`true` to start with.) Rays go through them. |
| `sphere` | `bool` | `true` makes it a **ball**, as wide as the smallest side of its box. (`false` to start with.) |
| `gravityScale` | `float` | how strongly gravity pulls it: 1 is normal, 0 floats, 2 is twice as heavy |
| `bounce` | `float` | how much speed it keeps when it hits something: `0` stops (to start with), `0.5` bounces back at half the speed, `1` as fast as it came |
| `stepHeight` | `float` | for moving boxes: steps up to this high are climbed by walking into them (stairs, curbs). `0` to start with: nothing is climbed |
| `layer` | `int` | its collision layer, `0` to `31` (`0` to start with). See [Layers](#layers-what-hits-what). |
| `onGround` | `bool` | **set by Forge:** true if it landed on something during the last step |
| `onCeiling` | `bool` | **set by Forge:** true if it bumped its head |
| `onWall` | `bool` | **set by Forge:** true if it hit something sideways |

| Method | Gives back | |
|---|---|---|
| `minCorner()` | `vec3` | the box's lowest x, y and z corner |
| `maxCorner()` | `vec3` | its highest corner |
| `overlaps(Body3D other)` | `bool` | true if the two touch right now (following balls' round shape) |

## The world

| Function | What it does |
|---|---|
| `physics3d.add(Body3D body)` | adds a body to the world. Adding it twice does nothing. |
| `physics3d.remove(Body3D body)` | takes it out |
| `physics3d.step(float dt)` | moves everything forward by `dt` seconds. Call it **once per frame** with the frame's `dt`. It behaves the same at any frame rate. |
| `physics3d.setGravity(vec3 gravity)` | gravity, in meters per second per second. Starts at `vec3(0, -9.81, 0)`, real gravity. Games often feel better with more: `vec3(0, -20, 0)` |
| `physics3d.gravity()` | the current gravity |
| `physics3d.collide(int layerA, int layerB, bool on)` | `false`: bodies on these two layers pass through each other; `true`: they collide again |
| `physics3d.collides(int layerA, int layerB)` | whether they collide |
| `physics3d.clear()` | removes every body |
| `physics3d.count()` | how many bodies are in the world |
| `physics3d.raycast(vec3 from, vec3 direction, float maxDistance = 1000)` | what a line from `from` toward `direction` hits first (a `RayHit`, below) |
| `physics3d.drawDebug()` | draws every body's outline (needs `#include <graphics3d>`) |

## Walking, jumping and stairs

```cpp
Body3D player = Body3D(vec3(0, 1, 0), vec3(0.6, 1.8, 0.6));

int main(){
    window.open(960, 540, "Walk");
    Body3D ground = Body3D(vec3(0, -0.5, 0), vec3(50, 1, 50));   // its top is at y 0
    ground.isStatic = true;
    physics3d.add(ground);
    player.stepHeight = 0.35;            // climbs stairs with steps up to 0.35 high
    physics3d.add(player);
    window.run();
    return 0;
}

void update(float dt){
    vec3 move = vec3.ZERO;
    if (input.keyDown(Key.W)) { move = move + vec3(0, 0, 5); }
    if (input.keyDown(Key.S)) { move = move - vec3(0, 0, 5); }
    player.velocity = vec3(move.x, player.velocity.y, move.z);   // keep the falling speed
    if (input.keyPressed(Key.SPACE) and player.onGround) {
        player.velocity = vec3(player.velocity.x, 6, player.velocity.z);
    }
    physics3d.step(dt);
}
```

Set the sideways speed yourself every frame and keep `velocity.y`, which gravity looks after. Jump only when `onGround`.

## Balls and bounce

```cpp
Body3D ball = Body3D(vec3(0, 5, 0), vec3(0.5, 0.5, 0.5));   // a ball 0.5 across
ball.sphere = true;
ball.bounce = 0.7;
ball.velocity = vec3(4, 3, 0);                              // thrown
physics3d.add(ball);
```

Draw it with `graphics3d.drawSphere(ball.position, ball.size.x / 2, color.RED)`.

## Rays: what's over there?

`physics3d.raycast(from, direction, maxDistance)` follows a line and gives back a `RayHit`:

| Field | Type | |
|---|---|---|
| `hit` | `bool` | whether it hit anything within `maxDistance` |
| `point` | `vec3` | where |
| `normal` | `vec3` | which way the surface it hit faces (straight up for the top of a floor) |
| `distance` | `float` | how far along the line |
| `body` | `Body3D?` | what it hit |

A ray that starts inside a body doesn't hit that body, so a ray from a character's eyes doesn't hit the character. Bodies with `solid = false` aren't hit.

```cpp
RayHit hit = physics3d.raycast(eyes, lookDirection, 50);
if (hit.hit) {
    graphics3d.drawSphere(hit.point, 0.1, color.YELLOW);    // where a shot would land
}
```

## Layers: what hits what

Every body has a `layer` (`0` to `31`), and normally everything collides with everything. `physics3d.collide(a, b, false)` makes layers `a` and `b` pass through each other (they can be the same layer: `collide(2, 2, false)` and balls on layer 2 go through each other). `overlaps()` and `raycast` ignore layers.

## Seeing the bodies

`physics3d.drawDebug()` outlines every body with the current 3D camera: static ones blue, moving ones green, ones with `solid = false` purple, and a yellow square under a moving body that's `onGround`. Call it in `draw`, after `graphics3d.setCamera`. The file that calls it needs `#include <graphics3d>`.

## What happens in a step

Moving bodies fall with gravity, then move by their velocity in small pieces (so a fast one can't pass through a thin wall). A box moves across, then along, then up or down, and is pushed back out of each static body it runs into; walking into a step no higher than its `stepHeight`, it climbs onto it instead. A ball is pushed out along the shortest way, so it rolls off edges and slides along walls. Then moving bodies are pushed apart from each other: the one on top is pushed up and stands on the other. `physics3d.step` cuts each frame into equal parts of at most 1/240 s, so a jump goes as high at 30 frames per second as at 144.

## Limits

- **Nothing turns.** Boxes stay upright and balls slide rather than spin, so there's no tipping over, tumbling or rolling stacks, and no ragdolls.
- No ramps or slopes yet: use steps (`stepHeight`) for stairs.
- Shapes are boxes and balls only: no capsules or meshes (a level made of many boxes works well).
- A moving ball and a moving box push each other apart as two boxes.
- Everything is checked against everything, which is fine for the few hundred bodies a game has, not for huge worlds.
