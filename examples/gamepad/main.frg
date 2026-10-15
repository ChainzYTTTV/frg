#include <window>
#include <graphics2d>
#include <input>
#include <math>

```
  A controller tester. Plug in a controller (Xbox-style works best) and everything
  it does shows up on screen: the buttons light up while they're held, the sticks
  move their dots, and the triggers fill their bars.
  The orange square moves with the left stick or the d-pad. A makes it jump.
```

list<GamepadButton> BUTTONS = [
    GamepadButton.A, GamepadButton.B, GamepadButton.X, GamepadButton.Y,
    GamepadButton.LEFT_BUMPER, GamepadButton.RIGHT_BUMPER, GamepadButton.LEFT_TRIGGER, GamepadButton.RIGHT_TRIGGER,
    GamepadButton.BACK, GamepadButton.START, GamepadButton.LEFT_STICK, GamepadButton.RIGHT_STICK,
    GamepadButton.UP, GamepadButton.DOWN, GamepadButton.LEFT, GamepadButton.RIGHT
];
list<string> NAMES = [
    "A", "B", "X", "Y",
    "LEFT_BUMPER", "RIGHT_BUMPER", "LEFT_TRIGGER", "RIGHT_TRIGGER",
    "BACK", "START", "LEFT_STICK", "RIGHT_STICK",
    "UP", "DOWN", "LEFT", "RIGHT"
];

vec2 player = vec2(700, 300);
float jump = 0;          // how high the square is off the ground
float jumpSpeed = 0;
string lastPressed = "nothing yet";

int main(){
    window.open(960, 540, "Forge controller tester");
    window.setBackground(color(24, 26, 40));
    window.run();
    return 0;
}

void update(float dt){
    if (!input.gamepadConnected()) {
        return;
    }

    // Remember the last button that went down, to check gamepadPressed.
    for (int i = 0; i < BUTTONS.length; i++) {
        if (input.gamepadPressed(BUTTONS[i])) {
            lastPressed = NAMES[i];
        }
    }

    // Move with the left stick, or the d-pad.
    vec2 move = input.leftStick();
    if (input.gamepadDown(GamepadButton.LEFT)) { move.x = -1; }
    if (input.gamepadDown(GamepadButton.RIGHT)) { move.x = 1; }
    if (input.gamepadDown(GamepadButton.UP)) { move.y = -1; }
    if (input.gamepadDown(GamepadButton.DOWN)) { move.y = 1; }
    // Diagonals: up + right is (1, -1), which is about 1.41 long, so it would move faster than straight.
    // A stick never goes past 1, so cap the d-pad the same way.
    if (move.length() > 1) {
        move = move.normalized();
    }
    player += move * 300 * dt;
    player.x = math.clamp(player.x, 520, 920);
    player.y = math.clamp(player.y, 120, 480);

    // A jumps.
    if (input.gamepadPressed(GamepadButton.A) and jump <= 0) {
        jumpSpeed = 500;
    }
    jumpSpeed -= 1500 * dt;
    jump += jumpSpeed * dt;
    if (jump < 0) {
        jump = 0;
        jumpSpeed = 0;
    }
}

void draw(){
    if (!input.gamepadConnected()) {
        graphics2d.drawText("No controller found.", 40, 40, 40, color.WHITE);
        graphics2d.drawText("Plug one in (or turn it on). It's picked up straight away.", 40, 100, 20, color.LIGHTGRAY);
        return;
    }
    graphics2d.drawText("Controller: {input.gamepadName()}", 40, 30, 24, color.WHITE);
    graphics2d.drawText("Last pressed: {lastPressed}", 40, 62, 20, color.GOLD);

    // Every button: green while it's held.
    for (int i = 0; i < BUTTONS.length; i++) {
        float x = 40 + (i % 2) * 220;
        float y = 110 + (i / 2) * 34;
        bool held = input.gamepadDown(BUTTONS[i]);
        graphics2d.drawRect(x, y, 200, 28, held ? color.GREEN : color(50, 54, 80));
        graphics2d.drawText(NAMES[i], x + 10, y + 6, 16, held ? color.BLACK : color.LIGHTGRAY);
    }

    // The sticks: the dot follows the stick inside its circle.
    drawStick("Left stick", vec2(110, 430), input.leftStick());
    drawStick("Right stick", vec2(330, 430), input.rightStick());

    // The triggers: bars that fill from 0 to 1.
    drawTrigger("LT", 470, input.leftTrigger());
    drawTrigger("RT", 500, input.rightTrigger());

    // The player square.
    graphics2d.drawRect(player.x - 20, player.y - 20 - jump, 40, 40, color.ORANGE);
    graphics2d.drawText("Move: left stick or d-pad. Jump: A", 560, 500, 18, color.LIGHTGRAY);
}

void drawStick(string label, vec2 center, vec2 stick){
    graphics2d.drawCircleOutline(center.x, center.y, 50, color.LIGHTGRAY);
    graphics2d.drawCircle(center + stick * 50, 10, color.SKYBLUE);
    graphics2d.drawText(label, center.x - 45, center.y + 58, 16, color.LIGHTGRAY);
    graphics2d.drawText("{stick.x:.2}, {stick.y:.2}", center.x - 45, center.y + 76, 16, color.WHITE);
}

void drawTrigger(string label, float x, float amount){
    graphics2d.drawRectOutline(x, 380, 20, 120, color.LIGHTGRAY);
    graphics2d.drawRect(x, 380 + 120 * (1 - amount), 20, 120 * amount, color.PINK);
    graphics2d.drawText(label, x, 506, 16, color.LIGHTGRAY);
}
