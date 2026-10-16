#include <window>
#include <graphics3d>
#include <input>
#include <math>

```
  The player: a 0.6 x 1.8 x 0.6 box that walks, jumps and flies, and bumps into every block (each block is a
  1 x 1 x 1 hitbox). The mouse looks around; the block in the middle of the screen can be broken or built on.
```

const float WIDTH = 0.6;
const float TALL = 1.8;
const float EYES = 1.62;
const float REACH = 6;

vec3 position = vec3(64, 40, 64);   // the middle of the feet
vec3 velocity = vec3.ZERO;
float yaw = 0;      // looking left and right, in radians (0 looks along +z)
float pitch = 0;    // looking up and down
bool flying = false;
bool onGround = false;
double lastJump = -1; // when Space was last pressed, for double-tapping it to fly

struct Hit {
    bool found = false;
    int x = 0;
    int y = 0;
    int z = 0;
    int nx = 0;   // the side of the block that was hit, pointing out of it
    int ny = 0;
    int nz = 0;
}

Hit target = Hit();   // the block the crosshair is on

void spawn(){
    int x = world.SIZE / 2;
    int z = world.SIZE / 2;
    position = vec3(x + 0.5, world.topAt(x, z) + 1, z + 0.5);
}

vec3 eye(){
    return position + vec3(0, EYES, 0);
}

vec3 lookDirection(){
    return vec3(math.cos(pitch) * math.sin(yaw), math.sin(pitch), math.cos(pitch) * math.cos(yaw));
}

Camera camera(){
    Camera c = Camera(eye(), eye() + lookDirection());
    c.fov = 70;
    return c;
}

int floorInt(float v){
    return int(math.floor(v));
}

