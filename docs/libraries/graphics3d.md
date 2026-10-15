# `<graphics3d>`: 3D Drawing

```cpp
#include <graphics3d>
```

Draws 3D shapes and models through a camera. See [examples/world3d](../../examples/world3d/main.frg) for a working scene.

## 3D coordinates

- Positions are [`vec3`](core-types.md#vec3): `(x, y, z)`.
- **y points up**, unlike the screen in 2D. The ground is usually y = 0.
- Units are whatever you decide (1 unit = 1 meter is common). Shapes are sized in the same units.

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
