# Sprites and Animation

A **sprite** is a 2D image for a game object. This guide covers drawing sprites, sprite sheets, animation, and flipping. The full reference is in [`<assets>`](../libraries/assets.md).

## Drawing one image

```cpp
#include <window>
#include <assets>

Texture ship = assets.loadTexture("ship.png");

int main(){
    window.open(800, 450, "Sprites");
    window.run();
    return 0;
}

void update(float dt){
}

void draw(){
    ship.draw(100, 100);                   // normal size, top-left at (100, 100)
    ship.drawScaled(300, 100, 3);          // three times bigger
}
```

Put `ship.png` in the game's folder, next to `main.frg`.

## Pixel art

Small pixel-art images (16×16, 32×32) are usually drawn several times bigger. Forge keeps them sharp, without blurring. Pick a whole-number scale (2, 3, 4...) so every pixel stays the same size.

## Sprite sheets

A **sprite sheet** is one image holding many pictures (**frames**) side by side, like a strip of film:

```
+------+------+------+------+
|  0   |  1   |  2   |  3   |      blob.png: 4 frames, each 16x16, so 64x16 overall
+------+------+------+------+
```

`drawPart(source, destination)` draws just one part of an image:

- `source` is a `rect` **inside the image**: which frame.
- `destination` is a `rect` **on the screen**: where to draw it, and how big.

```cpp
int frame = 2;
rect source = rect(frame * 16, 0, 16, 16);          // the 3rd frame
rect destination = rect(x, y, 64, 64);              // drawn 4x bigger
blob.drawPart(source, destination);
```

For a sheet with several rows (one row per animation), also pick the row: `rect(frame * 16, row * 16, 16, 16)`.

## Animation

To animate, show the next frame every so often. Count time with `dt`:

```cpp
int frame = 0;
float frameTimer = 0;
const float FRAME_TIME = 0.1;      // 10 frames per second
const int FRAME_COUNT = 4;

void update(float dt){
    frameTimer += dt;
    if (frameTimer >= FRAME_TIME) {
        frameTimer -= FRAME_TIME;
        frame = (frame + 1) mod FRAME_COUNT;       // 0, 1, 2, 3, 0, 1, ...
    }
}

void draw(){
    blob.drawPart(rect(frame * 16, 0, 16, 16), rect(x, y, 64, 64));
}
```

### Several animations

Keep the animation's row and frame count together in a struct:

```cpp
struct Animation {
    int row;
    int frames;
    float frameTime;
}

Animation IDLE = Animation(0, 2, 0.4);
Animation RUN = Animation(1, 6, 0.08);
Animation current = IDLE;

void setAnimation(Animation a){
    if (a != current) {
        current = a;
        frame = 0;
        frameTimer = 0;
    }
}
```

Then in `update`: `setAnimation(moving ? RUN : IDLE);`, and in `draw`: `sheet.drawPart(rect(frame * 16, current.row * 16, 16, 16), ...)`.

## Facing left and right

Draw sprites facing right, then flip them when moving left. A **negative width** in the source rectangle flips the image left-to-right:

```cpp
bool facingLeft = velocity.x < 0;
float w = facingLeft ? -16 : 16;
sheet.drawPart(rect(frame * 16, 0, w, 16), rect(x, y, 64, 64));
```

## Rotating

`drawRotated(center, degrees, scale)` draws the image **centered** on a point and turned. Positive degrees turn clockwise:

```cpp
float angle = 0;

void update(float dt){
    angle += 180 * dt;         // half a turn per second
}

void draw(){
    ship.drawRotated(vec2(400, 225), angle, 2);
}
```

To point at something (like the mouse), work out the angle with `math.atan2` and convert radians to degrees:

```cpp
vec2 toMouse = input.mousePosition() - shipPosition;
float degrees = math.atan2(toMouse.y, toMouse.x) * 180 / math.PI;
```

## Tinting and fading

The `tint` color multiplies the image's colors:

```cpp
ship.draw(x, y, color.RED);                    // reddish (e.g. when hit)
ship.draw(x, y, color.WHITE.fade(0.5));        // half see-through
```

A hit flash with a [task](../language/11-tasks.md):

```cpp
color shipTint = color.WHITE;

task hitFlash() {
    shipTint = color.RED;
    wait(0.1);
    shipTint = color.WHITE;
}
```

## A working example

[examples/sprites](../../examples/sprites/main.frg) loads a 4-frame sprite sheet, animates it, moves it with the arrow keys, and plays a sound when it hops.