// Whether a box (its lowest corner and size) overlaps any block.
bool boxHitsBlocks(vec3 low, vec3 size){
    for (int y in floorInt(low.y)..floorInt(low.y + size.y - 0.001)) {
        for (int z in floorInt(low.z)..floorInt(low.z + size.z - 0.001)) {
            for (int x in floorInt(low.x)..floorInt(low.x + size.x - 0.001)) {
                if (world.get(x, y, z) != blocks.AIR) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool hitsBlocks(vec3 at){
    return boxHitsBlocks(vec3(at.x - WIDTH / 2, at.y, at.z - WIDTH / 2), vec3(WIDTH, TALL, WIDTH));
}

void update(float dt){
    if (dt > 0.05) {
        dt = 0.05;
    }
    // Looking around with the mouse.
    if (input.isMouseLocked()) {
        vec2 d = input.mouseDelta();
        yaw -= d.x * 0.0025;
        pitch -= d.y * 0.0025;
        pitch = math.clamp(pitch, -1.55, 1.55);
    }
    if (input.keyDown(Key.LEFT)) { yaw += 2.5 * dt; }
    if (input.keyDown(Key.RIGHT)) { yaw -= 2.5 * dt; }
    if (input.keyDown(Key.UP)) { pitch = math.min(pitch + 1.5 * dt, 1.55); }
    if (input.keyDown(Key.DOWN)) { pitch = math.max(pitch - 1.5 * dt, -1.55); }

    // Space twice quickly (or F) starts and stops flying.
    if (input.keyPressed(Key.SPACE)) {
        if (window.time() - lastJump < 0.3) {
            flying = !flying;
            velocity.y = 0;
        }
        lastJump = window.time();
    }
    if (input.keyPressed(Key.F)) {
        flying = !flying;
        velocity.y = 0;
    }

    // Walking: W A S D, relative to where the player looks.
    vec3 forward = vec3(math.sin(yaw), 0, math.cos(yaw));
    vec3 right = vec3(-math.cos(yaw), 0, math.sin(yaw));
    vec3 wish = vec3.ZERO;
    if (input.keyDown(Key.W)) { wish += forward; }
    if (input.keyDown(Key.S)) { wish -= forward; }
    if (input.keyDown(Key.D)) { wish += right; }
    if (input.keyDown(Key.A)) { wish -= right; }
    if (wish.length() > 0) {
        wish = wish.normalized();
    }
    float speed = flying ? 11 : 4.3;
    if (input.keyDown(Key.LEFT_CONTROL)) {
        speed *= 1.6;
    }
    velocity.x = wish.x * speed;
    velocity.z = wish.z * speed;

    if (flying) {
        velocity.y = 0;
        if (input.keyDown(Key.SPACE)) { velocity.y = 8; }
        if (input.keyDown(Key.LEFT_SHIFT)) { velocity.y = -8; }
    } else {
        velocity.y -= 28 * dt;
        velocity.y = math.max(velocity.y, -40);
        if (input.keyDown(Key.SPACE) and onGround) {
            velocity.y = 8.4;
        }
    }

    // Moving one direction at a time, so sliding along walls works: if the move hits a block, step back to touch it.
    onGround = false;
    position.x += velocity.x * dt;
    if (hitsBlocks(position)) {
        position.x = velocity.x > 0 ? math.floor(position.x + WIDTH / 2) - WIDTH / 2 - 0.001 : math.floor(position.x - WIDTH / 2) + 1 + WIDTH / 2 + 0.001;
        velocity.x = 0;
    }
    position.z += velocity.z * dt;
    if (hitsBlocks(position)) {
        position.z = velocity.z > 0 ? math.floor(position.z + WIDTH / 2) - WIDTH / 2 - 0.001 : math.floor(position.z - WIDTH / 2) + 1 + WIDTH / 2 + 0.001;
        velocity.z = 0;
    }
    position.y += velocity.y * dt;
    if (hitsBlocks(position)) {
        if (velocity.y < 0) {
            position.y = math.floor(position.y) + 1;
            onGround = true;
            if (flying) {
                flying = false;  // landing ends flying, like in Minecraft
            }
        } else {
            position.y = math.floor(position.y + TALL) - TALL - 0.001;
        }
        velocity.y = 0;
    }
    if (position.y < -20) {
        spawn();
    }

    target = raycast(eye(), lookDirection(), REACH);
}

// Walks along the line from `from`, one block at a time, until it reaches a block (Amanatides and Woo's method).
Hit raycast(vec3 from, vec3 dir, float maxDistance){
    Hit hit = Hit();
    int x = floorInt(from.x);
    int y = floorInt(from.y);
    int z = floorInt(from.z);
    int stepX = dir.x > 0 ? 1 : -1;
    int stepY = dir.y > 0 ? 1 : -1;
    int stepZ = dir.z > 0 ? 1 : -1;
    // How far along the line to the next block boundary in each direction, and between boundaries.
    float big = 1000000;
    float deltaX = math.abs(dir.x) < 0.00001 ? big : math.abs(1 / dir.x);
    float deltaY = math.abs(dir.y) < 0.00001 ? big : math.abs(1 / dir.y);
    float deltaZ = math.abs(dir.z) < 0.00001 ? big : math.abs(1 / dir.z);
    float nextX = deltaX >= big ? big : (dir.x > 0 ? (x + 1 - from.x) : (from.x - x)) * deltaX;
    float nextY = deltaY >= big ? big : (dir.y > 0 ? (y + 1 - from.y) : (from.y - y)) * deltaY;
    float nextZ = deltaZ >= big ? big : (dir.z > 0 ? (z + 1 - from.z) : (from.z - z)) * deltaZ;
    float travelled = 0;
    while (travelled <= maxDistance) {
        if (nextX < nextY and nextX < nextZ) {
            x += stepX;
            travelled = nextX;
            nextX += deltaX;
            hit.nx = -stepX;
            hit.ny = 0;
            hit.nz = 0;
        } else if (nextY < nextZ) {
            y += stepY;
            travelled = nextY;
            nextY += deltaY;
            hit.nx = 0;
            hit.ny = -stepY;
            hit.nz = 0;
        } else {
            z += stepZ;
            travelled = nextZ;
            nextZ += deltaZ;
            hit.nx = 0;
            hit.ny = 0;
            hit.nz = -stepZ;
        }
        if (travelled > maxDistance) {
            break;
        }
        if (world.get(x, y, z) != blocks.AIR and world.inside(x, y, z)) {
            hit.found = true;
            hit.x = x;
            hit.y = y;
            hit.z = z;
            return hit;
        }
    }
    return Hit();
}

void breakTarget(){
    if (!target.found or world.get(target.x, target.y, target.z) == blocks.BEDROCK) {
        return;
    }
    world.set(target.x, target.y, target.z, blocks.AIR);
    mesher.blockChanged(target.x, target.y, target.z);
}

void placeOnTarget(int block){
    if (!target.found) {
        return;
    }
    int x = target.x + target.nx;
    int y = target.y + target.ny;
    int z = target.z + target.nz;
    if (!world.inside(x, y, z) or world.get(x, y, z) != blocks.AIR) {
        return;
    }
    world.set(x, y, z, block);
    if (hitsBlocks(position)) {  // not inside the player
        world.set(x, y, z, blocks.AIR);
        return;
    }
    mesher.blockChanged(x, y, z);
}

void drawTarget(){
    if (target.found) {
        // Moved a little toward the eye, so edges where the block meets a wall still show in front of the wall.
        vec3 middle = vec3(target.x + 0.5, target.y + 0.5, target.z + 0.5);
        vec3 nearer = middle + (eye() - middle).normalized() * 0.03;
        graphics3d.drawCubeOutline(nearer, vec3(1.004, 1.004, 1.004), color(20, 20, 20));
    }
}
