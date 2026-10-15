#include <OpenGL>
#include <math>
#include <stdio>

```
  Drawing with OpenGL directly, instead of Forge's engine.
  A spinning cube with a color at each corner, and a textured square next to it.
  Hold Left / Right to change how fast the cube spins.
```

string VERTEX_SHADER = """
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 vColor;
out vec2 vUv;
void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    vColor = color;
    vUv = uv;
}
""";

string FRAGMENT_SHADER = """
#version 330 core
in vec3 vColor;
in vec2 vUv;
uniform sampler2D tex;
uniform int useTexture;
out vec4 fragColor;
void main() {
    if (useTexture == 1) {
        fragColor = texture(tex, vUv);
        if (fragColor.a < 0.1) discard;
    } else {
        fragColor = vec4(vColor, 1.0);
    }
}
""";

int main(){
    OpenGL.openWindow(960, 540, "Forge + OpenGL");
    print("Running on {OpenGL.version()}");

    int program = OpenGL.buildProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    int cube = makeCube();
    int square = makeSquare();
    int blob = OpenGL.loadTexture("blob.png");
    OpenGL.bindTexture(OpenGL.TEXTURE_2D, blob);
    OpenGL.texParameteri(OpenGL.TEXTURE_2D, OpenGL.TEXTURE_MAG_FILTER, OpenGL.NEAREST);   // keep pixel art sharp

    int modelLoc = OpenGL.getUniformLocation(program, "model");
    int viewLoc = OpenGL.getUniformLocation(program, "view");
    int projectionLoc = OpenGL.getUniformLocation(program, "projection");
    int useTextureLoc = OpenGL.getUniformLocation(program, "useTexture");

    OpenGL.enable(OpenGL.DEPTH_TEST);
    OpenGL.enable(OpenGL.BLEND);
    OpenGL.blendFunc(OpenGL.SRC_ALPHA, OpenGL.ONE_MINUS_SRC_ALPHA);

    float angle = 0;
    float spinSpeed = 60;
    while (!OpenGL.shouldClose()) {
        float dt = OpenGL.deltaTime();
        if (OpenGL.keyDown(Key.RIGHT)) {
            spinSpeed += 120 * dt;
        }
        if (OpenGL.keyDown(Key.LEFT)) {
            spinSpeed -= 120 * dt;
        }
        angle += spinSpeed * dt;

        OpenGL.clearColor(0.08, 0.09, 0.14, 1);
        OpenGL.clear(OpenGL.COLOR_BUFFER_BIT bitor OpenGL.DEPTH_BUFFER_BIT);
        OpenGL.useProgram(program);

        float aspect = float(OpenGL.width()) / OpenGL.height();
        OpenGL.uniformMatrix4(projectionLoc, OpenGL.perspective(45, aspect, 0.1, 100));
        OpenGL.uniformMatrix4(viewLoc, OpenGL.lookAt(vec3(0, 1.5, 5), vec3(0, 0, 0), vec3.UP));

        // The cube: turned around two axes, a little to the left.
        list<float> spin = OpenGL.multiply(OpenGL.rotate(angle, vec3(0, 1, 0)), OpenGL.rotate(angle * 0.5, vec3(1, 0, 0)));
        OpenGL.uniformMatrix4(modelLoc, OpenGL.multiply(OpenGL.translate(vec3(-1.2, 0, 0)), spin));
        OpenGL.uniform1i(useTextureLoc, 0);
        OpenGL.bindVertexArray(cube);
        OpenGL.drawElements(OpenGL.TRIANGLES, 36);

        // The textured square, bobbing up and down on the right.
        float bob = math.sin(angle * 0.05) * 0.3;
        OpenGL.uniformMatrix4(modelLoc, OpenGL.translate(vec3(1.4, bob, 0)));
        OpenGL.uniform1i(useTextureLoc, 1);
        OpenGL.bindVertexArray(square);
        OpenGL.drawArrays(OpenGL.TRIANGLES, 0, 6);
    }
    OpenGL.closeWindow();
    return 0;
}

// Uploads vertices (position x y z, color r g b, texture u v) into a vertex array.
int makeVertexArray(list<float> vertices){
    int vao = OpenGL.genVertexArray();
    OpenGL.bindVertexArray(vao);
    int vbo = OpenGL.genBuffer();
    OpenGL.bindBuffer(OpenGL.ARRAY_BUFFER, vbo);
    OpenGL.bufferData(OpenGL.ARRAY_BUFFER, vertices, OpenGL.STATIC_DRAW);
    OpenGL.vertexAttribPointer(0, 3, 8, 0);       // position: 3 floats, every 8 floats, starting at 0
    OpenGL.enableVertexAttribArray(0);
    OpenGL.vertexAttribPointer(1, 3, 8, 3);       // color: 3 floats, starting at 3
    OpenGL.enableVertexAttribArray(1);
    OpenGL.vertexAttribPointer(2, 2, 8, 6);       // texture coordinate: 2 floats, starting at 6
    OpenGL.enableVertexAttribArray(2);
    return vao;
}

int makeCube(){
    // 8 corners, each with its own color.
    list<float> corners = [
        -0.7, -0.7, -0.7,   1, 0.3, 0.3,   0, 0,
         0.7, -0.7, -0.7,   0.3, 1, 0.3,   0, 0,
         0.7,  0.7, -0.7,   0.3, 0.3, 1,   0, 0,
        -0.7,  0.7, -0.7,   1, 1, 0.3,     0, 0,
        -0.7, -0.7,  0.7,   1, 0.3, 1,     0, 0,
         0.7, -0.7,  0.7,   0.3, 1, 1,     0, 0,
         0.7,  0.7,  0.7,   1, 1, 1,       0, 0,
        -0.7,  0.7,  0.7,   1, 0.6, 0.2,   0, 0
    ];
    int vao = makeVertexArray(corners);
    // 12 triangles (2 per side), as corner numbers.
    list<int> triangles = [
        0, 1, 2,  2, 3, 0,     4, 5, 6,  6, 7, 4,
        0, 4, 7,  7, 3, 0,     1, 5, 6,  6, 2, 1,
        3, 2, 6,  6, 7, 3,     0, 1, 5,  5, 4, 0
    ];
    int ebo = OpenGL.genBuffer();
    OpenGL.bindBuffer(OpenGL.ELEMENT_ARRAY_BUFFER, ebo);
    OpenGL.bufferDataInts(OpenGL.ELEMENT_ARRAY_BUFFER, triangles, OpenGL.STATIC_DRAW);
    return vao;
}

int makeSquare(){
    // Shows the first 16x16 frame of the 64x16 sprite sheet: u goes from 0 to 0.25.
    list<float> vertices = [
        -0.6, -0.6, 0,   1, 1, 1,   0,    0,
         0.6, -0.6, 0,   1, 1, 1,   0.25, 0,
         0.6,  0.6, 0,   1, 1, 1,   0.25, 1,
         0.6,  0.6, 0,   1, 1, 1,   0.25, 1,
        -0.6,  0.6, 0,   1, 1, 1,   0,    1,
        -0.6, -0.6, 0,   1, 1, 1,   0,    0
    ];
    return makeVertexArray(vertices);
}
