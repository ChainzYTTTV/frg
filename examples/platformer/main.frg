#include <window>
#include <graphics2d>
#include <physics>

```
  A tiny platformer: move with the arrow keys (or A / D), jump with Space (or W / Up).
  Collect all the coins. Don't fall in the pit!

  This game uses the engine loop: window.run() calls update(dt) and draw() every frame.
```

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
}

void draw(){
    level.draw();
    player.draw();

    graphics2d.drawText("Arrows / A D to move, Space to jump", 20, 16, 20, color.LIGHTGRAY);
    string coins = "Coins: {level.collected}/{level.totalCoins}";
    graphics2d.drawText(coins, 940 - graphics2d.textWidth(coins, 24), 14, 24, color.GOLD);

    if (level.coins.isEmpty()) {
        string win = "You got them all!";
        graphics2d.drawText(win, 480 - graphics2d.textWidth(win, 48) / 2, 200, 48, color.WHITE);
    }
}
