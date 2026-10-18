#include <window>
#include <graphics2d>
#include <graphics3d>
#include <input>
#include <math>

```
  A small 3D scene: cubes, a bouncing ball, a model loaded from a file,
  and a camera that circles around (hold Left / Right to spin it yourself).
```

Camera3D cam = Camera3D(vec3(8, 6, 8), vec3(0, 1, 0));
Model pyramid = graphics3d.loadModel("pyramid.obj");
float t = 0;

int main(){
    window.open(960, 540, "Forge 3D");
    window.setBackground(color(20, 24, 36));
    window.run();
    return 0;
}

void update(float dt){
    t += dt;
    float spin = 15 * dt;
    if (input.keyDown(Key.LEFT)) {
        spin = -60 * dt;
    }
    if (input.keyDown(Key.RIGHT)) {
        spin = 60 * dt;
    }
    cam.orbit(spin);
    graphics3d.setCamera(cam);
}

void draw(){
    graphics3d.drawGrid(20, 1);
    graphics3d.drawPlane(vec3(0, 0, 0), vec2(10, 10), color(40, 60, 50));

    // A ring of cubes.
    for (int i = 0; i < 8; i++) {
        float angle = i * math.TAU / 8;
        vec3 p = vec3(math.cos(angle) * 4, 0.5, math.sin(angle) * 4);
        graphics3d.drawCube(p, vec3(1, 1, 1), color.SKYBLUE.lerp(color.PURPLE, i / 7.0));
        graphics3d.drawCubeOutline(p, vec3(1, 1, 1), color.WHITE);
    }

    // A ball that bounces in the middle.
    float height = 1 + math.abs(math.sin(t * 3)) * 2;
    vec3 ball = vec3(0, height, 0);
    graphics3d.drawSphere(ball, 0.6, color.ORANGE);
    graphics3d.drawCylinder(vec3(0, 0, 0), 0.8, 0.1, color.DARKGRAY);

    pyramid.drawRotated(vec3(-2, 0, -1), vec3(0, 1, 0), t * 90, 1.5, color.GOLD);

    // 2D on top of the 3D: a label that follows the ball.
    vec2 label = graphics3d.toScreen(ball + vec3(0, 1, 0));
    graphics2d.drawText("ball", label.x - 20, label.y, 20, color.WHITE);
    graphics2d.drawText("Left / Right to spin the camera", 20, 500, 20, color.LIGHTGRAY);
}
