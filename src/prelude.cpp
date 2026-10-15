#include "prelude.hpp"

// Keep the order of enum values in sync with runtime/forge_window.hpp.

static const char* kCore = R"FRG(
struct vec2 {
    float x;
    float y;
    vec2(float x, float y);
    float length();
    float lengthSquared();
    vec2 normalized();
    float dot(vec2 other);
    float distance(vec2 other);
    float angle();
    vec2 rotated(float radians);
    vec2 lerp(vec2 target, float t);
    static vec2 ZERO;
    static vec2 ONE;
    static vec2 UP;
    static vec2 DOWN;
    static vec2 LEFT;
    static vec2 RIGHT;
}

struct vec3 {
    float x;
    float y;
    float z;
    vec3(float x, float y, float z);
    float length();
    float lengthSquared();
    vec3 normalized();
    float dot(vec3 other);
    vec3 cross(vec3 other);
    float distance(vec3 other);
    vec3 lerp(vec3 target, float t);
    static vec3 ZERO;
    static vec3 ONE;
    static vec3 UP;
    static vec3 FORWARD;
    static vec3 RIGHT;
}

struct vec4 {
    float x;
    float y;
    float z;
    float w;
    vec4(float x, float y, float z, float w);
    float length();
    vec4 normalized();
    float dot(vec4 other);
    static vec4 ZERO;
    static vec4 ONE;
}

struct color {
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;
    color(uint8 r, uint8 g, uint8 b, uint8 a = 255);
    color fade(float alpha);
    color lerp(color target, float t);
    static color WHITE;
    static color BLACK;
    static color BLANK;
    static color LIGHTGRAY;
    static color GRAY;
    static color DARKGRAY;
    static color RED;
    static color MAROON;
    static color ORANGE;
    static color YELLOW;
    static color GOLD;
    static color GREEN;
    static color LIME;
    static color DARKGREEN;
    static color SKYBLUE;
    static color BLUE;
    static color DARKBLUE;
    static color PURPLE;
    static color VIOLET;
    static color PINK;
    static color MAGENTA;
    static color BROWN;
    static color BEIGE;
}

struct rect {
    float x;
    float y;
    float width;
    float height;
    rect(float x, float y, float width, float height);
    bool overlaps(rect other);
    bool contains(vec2 point);
    vec2 center();
    vec2 position();
    vec2 size();
}
)FRG";

static const char* kWindow = R"FRG(
void open(int width, int height, string title);
void close();
bool shouldClose();
void run();
void setTitle(string title);
void setFPS(int fps);
int fps();
float deltaTime();
double time();
int width();
int height();
void setBackground(color c);
void toggleFullscreen();
void screenshot(string fileName);
)FRG";

static const char* kInput = R"FRG(
enum Key {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
    SPACE, ENTER, ESCAPE, BACKSPACE, TAB, LEFT, RIGHT, UP, DOWN,
    LEFT_SHIFT, RIGHT_SHIFT, LEFT_CONTROL, RIGHT_CONTROL, LEFT_ALT, RIGHT_ALT,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
}

enum MouseButton { LEFT, RIGHT, MIDDLE }

bool keyDown(Key key);
bool keyPressed(Key key);
bool keyReleased(Key key);
bool mouseDown(MouseButton button);
bool mousePressed(MouseButton button);
bool mouseReleased(MouseButton button);
vec2 mousePosition();
float mouseX();
float mouseY();
float mouseWheel();
)FRG";

static const char* kGraphics2d = R"FRG(
void clear(color c);
void drawRect(float x, float y, float width, float height, color c);
void drawRect(rect r, color c);
void drawRectOutline(float x, float y, float width, float height, color c, float thickness = 1);
void drawRectOutline(rect r, color c, float thickness = 1);
void drawCircle(float x, float y, float radius, color c);
void drawCircle(vec2 center, float radius, color c);
void drawCircleOutline(float x, float y, float radius, color c);
void drawLine(float x1, float y1, float x2, float y2, color c, float thickness = 1);
void drawLine(vec2 from, vec2 to, color c, float thickness = 1);
void drawTriangle(vec2 a, vec2 b, vec2 c, color fill);
void drawText(string text, float x, float y, int size, color c);
int textWidth(string text, int size);
void drawFPS(float x, float y);
)FRG";

static const char* kPhysics = R"FRG(
class Body {
    vec2 position;
    vec2 size;
    vec2 velocity;
    bool isStatic;
    float gravityScale;
    bool onGround;
    bool onCeiling;
    bool onWall;
    Body(float x, float y, float width, float height);
    rect bounds();
    vec2 center();
    bool overlaps(Body other);
}

void add(Body body);
void remove(Body body);
void clear();
void setGravity(vec2 gravity);
vec2 gravity();
void step(float dt);
int count();
)FRG";

static const char* kRandom = R"FRG(
void seed(int value);
int range(int min, int max);
float range(float min, float max);
float value();
bool chance(float probability);
)FRG";

const std::vector<PreludeLib>& preludeLibs() {
    static const std::vector<PreludeLib> libs = {
        {"core", kCore, false, ""},
        {"window", kWindow, true, "forge_window.hpp"},
        {"input", kInput, true, "forge_window.hpp"},
        {"graphics2d", kGraphics2d, true, "forge_window.hpp"},
        {"physics", kPhysics, false, "forge_physics.hpp"},
        {"random", kRandom, false, ""},
    };
    return libs;
}
