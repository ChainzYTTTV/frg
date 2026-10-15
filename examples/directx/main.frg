#include <DirectX>
#include <math>
#include <stdio>

```
  Drawing with Direct3D 11 directly, instead of Forge's engine.
  The same scene as examples/opengl: a spinning cube with a color at each corner,
  and a textured square. Hold Left / Right to change how fast the cube spins.
```

string SHADER = """
cbuffer Transform : register(b0) {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float useTexture;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct VertexIn {
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PixelIn {
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

PixelIn vsMain(VertexIn v) {
    PixelIn o;
    o.position = mul(projection, mul(view, mul(model, float4(v.position, 1.0))));
    o.color = v.color;
    o.uv = v.uv;
    return o;
}

float4 psMain(PixelIn p) : SV_TARGET {
    if (useTexture > 0.5) {
        float4 c = tex.Sample(samp, p.uv);
        clip(c.a - 0.1);
        return c;
    }
    return float4(p.color, 1.0);
}
""";

int main(){
    DirectX.openWindow(960, 540, "Forge + DirectX 11");
    print("Running on {DirectX.adapterName()}");

    int vs = DirectX.compileVertexShader(SHADER, "vsMain");
    int ps = DirectX.compilePixelShader(SHADER, "psMain");
    int layout = DirectX.createInputLayout(vs, ["POSITION", "COLOR", "TEXCOORD"], [3, 3, 2]);
    int transform = DirectX.createConstantBuffer(16 * 3 + 1);

    int cubeVertices = DirectX.createVertexBuffer(cubeCorners());
    int cubeIndices = DirectX.createIndexBuffer(cubeTriangles());
    int squareVertices = DirectX.createVertexBuffer(squareCorners());
    int blob = DirectX.loadTexture("blob.png");

    DirectX.setShaders(vs, ps);
    DirectX.setInputLayout(layout);
    DirectX.setConstantBuffer(0, transform);
    DirectX.setTexture(0, blob);
    DirectX.setTextureFilter(false);      // keep pixel art sharp
    DirectX.setBlending(true);

    float angle = 0;
    float spinSpeed = 60;
    while (!DirectX.shouldClose()) {
        float dt = DirectX.deltaTime();
        if (DirectX.keyDown(Key.RIGHT)) {
            spinSpeed += 120 * dt;
        }
        if (DirectX.keyDown(Key.LEFT)) {
            spinSpeed -= 120 * dt;
        }
        angle += spinSpeed * dt;

        DirectX.clear(0.08, 0.09, 0.14, 1);
        float aspect = float(DirectX.width()) / DirectX.height();
        list<float> view = DirectX.lookAt(vec3(0, 1.5, 5), vec3(0, 0, 0), vec3.UP);
        list<float> projection = DirectX.perspective(45, aspect, 0.1, 100);

        // The cube: turned around two axes, a little to the left.
        list<float> spin = DirectX.multiply(DirectX.rotate(angle, vec3(0, 1, 0)), DirectX.rotate(angle * 0.5, vec3(1, 0, 0)));
        list<float> model = DirectX.multiply(DirectX.translate(vec3(-1.2, 0, 0)), spin);
        DirectX.updateConstantBuffer(transform, transformData(model, view, projection, 0));
        DirectX.setVertexBuffer(cubeVertices, 8);
        DirectX.setIndexBuffer(cubeIndices);
        DirectX.drawIndexed(36);

        // The textured square, bobbing up and down on the right.
        float bob = math.sin(angle * 0.05) * 0.3;
        DirectX.updateConstantBuffer(transform, transformData(DirectX.translate(vec3(1.4, bob, 0)), view, projection, 1));
        DirectX.setVertexBuffer(squareVertices, 8);
        DirectX.draw(6);
    }
    DirectX.closeWindow();
    return 0;
}

// The constant buffer's contents, in the order the shader's cbuffer lists them.
list<float> transformData(list<float> model, list<float> view, list<float> projection, float useTexture){
    list<float> data = [];
    data.addAll(model);
    data.addAll(view);
    data.addAll(projection);
    data.add(useTexture);
    return data;
}

// Each corner: position x y z, color r g b, texture u v.
list<float> cubeCorners(){
    return [
        -0.7, -0.7, -0.7,   1, 0.3, 0.3,   0, 0,
         0.7, -0.7, -0.7,   0.3, 1, 0.3,   0, 0,
         0.7,  0.7, -0.7,   0.3, 0.3, 1,   0, 0,
        -0.7,  0.7, -0.7,   1, 1, 0.3,     0, 0,
        -0.7, -0.7,  0.7,   1, 0.3, 1,     0, 0,
         0.7, -0.7,  0.7,   0.3, 1, 1,     0, 0,
         0.7,  0.7,  0.7,   1, 1, 1,       0, 0,
        -0.7,  0.7,  0.7,   1, 0.6, 0.2,   0, 0
    ];
}

list<int> cubeTriangles(){
    return [
        0, 1, 2,  2, 3, 0,     4, 5, 6,  6, 7, 4,
        0, 4, 7,  7, 3, 0,     1, 5, 6,  6, 2, 1,
        3, 2, 6,  6, 7, 3,     0, 1, 5,  5, 4, 0
    ];
}

// In Direct3D, texture coordinate (0, 0) is the top-left of the image.
list<float> squareCorners(){
    return [
        -0.6, -0.6, 0,   1, 1, 1,   0,    1,
         0.6, -0.6, 0,   1, 1, 1,   0.25, 1,
         0.6,  0.6, 0,   1, 1, 1,   0.25, 0,
         0.6,  0.6, 0,   1, 1, 1,   0.25, 0,
        -0.6,  0.6, 0,   1, 1, 1,   0,    0,
        -0.6, -0.6, 0,   1, 1, 1,   0,    1
    ];
}
