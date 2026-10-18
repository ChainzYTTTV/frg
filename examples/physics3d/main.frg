#include <window>
#include <graphics2d>
#include <graphics3d>
#include <physics3d>
#include <input>
#include <math>

```
  A 3D playground for <physics3d>: walk around (W A S D), turn (Left / Right), jump (Space), and throw balls
  (Enter). Walk up the stairs onto the platform. The ray from the player shows where it's facing, and what it
  hits. Tab shows the physics bodies' outlines.
```

Body3D player = Body3D(vec3(0, 1, -6), vec3(0.6, 1.8, 0.6));
list<Body3D> crates = [];
list<Body3D> balls = [];
list<Body3D> scenery = [];        // the static bodies, drawn as solid boxes
float yaw = 0;                    // which way the player faces (0: along +z)
bool showBodies = false;

Body3D wall(vec3 middle, vec3 size){
    Body3D b = Body3D(middle, size);
    b.isStatic = true;
    physics3d.add(b);
    scenery.add(b);
    return b;
}

int main(){
    window.open(960, 540, "Forge 3D physics");
    window.setBackground(color(150, 185, 215));
    graphics3d.setLight(vec3(-0.4, -1, 0.3));
    physics3d.setGravity(vec3(0, -20, 0));     // a bit stronger than real gravity: snappier jumps

    wall(vec3(0, -0.5, 0), vec3(30, 1, 30));   // the ground (its top is at y 0)
    wall(vec3(0, 1, 14.5), vec3(30, 2, 1));    // low walls around it
    wall(vec3(0, 1, -14.5), vec3(30, 2, 1));
    wall(vec3(14.5, 1, 0), vec3(1, 2, 30));
    wall(vec3(-14.5, 1, 0), vec3(1, 2, 30));
    // stairs up to a platform
    for (int i in 0..5) {
        float h = (i + 1) * 0.3;
        wall(vec3(-4 - i * 0.6, h / 2, 6), vec3(0.6, h, 3));
    }
    wall(vec3(-9, 0.9, 6), vec3(4, 1.8, 6));

    player.stepHeight = 0.35;                  // climbs the 0.3 steps by itself
    physics3d.add(player);
    // a stack of crates to push and knock over
    for (int i in 0..3) {
        Body3D c = Body3D(vec3(5, 0.5 + i * 1.0, 3), vec3(1, 1, 1));
        physics3d.add(c);
        crates.add(c);
    }
    window.run();
    return 0;
}

vec3 forward(){
    return vec3(math.sin(yaw), 0, math.cos(yaw));
}

void update(float dt){
    if (input.keyDown(Key.LEFT)) {
        yaw += 2.5 * dt;
    }
    if (input.keyDown(Key.RIGHT)) {
        yaw -= 2.5 * dt;
    }
    // walking: relative to where the player faces (+x is on the left when looking along +z)
    vec3 f = forward();
    vec3 right = f.cross(vec3.UP);
    vec3 move = vec3.ZERO;
    if (input.keyDown(Key.W)) { move = move + f; }
    if (input.keyDown(Key.S)) { move = move - f; }
    if (input.keyDown(Key.D)) { move = move + right; }
    if (input.keyDown(Key.A)) { move = move - right; }
    if (move.length() > 0) {
        move = move.normalized() * 5;
    }
    player.velocity = vec3(move.x, player.velocity.y, move.z);
    if (input.keyPressed(Key.SPACE) and player.onGround) {
        player.velocity = vec3(player.velocity.x, 8, player.velocity.z);
    }
    if (input.keyPressed(Key.ENTER)) {
        Body3D ball = Body3D(player.position + f * 0.8 + vec3(0, 0.5, 0), vec3(0.5, 0.5, 0.5));
        ball.sphere = true;
        ball.bounce = 0.6;
        ball.velocity = f * 12 + vec3(0, 4, 0);
        physics3d.add(ball);
        balls.add(ball);
    }
    if (input.keyPressed(Key.TAB)) {
        showBodies = !showBodies;
    }
    physics3d.step(dt);

    // the camera, behind the player and a little above
    vec3 eye = player.position - f * 6 + vec3(0, 3, 0);
    graphics3d.setCamera(Camera3D(eye, player.position + vec3(0, 1, 0)));
}

void draw(){
    for (Body3D b in scenery) {
        graphics3d.drawCube(b.position, b.size, color(120, 130, 120));
    }
    for (Body3D c in crates) {
        graphics3d.drawCube(c.position, c.size, color(190, 140, 80));
    }
    for (Body3D b in balls) {
        graphics3d.drawSphere(b.position, b.size.x / 2, color(230, 80, 80));
    }
    graphics3d.drawCube(player.position, player.size, color(80, 140, 230));

    // what's in front of the player: a ray from its eyes
    vec3 eyes = player.position + vec3(0, 0.6, 0);
    RayHit hit = physics3d.raycast(eyes, forward(), 30);
    if (hit.hit) {
        graphics3d.drawLine(eyes, hit.point, color.YELLOW);
        graphics3d.drawSphere(hit.point, 0.1, color.YELLOW);
    }
    if (showBodies) {
        physics3d.drawDebug();
    }
    graphics2d.drawText("WASD walk   Left/Right turn   Space jump   Enter throw a ball   Tab outlines", 10, 10, 20, color.WHITE);
    string looking = hit.hit ? "{hit.distance:.1} m away" : "nothing";
    graphics2d.drawText("on the ground: {player.onGround}   the ray hits: {looking}", 10, 36, 20, color.WHITE);
}
