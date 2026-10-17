# `<DirectX>`: Drawing with Direct3D 11 Yourself

```cpp
#include <DirectX>
```

A low-level toolkit for drawing with **Direct3D 11**, the graphics API built into Windows. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read [Low-Level Toolkits](toolkits.md) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

The functions are close to Direct3D 11's own, with simpler arguments:

- objects are `int` handles;
- data is a `list<float>` or a `list<int>`, with sizes counted in floats;
- shaders are HLSL code in a `string`, usually [raw text](../language/03-text.md#raw-text) between `"""` marks.

If the computer has no suitable graphics card, Direct3D's software renderer (WARP) is used instead. It's slower, but everything still works.

## A whole program

One colored triangle:

```cpp
#include <DirectX>

string SHADER = """
struct VertexIn {
    float2 position : POSITION;
    float3 color : COLOR;
};

struct PixelIn {
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PixelIn vsMain(VertexIn v) {
    PixelIn o;
    o.position = float4(v.position, 0.0, 1.0);
    o.color = v.color;
    return o;
}

float4 psMain(PixelIn p) : SV_TARGET {
    return float4(p.color, 1.0);
}
""";

int main(){
    DirectX.openWindow(800, 600, "Triangle");
    int vs = DirectX.compileVertexShader(SHADER, "vsMain");
    int ps = DirectX.compilePixelShader(SHADER, "psMain");
    int layout = DirectX.createInputLayout(vs, ["POSITION", "COLOR"], [2, 3]);

    // Each corner: x y, then r g b.
    int corners = DirectX.createVertexBuffer([
        -0.5, -0.5,   1, 0, 0,
         0.5, -0.5,   0, 1, 0,
         0.0,  0.5,   0, 0, 1
    ]);

    DirectX.setShaders(vs, ps);
    DirectX.setInputLayout(layout);
    DirectX.setVertexBuffer(corners, 5);
    while (!DirectX.shouldClose()) {
        DirectX.clear(0.1, 0.1, 0.15, 1);
        DirectX.draw(3);
    }
    DirectX.closeWindow();
    return 0;
}
```

What happens, step by step:

1. `openWindow` opens the window and starts Direct3D.
2. The two shaders are compiled from the same HLSL text, each from its own function. The **vertex shader** runs for every corner, the **pixel shader** for every pixel.
3. The **input layout** tells Direct3D what each corner in the buffer holds: a 2-float `POSITION`, then a 3-float `COLOR`. The names match the `: POSITION` and `: COLOR` marks in the shader's `VertexIn`.
4. The corners go into a **vertex buffer** in the graphics card's memory.
5. Every frame: clear the screen and draw. `shouldClose()` at the top of the loop then shows the finished frame.

The settings made with `set...` stay until they're changed, so things that never change can be set once, before the loop.

The bigger example in [`examples/directx`](../../examples/directx/main.frg) draws a spinning 3D cube and a textured square.

## The window

| Function | What it does |
|---|---|
| `DirectX.adapterName()` | the graphics card's name, like `"Radeon RX 570 Series"` |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see [What every toolkit has](toolkits.md#what-every-toolkit-has).

## Drawing settings

| Function | What it does | To start with |
|---|---|---|
| `clear(float r, float g, float b, float a)` | fills the screen with a color (0 to 1 each) and resets the depth buffer. Call it at the start of each frame. | |
| `setDepthTest(bool on)` | whether near things hide far things | on |
| `setBlending(bool on)` | whether see-through colors (alpha below 1) mix with what's behind them | off |
| `setCulling(int mode)` | skip triangles facing away (`CULL_BACK`) or toward you (`CULL_FRONT`), or neither (`CULL_NONE`) | `CULL_NONE` |
| `setTopology(int topology)` | what the corners make: `TRIANGLES` (every 3), `TRIANGLE_STRIP`, `LINES` (every 2), `LINE_STRIP`, `POINTS` | `TRIANGLES` |

A triangle counts as facing you when its corners go **counter-clockwise** on screen, the same as in OpenGL.

## Shaders

| Function | What it does |
|---|---|
| `compileVertexShader(string hlsl, string entry = "main")` | compiles the function called `entry` as a vertex shader (shader model 5.0). Gives back its handle. |
| `compilePixelShader(string hlsl, string entry = "main")` | the same, as a pixel shader |
| `setShaders(int vertexShader, int pixelShader)` | draws with these two from now on |

If the HLSL has a mistake, the game stops with Direct3D's message, which says the line and what's wrong.

### Input layouts

| Function | What it does |
|---|---|
| `createInputLayout(int vertexShader, list<string> names, list<int> sizes)` | describes one corner: for each of the vertex shader's inputs, in order, its **semantic name** and how many floats it takes (1-4). Gives back its handle. |
| `setInputLayout(int layout)` | uses this layout from now on |

The names are the marks after the inputs in the shader: `POSITION`, `COLOR`, `TEXCOORD`, `NORMAL`... If a name is used twice, the second is number 1 (`TEXCOORD1` in the shader), the third number 2, and so on. If the names or sizes don't match the vertex shader, `createInputLayout` stops the game with an error.

## Buffers

| Function | What it does |
|---|---|
| `createVertexBuffer(list<float> data)` | a buffer of corners. It can't be changed later. |
| `createIndexBuffer(list<int> indices)` | a buffer of corner numbers, for `drawIndexed`. It can't be changed later. |
| `createConstantBuffer(int floatCount)` | a buffer for values sent to the shaders (matrices, colors, time), which can be changed every frame |
| `updateConstantBuffer(int buffer, list<float> data)` | replaces a constant buffer's values |
| `setVertexBuffer(int buffer, int strideFloats)` | draws corners from this buffer, each `strideFloats` floats long |
| `setIndexBuffer(int buffer)` | uses these corner numbers for `drawIndexed` |
| `setConstantBuffer(int slot, int buffer)` | gives this buffer to both shaders as `register(b<slot>)` |

### Constant buffers

A constant buffer matches a `cbuffer` in the shader. The floats go in the same order as the `cbuffer`'s variables:

```cpp
string SHADER = """
cbuffer Transform : register(b0) {
    float4x4 world;     // floats 0-15
    float4 tint;        // floats 16-19
};
...
""";

int transform = DirectX.createConstantBuffer(20);
DirectX.setConstantBuffer(0, transform);
// ... every frame:
list<float> data = DirectX.rotate(angle, vec3.UP);
data.addAll([1, 0.5, 0.5, 1]);
DirectX.updateConstantBuffer(transform, data);
```

Things to know:

- **Use `mul(matrix, vector)` in the shader**, like `mul(world, float4(position, 1.0))`. The [matrix helpers](toolkits.md#matrices) make matrices in the order that this expects.
- **Mind HLSL's packing.** HLSL packs a `cbuffer` into blocks of 4 floats, and one variable can't be split across two blocks. A `float3` followed by a `float2` leaves a gap. The simplest fix is to only use `float4` and `float4x4`, or to put single `float`s at the end.
- **Sizes are rounded up.** A constant buffer's size is rounded up to a multiple of 4 floats. Sending fewer floats than it holds is fine: the rest are 0.

## Drawing

| Function | What it does |
|---|---|
| `draw(int vertexCount, int startVertex = 0)` | draws `vertexCount` corners from the vertex buffer |
| `drawIndexed(int indexCount, int startIndex = 0)` | draws using the index buffer's corner numbers, so corners shared by several triangles are only stored once |

## Textures

| Function | What it does |
|---|---|
| `loadTexture(string path)` | loads a `.png` (or `.jpg`, `.bmp`...) from the game's folder and gives back its handle |
| `setTexture(int slot, int texture)` | gives the texture to the pixel shader as `register(t<slot>)` |
| `setTextureFilter(bool smooth)` | `true` (the start) blends pixels when the texture is stretched; `false` keeps them sharp, for pixel art |

Texture coordinate `(0, 0)` is the image's **top-left** corner. The texture repeats past its edges. In the shader, read it like this:

```
Texture2D tex : register(t0);
SamplerState samp : register(s0);
// ...
float4 c = tex.Sample(samp, uv);
```

## Render textures: drawing into a texture

A **render texture** is a texture you can draw into, then use like any other (a security-camera screen, a mirror, a minimap, or effects over the whole picture).

| Function | What it does |
|---|---|
| `createRenderTexture(int width, int height)` | a texture of this size that can be drawn into, with its own depth buffer; gives back its handle |
| `setRenderTarget(int texture)` | draws into this render texture from now on (and `clear` clears it); `0` is the window again. The viewport changes to its size |

```cpp
int picture = DirectX.createRenderTexture(256, 256);    // once

// every frame
DirectX.setRenderTarget(picture);
DirectX.clear(0, 0, 0, 1);
// ... draw the scene ...
DirectX.setRenderTarget(0);
DirectX.clear(0.1, 0.1, 0.15, 1);
DirectX.setTexture(0, picture);
// ... draw something with the texture on it ...
```

A texture can't be read while it's being drawn into, so `setRenderTarget` takes the textures away from the pixel shader: call `setTexture` again after it.

## Matrices

The helpers are listed in [Matrices](toolkits.md#matrices). DirectX's `perspective` and `orthographic` put depth from 0 to 1, as Direct3D expects.

## Constants

| Group | Names |
|---|---|
| topology | `POINTS`, `LINES`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP` |
| culling | `CULL_NONE`, `CULL_FRONT`, `CULL_BACK` |

## Not included

This toolkit covers what most games and tutorials need. It doesn't have:

- geometry, hull or compute shaders;
- instancing;
- vertex buffers that change after they're made.

These may come later.
