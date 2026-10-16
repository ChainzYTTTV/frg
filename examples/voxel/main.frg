#include <window>
#include <graphics3d>
#include <input>
#include <stdio>

```
  Forge Craft: a small creative-mode block world.

  Mouse to look, W A S D to walk, Space to jump, F (or Space twice) to fly - then Space / Left Shift go up / down.
  Left click breaks a block, right click places the one in your hand, middle click picks the one you look at.
  1 - 9 or the mouse wheel choose a hotbar slot, E opens the creative menu, F3 shows the debug numbers.

  The files: blocks.frg (the kinds of block), world.frg (the blocks, and making the hills and trees),
  mesher.frg (turning blocks into meshes), player.frg (moving and aiming), hud.frg (everything flat on top).
  blocks.png is the sprite sheet of 32 x 32 textures; make_textures.py draws it.
```

int main(){
    window.open(1280, 720, "Forge Craft");
    window.setBackground(color(135, 190, 245));

    double start = window.time();
    world.generate(2024);
    double generated = window.time();
    mesher.buildAll();
    double built = window.time();
    print("world made in {(generated - start) * 1000:.0} ms, meshes built in {(built - generated) * 1000:.0} ms ({mesher.triangles()} triangles)");

    player.spawn();
    input.lockMouse(true);
    window.run();
    return 0;
}

void update(float dt){
    hud.update();
    if (!hud.menuOpen) {
        player.update(dt);
    }
    graphics3d.setCamera(player.camera());
}

void draw(){
    mesher.drawAll(player.eye(), player.lookDirection());
    player.drawTarget();
    hud.draw();
}
