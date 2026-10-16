# Draws Forge's logo, an anvil with sparks, into PNGs and a Windows .ico, using only Pillow (pip install pillow).
# The shapes are the same as in art/forge-mark.svg and art/forge-icon.svg: change both if you change one.
import re, sys
from PIL import Image, ImageDraw

SPARK1 = "M44 3 L46.2 9.8 L53 12 L46.2 14.2 L44 21 L41.8 14.2 L35 12 L41.8 9.8 Z"
SPARK2 = "M56 17 L57.3 20.7 L61 22 L57.3 23.3 L56 27 L54.7 23.3 L51 22 L54.7 20.7 Z"
ANVIL = ("M2 27 C8 27 12 26 16 24 H56 V33 H48 C44 33 42 35 42 39 V44 H48 L52 52 H14 L18 44 H24 V39 "
         "C24 35 22 33 18 33 C12 33 6 31 2 27 Z")

def points(path):
    """Turns an SVG path made of absolute M, L, H, V, C and Z into a list of points (curves split into lines)."""
    toks = re.findall(r'[MLHVCZ]|-?[\d.]+', path)
    pts, i, cmd, x, y = [], 0, None, 0.0, 0.0
    while i < len(toks):
        if toks[i] in 'MLHVCZ':
            cmd = toks[i]; i += 1
            if cmd == 'Z':
                continue
        if cmd in 'ML':
            x, y = float(toks[i]), float(toks[i + 1]); i += 2; pts.append((x, y))
        elif cmd == 'H':
            x = float(toks[i]); i += 1; pts.append((x, y))
        elif cmd == 'V':
            y = float(toks[i]); i += 1; pts.append((x, y))
        elif cmd == 'C':
            x1, y1, x2, y2, x3, y3 = map(float, toks[i:i + 6]); i += 6
            for s in range(1, 17):
                t = s / 16
                u = 1 - t
                pts.append((u**3 * x + 3 * u * u * t * x1 + 3 * u * t * t * x2 + t**3 * x3,
                            u**3 * y + 3 * u * u * t * y1 + 3 * u * t * t * y2 + t**3 * y3))
            x, y = x3, y3
    return pts

def gradient(size, top, bottom, diagonal=False):
    g = Image.new('RGBA', (size, size))
    px = g.load()
    for yy in range(size):
        for xx in range(size):
            t = ((xx + yy) / (2 * size)) if diagonal else yy / size
            px[xx, yy] = tuple(int(top[k] + (bottom[k] - top[k]) * t) for k in range(3)) + (255,)
    return g

def draw(size, app=False):
    S = 8                      # draw 8x bigger, then shrink: smooth edges
    big = size * S
    img = Image.new('RGBA', (big, big), (0, 0, 0, 0))
    if app:
        mask = Image.new('L', (big, big), 0)
        ImageDraw.Draw(mask).rounded_rectangle([0, 0, big - 1, big - 1], radius=big * 14 // 64, fill=255)
        img.paste(gradient(big, (42, 45, 69), (21, 23, 42), diagonal=True), (0, 0), mask)
        scale, ox, oy = big / 64 * 0.84, big / 64 * 5, big / 64 * 6
    else:
        scale, ox, oy = big / 64, 0, 0
    tf = lambda pts: [(ox + px * scale, oy + py * scale) for px, py in pts]
    d = ImageDraw.Draw(img)
    for spark in (SPARK1, SPARK2):
        d.polygon(tf(points(spark)), fill=(255, 212, 59, 255))
    mask = Image.new('L', (big, big), 0)
    ImageDraw.Draw(mask).polygon(tf(points(ANVIL)), fill=255)
    # the anvil's top-to-bottom gradient, over the part of the image it covers
    metal = Image.new('RGBA', (big, big))
    top, bottom = (255, 169, 77), (232, 89, 12)
    y0, y1 = oy + 24 * scale, oy + 52 * scale
    mpx = metal.load()
    for yy in range(big):
        t = min(1, max(0, (yy - y0) / (y1 - y0)))
        c = tuple(int(top[k] + (bottom[k] - top[k]) * t) for k in range(3)) + (255,)
        for xx in range(big):
            mpx[xx, yy] = c
    img.paste(metal, (0, 0), mask)
    return img.resize((size, size), Image.LANCZOS)

if __name__ == '__main__':
    # Run from anywhere: python art/make_icons.py
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    repo = os.path.dirname(here)
    sizes = [16, 24, 32, 48, 64, 128, 256]
    marks = {s: draw(s) for s in sizes}
    draw(512, app=True).save(os.path.join(here, 'forge-icon-512.png'))
    draw(128, app=True).save(os.path.join(repo, 'editors', 'vscode', 'icons', 'forge-128.png'))
    # The .frg file icon for Windows, with every size, so Explorer picks a sharp one at each zoom level.
    marks[256].save(os.path.join(here, 'frg-file.ico'), sizes=[(s, s) for s in sizes],
                    append_images=[marks[s] for s in sizes[:-1]])
    print('Made art/forge-icon-512.png, art/frg-file.ico and editors/vscode/icons/forge-128.png')
