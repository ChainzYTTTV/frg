#include "templates.hpp"

static const char* kConsole = R"FRG(#include <stdio>

int main(){
    print("Hello, World!");
    return 0;
}
)FRG";

static const char* kWindow = R"FRG(#include <window>
#include <graphics2d>
#include <input>

```
  $NAME$: a window with a ball you move with the arrow keys.
  window.run() calls update(dt) and then draw() every frame, until the window is closed.
```

const float SPEED = 300;    // pixels per second
vec2 ball = vec2(400, 300);

int main(){
    window.open(800, 600, "$NAME$");
    window.run();
    return 0;
}

// dt is the time since the last frame, in seconds. Multiplying by it makes the speed the same on every PC.
void update(float dt){
    vec2 direction = vec2(0, 0);
    if (input.keyDown(Key.LEFT)) {
        direction.x -= 1;
    }
    if (input.keyDown(Key.RIGHT)) {
        direction.x += 1;
    }
    if (input.keyDown(Key.UP)) {
        direction.y -= 1;      // y goes down the screen, so up is negative
    }
    if (input.keyDown(Key.DOWN)) {
        direction.y += 1;
    }
    ball += direction * SPEED * dt;
}

void draw(){
    graphics2d.drawCircle(ball, 30, color.ORANGE);
    graphics2d.drawText("Arrow keys to move", 20, 20, 20, color.LIGHTGRAY);
}
)FRG";

static const char* kPlatformer = R"FRG(#include <window>
#include <graphics2d>
#include <input>
#include <physics>
#include <math>

```
  $NAME$: a small platformer. Arrow keys (or A / D) to move, Space to jump.
  The level is wider than the window, so a camera follows the player.
```

const float LEVEL_WIDTH = 2000;
const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;

Body hero = Body(100, 300, 36, 36);
list<Body> platforms = [];
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));

int main(){
    window.open(960, 540, "$NAME$");
    window.setBackground(color(24, 26, 40));
    addPlatform(0, 500, 700, 40);        // the ground, with a pit in it
    addPlatform(850, 500, 1150, 40);
    addPlatform(300, 380, 160, 20);      // platforms to jump on
    addPlatform(600, 280, 160, 20);
    addPlatform(1100, 380, 200, 20);
    addPlatform(1450, 300, 160, 20);
    addPlatform(-20, 0, 20, 540);        // walls at both ends
    addPlatform(LEVEL_WIDTH, 0, 20, 540);
    physics.add(hero);
    window.run();
    return 0;
}

void addPlatform(float x, float y, float width, float height){
    Body b = Body(x, y, width, height);
    b.isStatic = true;
    physics.add(b);
    platforms.add(b);
}

void update(float dt){
    float direction = 0;
    if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) {
        direction -= 1;
    }
    if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) {
        direction += 1;
    }
    hero.velocity.x = direction * MOVE_SPEED;
    if (input.keyPressed(Key.SPACE) and hero.onGround) {
        hero.velocity.y = -JUMP_SPEED;
    }

    physics.step(dt);

    // Fell into the pit: back to the start.
    if (hero.position.y > window.height() + 100) {
        hero.position = vec2(100, 300);
        hero.velocity = vec2.ZERO;
    }

    // The camera glides after the player, and stops at the ends of the level.
    float half = window.width() / 2.0;
    float x = math.clamp(hero.center().x, half, LEVEL_WIDTH - half);
    camera.target = camera.target.lerp(vec2(x, window.height() / 2.0), 5 * dt);
}

void draw(){
    graphics2d.setCamera(camera);    // the world...
    for (Body p in platforms) {
        graphics2d.drawRect(p.bounds(), color(70, 86, 120));
    }
    graphics2d.drawRect(hero.bounds(), color.ORANGE);
    graphics2d.resetCamera();        // ...and then text that stays on the screen
    graphics2d.drawText("Arrows to move, Space to jump", 20, 16, 20, color.LIGHTGRAY);
}
)FRG";

static const char* k3D = R"FRG(#include <window>
#include <graphics2d>
#include <graphics3d>
#include <input>

```
  $NAME$: a 3D scene. Hold Left / Right to turn the camera around the cube.
```

Camera camera = Camera(vec3(6, 4, 6), vec3(0, 0.5, 0));    // where the camera is, and the point it looks at

int main(){
    window.open(960, 540, "$NAME$");
    window.setBackground(color(20, 24, 36));
    window.run();
    return 0;
}

void update(float dt){
    if (input.keyDown(Key.LEFT)) {
        camera.orbit(-90 * dt);
    }
    if (input.keyDown(Key.RIGHT)) {
        camera.orbit(90 * dt);
    }
}

void draw(){
    graphics3d.setCamera(camera);
    graphics3d.drawGrid(10, 1);
    graphics3d.drawCube(vec3(0, 0.5, 0), vec3(1, 1, 1), color.RED);
    graphics3d.drawCubeOutline(vec3(0, 0.5, 0), vec3(1, 1, 1), color.WHITE);
    graphics3d.drawSphere(vec3(2, 0.5, 0), 0.5, color.SKYBLUE);
    graphics2d.drawText("Left / Right to turn the camera", 20, 20, 20, color.LIGHTGRAY);
}
)FRG";

const std::vector<std::string>& templateNames() {
    static const std::vector<std::string> names = {"console", "window", "platformer", "3d"};
    return names;
}

std::vector<std::pair<std::string, std::string>> templateFiles(const std::string& templateName, const std::string& gameName) {
    const char* text = templateName == "window" ? kWindow : templateName == "platformer" ? kPlatformer : templateName == "3d" ? k3D : kConsole;
    std::string s = text;
    for (size_t i; (i = s.find("$NAME$")) != std::string::npos;) s.replace(i, 6, gameName);
    return {{"main.frg", s}};
}
