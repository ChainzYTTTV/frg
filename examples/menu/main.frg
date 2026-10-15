#include <window>
#include <graphics2d>
#include <input>
#include <scenes>
#include <ui>
#include <time>
#include <tween>

```
  A main menu and a play screen, using <scenes> to switch between them
  and <ui> for the buttons, the name box, the checkbox and the slider.
  The title drops in with a bounce, using <tween>.
```

string playerName = "Player";
bool showFps = true;
float volume = 0.7;
bool quitting = false;

class Menu : Scene {
    // From above the window (y = -80) down to y = 70, in 0.9 seconds, bouncing at the end.
    Tween titleDrop = Tween(-80, 70, 0.9, Ease.BOUNCE);

    void enter() {
        titleDrop.restart();
    }

    void update(float dt) {
        titleDrop.update(dt);
    }

    void draw() {
        graphics2d.drawText("FORGE", 380, titleDrop.value(), 64, color.ORANGE);
        ui.panel(rect(330, 150, 300, 370));

        ui.label("Your name:", 360, 170, 20, color.LIGHTGRAY);
        playerName = ui.textBox(rect(360, 195, 240, 40), playerName, 12);    // at most 12 letters
        if (ui.button(rect(360, 250, 240, 50), "Play")) {
            scenes.change(Playing());
        }
        showFps = ui.checkbox(rect(360, 320, 30, 30), "Show FPS", showFps);
        ui.label("Volume: {int(volume * 100)}%", 360, 370, 20, color.LIGHTGRAY);
        volume = ui.slider(rect(360, 395, 240, 24), volume, 0, 1);
        if (ui.button(rect(360, 445, 240, 50), "Quit")) {
            quitting = true;
        }
    }
}

class Playing : Scene {
    Timer clock = Timer(0);

    void enter() {
        clock.restart();
    }

    void update(float dt) {
        if (input.keyPressed(Key.BACKSPACE)) {
            scenes.change(Menu());
        }
    }

    void draw() {
        graphics2d.drawText("Good luck, {playerName}!", 300, 140, 30, color.ORANGE);
        graphics2d.drawText("Playing for {int(clock.elapsed())} seconds", 300, 200, 30, color.WHITE);
        graphics2d.drawText("Press Backspace to go back to the menu", 270, 260, 20, color.LIGHTGRAY);
    }
}

int main(){
    window.open(960, 540, "Forge menu");
    scenes.change(Menu());
    while (!window.shouldClose() and !quitting) {
        scenes.update(window.deltaTime());
        scenes.draw();
        if (showFps) {
            graphics2d.drawFPS(10, 10);
        }
    }
    window.close();
    return 0;
}
