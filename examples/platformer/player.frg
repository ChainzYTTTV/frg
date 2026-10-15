#include <window>
#include <graphics2d>
#include <input>
#include <physics>
#include <particles>

const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;
const vec2 START = vec2(80, 380);

// A Player is a physics Body (it inherits position, size, velocity, onGround...)
// with a few extra things of its own.
class Player : Body {
    color tint = color.ORANGE;
    int deaths = 0;

    void respawn() {
        position = START;
        velocity = vec2.ZERO;
        deaths++;
        flash();
    }

    // A task: it runs alongside the game, and wait() pauses just this task, not the game.
    task flash() {
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
}

Player hero = makeHero();

// A puff of dust when the player lands.
Emitter dust = makeDust();
bool wasOnGround = false;
float fallSpeed = 0;

Emitter makeDust(){
    Emitter e = Emitter(vec2(0, 0));
    e.lifetime = 0.4;
    e.speed = 90;
    e.direction = -90;          // up...
    e.spread = 160;             // ...and out to both sides
    e.gravity = vec2(0, 300);
    e.startColor = color(200, 200, 210);
    e.endColor = color(200, 200, 210, 0);   // fades out
    e.startSize = 9;
    e.endSize = 3;
    return e;
}

Player makeHero(){
    Player p = Player(START.x, START.y, 36, 36);
    physics.add(p);
    return p;
}

void handleInput(){
    fallSpeed = hero.velocity.y;    // remembered before physics.step stops the fall
    float direction = 0;
    if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) {
        direction -= 1;
    }
    if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) {
        direction += 1;
    }
    hero.velocity.x = direction * MOVE_SPEED;

    bool jumpPressed = input.keyPressed(Key.SPACE) or input.keyPressed(Key.W) or input.keyPressed(Key.UP);
    if (jumpPressed and hero.onGround) {
        hero.velocity.y = -JUMP_SPEED;
    }

    // Letting go of jump early makes a shorter jump.
    bool jumpReleased = input.keyReleased(Key.SPACE) or input.keyReleased(Key.W) or input.keyReleased(Key.UP);
    if (jumpReleased and hero.velocity.y < 0) {
        hero.velocity.y *= 0.5;
    }
}

// Called after physics.step: a hard landing makes a puff of dust at the player's feet.
void checkLanding(float dt){
    if (hero.onGround and !wasOnGround and fallSpeed > 250) {
        dust.position = vec2(hero.center().x, hero.position.y + hero.size.y);
        dust.burst(14);
    }
    wasOnGround = hero.onGround;
    dust.update(dt);
}

void checkFall(){
    if (hero.position.y > window.height() + 100) {
        hero.respawn();
    }
}

void draw(){
    dust.draw();
    graphics2d.drawRect(hero.bounds(), hero.tint);
    // Two eyes that look the way the player is moving.
    float look = hero.velocity.x > 0 ? 4 : hero.velocity.x < 0 ? -4 : 0;
    float eyeY = hero.position.y + 12;
    graphics2d.drawRect(hero.position.x + 8 + look, eyeY, 6, 8, color.BLACK);
    graphics2d.drawRect(hero.position.x + 22 + look, eyeY, 6, 8, color.BLACK);
}
