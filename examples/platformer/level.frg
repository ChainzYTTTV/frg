#include <graphics2d>
#include <physics>

list<Body> platforms = [];
list<vec2> coins = [];
int totalCoins = 0;
int collected = 0;

void build(){
    // The ground has a pit in the middle.
    addPlatform(0, 500, 400, 40);
    addPlatform(540, 500, 420, 40);

    // Platforms to jump between.
    addPlatform(140, 400, 160, 20);
    addPlatform(380, 320, 160, 20);
    addPlatform(620, 240, 160, 20);
    addPlatform(410, 150, 100, 20);

    // Walls at the edges of the screen.
    addPlatform(-20, 0, 20, 540);
    addPlatform(960, 0, 20, 540);

    coins = [vec2(220, 370), vec2(460, 290), vec2(700, 210), vec2(460, 120), vec2(880, 470)];
    totalCoins = coins.length;
}

void addPlatform(float x, float y, float width, float height){
    Body b = Body(x, y, width, height);
    b.isStatic = true;
    physics.add(b);
    platforms.add(b);
}

void collectCoins(rect playerBox){
    // Go backwards so removing a coin doesn't skip the next one.
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
        }
    }
}

void draw(){
    for (Body p in platforms) {
        graphics2d.drawRect(p.bounds(), color(70, 86, 120));
        graphics2d.drawRect(p.position.x, p.position.y, p.size.x, 5, color(110, 200, 130));
    }
    for (vec2 c in coins) {
        graphics2d.drawCircle(c, 10, color.GOLD);
        graphics2d.drawCircleOutline(c.x, c.y, 10, color.ORANGE);
    }
}
