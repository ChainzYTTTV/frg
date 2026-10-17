#include <window>
#include <graphics2d>
#include <input>
#include <assets>
#include <math>

```
  Everything drawn flat on top of the world: the crosshair, the hotbar of 9 blocks, the creative menu
  (E opens it: click a block to put it in the selected slot) and the F3 debug numbers.
```

list<int> hotbar = [blocks.GRASS, blocks.DIRT, blocks.STONE, blocks.COBBLESTONE, blocks.LOG,
                    blocks.PLANKS, blocks.LEAVES, blocks.GLASS, blocks.BRICKS];
int selected = 0;
bool menuOpen = false;
bool showDebug = false;

const int SLOT = 52;       // hotbar slot size in pixels
const int CELL = 64;       // creative menu cell size
const int MENU_COLUMNS = 8;

int selectedBlock(){
    return hotbar[selected];
}

// Draws a block's side tile from the sprite sheet into a square on the screen.
void drawIcon(int block, float x, float y, float size){
    int t = blocks.SIDE[block];
    mesher.atlas.drawPart(rect((t % 8) * 32, (t / 8) * 32, 32, 32), rect(x, y, size, size));
}

void update(){
    // 1 to 9 pick a slot; the mouse wheel moves through them.
    list<Key> numbers = [Key.NUM1, Key.NUM2, Key.NUM3, Key.NUM4, Key.NUM5, Key.NUM6, Key.NUM7, Key.NUM8, Key.NUM9];
    for (int i in 0..8) {
        if (input.keyPressed(numbers[i])) {
            selected = i;
        }
    }
    float wheel = input.mouseWheel();
    if (wheel < 0) {
        selected = (selected + 1) % 9;
    } else if (wheel > 0) {
        selected = (selected + 8) % 9;
    }
    if (input.keyPressed(Key.F3)) {
        showDebug = !showDebug;
    }
    if (input.keyPressed(Key.E)) {
        menuOpen = !menuOpen;
        input.lockMouse(!menuOpen);
    }
    // Escape closes the menu, or lets go of the mouse (to reach the window's X); a click grabs it again.
    if (input.keyPressed(Key.ESCAPE)) {
        menuOpen = false;
        input.lockMouse(false);
    }
    if (menuOpen) {
        if (input.mousePressed(MouseButton.LEFT)) {
            int block = blockUnderMouse();
            if (block != blocks.AIR) {
                hotbar[selected] = block;
            }
        }
        return;
    }
    if (!input.isMouseLocked()) {
        if (input.mousePressed(MouseButton.LEFT)) {
            input.lockMouse(true);  // clicking the window grabs the mouse again
        }
        return;
    }
    if (input.mousePressed(MouseButton.LEFT)) {
        player.breakTarget();
    }
    if (input.mousePressed(MouseButton.RIGHT)) {
        player.placeOnTarget(selectedBlock());
    }
    if (input.mousePressed(MouseButton.MIDDLE) and player.target.found) {  // pick the block you look at
        hotbar[selected] = world.get(player.target.x, player.target.y, player.target.z);
    }
}

// Where the creative menu starts on the screen, and the block the mouse is over (AIR if none).
float menuX(){
    return (window.width() - MENU_COLUMNS * CELL) / 2.0;
}

float menuY(){
    return window.height() / 2 - 120;
}

int blockUnderMouse(){
    vec2 m = input.mousePosition();
    int column = int(math.floor((m.x - menuX()) / CELL));
    int row = int(math.floor((m.y - menuY()) / CELL));
    if (column < 0 or column >= MENU_COLUMNS or row < 0) {
        return blocks.AIR;
    }
    int block = 1 + row * MENU_COLUMNS + column;
    return block < blocks.COUNT ? block : blocks.AIR;
}

