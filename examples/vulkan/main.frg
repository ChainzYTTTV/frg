#include <Vulkan>
#include <math>
#include <stdio>

```
  Drawing with Vulkan directly, instead of Forge's engine.
  A spinning cube with a color at each corner, and a triangle bobbing next to it.
  Hold Left / Right to change how fast the cube spins.

  The shaders are shape.vert and shape.frag. frg build turns them into
  shape.vert.spv and shape.frag.spv, which is what Vulkan loads.
```

int main(){
    Vulkan.openWindow(960, 540, "Forge + Vulkan");
    print("Running on {Vulkan.deviceName()}");

    int vertexShader = Vulkan.loadShader("shape.vert.spv");
    int fragmentShader = Vulkan.loadShader("shape.frag.spv");
    Vulkan.setCulling(Vulkan.CULL_BACK);    // skip the cube's far sides
    // Each corner is 3 floats of position, then 3 of color; the shader gets 16 floats (one matrix) per draw.
    int pipeline = Vulkan.createPipeline(vertexShader, fragmentShader, [3, 3], 16);

    int cubeVertices = Vulkan.createVertexBuffer(cubeCorners());
    int cubeIndices = Vulkan.createIndexBuffer(cubeTriangles());
    int triangleVertices = Vulkan.createVertexBuffer(triangleCorners());

    float angle = 0;
    float spinSpeed = 60;
    while (!Vulkan.shouldClose()) {
        float dt = Vulkan.deltaTime();
        if (Vulkan.keyDown(Key.RIGHT)) {
            spinSpeed += 120 * dt;
        }
        if (Vulkan.keyDown(Key.LEFT)) {
            spinSpeed -= 120 * dt;
        }
        angle += spinSpeed * dt;

        float aspect = float(Vulkan.width()) / Vulkan.height();
        list<float> camera = Vulkan.multiply(Vulkan.perspective(45, aspect, 0.1, 100),
                                             Vulkan.lookAt(vec3(0, 1.5, 5), vec3(0, 0, 0), vec3.UP));

        // beginFrame is false when there's nothing to draw into (say, the window is minimized).
        if (Vulkan.beginFrame(0.08, 0.09, 0.14, 1)) {
            Vulkan.bindPipeline(pipeline);

            // The cube: turned around two axes, a little to the left.
            list<float> spin = Vulkan.multiply(Vulkan.rotate(angle, vec3(0, 1, 0)), Vulkan.rotate(angle * 0.5, vec3(1, 0, 0)));
            list<float> model = Vulkan.multiply(Vulkan.translate(vec3(-1.2, 0, 0)), spin);
            Vulkan.pushConstants(pipeline, Vulkan.multiply(camera, model));
            Vulkan.bindVertexBuffer(cubeVertices);
            Vulkan.bindIndexBuffer(cubeIndices);
            Vulkan.drawIndexed(36);

            // The triangle, bobbing up and down on the right.
            float bob = math.sin(angle * 0.05) * 0.3;
            Vulkan.pushConstants(pipeline, Vulkan.multiply(camera, Vulkan.translate(vec3(1.4, bob, 0))));
            Vulkan.bindVertexBuffer(triangleVertices);
            Vulkan.draw(3);

            Vulkan.endFrame();
        }
    }
    Vulkan.closeWindow();
    return 0;
}

// Each corner: position x y z, then color r g b.
list<float> cubeCorners(){
    return [
        -0.7, -0.7, -0.7,   1, 0.3, 0.3,
         0.7, -0.7, -0.7,   0.3, 1, 0.3,
         0.7,  0.7, -0.7,   0.3, 0.3, 1,
        -0.7,  0.7, -0.7,   1, 1, 0.3,
        -0.7, -0.7,  0.7,   1, 0.3, 1,
         0.7, -0.7,  0.7,   0.3, 1, 1,
         0.7,  0.7,  0.7,   1, 1, 1,
        -0.7,  0.7,  0.7,   1, 0.6, 0.2
    ];
}

// Two triangles per side. Seen from outside the cube, each one's corners go counter-clockwise,
// which is what makes it count as facing you (so culling skips only the far sides).
list<int> cubeTriangles(){
    return [
        4, 5, 6,  6, 7, 4,     1, 0, 3,  3, 2, 1,
        0, 4, 7,  7, 3, 0,     5, 1, 2,  2, 6, 5,
        7, 6, 2,  2, 3, 7,     0, 1, 5,  5, 4, 0
    ];
}

list<float> triangleCorners(){
    return [
        -0.6, -0.5, 0,   1, 0.2, 0.2,
         0.6, -0.5, 0,   0.2, 1, 0.2,
         0,    0.6, 0,   0.2, 0.4, 1
    ];
}
