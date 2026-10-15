#include <window>
#include <graphics2d>
#include <physics>
#include <math>

```
  A tiny platformer: move with the arrow keys (or A / D), jump with Space (or W / Up).
  Collect all the coins. Don't fall in the pits!

  This game uses the engine loop: window.run() calls update(dt) and draw() every frame.
  The level is wider than the window, so a camera follows the player.
```

// The camera looks at `target` (a point in the level) and shows it at `offset` (the middle of the window).
Camera2D camera = Camera2D(vec2(480, 270), vec2(480, 270));

int main(){
    window.open(960, 540, "Forge Platformer");
    window.setBackground(color(24, 26, 40));
    level.build();
    window.run();
    return 0;
}

void update(float dt){
    player.handleInput();
    physics.step(dt);
    player.checkFall();
    level.collectCoins(player.hero.bounds());
    followPlayer(dt);
}

// The camera glides toward the player, but stops at the ends of the level so it never shows past them.
void followPlayer(float dt){
    float half = window.width() / 2.0;
    float x = math.clamp(player.hero.center().x, half, level.WIDTH - half);
    camera.target = camera.target.lerp(vec2(x, window.height() / 2.0), 5 * dt);
}

void draw(){
    graphics2d.setCamera(camera);    // the level and the player are drawn where the camera looks...
    level.draw();
    player.draw();
    graphics2d.resetCamera();        // ...and the text stays in the same place on the screen

    graphics2d.drawText("Arrows / A D to move, Space to jump", 20, 16, 20, color.LIGHTGRAY);
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);

    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2.0, 200, 48, color.WHITE);
    }
}
