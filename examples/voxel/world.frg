#include <math>
#include <random>

```
  The world: SIZE x HEIGHT x SIZE blocks, kept in one list. The block at (x, y, z) is at
  position (y * SIZE + z) * SIZE + x. y goes up; the ground is around y = 24.
```

const int SIZE = 128;
const int HEIGHT = 64;

list<int> cells = [];

bool inside(int x, int y, int z){
    return x >= 0 and x < SIZE and y >= 0 and y < HEIGHT and z >= 0 and z < SIZE;
}

// The block at (x, y, z). Outside the world it's air, except below it, which is solid.
int get(int x, int y, int z){
    if (y < 0) {
        return blocks.BEDROCK;
    }
    if (!inside(x, y, z)) {
        return blocks.AIR;
    }
    return cells[(y * SIZE + z) * SIZE + x];
}

void set(int x, int y, int z, int block){
    if (inside(x, y, z)) {
        cells[(y * SIZE + z) * SIZE + x] = block;
    }
}

// The y of the highest block in a column that isn't air (-1 if there's none).
int topAt(int x, int z){
    for (int y = HEIGHT - 1; y >= 0; y--) {
        if (get(x, y, z) != blocks.AIR) {
            return y;
        }
    }
    return -1;
}

// ---- making the world

// Smooth random hills: random heights on a grid every `spacing` blocks, blended between the grid points.
list<float> noiseGrid(int spacing){
    list<float> grid = [];
    int n = SIZE / spacing + 2;
    for (int i in 1..n * n) {
        grid.add(random.value());
    }
    return grid;
}

float smooth(float t){
    return t * t * (3 - 2 * t);
}

float noiseAt(list<float> grid, int spacing, int x, int z){
    int n = SIZE / spacing + 2;
    int gx = x / spacing;
    int gz = z / spacing;
    float fx = smooth(float(x % spacing) / spacing);
    float fz = smooth(float(z % spacing) / spacing);
    float a = grid[gz * n + gx];
    float b = grid[gz * n + gx + 1];
    float c = grid[(gz + 1) * n + gx];
    float d = grid[(gz + 1) * n + gx + 1];
    float top = a + (b - a) * fx;
    float bottom = c + (d - c) * fx;
    return top + (bottom - top) * fz;
}

void generate(int seed){
    random.seed(seed);
    cells = [];
    for (int i in 1..SIZE * SIZE * HEIGHT) {
        cells.add(blocks.AIR);
    }
    list<float> big = noiseGrid(32);
    list<float> medium = noiseGrid(16);
    list<float> small = noiseGrid(8);
    for (int z in 0..SIZE - 1) {
        for (int x in 0..SIZE - 1) {
            float h = 16 + noiseAt(big, 32, x, z) * 14 + noiseAt(medium, 16, x, z) * 6 + noiseAt(small, 8, x, z) * 2;
            int height = int(h);
            for (int y in 0..height) {
                int block = blocks.STONE;
                if (y == 0) {
                    block = blocks.BEDROCK;
                } else if (y == height) {
                    block = height <= 19 ? blocks.SAND : blocks.GRASS;
                } else if (y >= height - 3) {
                    block = height <= 19 ? blocks.SAND : blocks.DIRT;
                } else if (random.chance(0.02)) {
                    block = blocks.GRAVEL;
                }
                set(x, y, z, block);
            }
        }
    }
    for (int i in 1..90) {
        plantTree(random.range(3, SIZE - 4), random.range(3, SIZE - 4));
    }
}

// An oak: a trunk of 4 to 6 logs, with a round top of leaves.
void plantTree(int x, int z){
    int ground = topAt(x, z);
    if (ground < 0 or get(x, ground, z) != blocks.GRASS or ground + 9 >= HEIGHT) {
        return;
    }
    for (int dz in -2..2) {  // not right next to another tree
        for (int dx in -2..2) {
            if (get(x + dx, ground + 1, z + dz) != blocks.AIR) {
                return;
            }
        }
    }
    int trunk = random.range(4, 6);
    int top = ground + trunk;
    for (int y in -1..2) {
        int r = y <= 0 ? 2 : 1;
        for (int dz in -r..r) {
            for (int dx in -r..r) {
                bool corner = (dx == -r or dx == r) and (dz == -r or dz == r);
                if (corner and (y == 2 or random.chance(0.5))) {
                    continue;
                }
                if (get(x + dx, top + y, z + dz) == blocks.AIR) {
                    set(x + dx, top + y, z + dz, blocks.LEAVES);
                }
            }
        }
    }
    for (int y in ground + 1..top) {
        set(x, y, z, blocks.LOG);
    }
    set(x, ground, z, blocks.DIRT);
}
