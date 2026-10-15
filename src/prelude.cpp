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

// The same keys for <input> and the toolkits' own windows (runtime/forge_window.hpp, runtime/forge_native.hpp).
#define FRG_KEY_ENUMS R"FRG(
enum Key {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
    SPACE, ENTER, ESCAPE, BACKSPACE, TAB, LEFT, RIGHT, UP, DOWN,
    LEFT_SHIFT, RIGHT_SHIFT, LEFT_CONTROL, RIGHT_CONTROL, LEFT_ALT, RIGHT_ALT,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
}

enum MouseButton { LEFT, RIGHT, MIDDLE }
)FRG"

// The window, keyboard and mouse functions every low-level toolkit has.
#define FRG_TOOLKIT_WINDOW R"FRG(
void openWindow(int width, int height, string title);
bool shouldClose();
void closeWindow();
int width();
int height();
float deltaTime();
double time();
void setTitle(string title);
void setVSync(bool on);
bool keyDown(Key key);
bool keyPressed(Key key);
bool keyReleased(Key key);
bool mouseDown(MouseButton button);
bool mousePressed(MouseButton button);
bool mouseReleased(MouseButton button);
vec2 mousePosition();
float mouseWheel();
)FRG"

static const char* kInput = FRG_KEY_ENUMS R"FRG(
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
    bool solid;
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

static const char* kAssets = R"FRG(
class Texture {
    int width();
    int height();
    void draw(float x, float y, color tint = color.WHITE);
    void drawScaled(float x, float y, float scale, color tint = color.WHITE);
    void drawRotated(vec2 center, float degrees, float scale = 1, color tint = color.WHITE);
    void drawPart(rect source, rect destination, color tint = color.WHITE);
}

class Font {
    void drawText(string text, float x, float y, float size, color c);
    float measure(string text, float size);
}

Texture loadTexture(string path);
Font loadFont(string path, int quality = 64);
bool exists(string path);
)FRG";

static const char* kAudio = R"FRG(
class Sound {
    void play();
    void stop();
    bool isPlaying();
    void setVolume(float volume);
    void setPitch(float pitch);
}

class Music {
    void play();
    void stop();
    void pause();
    void resume();
    bool isPlaying();
    void setVolume(float volume);
    void setLooping(bool looping);
    float length();
    float position();
}

Sound loadSound(string path);
Music loadMusic(string path);
void setVolume(float volume);
void update();
)FRG";

static const char* kTime = R"FRG(
class Timer {
    Timer(float seconds);
    bool isDone();
    float left();
    float elapsed();
    void restart();
}

double now();
void sleep(float seconds);
string clock();
string date();
int year();
int month();
int day();
int hour();
int minute();
int second();
)FRG";

static const char* kFile = R"FRG(
string read(string path);
bool write(string path, string text);
bool append(string path, string text);
bool exists(string path);
bool remove(string path);
list<string> readLines(string path);
bool writeLines(string path, list<string> lines);
list<string> files(string folder);
)FRG";

static const char* kScenes = R"FRG(
class Scene {
    void enter();
    void update(float dt);
    void draw();
    void exit();
}

void change(Scene next);
void update(float dt);
void draw();
Scene? current();
)FRG";

static const char* kUi = R"FRG(
bool button(rect area, string label);
void label(string text, float x, float y, int size = 20, color c = color.WHITE);
bool checkbox(rect area, string label, bool checked);
float slider(rect area, float value, float min, float max);
void panel(rect area);
void setTheme(color background, color accent, color text);
)FRG";

static const char* kGraphics3d = R"FRG(
struct Camera {
    vec3 position;
    vec3 target;
    vec3 up;
    float fov;
    Camera(vec3 position, vec3 target);
    void orbit(float degrees);
}

class Model {
    void draw(vec3 position, float scale = 1, color tint = color.WHITE);
    void drawRotated(vec3 position, vec3 axis, float degrees, float scale = 1, color tint = color.WHITE);
}

void setCamera(Camera camera);
Camera camera();
void drawCube(vec3 position, vec3 size, color c);
void drawCubeOutline(vec3 position, vec3 size, color c);
void drawSphere(vec3 center, float radius, color c);
void drawSphereOutline(vec3 center, float radius, color c);
void drawCylinder(vec3 position, float radius, float height, color c);
void drawPlane(vec3 center, vec2 size, color c);
void drawGrid(int slices, float spacing);
void drawLine(vec3 from, vec3 to, color c);
vec2 toScreen(vec3 point);
Model loadModel(string path);
)FRG";

static const char* kEcs = R"FRG(
class Component {
    void start();
    void update(float dt);
    void draw();
    Entity? entity();
}

class Entity {
    string name;
    bool active;
    list<Component> components;
    Entity(string name);
    void add(Component component);
    Component? get(string type);
    bool has(string type);
    bool remove(string type);
    void destroy();
}

Entity create(string name);
void add(Entity entity);
void update(float dt);
void draw();
Entity? find(string name);
list<Entity> all();
list<Entity> withComponent(string type);
void clear();
int count();
)FRG";

