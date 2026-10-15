# Platformer Walkthrough

This guide goes through [examples/platformer](../../examples/platformer/) line by line. It's a small but complete game: a cube that runs and jumps across platforms, collects twelve coins, and falls back to the start if it drops into a pit. The level is wider than the window, so a **camera** follows the player.

Run it from the Forge folder with `frg run examples/platformer`. The controls are the arrow keys or A/D to move, and Space, W or Up to jump.

The game is split into three files:

| File | Job |
|---|---|
| `main.frg` | opens the window and runs the loop, calling the other two files |
| `player.frg` | the player: input, jumping, falling into a pit, drawing |
| `level.frg` | the level, drawn as text, and the coins |

## main.frg

```cpp
#include <window>
#include <graphics2d>
#include <physics>
#include <math>
```

Each file includes only the libraries it uses itself. `main.frg` opens the window, moves the camera, draws the HUD text, and steps the physics. (`<math>` is for `math.clamp`, below.)

```cpp
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));
```

The [2D camera](../libraries/graphics2d.md#camera-scrolling-and-zooming). It looks at its `target` (a point in the level) and shows that point at its `offset` (a point on the screen: here the middle of the 960×540 window). It starts out looking at the middle of the first screen.

```cpp
int main(){
    window.open(960, 540, "Forge Platformer");
    window.setBackground(color(24, 26, 40));
    level.build();
    player.start = level.start;     // the P in the level
    player.hero.position = level.start;
    window.run();
    return 0;
}
```

- `window.open` makes a 960×540 window.
- `level.build()` calls `build` in `level.frg`, which turns the level's text into physics bodies and coins. The file-name prefix is required, because it's in another file. Then the player is moved to the `P` in the level.
- `window.run()` starts the [engine loop](game-loop.md): from now on Forge calls `update(dt)` and `draw()` every frame, until the window closes.

```cpp
void update(float dt){
    player.handleInput();
    physics.step(dt);
    player.checkFall();
    level.collectCoins(player.hero.bounds());
    followPlayer(dt);
}
```

This order matters:

1. **Input first**: read the keys and set the player's velocity.
2. **Then physics**: move everything by its velocity, with gravity and collisions.
3. **Then the results**: did the player fall into a pit? Did they touch a coin?
4. **Last, the camera**, so it follows where the player is now.

`player.hero` is the global variable `hero` from `player.frg`, and `.bounds()` is its rectangle.

```cpp
void followPlayer(float dt){
    float half = window.width() / 2.0;
    float x = math.clamp(player.hero.center().x, half, level.tiles.width() - half);
    camera.target = camera.target.lerp(vec2(x, window.height() / 2.0), 5 * dt);
}
```

The camera follows the player smoothly:

- `x` is where the camera *wants* to look: the middle of the player. `math.clamp` keeps it at least half a window away from both ends of the level, so the camera never shows the empty space past the edges.
- The camera only follows sideways: its `y` stays in the middle of the window, because the level is exactly one window tall.
- `lerp` moves the target **part of the way** there each frame (`5 * dt` is about 8% at 60 frames per second). Far away, that's a big step; close by, a small one. That's what makes it glide instead of jerk. A bigger number than 5 follows more tightly.

```cpp
void draw(){
    graphics2d.setCamera(camera);
    level.draw();
    player.draw();
    graphics2d.resetCamera();
    ...
}
```

`setCamera` makes everything drawn after it go through the camera: the level and the player are drawn at their **world** positions (a platform at x = 2000 really is at 2000), and the camera decides which part shows. `resetCamera` switches back to plain **screen** positions, so the text stays in the same place however far the player runs.

The level is drawn first, then the player on top of it, then the text on top of everything. Later drawing covers earlier drawing.

```cpp
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);
```

This lines text up against the right edge: measure it, then start it `width` pixels from the right.

```cpp
    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2.0, 200, 48, color.WHITE);
    }
```

When the list of coins is empty, a centered win message is drawn: start half its width left of the middle.

## player.frg

```cpp
const float MOVE_SPEED = 320;
const float JUMP_SPEED = 620;
vec2 start = vec2(80, 380);
```

`start` is where the player starts, and where it goes back to after falling. It's a variable, not a constant, because `main.frg` moves it to the `P` in the level. Constants keep the "tuning numbers" in one place. Try changing them: a higher `JUMP_SPEED` means higher jumps.

```cpp
class Player : Body {
    color tint = color.ORANGE;
    int deaths = 0;
    ...
}
```

`Player` **inherits** from the physics `Body`, so a player *is* a body: it has `position`, `size`, `velocity`, `onGround` and the rest. It adds its own `tint` and `deaths`.

```cpp
    void respawn() {
        position = start;
        velocity = vec2.ZERO;
        deaths++;
        flash();
    }
```

Inside a method, `position` and `velocity` are the player's own fields (inherited from `Body`). `flash()` starts a task.

```cpp
    task flash() {
        for (int i = 0; i < 5; i++) {
            tint = color.WHITE;
            wait(0.08);
            tint = color.ORANGE;
            wait(0.08);
        }
    }
```

A [task](../language/11-tasks.md): it switches the color, waits 0.08 seconds, switches back, and so on, while the game keeps running. Without tasks, this would need a timer variable and some state-tracking.

```cpp
Player hero = makeHero();

Player makeHero(){
    Player p = Player(start.x, start.y, 36, 36);
    physics.add(p);
    return p;
}
```

A global variable's starting value can call a function, even one written below it. `Player(...)` uses `Body`'s constructor (x, y, width, height), since `Player` doesn't have its own. `physics.add` puts it in the physics world so `physics.step` moves it.

```cpp
void handleInput(){
    float direction = 0;
    if (input.keyDown(Key.LEFT) or input.keyDown(Key.A)) {
        direction -= 1;
    }
    if (input.keyDown(Key.RIGHT) or input.keyDown(Key.D)) {
        direction += 1;
    }
    hero.velocity.x = direction * MOVE_SPEED;
```

`direction` ends up -1 (left), 1 (right) or 0 (neither, or both). Setting `velocity.x` directly makes movement instant and responsive, with no sliding.

```cpp
    bool jumpPressed = input.keyPressed(Key.SPACE) or input.keyPressed(Key.W) or input.keyPressed(Key.UP);
    if (jumpPressed and hero.onGround) {
        hero.velocity.y = -JUMP_SPEED;
    }
```

`keyPressed` (not `keyDown`) means one jump per press. `onGround` was set by the previous physics step, so you can only jump while standing on something. The velocity is **negative** because up is negative y on the screen.

```cpp
    bool jumpReleased = input.keyReleased(Key.SPACE) or input.keyReleased(Key.W) or input.keyReleased(Key.UP);
    if (jumpReleased and hero.velocity.y < 0) {
        hero.velocity.y *= 0.5;
    }
}
```

**Variable jump height**: letting go of the button while still rising halves the upward speed. A tap gives a short hop, and holding gives a full jump. Small touches like this make a platformer feel good.

```cpp
void checkFall(){
    if (hero.position.y > window.height() + 100) {
        hero.respawn();
    }
}
```

If the player falls more than 100 pixels below the bottom of the window, it's sent back to the start.

```cpp
void draw(){
    graphics2d.drawRect(hero.bounds(), hero.tint);
    float look = hero.velocity.x > 0 ? 4 : hero.velocity.x < 0 ? -4 : 0;
    float eyeY = hero.position.y + 12;
    graphics2d.drawRect(hero.position.x + 8 + look, eyeY, 6, 8, color.BLACK);
    graphics2d.drawRect(hero.position.x + 22 + look, eyeY, 6, 8, color.BLACK);
}
```

The body is a colored rectangle. The eyes are two small black rectangles, shifted 4 pixels in the direction of movement, so the cube "looks" where it's going. The two `? :` choose 4, -4 or 0.

## level.frg

The level is **drawn as text**, with a [tile map](../libraries/tilemap.md): each character is one 36 × 36 square of the level.

```cpp
TileMap tiles = TileMap("""
#                                                          #
#                                                          #
#                                                          #
#                                                          #
#                                             C            #
#                            C               ====          #
#                      C    ####                           #
#         C           ====                C                #
#        ====                            ====              #
#    C              C                  C             C     #
#   ====          =====         C    =====          ====   #
#                                                          #
# P     C                                               C  #
############    ##############     ###########    ##########
############    ##############     ###########    ##########
""", 36);
```

| Character | Means |
|---|---|
| `#` | ground and walls |
| `=` | a one-way platform: you can jump up through it and land on top |
| `C` | a coin |
| `P` | where the player starts |
| (a space) | nothing |

The meaning isn't built into Forge: it's what `build` and `draw` below do with each character. To change the level, change the text. The 60 columns × 36 pixels make it 2160 pixels wide, and the 15 rows make it exactly one window (540 pixels) tall.

```cpp
void build(){
    tiles.addBodies("#");
    tiles.addBodies("=", true);
    for (vec2 corner in tiles.find("C")) {
        coins.add(corner + vec2(tiles.tileSize / 2, tiles.tileSize / 2));
    }
    totalCoins = coins.length;
    start = tiles.find("P")[0];
}
```

- `addBodies("#")` makes static physics bodies for the `#` tiles. Neighbouring tiles are **merged**: a row of 12 ground tiles becomes one long body, not 12 small ones. `addBodies("=", true)` does the same for the planks, and `true` makes them [one-way](../libraries/physics.md#one-way-platforms).
- `find("C")` gives the top-left corner of every `C` tile. Each coin is kept as the middle of its tile, half a tile further right and down.
- `find("P")[0]` is the first (and only) `P`: the player's start.

```cpp
void collectCoins(rect playerBox){
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
            ...
        }
    }
}
```

Each coin is checked as a 20×20 box around its center. The loop goes **backwards**: removing item `i` moves every later item down one place, and going backwards means those have already been checked.

```cpp
void draw(){
    tiles.draw("#", color(70, 86, 120));
    for (int row = 0; row < tiles.rows(); row++) {
        for (int column = 0; column < tiles.columns(); column++) {
            if (tiles.get(column, row) == "#" and tiles.get(column, row - 1) != "#") {
                rect r = tiles.tileRect(column, row);
                graphics2d.drawRect(r.x, r.y, r.width, 5, color(110, 200, 130));
            }
        }
    }
    for (vec2 p in tiles.find("=")) {
        graphics2d.drawRect(p.x, p.y, tiles.tileSize, 10, color(170, 130, 90));
    }
    ...
}
```

- `tiles.draw("#", color)` fills every `#` tile with one color.
- The two loops go through every tile and add a strip of green **grass** on top of each `#` that has nothing above it. `get(column, row - 1)` is the tile above; outside the map, `get` gives `" "`, so the top row works too.
- The one-way platforms are drawn as thin wooden planks.
- Then the coins: a gold circle with an orange outline each.

With an image of tiles instead of colors, `tiles.drawTiles(tileset, 16, "#=")` draws each `#` as the tileset's first 16 × 16 picture and each `=` as the second.

## Particles: dust and sparkles

Two [particle emitters](../libraries/particles.md) make the game feel more alive:

- **Dust when landing** (`player.frg`). `handleInput` remembers `fallSpeed = hero.velocity.y` *before* `physics.step`, because landing sets the velocity to 0. After the step, `checkLanding(dt)` sees that the player is on the ground now but wasn't last frame, and if it was falling fast enough, puts the `dust` emitter at the player's feet and calls `dust.burst(14)`.

```cpp
void checkLanding(float dt){
    if (hero.onGround and !wasOnGround and fallSpeed > 250) {
        dust.position = vec2(hero.center().x, hero.position.y + hero.size.y);
        dust.burst(14);
    }
    wasOnGround = hero.onGround;
    dust.update(dt);
}
```

- **Sparkles when collecting a coin** (`level.frg`). `collectCoins` moves the `sparkles` emitter to the coin and bursts 24 golden particles that fall and fade. `updateEffects(dt)` updates them, and `draw` draws them.

Both emitters are made by small functions (`makeDust`, `makeSparkles`) that set their colors, sizes, speed and gravity. They're drawn between `setCamera` and `resetCamera`, so they stay in the right place in the level.

## Things to try

1. Show the death count on screen (`player.hero.deaths`).
2. Add another coin and a platform to reach it, by typing a `C` and some `=` into the level text. Then make the level longer: add more columns to every row.
3. Make the jump higher, or gravity weaker (`physics.setGravity(vec2(0, 700))` in `main`).
4. Add a timer: `window.time()` at the start, and show how long the run took when all coins are collected.
5. A moving platform: keep a `Body` in a variable and change its `position.x` in `update` using `math.sin(window.time())`.
6. Swap the rectangle for a sprite (see [Sprites and animation](sprites-and-animation.md)) and add a jump sound (see [`<audio>`](../libraries/audio.md)).
7. Add a second level and a title screen with [`<scenes>`](../libraries/scenes.md).
8. Zoom the camera in a little (`camera.zoom = 1.5;` in `main`), or let it follow the player up and down too.
