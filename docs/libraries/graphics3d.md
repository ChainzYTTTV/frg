# `<graphics3d>`: 3D Drawing

```cpp
#include <graphics3d>
```

Draws 3D shapes and models through a camera. See [examples/world3d](../../examples/world3d/main.frg) for a working scene, and [examples/voxel](../../examples/voxel/main.frg) for a block world built from meshes.

## 3D coordinates

- Positions are [`vec3`](core-types.md#vec3): `(x, y, z)`.
- **y points up**, unlike the screen in 2D. The ground is usually y = 0.
- Units are whatever you decide (1 unit = 1 meter is common). Shapes are sized in the same units.
- The directions are "right-handed": looking along **+z**, **+x is to your left** (and looking along -z, +x is to your right). So the direction to the right of a camera is `forward.cross(vec3.UP)`, and its left is the opposite. Don't assume "+x is right": it depends on which way the camera faces.

<!-- check: output -->

```cpp
#include <stdio>

int main(){
    vec3 forward = vec3(0, 0, 1);              // looking along +z
    vec3 right = forward.cross(vec3.UP);
    print("right of +z is {right.x}, {right.y}, {right.z}");
    forward = vec3(1, 0, 0);                   // turned to look along +x
    right = forward.cross(vec3.UP);
    print("right of +x is {right.x}, {right.y}, {right.z}");
    return 0;
}
```

```text
right of +z is -1.0, 0.0, 0.0
right of +x is 0.0, 0.0, 1.0
```

## The camera

What you see depends on the camera: where it is, and what it's looking at.

```cpp
Camera cam = Camera(vec3(8, 6, 8), vec3(0, 0, 0));   // standing at (8, 6, 8), looking at the middle
graphics3d.setCamera(cam);
```

`Camera` is a struct:

| Field | Type | Meaning |
|---|---|---|
| `position` | `vec3` | where the camera is |
| `target` | `vec3` | the point it looks at |
| `up` | `vec3` | which way is up for the camera, `(0, 1, 0)` to start with |
| `fov` | `float` | field of view in degrees (how wide it sees), 45 to start with |

| Making one | |
|---|---|
| `Camera(vec3 position, vec3 target)` | |
| `Camera()` | at `(0, 5, 10)`, looking at `(0, 0, 0)` |

| Method | What it does |
|---|---|
| `orbit(float degrees)` | moves the camera around its target (around the up axis), keeping its distance and height |

| Function | What it does |
|---|---|
| `graphics3d.setCamera(Camera camera)` | uses this camera for the 3D drawing that comes after |
| `graphics3d.camera()` | gives back the camera being used |

Because `Camera` is a struct, changing your variable doesn't change the camera being used. Call `setCamera` again after changing it, usually every frame:

```cpp
void update(float dt){
    cam.orbit(30 * dt);           // 30 degrees per second
    graphics3d.setCamera(cam);
}
```

## Shapes

| Function | Draws |
|---|---|
| `graphics3d.drawCube(vec3 position, vec3 size, color c)` | a solid box **centered** on position |
| `graphics3d.drawCubeOutline(vec3 position, vec3 size, color c)` | the edges of a box |
| `graphics3d.drawSphere(vec3 center, float radius, color c)` | a solid ball |
| `graphics3d.drawSphereOutline(vec3 center, float radius, color c)` | a wireframe ball |
| `graphics3d.drawCylinder(vec3 position, float radius, float height, color c)` | a cylinder standing on `position`, going up by `height` |
| `graphics3d.drawPlane(vec3 center, vec2 size, color c)` | a flat rectangle on the ground (x-z plane) |
| `graphics3d.drawGrid(int slices, float spacing)` | grid lines on the ground around the middle, handy for seeing where things are |
| `graphics3d.drawLine(vec3 from, vec3 to, color c)` | a line |

There's no lighting yet: shapes are drawn in flat colors. Outlines (`drawCubeOutline`) help show their shape.

## Models

Load 3D models made in programs like Blender:

```cpp
Model ship = graphics3d.loadModel("ship.glb");
```

| Function | What it does |
|---|---|
| `graphics3d.loadModel(string path)` | loads a model file from the game folder. Supported: `.obj`, `.gltf`, `.glb`, `.iqm`, `.vox`, `.m3d`. |

| `Model` method | What it does |
|---|---|
| `draw(vec3 position, float scale = 1, color tint = color.WHITE)` | draws the model there |
| `drawRotated(vec3 position, vec3 axis, float degrees, float scale = 1, color tint = color.WHITE)` | draws it turned `degrees` around `axis` (for example `vec3(0, 1, 0)` spins around the up axis) |

Like [textures](assets.md), the file is found when it's loaded (a missing file is reported straight away), but it's only sent to the graphics card when it's first drawn, so loading as a global variable works. Models that include their own textures and colors (`.glb`, `.gltf`) are drawn with them; `tint` multiplies them.

## Meshes: shapes you build yourself

A `Mesh` is a set of triangles your game makes, sent to the graphics card once. Drawing it is one quick call however many triangles it has, so a whole chunk of a block world (thousands of faces) costs about the same to draw as one cube. Make the triangles once (or again when something changes), not every frame.

| Function | Gives back | What it does |
|---|---|---|
| `graphics3d.makeMesh(list<float> positions, list<float> uvs, list<float> shades, Texture texture)` | `Mesh` | makes a mesh out of triangles, painted with a [texture](assets.md) |

- `positions`: `x, y, z` for each corner, 3 corners for each triangle (so 9 numbers per triangle). The front of a triangle is the side where its corners go **counter-clockwise**; the back isn't drawn.
- `uvs`: `u, v` for each corner: where in the texture it is, from `0` to `1` (`v = 0` is the top of the image). A [sprite sheet](assets.md) works: give each face the corners of its own tile.
- `shades`: one brightness per corner, from `0` (black) to `1` (the texture's own colors), or `[]` for 1 everywhere. Darker sides make blocks look solid without lighting.
- Pixels of the texture that are mostly see-through (alpha under half) are cut out and not drawn at all, so leaves, glass and fences can have holes, in any order.

| `Mesh` method | What it does |
|---|---|
| `draw(vec3 position, color tint = color.WHITE)` | draws it moved by `position` (`vec3.ZERO` to draw it where its corners say) |
| `vertexCount()` | how many corners it has (3 per triangle) |

```cpp
Texture tiles = assets.loadTexture("tiles.png");
Mesh? floor = null;   // made once the window is open

int main(){
    window.open(960, 540, "A floor");
    // one square, as two triangles, seen from above
    list<float> positions = [0, 0, 0,  0, 0, 4,  4, 0, 4,    0, 0, 0,  4, 0, 4,  4, 0, 0];
    list<float> uvs =       [0, 0,     0, 1,     1, 1,       0, 0,     1, 1,     1, 0];
    floor = graphics3d.makeMesh(positions, uvs, [], tiles);
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    if (floor != null) {
        floor.draw(vec3.ZERO);
    }
}
```

Meshes are made on the graphics card, so the window has to be open first. To change one, make a new mesh and put it in the old one's place; the old one is freed when nothing uses it.

## Turning, moving and scaling drawings

`drawCube` always draws boxes lined up with the x, y and z directions. To draw something turned (a character facing the way it walks, a door swinging open, a gun tilting while it reloads), wrap the drawing in `pushTransform` and `popTransform`:

| Function | What it does |
|---|---|
| `graphics3d.pushTransform(vec3 position, vec3 axis = vec3.UP, float degrees = 0, float scale = 1)` | everything drawn after it is turned `degrees` around `axis`, made `scale` times bigger, then moved by `position` |
| `graphics3d.popTransform()` | ends the latest `pushTransform`: drawing goes back to how it was before it |

- Inside a transform, positions are relative to it: `drawCube(vec3.ZERO, ...)` draws at the transform's `position`.
- Transforms can go inside each other. The inner one is relative to the outer one, like a hand that moves with the arm it's on.
- Every `pushTransform` needs a `popTransform` before the frame ends, before 2D drawing, and before `setCamera`. Otherwise the game stops with an error that says so. A `popTransform` without a `pushTransform` before it is an error too.
- Models and meshes drawn inside a transform are turned with it.

<!-- check: builds -->

```cpp
#include <window>
#include <graphics3d>

float turn = 0;

int main(){
    window.open(960, 540, "Turning");
    window.run();
    return 0;
}

void update(float dt){
    turn += 90 * dt;   // a quarter turn per second
}

void draw(){
    graphics3d.setCamera(Camera(vec3(0, 3, -6), vec3(0, 0, 0)));
    graphics3d.drawGrid(10, 1);
    graphics3d.pushTransform(vec3(0, 0.5, 0), vec3.UP, turn);                       // a body that turns
    graphics3d.drawCube(vec3.ZERO, vec3(0.6, 1, 0.4), color.RED);
    graphics3d.pushTransform(vec3(0.2, 0.1, 0.4), vec3(1, 0, 0), 0);               // its gun, held in front
    graphics3d.drawCube(vec3(0, 0, 0.3), vec3(0.08, 0.1, 0.6), color.DARKGRAY);
    graphics3d.popTransform();
    graphics3d.popTransform();
}
```

## Drawing on top: clearDepth

3D shapes hide the ones behind them. `graphics3d.clearDepth()` makes everything drawn **after** it show in front of everything drawn **before** it (the picture so far stays). That's how a first-person game draws the player's gun: the world first, then `clearDepth()`, then the gun, so the gun never sinks into a wall the player stands against.

| Function | What it does |
|---|---|
| `graphics3d.clearDepth()` | 3D drawn after this is in front of all the 3D drawn before it |

The gun is easiest to draw with its own camera that never moves, so it's always in the same place on the screen:

<!-- check: builds -->

```cpp
#include <window>
#include <graphics3d>

Camera player = Camera(vec3(0, 1.6, -4), vec3(0, 1.6, 0));
Camera gunView = Camera(vec3(0, 0, 0), vec3(0, 0, 1));   // looking along +z, so +x is on the left

int main(){
    window.open(960, 540, "First person");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    graphics3d.setCamera(player);
    graphics3d.drawGrid(20, 1);
    graphics3d.drawCube(vec3(0, 1, 0), vec3(4, 2, 0.2), color.GRAY);   // a wall

    graphics3d.clearDepth();
    graphics3d.setCamera(gunView);
    graphics3d.drawCube(vec3(-0.25, -0.2, 0.6), vec3(0.08, 0.1, 0.5), color.DARKGRAY);   // the gun, low on the right
}
```

## 3D points on the 2D screen

| Function | Gives back | What it does |
|---|---|---|
| `graphics3d.toScreen(vec3 point)` | `vec2` | where a 3D point appears in the window, using the current camera |

Use it to draw 2D labels, health bars or markers over 3D things:

```cpp
vec2 p = graphics3d.toScreen(enemyPosition + vec3(0, 2, 0));
graphics2d.drawText("Enemy", p.x - 30, p.y, 20, color.WHITE);
```

## Mixing 2D and 3D

Every 3D drawing call switches into 3D mode by itself, and every [2D call](graphics2d.md) switches back. You can mix them freely in one frame; 2D drawn after 3D appears on top. The usual order is: 3D world first, then the 2D HUD.

## Example

```cpp
#include <window>
#include <graphics3d>
#include <math>

Camera cam = Camera(vec3(6, 5, 6), vec3(0, 0, 0));
float t = 0;

int main(){
    window.open(960, 540, "Bouncing ball");
    window.run();
    return 0;
}

void update(float dt){
    t += dt;
    cam.orbit(20 * dt);
    graphics3d.setCamera(cam);
}

void draw(){
    graphics3d.drawGrid(10, 1);
    graphics3d.drawCube(vec3(0, 1, 0), vec3(2, 2, 2), color.SKYBLUE);
    graphics3d.drawCubeOutline(vec3(0, 1, 0), vec3(2, 2, 2), color.WHITE);
    graphics3d.drawSphere(vec3(0, 3 + math.abs(math.sin(t * 3)), 0), 0.5, color.ORANGE);
}
```