static const char* kTasks = R"FRG(
void update(float dt);
void runAll();
int count();
void stopAll();
)FRG";

static const char* kRandom = R"FRG(
void seed(int value);
int range(int min, int max);
float range(float min, float max);
float value();
bool chance(float probability);
)FRG";

static const char* kOpenGL = FRG_KEY_ENUMS FRG_TOOLKIT_WINDOW R"FRG(
string version();

void clearColor(float r, float g, float b, float a);
void clear(int mask);
void viewport(int x, int y, int width, int height);
void enable(int capability);
void disable(int capability);
void blendFunc(int source, int destination);
void depthFunc(int function);
void cullFace(int face);
void lineWidth(float width);
int getError();

int createShader(int type);
void shaderSource(int shader, string source);
void compileShader(int shader);
bool shaderCompiled(int shader);
string shaderLog(int shader);
void deleteShader(int shader);
int createProgram();
void attachShader(int program, int shader);
void linkProgram(int program);
bool programLinked(int program);
string programLog(int program);
void useProgram(int program);
void deleteProgram(int program);
int buildProgram(string vertexSource, string fragmentSource);
int getUniformLocation(int program, string name);
int getAttribLocation(int program, string name);
void uniform1f(int location, float x);
void uniform2f(int location, float x, float y);
void uniform3f(int location, float x, float y, float z);
void uniform4f(int location, float x, float y, float z, float w);
void uniform1i(int location, int x);
void uniformMatrix4(int location, list<float> matrix);

int genVertexArray();
void bindVertexArray(int vertexArray);
void deleteVertexArray(int vertexArray);
int genBuffer();
void bindBuffer(int target, int buffer);
void deleteBuffer(int buffer);
void bufferData(int target, list<float> data, int usage);
void bufferDataInts(int target, list<int> data, int usage);
void vertexAttribPointer(int index, int size, int strideFloats, int offsetFloats);
void enableVertexAttribArray(int index);
void drawArrays(int mode, int first, int count);
void drawElements(int mode, int count);

int genTexture();
void bindTexture(int target, int texture);
void deleteTexture(int texture);
void activeTexture(int unit);
void texParameteri(int target, int name, int value);
void generateMipmap(int target);
int loadTexture(string path);

list<float> identity();
list<float> multiply(list<float> a, list<float> b);
list<float> perspective(float fovDegrees, float aspect, float near, float far);
list<float> orthographic(float left, float right, float bottom, float top, float near, float far);
list<float> lookAt(vec3 eye, vec3 target, vec3 up);
list<float> translate(vec3 offset);
list<float> rotate(float degrees, vec3 axis);
list<float> scale(vec3 amount);

const int COLOR_BUFFER_BIT = 16384;
const int DEPTH_BUFFER_BIT = 256;
const int STENCIL_BUFFER_BIT = 1024;
const int POINTS = 0;
const int LINES = 1;
const int LINE_LOOP = 2;
const int LINE_STRIP = 3;
const int TRIANGLES = 4;
const int TRIANGLE_STRIP = 5;
const int TRIANGLE_FAN = 6;
const int ARRAY_BUFFER = 34962;
const int ELEMENT_ARRAY_BUFFER = 34963;
const int STATIC_DRAW = 35044;
const int DYNAMIC_DRAW = 35048;
const int STREAM_DRAW = 35040;
const int VERTEX_SHADER = 35633;
const int FRAGMENT_SHADER = 35632;
const int DEPTH_TEST = 2929;
const int BLEND = 3042;
const int CULL_FACE = 2884;
const int ZERO = 0;
const int ONE = 1;
const int SRC_ALPHA = 770;
const int ONE_MINUS_SRC_ALPHA = 771;
const int LESS = 513;
const int LEQUAL = 515;
const int ALWAYS = 519;
const int FRONT = 1028;
const int BACK = 1029;
const int FRONT_AND_BACK = 1032;
const int TEXTURE_2D = 3553;
const int TEXTURE0 = 33984;
const int TEXTURE1 = 33985;
const int TEXTURE2 = 33986;
const int TEXTURE3 = 33987;
const int TEXTURE_MIN_FILTER = 10241;
const int TEXTURE_MAG_FILTER = 10240;
const int TEXTURE_WRAP_S = 10242;
const int TEXTURE_WRAP_T = 10243;
const int NEAREST = 9728;
const int LINEAR = 9729;
const int NEAREST_MIPMAP_NEAREST = 9984;
const int LINEAR_MIPMAP_LINEAR = 9987;
const int REPEAT = 10497;
const int CLAMP_TO_EDGE = 33071;
const int NO_ERROR = 0;
const int INVALID_ENUM = 1280;
const int INVALID_VALUE = 1281;
const int INVALID_OPERATION = 1282;
const int OUT_OF_MEMORY = 1285;
)FRG";

static const char* kDirectX = FRG_KEY_ENUMS FRG_TOOLKIT_WINDOW R"FRG(
string adapterName();

void clear(float r, float g, float b, float a);
void setDepthTest(bool on);
void setBlending(bool on);
void setCulling(int mode);
void setTopology(int topology);

