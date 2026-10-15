# `<Vulkan>`: Drawing with Vulkan Yourself

```cpp
#include <Vulkan>
```

A low-level toolkit for drawing with **Vulkan**, the most modern of the three graphics APIs. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read [Low-Level Toolkits](toolkits.md) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

## What Forge does, and what you do

Using Vulkan by hand takes hundreds of lines before the first triangle appears, and almost all of them are the same in every program. So `Vulkan.openWindow` does that part for you. It sets up:

- the Vulkan instance and the window's surface;
- the graphics card (a separate card is picked over one built into the processor);
- the **swapchain**: the window's images, which are drawn into in turn;
- a depth buffer;
- the render pass, framebuffers and command buffers;
- the syncing that lets the processor prepare one frame while the graphics card draws the last.

What's left is the part that's different in every game, and that's yours:

- **shaders**, written in GLSL;
- **pipelines**: a pair of shaders, plus how corners are laid out and drawn;
- **buffers** of corners and corner numbers;
- each frame's drawing commands, recorded between `beginFrame` and `endFrame`.

Games built with `<Vulkan>` need a Vulkan driver to run. Graphics drivers from the last several years include one. Building a game needs nothing extra: Forge comes with everything it needs.

## Shaders are files

Unlike `<OpenGL>` and `<DirectX>`, Vulkan doesn't take shader code as text. It takes **SPIR-V**, a compiled form. So Vulkan shaders go in their own files in the game's folder, named by what they are:

| File ending | Shader |
|---|---|
| `.vert` | vertex shader (runs for every corner) |
| `.frag` | fragment shader (runs for every pixel) |
| `.comp`, `.geom`, `.tesc`, `.tese` | compute, geometry and tessellation shaders (compiled, but this toolkit only draws with vertex and fragment shaders) |

When a game uses `<Vulkan>`, **`frg build` compiles every shader file into a `.spv` file next to it**: `shape.vert` becomes `shape.vert.spv`. The game then loads the `.spv`:

```
   Compiling mygame (1 file)
      Shader shape.frag
      Shader shape.vert
    Finished build/mygame.exe in 3.10s
```

Only shaders that changed since the last build are compiled again. If a shader has a mistake, the build stops and shows where:

```
error: the shader shape.frag has errors

    shape.frag:4: 'shade' : undeclared identifier
Build failed: a shader has errors.
```

Shaders are GLSL starting with `#version 450`. A few things are different from OpenGL's GLSL:

