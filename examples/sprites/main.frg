#include <window>
#include <graphics2d>
#include <input>
#include <assets>
#include <audio>
#include <math>

```
  Images and sounds: an animated blob that hops when you press Space.
  blob.png is a sprite sheet - 4 pictures (frames) side by side, each 16x16.
  An Animation plays them one after the other.
```

Texture blob = assets.loadTexture("blob.png");
Sound bloop = audio.loadSound("bloop.wav");

const int FRAME_SIZE = 16;
const float SCALE = 6;

// The sheet, the size of one frame, how many frames, and how many to show per second.
Animation wobble = Animation(blob, FRAME_SIZE, FRAME_SIZE, 4, 8);
float x = 200;
float hop = 0;

int main(){
    window.open(800, 450, "Forge sprites");
    window.setBackground(color(40, 44, 70));
    window.run();
    return 0;
}

void update(float dt){
    wobble.update(dt);    // moves on to the next frame when it's time

    if (input.keyDown(Key.RIGHT)) {
        x += 250 * dt;
        wobble.flip(false);
    }
    if (input.keyDown(Key.LEFT)) {
        x -= 250 * dt;
        wobble.flip(true);    // face the other way
    }
    if (input.keyPressed(Key.SPACE) and hop <= 0) {
        hop = 1;
        bloop.play();
    }
    hop = math.max(0.0, hop - dt * 3);
}

void draw(){
    float size = FRAME_SIZE * SCALE;
    float y = 300 - math.sin(hop * math.PI) * 120;
    wobble.drawScaled(x, y, SCALE);

    // The whole sheet, small, so you can see all 4 frames.
    blob.drawScaled(20, 20, 3);
    graphics2d.drawText("frame {wobble.frame()}", 20, 74, 20, color.WHITE);
    graphics2d.drawText("Arrows to move, Space to hop", 20, 410, 20, color.LIGHTGRAY);
    graphics2d.drawRect(0, 300 + size, 800, 4, color.SKYBLUE);
}
