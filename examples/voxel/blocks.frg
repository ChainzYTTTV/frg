```
  Every kind of block: its name, which tiles of blocks.png it shows, and whether you can see through it.
  The sprite sheet has 8 tiles of 32 x 32 pixels in each row.
```

const int AIR = 0;
const int GRASS = 1;
const int DIRT = 2;
const int STONE = 3;
const int COBBLESTONE = 4;
const int LOG = 5;
const int LEAVES = 6;
const int PLANKS = 7;
const int SAND = 8;
const int GLASS = 9;
const int BRICKS = 10;
const int GRAVEL = 11;
const int WHITE_WOOL = 12;
const int RED_WOOL = 13;
const int BLUE_WOOL = 14;
const int YELLOW_WOOL = 15;
const int BEDROCK = 16;
const int COUNT = 17;

list<string> NAMES = ["Air", "Grass", "Dirt", "Stone", "Cobblestone", "Oak Log", "Oak Leaves", "Oak Planks", "Sand", "Glass",
                      "Bricks", "Gravel", "White Wool", "Red Wool", "Blue Wool", "Yellow Wool", "Bedrock"];

// The tile each block shows on its top, its sides and its bottom.
list<int> TOP = [0, 0, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17];
list<int> SIDE = [0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17];
list<int> BOTTOM = [0, 2, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17];

// Blocks with see-through pixels: the faces of the blocks behind them still have to be drawn.
bool seeThrough(int block){
    return block == AIR or block == LEAVES or block == GLASS;
}
