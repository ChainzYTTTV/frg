# `<tilemap>`: Levels Drawn as Text

```cpp
#include <tilemap>
```

Instead of placing every platform with numbers, **draw the level with characters**. Each character is one square **tile** of the level:

```cpp
TileMap level = TileMap("""
##################
#                #
#   C      ===   #
#  ===           #
# P       C      #
##################
""", 32);
```

What each character means is up to your game. Here `#` is a wall, `=` a platform, `C` a coin and `P` where the player starts, and a space is nothing. The `32` is the size of a tile in pixels, so this level is 18 × 32 = 576 pixels wide.

Then use the map to make the level:

```cpp
void build(){
    level.addBodies("#");                    // walls become static physics bodies
    level.addBodies("=", true);              // one-way platforms
    for (vec2 corner in level.find("C")) {   // coins
        coins.add(corner + vec2(16, 16));    // the middle of the tile
    }
    hero.position = level.find("P")[0];      // the player's start
}

void draw(){
    level.draw("#", color.DARKGRAY);         // walls as gray squares
    level.draw("=", color.BROWN);
}
```

## Making one

| | |
|---|---|
| `TileMap(string text, float tileSize)` | a map from text: one line per row of tiles. `"""raw text"""` is the easiest way to write it in your code |
| `tilemap.load(string path, float tileSize)` | a map from a text file in the game's folder, like `tilemap.load("level1.txt", 32)` |

The top-left character is column 0, row 0. Lines can have different lengths: short lines are filled with spaces. Blank lines at the end are left out. Characters are single letters and symbols (not `é` or emoji).

Keeping each level in its own file makes it easy to have many levels: `level1.txt`, `level2.txt`...

## Reading and changing tiles

| Method | Gives back | What it does |
|---|---|---|
| `columns()`, `rows()` | `int` | the size of the map in tiles |
| `width()`, `height()` | `float` | the size of the map in pixels |
| `float tileSize` | | the size of one tile, in pixels (a field) |
| `get(int column, int row)` | `string` | the tile there, like `"#"`. Outside the map it's `" "`, so checking neighbours at the edges is safe |
| `set(int column, int row, string tile)` | — | changes a tile, like `level.set(5, 2, " ")` to remove it. Outside the map, the game stops with an error |
| `find(string tiles)` | `list<vec2>` | the **top-left corner**, in pixels, of every tile that's one of these characters, row by row. `find("C")` finds the coins; `find("CD")` finds both `C` and `D` |
| `tileRect(int column, int row)` | `rect` | the rectangle of a tile, in pixels |
| `columnAt(float x)`, `rowAt(float y)` | `int` | which column and row a point in the level is in: `level.get(level.columnAt(p.x), level.rowAt(p.y))` is the tile under `p` |

## Walls and physics

| Method | Gives back | What it does |
|---|---|---|
| `addBodies(string tiles, bool oneWay = false)` | `list<Body>` | makes static [physics](physics.md) bodies for those tiles, adds them to the physics world, and gives them back. With `true`, they're one-way platforms |
| `solidAreas(string tiles)` | `list<rect>` | the rectangles `addBodies` would use, if you want to do something else with them |

Neighbouring tiles are **merged** into bigger rectangles: a row of 30 wall tiles becomes one body, and a solid block becomes one box. That keeps the number of bodies small, which keeps physics fast, and stops players from catching on the seams between tiles.

Because `addBodies` uses `Body`, a file that includes `<tilemap>` can use the bodies it gives back without including `<physics>` itself, but needs `#include <physics>` to call `physics.step` and to write the type `Body`.

## Drawing

| Method | What it does |
|---|---|
| `draw(string tiles, color c)` | fills every tile that's one of these characters with a color |
| `drawTiles(Texture tileset, int sourceSize, string tiles)` | draws those tiles with pictures from a **tileset** image |

A tileset is one image with many tile pictures side by side, each `sourceSize` pixels square. The first character in `tiles` uses the first picture (top-left), the second character the next picture to its right, and so on (then the next row):

```cpp
Texture tileset = assets.loadTexture("tiles.png");    // 16 x 16 pictures: grass, dirt, plank

void draw(){
    level.drawTiles(tileset, 16, "#.=");     // # is picture 0, . is picture 1, = is picture 2
}
```

Pictures are stretched to the map's `tileSize`, and stay sharp (good for pixel art).

Drawing goes through the [2D camera](graphics2d.md#camera-scrolling-and-zooming), so a level wider than the window scrolls like everything else. To stop the camera at the level's edges, use `level.width()`:

```cpp
float half = window.width() / 2.0;
camera.target.x = math.clamp(hero.center().x, half, level.width() - half);
```

For your own extra drawing, go through the tiles with two loops:

```cpp
for (int row = 0; row < level.rows(); row++) {
    for (int column = 0; column < level.columns(); column++) {
        if (level.get(column, row) == "#" and level.get(column, row - 1) != "#") {
            rect r = level.tileRect(column, row);
            graphics2d.drawRect(r.x, r.y, r.width, 5, color.GREEN);    // grass on top
        }
    }
}
```

## Things to know

- A variable can't be called `map` (that's the type `map<K, V>`), so call yours `level`, `tiles` or `world`.
- [examples/platformer](../../examples/platformer/level.frg) builds its whole level from a tile map. The [walkthrough](../guides/platformer-walkthrough.md#levelfrg) explains it.