int compileVertexShader(string hlsl, string entry = "main");
int compilePixelShader(string hlsl, string entry = "main");
void setShaders(int vertexShader, int pixelShader);
int createInputLayout(int vertexShader, list<string> names, list<int> sizes);
void setInputLayout(int layout);

int createVertexBuffer(list<float> data);
int createIndexBuffer(list<int> indices);
int createConstantBuffer(int floatCount);
void updateConstantBuffer(int buffer, list<float> data);
void setVertexBuffer(int buffer, int strideFloats);
void setIndexBuffer(int buffer);
void setConstantBuffer(int slot, int buffer);
void draw(int vertexCount, int startVertex = 0);
void drawIndexed(int indexCount, int startIndex = 0);

int loadTexture(string path);
void setTexture(int slot, int texture);
void setTextureFilter(bool smooth);

list<float> identity();
list<float> multiply(list<float> a, list<float> b);
list<float> perspective(float fovDegrees, float aspect, float near, float far);
list<float> orthographic(float left, float right, float bottom, float top, float near, float far);
list<float> lookAt(vec3 eye, vec3 target, vec3 up);
list<float> translate(vec3 offset);
list<float> rotate(float degrees, vec3 axis);
list<float> scale(vec3 amount);

const int POINTS = 1;
const int LINES = 2;
const int LINE_STRIP = 3;
const int TRIANGLES = 4;
const int TRIANGLE_STRIP = 5;
const int CULL_NONE = 1;
const int CULL_FRONT = 2;
const int CULL_BACK = 3;
)FRG";

static const char* kVulkan = FRG_KEY_ENUMS FRG_TOOLKIT_WINDOW R"FRG(
string deviceName();

void setTopology(int topology);
void setDepthTest(bool on);
void setCulling(int mode);
void setBlending(bool on);

int loadShader(string spvPath);
int createPipeline(int vertexShader, int fragmentShader, list<int> attributeSizes, int pushConstantFloats = 0);
int createVertexBuffer(list<float> data);
int createIndexBuffer(list<int> indices);

bool beginFrame(float r, float g, float b, float a);
void bindPipeline(int pipeline);
void bindVertexBuffer(int buffer);
void bindIndexBuffer(int buffer);
void pushConstants(int pipeline, list<float> data);
void draw(int vertexCount, int firstVertex = 0);
void drawIndexed(int indexCount, int firstIndex = 0);
void endFrame();

list<float> identity();
list<float> multiply(list<float> a, list<float> b);
list<float> perspective(float fovDegrees, float aspect, float near, float far);
list<float> orthographic(float left, float right, float bottom, float top, float near, float far);
list<float> lookAt(vec3 eye, vec3 target, vec3 up);
list<float> translate(vec3 offset);
list<float> rotate(float degrees, vec3 axis);
list<float> scale(vec3 amount);

const int POINTS = 0;
const int LINES = 1;
const int LINE_STRIP = 2;
const int TRIANGLES = 3;
const int TRIANGLE_STRIP = 4;
const int TRIANGLE_FAN = 5;
const int CULL_NONE = 0;
const int CULL_FRONT = 1;
const int CULL_BACK = 2;
)FRG";

const std::vector<PreludeLib>& preludeLibs() {
    using K = LibKind;
    static const std::vector<PreludeLib> libs = {
        {"core", kCore, false, ""},
        {"window", kWindow, true, "forge_window.hpp", K::Engine},
        {"input", kInput, true, "forge_window.hpp", K::Engine},
        {"graphics2d", kGraphics2d, true, "forge_window.hpp", K::Engine},
        {"physics", kPhysics, false, "forge_physics.hpp"},
        {"assets", kAssets, true, "forge_assets.hpp", K::Engine},
        {"audio", kAudio, true, "forge_audio.hpp"},
        {"random", kRandom, false, ""},
        {"tasks", kTasks, false, ""},
        {"time", kTime, false, "forge_time.hpp"},
        {"file", kFile, false, "forge_file.hpp"},
        {"scenes", kScenes, false, "forge_scenes.hpp"},
        {"ui", kUi, true, "forge_ui.hpp", K::Engine},
        {"graphics3d", kGraphics3d, true, "forge_graphics3d.hpp", K::Engine},
        {"ECS", kEcs, false, "forge_ecs.hpp"},
        {"OpenGL", kOpenGL, true, "forge_opengl.hpp", K::Toolkit, {"forge_opengl_impl.cpp"}, {"-lopengl32", "-lgdi32"}},
        {"DirectX", kDirectX, true, "forge_directx.hpp", K::Toolkit, {"forge_directx_impl.cpp"}, {"-ld3d11", "-ldxgi", "-ld3dcompiler_47"}},
        {"Vulkan", kVulkan, true, "forge_vulkan.hpp", K::Toolkit, {"forge_vulkan_impl.cpp"}, {}},
    };
    return libs;
}

const PreludeLib* findPreludeLib(const std::string& name) {
    for (const PreludeLib& lib : preludeLibs())
        if (lib.name == name) return &lib;
    return nullptr;
}
