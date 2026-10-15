#include <graphics2d>
#include <physics>
#include <particles>
#include <tilemap>

// The level, drawn with characters. Each character is one 36 x 36 tile:
//   #  ground and walls          =  a platform you can jump up through
//   C  a coin                    P  where the player starts
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

list<vec2> coins = [];
int totalCoins = 0;
int collected = 0;
vec2 start = vec2(0, 0);

// Golden sparkles where a coin was picked up.
Emitter sparkles = makeSparkles();

Emitter makeSparkles(){
    Emitter e = Emitter(vec2(0, 0));
    e.lifetime = 0.6;
    e.speed = 140;
    e.gravity = vec2(0, 250);
    e.startColor = color.GOLD;
    e.endColor = color(255, 255, 160, 0);
    e.startSize = 7;
    e.endSize = 1;
    return e;
}

void build(){
    // Neighbouring # tiles become a few big static bodies, not hundreds of small ones.
    tiles.addBodies("#");
    tiles.addBodies("=", true);     // true: one-way platforms

    // find gives the top-left corner of each tile; coins are kept as the middle of their tile.
    for (vec2 corner in tiles.find("C")) {
        coins.add(corner + vec2(tiles.tileSize / 2, tiles.tileSize / 2));
    }
    totalCoins = coins.length;
    start = tiles.find("P")[0];
}

void collectCoins(rect playerBox){
    // Go backwards so removing a coin doesn't skip the next one.
    for (int i = coins.length - 1; i >= 0; i--) {
        vec2 c = coins[i];
        if (playerBox.overlaps(rect(c.x - 10, c.y - 10, 20, 20))) {
            coins.removeAt(i);
            collected++;
            sparkles.position = c;
            sparkles.burst(24);
        }
    }
}

void updateEffects(float dt){
    sparkles.update(dt);
}

void draw(){
    tiles.draw("#", color(70, 86, 120));
    // Grass on every # tile that has nothing above it.
    for (int row = 0; row < tiles.rows(); row++) {
        for (int column = 0; column < tiles.columns(); column++) {
            if (tiles.get(column, row) == "#" and tiles.get(column, row - 1) != "#") {
                rect r = tiles.tileRect(column, row);
                graphics2d.drawRect(r.x, r.y, r.width, 5, color(110, 200, 130));
            }
        }
    }
    // One-way platforms are drawn as thin planks.
    for (vec2 p in tiles.find("=")) {
        graphics2d.drawRect(p.x, p.y, tiles.tileSize, 10, color(170, 130, 90));
    }
    for (vec2 c in coins) {
        graphics2d.drawCircle(c, 10, color.GOLD);
        graphics2d.drawCircleOutline(c.x, c.y, 10, color.ORANGE);
    }
    sparkles.draw();
}