void draw(){
    int w = window.width();
    int h = window.height();

    // crosshair
    if (!menuOpen) {
        graphics2d.drawRect(w / 2 - 10, h / 2 - 1, 20, 2, color(255, 255, 255, 200));
        graphics2d.drawRect(w / 2 - 1, h / 2 - 10, 2, 20, color(255, 255, 255, 200));
    }

    // hotbar
    float x0 = (w - 9 * SLOT) / 2.0;
    float y0 = h - SLOT - 12;
    graphics2d.drawRect(x0 - 4, y0 - 4, 9 * SLOT + 8, SLOT + 8, color(0, 0, 0, 120));
    for (int i in 0..8) {
        float x = x0 + i * SLOT;
        drawIcon(hotbar[i], x + 8, y0 + 8, SLOT - 16);
        graphics2d.drawRectOutline(x + 1, y0 + 1, SLOT - 2, SLOT - 2, color(90, 90, 90), 2);
    }
    graphics2d.drawRectOutline(x0 + selected * SLOT - 2, y0 - 2, SLOT + 4, SLOT + 4, color.WHITE, 3);
    string name = blocks.NAMES[selectedBlock()];
    graphics2d.drawText(name, (w - graphics2d.textWidth(name, 20)) / 2.0, y0 - 32, 20, color.WHITE);

    if (menuOpen) {
        drawMenu();
    }
    if (showDebug) {
        graphics2d.drawRect(6, 6, 470, 160, color(0, 0, 0, 120));
        graphics2d.drawFPS(12, 12);
        vec3 p = player.position;
        graphics2d.drawText("x {math.round(p.x * 10) / 10}  y {math.round(p.y * 10) / 10}  z {math.round(p.z * 10) / 10}", 12, 38, 20, color.WHITE);
        graphics2d.drawText("triangles: {mesher.triangles()}   sections drawn: {mesher.drawn} / {mesher.meshes.length}", 12, 62, 20, color.WHITE);
        graphics2d.drawText("last block change rebuilt in {mesher.rebuildMs:.2} ms", 12, 86, 20, color.WHITE);
        graphics2d.drawText(player.flying ? "flying (F)" : "walking (F to fly)", 12, 110, 20, color.WHITE);
        player.Hit t = player.target;
        if (t.found) {
            graphics2d.drawText("looking at: {blocks.NAMES[world.get(t.x, t.y, t.z)]} at {t.x}, {t.y}, {t.z}", 12, 134, 20, color.WHITE);
        }
    }
}

void drawMenu(){
    int w = window.width();
    int h = window.height();
    graphics2d.drawRect(0, 0, w, h, color(0, 0, 0, 110));
    float x0 = menuX();
    float y0 = menuY();
    int rows = (blocks.COUNT - 2) / MENU_COLUMNS + 1;
    graphics2d.drawRect(x0 - 16, y0 - 52, MENU_COLUMNS * CELL + 32, rows * CELL + 68, color(198, 198, 198));
    graphics2d.drawRectOutline(x0 - 16, y0 - 52, MENU_COLUMNS * CELL + 32, rows * CELL + 68, color(60, 60, 60), 3);
    graphics2d.drawText("Building Blocks", x0, y0 - 40, 24, color(60, 60, 60));
    int hovered = blockUnderMouse();
    for (int block in 1..blocks.COUNT - 1) {
        int i = block - 1;
        float x = x0 + (i % MENU_COLUMNS) * CELL;
        float y = y0 + (i / MENU_COLUMNS) * CELL;
        graphics2d.drawRect(x + 2, y + 2, CELL - 4, CELL - 4, block == hovered ? color(240, 240, 240) : color(139, 139, 139));
        drawIcon(block, x + 10, y + 10, CELL - 20);
    }
    if (hovered != blocks.AIR) {
        vec2 m = input.mousePosition();
        string name = blocks.NAMES[hovered];
        int tw = graphics2d.textWidth(name, 20);
        graphics2d.drawRect(m.x + 14, m.y - 30, tw + 12, 28, color(20, 10, 30, 230));
        graphics2d.drawText(name, m.x + 20, m.y - 26, 20, color.WHITE);
    }
    graphics2d.drawText("click a block to put it in the selected slot   -   E to close", x0, y0 + rows * CELL + 30, 18, color.WHITE);
}
