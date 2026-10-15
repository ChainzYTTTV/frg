#include <window>
#include <graphics2d>
#include <input>

``` 
  Draws some shapes and moves a ball with the arrow keys.
  This one writes its own loop: main calls mainLoop() every frame.
```

vec2 ball = vec2(400, 225);
float speed = 300;

int main(){
    window.open(800, 450, "Forge shapes");
    window.setBackground(color(30, 30, 46));

    while (!window.shouldClose()) {
        mainLoop();
    }

    window.close();
    return 0;
}

void mainLoop(){
    float dt = window.deltaTime();
    if (input.keyDown(Key.RIGHT)) {
        ball.x += speed * dt;
    }
    if (input.keyDown(Key.LEFT)) {
        ball.x -= speed * dt;
    }

    graphics2d.drawRect(40, 40, 200, 120, color.SKYBLUE);
    graphics2d.drawRectOutline(rect(260, 40, 200, 120), color.GOLD, 4);
    graphics2d.drawCircle(ball, 30, color.RED);
    graphics2d.drawCircleOutline(ball.x, ball.y, 40, color.WHITE);
    graphics2d.drawLine(vec2(40, 400), vec2(760, 400), color.GREEN, 3);
    graphics2d.drawTriangle(vec2(600, 60), vec2(700, 160), vec2(500, 160), color.PURPLE);
    graphics2d.drawText("Hello from Forge!", 40, 200, 32, color.WHITE);
    string info = "ball.x = {int(ball.x)}";
    graphics2d.drawText(info, 760 - graphics2d.textWidth(info, 20), 420, 20, color.LIGHTGRAY);
}
