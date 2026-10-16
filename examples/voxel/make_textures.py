# Draws the block textures for examples/voxel: blocks.png, a 256 x 256 sprite sheet of 32 x 32 tiles (8 per row).
# Run it again (python make_textures.py) to remake blocks.png. Needs Pillow (pip install pillow).
import os, random
from PIL import Image

T = 32
sheet = Image.new('RGBA', (T * 8, T * 8), (0, 0, 0, 0))
rnd = random.Random(7)

def clamp(v):
    return max(0, min(255, int(v)))

def shade(c, amount):
    return tuple(clamp(x * amount) for x in c[:3]) + ((c[3],) if len(c) > 3 else (255,))

def noisy(base, spread, lo=0.82, hi=1.12):
    """A tile of one color with small random light and dark pixels, in 2x2 blocks for a chunky pixel-art look."""
    img = Image.new('RGBA', (T, T))
    for y in range(0, T, 2):
        for x in range(0, T, 2):
            f = rnd.uniform(lo, hi)
            if rnd.random() < spread:
                f *= rnd.choice([0.85, 1.12])
            c = shade(base, f)
            for dy in range(2):
                for dx in range(2):
                    img.putpixel((x + dx, y + dy), c)
    return img

def put(index, img):
    sheet.paste(img, ((index % 8) * T, (index // 8) * T))

GRASS = (95, 159, 53)
DIRT = (134, 96, 67)

# 0 grass top
put(0, noisy(GRASS, 0.35))
# 1 grass side: dirt with a green, ragged top edge
side = noisy(DIRT, 0.3)
for x in range(0, T, 2):
    depth = rnd.choice([4, 6, 6, 8, 10])
    for y in range(depth):
        c = shade(GRASS, rnd.uniform(0.85, 1.1))
        for dx in range(2):
            side.putpixel((x + dx, y), c)
put(1, side)
# 2 dirt
put(2, noisy(DIRT, 0.3))
# 3 stone
stone = noisy((125, 125, 125), 0.25, 0.85, 1.08)
for _ in range(10):  # darker cracks
    x, y = rnd.randrange(0, T - 6, 2), rnd.randrange(0, T, 2)
    for k in range(rnd.choice([4, 6])):
        if x + k < T:
            stone.putpixel((x + k, y), shade((125, 125, 125), 0.7))
put(3, stone)
# 4 cobblestone: rounded stones with dark gaps
cob = Image.new('RGBA', (T, T), shade((70, 70, 70), 1))
for _ in range(14):
    cx, cy, r = rnd.randrange(T), rnd.randrange(T), rnd.choice([4, 5, 6])
    base = shade((128, 128, 128), rnd.uniform(0.8, 1.15))
    for y in range(cy - r, cy + r):
        for x in range(cx - r, cx + r):
            if (x - cx) ** 2 + (y - cy) ** 2 < r * r:
                cob.putpixel((x % T, y % T), shade(base, rnd.uniform(0.9, 1.08)))
put(4, cob)
# 5 oak log side: vertical bark stripes
LOG = (103, 80, 50)
log = Image.new('RGBA', (T, T))
for x in range(0, T, 2):
    f = rnd.choice([0.75, 0.9, 1.0, 1.1])
    for y in range(T):
        c = shade(LOG, f * rnd.uniform(0.92, 1.06))
        log.putpixel((x, y), c)
        log.putpixel((x + 1, y), c)
put(5, log)
# 6 oak log top: rings
top = Image.new('RGBA', (T, T))
for y in range(T):
    for x in range(T):
        d = ((x - 15.5) ** 2 + (y - 15.5) ** 2) ** 0.5
        if d > 14:
            c = shade(LOG, 0.85)
        else:
            ring = int(d) // 3 % 2
            c = shade((176, 143, 93), 0.9 if ring else 1.05)
        top.putpixel((x, y), shade(c, rnd.uniform(0.95, 1.05)))
put(6, top)
# 7 oak leaves: green with see-through gaps (alpha 0)
leaves = Image.new('RGBA', (T, T), (0, 0, 0, 0))
for y in range(0, T, 2):
    for x in range(0, T, 2):
        if rnd.random() < 0.78:
            c = shade((58, 120, 40), rnd.uniform(0.7, 1.2))
            for dy in range(2):
                for dx in range(2):
                    leaves.putpixel((x + dx, y + dy), c)
put(7, leaves)
# 8 oak planks
PLANK = (162, 130, 78)
planks = noisy(PLANK, 0.2, 0.9, 1.06)
for y in (7, 15, 23, 31):
    for x in range(T):
        planks.putpixel((x, y), shade(PLANK, 0.62))
for row, x0 in enumerate((6, 22, 10, 26)):
    for y in range(row * 8, row * 8 + 7):
        planks.putpixel((x0, y), shade(PLANK, 0.62))
put(8, planks)
# 9 sand
put(9, noisy((219, 207, 160), 0.3, 0.92, 1.05))
# 10 glass: a light frame and a clear middle (a few shine marks)
glass = Image.new('RGBA', (T, T), (0, 0, 0, 0))
for i in range(T):
    for j in (0, 1, T - 2, T - 1):
        glass.putpixel((i, j), (215, 235, 245, 255))
        glass.putpixel((j, i), (215, 235, 245, 255))
for k in range(6):
    glass.putpixel((6 + k, 9 - k), (235, 245, 255, 255))
    glass.putpixel((7 + k, 9 - k), (235, 245, 255, 255))
put(10, glass)
# 11 bricks
BRICK = (150, 74, 58)
bricks = Image.new('RGBA', (T, T), (180, 170, 160, 255))
for row in range(4):
    off = 0 if row % 2 == 0 else 8
    for col in range(-1, 3):
        x0 = col * 16 + off
        base = shade(BRICK, rnd.uniform(0.85, 1.1))
        for y in range(row * 8 + 1, row * 8 + 8):
            for x in range(x0 + 1, x0 + 16):
                if 0 <= x < T:
                    bricks.putpixel((x, y), shade(base, rnd.uniform(0.92, 1.06)))
put(11, bricks)
# 12 gravel
gravel = Image.new('RGBA', (T, T))
for y in range(0, T, 2):
    for x in range(0, T, 2):
        c = rnd.choice([(130, 124, 122), (100, 96, 94), (155, 148, 140), (88, 80, 78)])
        c = shade(c, rnd.uniform(0.9, 1.1))
        for dy in range(2):
            for dx in range(2):
                gravel.putpixel((x + dx, y + dy), c)
put(12, gravel)
# 13 white wool, 14 red wool, 15 blue wool, 16 yellow wool
for i, c in enumerate([(233, 236, 236), (161, 39, 34), (53, 57, 157), (248, 197, 39)]):
    put(13 + i, noisy(c, 0.2, 0.92, 1.05))
# 17 bedrock
put(17, noisy((80, 80, 80), 0.6, 0.45, 1.2))

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'blocks.png')
sheet.save(out)
print('wrote', out)
