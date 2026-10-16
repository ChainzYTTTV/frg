#include <window>
#include <graphics3d>
#include <assets>

```
  Turns the world into meshes the graphics card draws in one go: one per 16 x 16 x 16 section.
  A face is only added where a block touches air or a see-through block, so the hidden ones inside the
  ground are never sent at all. Changing a block rebuilds just its section (and the next one, at an edge),
  and sections behind the camera aren't drawn.
```

const int CHUNK = 16;
const int CHUNKS = world.SIZE / CHUNK;     // sections along each side
const int LAYERS = world.HEIGHT / CHUNK;   // sections on top of each other

Texture atlas = assets.loadTexture("blocks.png");
list<Mesh> meshes = [];

// The 6 faces: up, down, +x, -x, +z, -z. For each: the neighbor's direction, the 4 corners (counter-clockwise seen
// from outside, as 0/1 offsets from the block's corner), each corner's place in the tile (s across, t up), and a shade.
list<int> DIR = [0, 1, 0,   0, -1, 0,   1, 0, 0,   -1, 0, 0,   0, 0, 1,   0, 0, -1];
list<int> CORNERS = [
    0, 1, 0,   0, 1, 1,   1, 1, 1,   1, 1, 0,
    0, 0, 0,   1, 0, 0,   1, 0, 1,   0, 0, 1,
    1, 0, 0,   1, 1, 0,   1, 1, 1,   1, 0, 1,
    0, 0, 0,   0, 0, 1,   0, 1, 1,   0, 1, 0,
    0, 0, 1,   1, 0, 1,   1, 1, 1,   0, 1, 1,
    0, 0, 0,   0, 1, 0,   1, 1, 0,   1, 0, 0
];
list<int> ST = [
    0, 0,   0, 1,   1, 1,   1, 0,
    0, 0,   1, 0,   1, 1,   0, 1,
    0, 0,   0, 1,   1, 1,   1, 0,
    0, 0,   1, 0,   1, 1,   0, 1,
    0, 0,   1, 0,   1, 1,   0, 1,
    0, 0,   0, 1,   1, 1,   1, 0
];
list<float> SHADE = [1.0, 0.5, 0.8, 0.8, 0.65, 0.65];
list<int> TRIANGLES = [0, 1, 2, 0, 2, 3];   // the two triangles of a face, as corners
int drawn = 0;   // sections drawn last frame
double rebuildMs = 0;   // how long the last block change took to rebuild

// Whether the face between `block` and its neighbor `other` can be seen.
bool showFace(int block, int other){
    if (other == blocks.AIR) {
        return true;
    }
    if (!blocks.seeThrough(other)) {
        return false;
    }
    return other != block or block == blocks.LEAVES;  // glass next to glass hides the face between them
}

Mesh build(int cx, int cy, int cz){
    list<float> positions = [];
    list<float> uvs = [];
    list<float> shades = [];
    float tile = 1.0 / 8;
    float edge = 0.5 / 256;  // half a pixel in, so the next tile never shows at the edges
    for (int y in cy * CHUNK..cy * CHUNK + CHUNK - 1) {
        for (int z in cz * CHUNK..cz * CHUNK + CHUNK - 1) {
            for (int x in cx * CHUNK..cx * CHUNK + CHUNK - 1) {
                int block = world.get(x, y, z);
                if (block == blocks.AIR) {
                    continue;
                }
                for (int f in 0..5) {
                    int other = world.get(x + DIR[f * 3], y + DIR[f * 3 + 1], z + DIR[f * 3 + 2]);
                    if (!showFace(block, other)) {
                        continue;
                    }
                    int t = f == 0 ? blocks.TOP[block] : f == 1 ? blocks.BOTTOM[block] : blocks.SIDE[block];
                    int column = t % 8;
                    int row = t / 8;
                    float u0 = column * tile + edge;
                    float v0 = row * tile + edge;
                    float size = tile - 2 * edge;
                    for (int k in TRIANGLES) {
                        int c = f * 12 + k * 3;
                        positions.add(x + CORNERS[c]);
                        positions.add(y + CORNERS[c + 1]);
                        positions.add(z + CORNERS[c + 2]);
                        int s = f * 8 + k * 2;
                        uvs.add(u0 + ST[s] * size);
                        uvs.add(v0 + (1 - ST[s + 1]) * size);
                        shades.add(SHADE[f]);
                    }
                }
            }
        }
    }
    return graphics3d.makeMesh(positions, uvs, shades, atlas);
}

void buildAll(){
    meshes = [];
    for (int cy in 0..LAYERS - 1) {
        for (int cz in 0..CHUNKS - 1) {
            for (int cx in 0..CHUNKS - 1) {
                meshes.add(build(cx, cy, cz));
            }
        }
    }
}

void rebuild(int cx, int cy, int cz){
    if (cx >= 0 and cx < CHUNKS and cy >= 0 and cy < LAYERS and cz >= 0 and cz < CHUNKS) {
        meshes[(cy * CHUNKS + cz) * CHUNKS + cx] = build(cx, cy, cz);
    }
}

// After the block at (x, y, z) changed: its section, and the one next to it if it's on an edge.
void blockChanged(int x, int y, int z){
    int cx = x / CHUNK;
    int cy = y / CHUNK;
    int cz = z / CHUNK;
    double start = window.time();
    rebuild(cx, cy, cz);
    if (x % CHUNK == 0) { rebuild(cx - 1, cy, cz); }
    if (x % CHUNK == CHUNK - 1) { rebuild(cx + 1, cy, cz); }
    if (y % CHUNK == 0) { rebuild(cx, cy - 1, cz); }
    if (y % CHUNK == CHUNK - 1) { rebuild(cx, cy + 1, cz); }
    if (z % CHUNK == 0) { rebuild(cx, cy, cz - 1); }
    if (z % CHUNK == CHUNK - 1) { rebuild(cx, cy, cz + 1); }
    rebuildMs = (window.time() - start) * 1000;
}

// Draws the sections the camera can see: ones inside a cone a bit wider than the screen. Each section is
// treated as a ball around its middle; moving the cone's tip back by (radius / sin of its angle) means
// a ball that touches the cone has its middle inside the moved cone.
void drawAll(vec3 eye, vec3 look){
    float radius = CHUNK * 0.87;   // half a section's diagonal
    float cosAngle = 0.55;         // the cone's half angle is 57 degrees (70 up and down, wider across)
    float sinAngle = 0.84;
    vec3 tip = eye - look * (radius / sinAngle);
    drawn = 0;
    for (int i in 0..meshes.length - 1) {
        Mesh m = meshes[i];
        if (m.vertexCount() == 0) {
            continue;
        }
        int cx = i % CHUNKS;
        int cz = (i / CHUNKS) % CHUNKS;
        int cy = i / (CHUNKS * CHUNKS);
        vec3 middle = vec3(cx * CHUNK + CHUNK / 2, cy * CHUNK + CHUNK / 2, cz * CHUNK + CHUNK / 2);
        vec3 v = middle - tip;
        if (v.dot(look) < v.length() * cosAngle) {
            continue;
        }
        m.draw(vec3.ZERO);
        drawn++;
    }
}

int triangles(){
    int n = 0;
    for (Mesh m in meshes) {
        n += m.vertexCount() / 3;
    }
    return n;
}
