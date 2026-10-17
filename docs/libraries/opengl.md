# `<OpenGL>`: Drawing with OpenGL Yourself

```cpp
#include <OpenGL>
```

A low-level toolkit for drawing with **OpenGL 3.3** (core profile) directly. It is **separate from Forge's engine**: a game that uses it can't use `<window>`, `<graphics2d>` or the other engine libraries, and does all its own drawing. Read [Low-Level Toolkits](toolkits.md) first. It explains that rule, and lists the window, keyboard, mouse and matrix functions every toolkit shares.

The functions here are thin wrappers around the real OpenGL functions, with the `gl` taken off the front: `glClearColor(...)` is `OpenGL.clearColor(...)`, and `GL_TRIANGLES` is `OpenGL.TRIANGLES`. So OpenGL tutorials (like learnopengl.com) can be followed almost line by line. The differences are:

- object handles are `int`s;
- data is a `list<float>` (or a `list<int>`), with sizes counted in floats instead of bytes;
- shader code is a `string`, usually [raw text](../language/03-text.md#raw-text) between `"""` marks.

## A whole program

The smallest OpenGL game: one colored triangle.

```cpp
#include <OpenGL>

string VERTEX = """
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
out vec3 vColor;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
""";

string FRAGMENT = """
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
""";

int main(){
    OpenGL.openWindow(800, 600, "Triangle");
    int program = OpenGL.buildProgram(VERTEX, FRAGMENT);

    // Each corner: x y, then r g b.
    list<float> corners = [
        -0.5, -0.5,   1, 0, 0,
         0.5, -0.5,   0, 1, 0,
         0.0,  0.5,   0, 0, 1
    ];
    int vao = OpenGL.genVertexArray();
    OpenGL.bindVertexArray(vao);
    int vbo = OpenGL.genBuffer();
    OpenGL.bindBuffer(OpenGL.ARRAY_BUFFER, vbo);
    OpenGL.bufferData(OpenGL.ARRAY_BUFFER, corners, OpenGL.STATIC_DRAW);
    OpenGL.vertexAttribPointer(0, 2, 5, 0);    // position: 2 floats, every 5 floats, starting at 0
    OpenGL.enableVertexAttribArray(0);
    OpenGL.vertexAttribPointer(1, 3, 5, 2);    // color: 3 floats, starting at float 2
    OpenGL.enableVertexAttribArray(1);

    while (!OpenGL.shouldClose()) {
        OpenGL.clearColor(0.1, 0.1, 0.15, 1);
        OpenGL.clear(OpenGL.COLOR_BUFFER_BIT);
        OpenGL.useProgram(program);
        OpenGL.bindVertexArray(vao);
        OpenGL.drawArrays(OpenGL.TRIANGLES, 0, 3);
    }
    OpenGL.closeWindow();
    return 0;
}
```

What happens, step by step:

1. `openWindow` opens the window and starts OpenGL.
2. `buildProgram` compiles the two shaders and links them into a **program**: the code the graphics card runs for every corner (the vertex shader) and every pixel (the fragment shader).
3. The corners go into a **buffer** in the graphics card's memory. A **vertex array** remembers how that buffer is laid out: which floats are the position and which are the color.
4. Every frame: clear the screen, choose the program and the vertex array, and draw. `shouldClose()` at the top of the loop then shows the finished frame.

The bigger example in [`examples/opengl`](../../examples/opengl/main.frg) draws a spinning 3D cube and a textured square.

## The window

| Function | What it does |
|---|---|
| `OpenGL.version()` | the OpenGL version and graphics card, as text (like `"3.3.0 Core Profile ..."`) |

The rest (`openWindow`, `shouldClose`, `keyDown`, `deltaTime`...) is the same in every toolkit: see [What every toolkit has](toolkits.md#what-every-toolkit-has).

## Drawing settings

| Function | What it does | Real OpenGL |
|---|---|---|
| `clearColor(float r, float g, float b, float a)` | the color `clear` fills the screen with (0 to 1 each) | `glClearColor` |
| `clear(int mask)` | clears the screen. Pass `COLOR_BUFFER_BIT`, and in 3D also `DEPTH_BUFFER_BIT`: `OpenGL.COLOR_BUFFER_BIT bitor OpenGL.DEPTH_BUFFER_BIT` | `glClear` |
| `viewport(int x, int y, int width, int height)` | which part of the window to draw into (the whole window to start with) | `glViewport` |
| `enable(int what)`, `disable(int what)` | turns on/off `DEPTH_TEST` (near things hide far things), `BLEND` (see-through colors) or `CULL_FACE` (skip triangles facing away) | `glEnable`, `glDisable` |
| `blendFunc(int source, int destination)` | how see-through colors mix. The usual: `blendFunc(OpenGL.SRC_ALPHA, OpenGL.ONE_MINUS_SRC_ALPHA)` | `glBlendFunc` |
| `depthFunc(int function)` | when a pixel passes the depth test: `LESS` (the start), `LEQUAL`, `ALWAYS` | `glDepthFunc` |
| `cullFace(int face)` | which side `CULL_FACE` skips: `BACK` (the start), `FRONT`, `FRONT_AND_BACK` | `glCullFace` |
| `lineWidth(float width)` | how thick `LINES` are, in pixels (many cards only allow 1) | `glLineWidth` |
| `getError()` | the oldest error OpenGL noticed, or `NO_ERROR` (0). Useful when the screen stays black. | `glGetError` |

A triangle counts as facing you when its corners go **counter-clockwise** on screen.

## Shaders and programs

The easy way: `buildProgram` does all the steps at once, and stops the game with OpenGL's message if a shader has a mistake.

| Function | What it does |
|---|---|
| `buildProgram(string vertexSource, string fragmentSource)` | compiles both shaders and links them. Gives back the program's handle. |
| `useProgram(int program)` | draws with this program from now on |
| `deleteProgram(int program)` | frees it |

Shaders are written in GLSL and start with `#version 330 core`.

The step-by-step way, which works exactly like real OpenGL:

| Function | What it does |
|---|---|
| `createShader(int type)` | a new shader: `VERTEX_SHADER` or `FRAGMENT_SHADER` |
| `shaderSource(int shader, string source)` | gives it its code |
| `compileShader(int shader)` | compiles it |
| `shaderCompiled(int shader)` | `true` if it compiled |
| `shaderLog(int shader)` | the compiler's messages |
| `deleteShader(int shader)` | frees it (fine once it's linked into a program) |
| `createProgram()` | a new, empty program |
| `attachShader(int program, int shader)` | adds a compiled shader to the program |
| `linkProgram(int program)` | links the program's shaders together |
| `programLinked(int program)` | `true` if it linked |
| `programLog(int program)` | the linker's messages |

### Uniforms

A **uniform** is a value the program sends to the shaders, which stays the same for a whole draw (a matrix, a color, a time). First look up where it is, once, then set it whenever it changes. Uniforms belong to the program in use, so call `useProgram` first.

| Function | What it does |
|---|---|
| `getUniformLocation(int program, string name)` | where the uniform called `name` is, or `-1` if the shaders don't use it |
| `getAttribLocation(int program, string name)` | the location number of a vertex shader input |
| `uniform1f(int location, float x)` ... `uniform4f(int location, float x, float y, float z, float w)` | sets a `float`, `vec2`, `vec3` or `vec4` uniform |
| `uniform1i(int location, int x)` | sets an `int` or `bool` uniform, or which texture unit a `sampler2D` reads |
| `uniformMatrix4(int location, list<float> matrix)` | sets a `mat4` uniform from 16 numbers, like the [matrix helpers](toolkits.md#matrices) make |

```cpp
int projectionLoc = OpenGL.getUniformLocation(program, "projection");
// ... every frame:
OpenGL.useProgram(program);
OpenGL.uniformMatrix4(projectionLoc, OpenGL.perspective(45, float(OpenGL.width()) / OpenGL.height(), 0.1, 100));
```

## Buffers and vertex arrays

| Function | What it does |
|---|---|
| `genVertexArray()` | a new vertex array: it remembers the buffers and layout set up while it's bound |
| `bindVertexArray(int vertexArray)` | uses this vertex array (for setting up, and for drawing) |
| `deleteVertexArray(int vertexArray)` | frees it |
| `genBuffer()` | a new buffer |
| `bindBuffer(int target, int buffer)` | uses this buffer as `ARRAY_BUFFER` (corners) or `ELEMENT_ARRAY_BUFFER` (corner numbers) |
| `deleteBuffer(int buffer)` | frees it |
| `bufferData(int target, list<float> data, int usage)` | copies floats into the bound buffer. `usage` is a hint: `STATIC_DRAW` (set once), `DYNAMIC_DRAW` (changed often), `STREAM_DRAW` (changed every frame) |
| `bufferDataInts(int target, list<int> data, int usage)` | the same with whole numbers: for `ELEMENT_ARRAY_BUFFER` |
| `vertexAttribPointer(int index, int size, int strideFloats, int offsetFloats)` | says where shader input number `index` is in the bound `ARRAY_BUFFER`: `size` floats, one every `strideFloats` floats, starting `offsetFloats` in |
| `enableVertexAttribArray(int index)` | turns that input on |

`vertexAttribPointer` counts in **floats**, where real OpenGL counts in bytes. For corners laid out as `x y z r g b` (6 floats each), position is `(0, 3, 6, 0)` and color is `(1, 3, 6, 3)`.

## Drawing

| Function | What it does |
|---|---|
| `drawArrays(int mode, int first, int count)` | draws `count` corners from the bound vertex array, starting at corner `first` |
| `drawElements(int mode, int count)` | draws using the corner numbers in the vertex array's `ELEMENT_ARRAY_BUFFER`, so corners shared by several triangles are only stored once |

`mode` is what the corners make: `TRIANGLES` (every 3 corners), `TRIANGLE_STRIP`, `TRIANGLE_FAN`, `LINES` (every 2), `LINE_STRIP`, `LINE_LOOP` or `POINTS`.

## Textures

| Function | What it does |
|---|---|
| `loadTexture(string path)` | loads a `.png` (or `.jpg`, `.bmp`...) from the game's folder into a new texture and gives back its handle |
| `genTexture()` | a new, empty texture |
| `bindTexture(int target, int texture)` | uses this texture: the target is `TEXTURE_2D` |
| `activeTexture(int unit)` | which texture unit `bindTexture` sets: `TEXTURE0` (the start) to `TEXTURE3` |
| `texParameteri(int target, int name, int value)` | a texture setting, see below |
| `generateMipmap(int target)` | makes the smaller copies used when the texture is drawn small |
| `deleteTexture(int texture)` | frees it |

`loadTexture` sets the texture up so that:

- texture coordinate `(0, 0)` is the image's **bottom-left** corner, as usual in OpenGL;
- it's smooth when stretched;
- it repeats past the edges.

For sharp pixel art, change the stretching setting after loading:

```cpp
int sheet = OpenGL.loadTexture("player.png");
OpenGL.bindTexture(OpenGL.TEXTURE_2D, sheet);
OpenGL.texParameteri(OpenGL.TEXTURE_2D, OpenGL.TEXTURE_MAG_FILTER, OpenGL.NEAREST);
```

| Setting (`name`) | Values |
|---|---|
| `TEXTURE_MAG_FILTER` (drawn bigger than the image) | `NEAREST` (sharp pixels), `LINEAR` (smooth) |
| `TEXTURE_MIN_FILTER` (drawn smaller) | `NEAREST`, `LINEAR`, `NEAREST_MIPMAP_NEAREST`, `LINEAR_MIPMAP_LINEAR` |
| `TEXTURE_WRAP_S`, `TEXTURE_WRAP_T` (past the edges, across and up) | `REPEAT`, `CLAMP_TO_EDGE` |

In the fragment shader, read the texture with a `uniform sampler2D` and `texture(...)`. A `sampler2D` reads texture unit 0 unless you set it with `uniform1i`.

## Framebuffers: drawing into a texture

A **framebuffer** is somewhere to draw other than the window: draw a scene into a texture, then use that texture like any other (a security-camera screen, a mirror, a minimap, or effects over the whole picture).

| Function | What it does |
|---|---|
| `texImage2D(int target, int width, int height)` | gives the bound texture an empty picture of this size (RGBA, smooth filtering), ready to be drawn into |
| `genFramebuffer()` | a new framebuffer |
| `bindFramebuffer(int target, int framebuffer)` | draws into it from now on (`FRAMEBUFFER`); `0` is the window |
| `framebufferTexture2D(int target, int attachment, int textarget, int texture)` | the texture its colors go into (`COLOR_ATTACHMENT0`, `TEXTURE_2D`) |
| `checkFramebufferStatus(int target)` | `FRAMEBUFFER_COMPLETE` when it's ready to draw into |
| `genRenderbuffer()`, `bindRenderbuffer(int target, int renderbuffer)` | a buffer that's drawn into but never read: a depth buffer (`RENDERBUFFER`) |
| `renderbufferStorage(int target, int format, int width, int height)` | gives it a size, for depth `DEPTH_COMPONENT24` |
| `framebufferRenderbuffer(int target, int attachment, int rbtarget, int renderbuffer)` | uses it for the framebuffer's depth (`DEPTH_ATTACHMENT`) |
| `deleteFramebuffer(int framebuffer)`, `deleteRenderbuffer(int renderbuffer)` | frees them |

```cpp
// once: a 256 x 256 texture to draw into, with a depth buffer
int picture = OpenGL.genTexture();
OpenGL.bindTexture(OpenGL.TEXTURE_2D, picture);
OpenGL.texImage2D(OpenGL.TEXTURE_2D, 256, 256);
int fb = OpenGL.genFramebuffer();
OpenGL.bindFramebuffer(OpenGL.FRAMEBUFFER, fb);
OpenGL.framebufferTexture2D(OpenGL.FRAMEBUFFER, OpenGL.COLOR_ATTACHMENT0, OpenGL.TEXTURE_2D, picture);
int depth = OpenGL.genRenderbuffer();
OpenGL.bindRenderbuffer(OpenGL.RENDERBUFFER, depth);
OpenGL.renderbufferStorage(OpenGL.RENDERBUFFER, OpenGL.DEPTH_COMPONENT24, 256, 256);
OpenGL.framebufferRenderbuffer(OpenGL.FRAMEBUFFER, OpenGL.DEPTH_ATTACHMENT, OpenGL.RENDERBUFFER, depth);
OpenGL.bindFramebuffer(OpenGL.FRAMEBUFFER, 0);

// every frame: draw into the texture, then use it on the window
OpenGL.bindFramebuffer(OpenGL.FRAMEBUFFER, fb);
OpenGL.viewport(0, 0, 256, 256);                    // the texture's size
OpenGL.clear(OpenGL.COLOR_BUFFER_BIT bitor OpenGL.DEPTH_BUFFER_BIT);
// ... draw the scene ...
OpenGL.bindFramebuffer(OpenGL.FRAMEBUFFER, 0);
OpenGL.viewport(0, 0, OpenGL.width(), OpenGL.height());
OpenGL.bindTexture(OpenGL.TEXTURE_2D, picture);
// ... draw something with the texture on it ...
```

Remember the `viewport`: it's the size being drawn into, and it doesn't change by itself when the framebuffer does.

## Constants

| Group | Names |
|---|---|
| `clear` | `COLOR_BUFFER_BIT`, `DEPTH_BUFFER_BIT`, `STENCIL_BUFFER_BIT` |
| draw modes | `POINTS`, `LINES`, `LINE_LOOP`, `LINE_STRIP`, `TRIANGLES`, `TRIANGLE_STRIP`, `TRIANGLE_FAN` |
| buffers | `ARRAY_BUFFER`, `ELEMENT_ARRAY_BUFFER`, `STATIC_DRAW`, `DYNAMIC_DRAW`, `STREAM_DRAW` |
| shaders | `VERTEX_SHADER`, `FRAGMENT_SHADER` |
| `enable`/`disable` | `DEPTH_TEST`, `BLEND`, `CULL_FACE` |
| blending | `ZERO`, `ONE`, `SRC_ALPHA`, `ONE_MINUS_SRC_ALPHA` |
| depth | `LESS`, `LEQUAL`, `ALWAYS` |
| culling | `FRONT`, `BACK`, `FRONT_AND_BACK` |
| textures | `TEXTURE_2D`, `TEXTURE0`-`TEXTURE3`, `TEXTURE_MIN_FILTER`, `TEXTURE_MAG_FILTER`, `TEXTURE_WRAP_S`, `TEXTURE_WRAP_T`, `NEAREST`, `LINEAR`, `NEAREST_MIPMAP_NEAREST`, `LINEAR_MIPMAP_LINEAR`, `REPEAT`, `CLAMP_TO_EDGE` |
| framebuffers | `FRAMEBUFFER`, `RENDERBUFFER`, `COLOR_ATTACHMENT0`, `DEPTH_ATTACHMENT`, `DEPTH_COMPONENT24`, `FRAMEBUFFER_COMPLETE` |
| errors | `NO_ERROR`, `INVALID_ENUM`, `INVALID_VALUE`, `INVALID_OPERATION`, `OUT_OF_MEMORY` |

They have the same numbers as in real OpenGL, so a value from a tutorial can be used as a plain number too.

## Not included

This toolkit covers the parts of OpenGL that most games and tutorials use. It doesn't have:

- uniform buffers, instancing, or geometry shaders;
- 3D textures and cube maps.

These may come later.
