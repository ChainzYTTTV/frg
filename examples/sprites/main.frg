#include <window>
#include <graphics2d>
#include <input>
#include <assets>
#include <audio>
#include <math>

```
  Images and sounds: an animated blob that hops when you press Space.
  blob.png is a sprite sheet - 4 pictures (frames) side by side, each 16x16.
```

Texture blob = assets.loadTexture("blob.png");
Sound bloop = audio.loadSound("bloop.wav");

const int FRAME_SIZE = 16;
const float SCALE = 6;

float frameTimer = 0;
int frame = 0;
float x = 200;
float hop = 0;

int main(){
    window.open(800, 450, "Forge sprites");
    window.setBackground(color(40, 44, 70));
    window.run();
    return 0;
}

void update(float dt){
    // Show the next frame of the animation 8 times a second.
    frameTimer += dt;
    if (frameTimer > 0.125) {
        frameTimer = 0;
        frame = (frame + 1) % 4;
    }

    if (input.keyDown(Key.RIGHT)) {
        x += 250 * dt;
    }
    if (input.keyDown(Key.LEFT)) {
        x -= 250 * dt;
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
    rect source = rect(frame * FRAME_SIZE, 0, FRAME_SIZE, FRAME_SIZE);
    blob.drawPart(source, rect(x, y, size, size));

    // The whole sheet, small, so you can see all 4 frames.
    blob.drawScaled(20, 20, 3);
    graphics2d.drawText("frame {frame}", 20, 74, 20, color.WHITE);
    graphics2d.drawText("Arrows to move, Space to hop", 20, 410, 20, color.LIGHTGRAY);
    graphics2d.drawRect(0, 300 + size, 800, 4, color.SKYBLUE);
}