- every input and output needs a `layout(location = N)`;
- values from the program come in a `push_constant` block (see [Push constants](#push-constants));
- there's no `gl_FragColor`: the fragment shader writes to its own `out vec4`.

The `.spv` files are made from the shader files, so they don't need to be kept or shared: the next build makes them again.

## A whole program

One colored triangle. Three files go in the game's folder.

`triangle.vert`:

```glsl
#version 450
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 vColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
```

`triangle.frag`:

```glsl
#version 450
layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vColor, 1.0);
}
```

`main.frg`:

```cpp
#include <Vulkan>

int main(){
    Vulkan.openWindow(800, 600, "Triangle");

    int vertexShader = Vulkan.loadShader("triangle.vert.spv");
    int fragmentShader = Vulkan.loadShader("triangle.frag.spv");
    // Each corner: 2 floats of position (location 0), then 3 of color (location 1).
    int pipeline = Vulkan.createPipeline(vertexShader, fragmentShader, [2, 3]);

    int corners = Vulkan.createVertexBuffer([
        -0.5,  0.5,   1, 0, 0,
         0.5,  0.5,   0, 1, 0,
         0.0, -0.5,   0, 0, 1
    ]);

    while (!Vulkan.shouldClose()) {
        if (Vulkan.beginFrame(0.1, 0.1, 0.15, 1)) {
            Vulkan.bindPipeline(pipeline);
            Vulkan.bindVertexBuffer(corners);
            Vulkan.draw(3);
            Vulkan.endFrame();
        }
    }
    Vulkan.closeWindow();
    return 0;
}
```

Notice the corners: without a matrix, Vulkan's screen has **y pointing down**, so `-0.5` is near the top. Once you use the [matrix helpers](#matrices), y points up again, like in OpenGL.

The bigger example in [`examples/vulkan`](../../examples/vulkan/main.frg) draws a spinning 3D cube using a matrix.

## The window

| Function | What it does |
|---|---|
| `Vulkan.deviceName()` | the graphics card's name, like `"Radeon RX 570 Series"` |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see [What every toolkit has](toolkits.md#what-every-toolkit-has). `setVSync(false)` takes effect from the next frame.

## Pipelines

In Vulkan, nearly every drawing setting is fixed when a **pipeline** is made, instead of being changed while drawing. Different settings (say, lines instead of triangles, or no depth test for a 2D overlay) mean different pipelines.

| Function | What it does |
|---|---|
| `loadShader(string spvPath)` | loads a compiled shader (`.spv`) from the game's folder and gives back its handle |
| `createPipeline(int vertexShader, int fragmentShader, list<int> attributeSizes, int pushConstantFloats = 0)` | makes a pipeline and gives back its handle |

`attributeSizes` describes one corner in the vertex buffer: how many floats each of the vertex shader's inputs takes (1-4), in order of their `location`. `[3, 3]` means `location = 0` is 3 floats and `location = 1` is the next 3. A pipeline whose shader makes its own corners (with no inputs) uses `[]`.

`pushConstantFloats` is how many floats [`pushConstants`](#push-constants) can send to this pipeline's shaders.

### Settings for the next pipelines

These change the pipelines made **after** them. They don't change pipelines that already exist:

| Function | What it does | To start with |
|---|---|---|
| `setTopology(int topology)` | what the corners make: `TRIANGLES` (every 3), `TRIANGLE_STRIP`, `TRIANGLE_FAN`, `LINES` (every 2), `LINE_STRIP`, `POINTS` | `TRIANGLES` |
| `setDepthTest(bool on)` | whether near things hide far things | on |
| `setCulling(int mode)` | skip triangles facing away (`CULL_BACK`) or toward you (`CULL_FRONT`), or neither (`CULL_NONE`) | `CULL_NONE` |
| `setBlending(bool on)` | whether see-through colors (alpha below 1) mix with what's behind them | off |

A triangle counts as facing you when its corners go **counter-clockwise** on screen, the same as in OpenGL and DirectX.

```cpp
Vulkan.setCulling(Vulkan.CULL_BACK);
int solid = Vulkan.createPipeline(vs, fs, [3, 3], 16);

Vulkan.setTopology(Vulkan.LINES);
Vulkan.setCulling(Vulkan.CULL_NONE);
int wireframe = Vulkan.createPipeline(vs, fs, [3, 3], 16);
```

## Buffers

| Function | What it does |
|---|---|
| `createVertexBuffer(list<float> data)` | a buffer of corners, laid out as the pipeline's `attributeSizes` says |
| `createIndexBuffer(list<int> indices)` | a buffer of corner numbers, for `drawIndexed` |

Buffers can't be changed after they're made. Send values that change every frame (matrices, colors, time) with push constants instead.

## Drawing a frame

Each frame is recorded between `beginFrame` and `endFrame`:

| Function | What it does |
|---|---|
| `beginFrame(float r, float g, float b, float a)` | starts the frame, cleared to this color (0 to 1 each), and resets the depth buffer. Gives back `false` when there's nothing to draw into this time (say, the window is minimized): then skip the drawing, and don't call `endFrame`. |
| `bindPipeline(int pipeline)` | draws with this pipeline from now on |
| `bindVertexBuffer(int buffer)` | draws corners from this buffer |
| `bindIndexBuffer(int buffer)` | uses these corner numbers for `drawIndexed` |
| `pushConstants(int pipeline, list<float> data)` | sends floats to the shaders, for the draws after it |
| `draw(int vertexCount, int firstVertex = 0)` | draws `vertexCount` corners from the vertex buffer |
| `drawIndexed(int indexCount, int firstIndex = 0)` | draws using the index buffer's corner numbers |
| `endFrame()` | finishes the frame and sends it to the graphics card. It appears in the window at the next `shouldClose()`. |

So the game loop is always shaped like this:

```cpp
while (!Vulkan.shouldClose()) {
    // update the game...
    if (Vulkan.beginFrame(0, 0, 0, 1)) {
        // bind, push and draw...
        Vulkan.endFrame();
    }
}
```

The drawing functions only work between `beginFrame` and `endFrame`: anywhere else they stop the game with an error saying so. Things bound during a frame stay bound until the end of that frame, and each frame starts with nothing bound.

### Push constants

**Push constants** are the quick way to send a few values to the shaders for each draw: a matrix, a color, a time. In the shader they're a `push_constant` block:

```glsl
layout(push_constant) uniform Push {
    mat4 transform;     // floats 0-15
    vec4 tint;          // floats 16-19
} push;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);
}
```

In the game, make the pipeline with room for that many floats, then send them before each draw. Both shaders can read them:

```cpp
int pipeline = Vulkan.createPipeline(vs, fs, [3, 3], 20);
// ... in the frame:
list<float> data = Vulkan.multiply(camera, model);
data.addAll([1, 1, 1, 1]);
Vulkan.pushConstants(pipeline, data);
Vulkan.drawIndexed(36);
```

Every graphics card allows at least **32 floats** (two matrices) of push constants. Asking for more than the card allows stops `createPipeline` with an error that says the card's limit.

## Matrices

The helpers are listed in [Matrices](toolkits.md#matrices). Vulkan's `perspective` and `orthographic` put depth from 0 to 1. They also flip y, so that up is up on screen, like in OpenGL. The usual way is to multiply `projection × view × model` in the game and push the result as one `mat4`:

```cpp
list<float> camera = Vulkan.multiply(Vulkan.perspective(45, float(Vulkan.width()) / Vulkan.height(), 0.1, 100),
                                     Vulkan.lookAt(vec3(0, 2, 6), vec3(0, 0, 0), vec3.UP));
Vulkan.pushConstants(pipeline, Vulkan.multiply(camera, Vulkan.rotate(angle, vec3.UP)));
```

## Constants

| Group | Names |
|---|---|
| topology | `POINTS`, `LINES`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP`, `TRIANGLE_FAN` |
| culling | `CULL_NONE`, `CULL_FRONT`, `CULL_BACK` |

## Not included

This first version of the toolkit covers drawing shapes with your own shaders. It doesn't have:

- **textures** (images) or uniform buffers: they need *descriptor sets*, which aren't here yet. Colors come from the corners and from push constants.
- buffers that change after they're made;
- compute shaders, or drawing into images other than the window;
- multisampling (smoothed edges).

These may come later.
