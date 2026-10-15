// Forge runtime: <window>, <input> and <graphics2d>, built on raylib.
#pragma once

#include "forge.hpp"
#include "raylib.h"

#include <cstring>
#include <filesystem>

namespace frg {

inline ::Color rl(core::color_ c) { return ::Color{c.r_, c.g_, c.b_, c.a_}; }

// Only show raylib's warnings, not its info messages - set before any game variable is created.
inline const bool frg$quietRaylib = (SetTraceLogLevel(LOG_WARNING), true);

extern "C" __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);

// Finds a file the game uses: next to where the game was started from, or next to the .exe
// (or one folder up, since frg puts the .exe in build/). Returns "" if it isn't anywhere.
inline std::string findGameFile(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(path, ec)) return path;
    char buf[4096];
    unsigned long n = GetModuleFileNameA(nullptr, buf, sizeof buf);
    fs::path exeDir = fs::path(std::string(buf, n)).parent_path();
    for (fs::path candidate : {exeDir / path, exeDir.parent_path() / path})
        if (fs::exists(candidate, ec)) return candidate.string();
    return "";
}

inline std::string requireGameFile(const std::string& path, const char* what) {
    std::string found = findGameFile(path);
    if (found.empty())
        panic(what, "can't find the file '" + path + "' - put it in the game's folder (next to main.frg), or check the spelling");
    return found;
}
inline ::Vector2 rl(core::vec2_ v) { return ::Vector2{v.x_, v.y_}; }
inline ::Rectangle rl(core::rect_ r) { return ::Rectangle{r.x_, r.y_, r.width_, r.height_}; }

// ---------------------------------------------------------------- automated testing
// FRG_AUTOTEST_FRAMES=N        stop after N frames (taking a screenshot first, see below)
// FRG_AUTOTEST_SHOT=name.png   the screenshot's file name (saved in the game's folder)
// FRG_AUTOTEST_INPUT=RIGHT:10-60,SPACE:30-31   hold keys during those frames
namespace autotest {
struct Hold {
    int key;
    long from, to;
};
struct State {
    long frames = -1;
    std::string shot = "autotest.png";
    std::vector<Hold> holds;
};
inline State& st() {
    static State s;
    return s;
}
}  // namespace autotest

// ---------------------------------------------------------------- 2D camera (the type; the functions are in graphics2d below)

namespace graphics2d {
struct Camera2D_ {
    core::vec2_ target_{0, 0};  // the point in the world the camera looks at
    core::vec2_ offset_{0, 0};  // where on the screen that point is shown (usually the middle of the window)
    float zoom_ = 1;
    float rotation_ = 0;        // in degrees
    static Camera2D_ frg$make() { return {}; }
    static Camera2D_ frg$make(core::vec2_ target, core::vec2_ offset, float zoom, float rotation) {
        return {target, offset, zoom, rotation};
    }
    bool operator==(const Camera2D_&) const = default;
};
inline std::string frg$str(const Camera2D_& c) {
    return "Camera2D(target: " + core::frg$str(c.target_) + ", offset: " + core::frg$str(c.offset_) + ", zoom: " + str(c.zoom_) +
           ", rotation: " + str(c.rotation_) + ")";
}
inline ::Camera2D rlCamera(const Camera2D_& c) { return ::Camera2D{rl(c.offset_), rl(c.target_), c.rotation_, c.zoom_}; }
}  // namespace graphics2d

// ---------------------------------------------------------------- window

namespace window {
struct State {
    bool open = false;
    bool inFrame = false;
    core::color_ background{24, 24, 32, 255};
    long frame = 0;
    void (*leave3D)() = nullptr;  // set while 3D drawing is active; 2D drawing calls it first
    // The 2D camera set with graphics2d.setCamera. It stays set until resetCamera, but raylib's camera mode is only
    // switched on at the first 2D drawing of each frame, and always switched off before the frame is shown.
    graphics2d::Camera2D_ camera2d;
    bool camera2dSet = false;
    bool camera2dOn = false;
};
inline State& st() {
    static State s;
    return s;
}

// Other libraries hook in here: unloading textures before the window (and its graphics memory) goes away,
// and finishing 3D drawing before a frame is shown.
inline std::vector<void (*)()>& beforeCloseHooks() {
    static std::vector<void (*)()> hooks;
    return hooks;
}
inline std::vector<void (*)()>& beforeEndFrameHooks() {
    static std::vector<void (*)()> hooks;
    return hooks;
}

inline void leaveCamera2D() {
    State& s = st();
    if (!s.camera2dOn) return;
    EndMode2D();
    s.camera2dOn = false;
}

inline void close_() {
    State& s = st();
    if (!s.open) return;
    if (s.inFrame) {
        leaveCamera2D();
        for (auto fn : beforeEndFrameHooks()) fn();
        EndDrawing();
    }
    s.inFrame = false;
    for (auto fn : beforeCloseHooks()) fn();
    CloseWindow();
    s.open = false;
}

inline void beginFrame() {
    BeginDrawing();
    ClearBackground(rl(st().background));
    st().inFrame = true;
    tasks::update_(GetFrameTime());  // wake up any task whose wait() is over
}

inline void endFrame() {
    State& s = st();
    leaveCamera2D();
    for (auto fn : beforeEndFrameHooks()) fn();
    EndDrawing();
    for (auto fn : frameHooks()) fn();
    s.inFrame = false;
    s.frame++;
    auto& at = autotest::st();
    if (at.frames >= 0 && s.frame == at.frames) TakeScreenshot(at.shot.c_str());
}

// Drawing works any time after open(); a frame starts by itself if one isn't running.
inline void ensureFrame(const char* what) {
    if (!st().open) panic(what, "the window isn't open yet - call window.open(width, height, \"title\") first");
    if (!st().inFrame) beginFrame();
    if (st().leave3D) st().leave3D();  // 2D drawing goes on top of any 3D drawn so far
    State& s = st();
    if (s.camera2dSet && !s.camera2dOn) {
        BeginMode2D(graphics2d::rlCamera(s.camera2d));
        s.camera2dOn = true;
    }
}

inline void parseAutotestInput(const char* spec);

inline void open_(int32_t width, int32_t height, const std::string& title) {
    State& s = st();
    if (s.open) return;
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
    s.open = true;
    addHook(shutdownHooks(), &close_);
    auto& at = autotest::st();
    if (const char* f = std::getenv("FRG_AUTOTEST_FRAMES")) at.frames = std::atol(f);
    if (const char* f = std::getenv("FRG_AUTOTEST_SHOT")) at.shot = f;
    if (const char* f = std::getenv("FRG_AUTOTEST_INPUT")) parseAutotestInput(f);
}

// The first call starts a frame; each call after that shows the frame that was drawn and starts the next one.
inline bool shouldClose_() {
    State& s = st();
    if (!s.open) return true;
    if (s.inFrame) endFrame();
    auto& at = autotest::st();
    if (WindowShouldClose() || (at.frames >= 0 && s.frame > at.frames)) return true;
    beginFrame();
    return false;
}

template <class Update, class Draw>
void run_(Update update, Draw draw) {
    if (!st().open) panic("window.run()", "the window isn't open yet - call window.open(width, height, \"title\") first");
    while (!shouldClose_()) {
        update(GetFrameTime());
        draw();
    }
    close_();
}

inline void setTitle_(const std::string& t) { SetWindowTitle(t.c_str()); }
inline void setFPS_(int32_t fps) { SetTargetFPS(fps); }
inline int32_t fps_() { return GetFPS(); }
inline float deltaTime_() { return GetFrameTime(); }
inline double time_() { return GetTime(); }
inline int32_t width_() { return GetScreenWidth(); }
inline int32_t height_() { return GetScreenHeight(); }
inline void setBackground_(core::color_ c) { st().background = c; }
inline void toggleFullscreen_() { ToggleFullscreen(); }
inline void screenshot_(const std::string& name) { TakeScreenshot(name.c_str()); }
}  // namespace window

// ---------------------------------------------------------------- input

namespace input {
enum class Key_ : int32_t {
    A_, B_, C_, D_, E_, F_, G_, H_, I_, J_, K_, L_, M_, N_, O_, P_, Q_, R_, S_, T_, U_, V_, W_, X_, Y_, Z_,
    NUM0_, NUM1_, NUM2_, NUM3_, NUM4_, NUM5_, NUM6_, NUM7_, NUM8_, NUM9_,
    SPACE_, ENTER_, ESCAPE_, BACKSPACE_, TAB_, LEFT_, RIGHT_, UP_, DOWN_,
    LEFT_SHIFT_, RIGHT_SHIFT_, LEFT_CONTROL_, RIGHT_CONTROL_, LEFT_ALT_, RIGHT_ALT_,
    F1_, F2_, F3_, F4_, F5_, F6_, F7_, F8_, F9_, F10_, F11_, F12_,
};
enum class MouseButton_ : int32_t { LEFT_, RIGHT_, MIDDLE_ };

inline const char* const* keyNames() {
    static const char* names[] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        "NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9",
        "SPACE", "ENTER", "ESCAPE", "BACKSPACE", "TAB", "LEFT", "RIGHT", "UP", "DOWN",
        "LEFT_SHIFT", "RIGHT_SHIFT", "LEFT_CONTROL", "RIGHT_CONTROL", "LEFT_ALT", "RIGHT_ALT",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", nullptr,
    };
    return names;
}

inline int rlKey(Key_ k) {
    int i = (int)k;
    if (i <= 25) return KEY_A + i;
    if (i <= 35) return KEY_ZERO + (i - 26);
    static const int rest[] = {KEY_SPACE,      KEY_ENTER,       KEY_ESCAPE,   KEY_BACKSPACE, KEY_TAB,         KEY_LEFT,
                               KEY_RIGHT,      KEY_UP,          KEY_DOWN,     KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, KEY_LEFT_CONTROL,
                               KEY_RIGHT_CONTROL, KEY_LEFT_ALT, KEY_RIGHT_ALT};
    if (i <= 50) return rest[i - 36];
    return KEY_F1 + (i - 51);
}

inline int rlButton(MouseButton_ b) {
    return b == MouseButton_::LEFT_ ? MOUSE_BUTTON_LEFT : b == MouseButton_::RIGHT_ ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_MIDDLE;
}

// Keys held down by FRG_AUTOTEST_INPUT: returns -1 if the key isn't scripted, otherwise 0/1 for `frameOffset` frames ago.
inline int scripted(Key_ k, long frameOffset) {
    long f = window::st().frame - frameOffset;
    int result = -1;
    for (auto& h : autotest::st().holds) {
        if (h.key != (int)k) continue;
        if (result < 0) result = 0;
        if (f >= h.from && f <= h.to) result = 1;
    }
    return result;
}

inline bool keyDown_(Key_ k) {
    int s = scripted(k, 0);
    return s >= 0 ? s == 1 : IsKeyDown(rlKey(k));
}
inline bool keyPressed_(Key_ k) {
    int s = scripted(k, 0);
    return s >= 0 ? (s == 1 && scripted(k, 1) == 0) : IsKeyPressed(rlKey(k));
}
inline bool keyReleased_(Key_ k) {
    int s = scripted(k, 0);
    return s >= 0 ? (s == 0 && scripted(k, 1) == 1) : IsKeyReleased(rlKey(k));
}
inline bool mouseDown_(MouseButton_ b) { return IsMouseButtonDown(rlButton(b)); }
inline bool mousePressed_(MouseButton_ b) { return IsMouseButtonPressed(rlButton(b)); }
inline bool mouseReleased_(MouseButton_ b) { return IsMouseButtonReleased(rlButton(b)); }
inline core::vec2_ mousePosition_() {
    ::Vector2 p = GetMousePosition();
    return {p.x, p.y};
}
inline float mouseX_() { return GetMousePosition().x; }
inline float mouseY_() { return GetMousePosition().y; }
inline float mouseWheel_() { return GetMouseWheelMove(); }

inline std::string frg$str(Key_ k) { return keyNames()[(int)k]; }
inline std::string frg$str(MouseButton_ b) {
    return b == MouseButton_::LEFT_ ? "LEFT" : b == MouseButton_::RIGHT_ ? "RIGHT" : "MIDDLE";
}
}  // namespace input

inline void window::parseAutotestInput(const char* spec) {
    std::string s = spec;
    size_t start = 0;
    while (start < s.size()) {
        size_t end = s.find(',', start);
        if (end == std::string::npos) end = s.size();
        std::string item = s.substr(start, end - start);
        size_t colon = item.find(':'), dash = item.find('-');
        if (colon != std::string::npos && dash != std::string::npos) {
            std::string name = item.substr(0, colon);
            for (int i = 0; input::keyNames()[i]; i++)
                if (name == input::keyNames()[i])
                    autotest::st().holds.push_back({i, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
        }
        start = end + 1;
    }
}

// ---------------------------------------------------------------- graphics2d

namespace graphics2d {
inline void clear_(core::color_ c) {
    window::ensureFrame("graphics2d.clear()");
    ClearBackground(rl(c));
}
inline void drawRect_(float x, float y, float w, float h, core::color_ c) {
    window::ensureFrame("graphics2d.drawRect()");
    DrawRectangleRec(::Rectangle{x, y, w, h}, rl(c));
}
inline void drawRect_(core::rect_ r, core::color_ c) { drawRect_(r.x_, r.y_, r.width_, r.height_, c); }
inline void drawRectOutline_(float x, float y, float w, float h, core::color_ c, float thickness) {
    window::ensureFrame("graphics2d.drawRectOutline()");
    DrawRectangleLinesEx(::Rectangle{x, y, w, h}, thickness, rl(c));
}
inline void drawRectOutline_(core::rect_ r, core::color_ c, float thickness) {
    drawRectOutline_(r.x_, r.y_, r.width_, r.height_, c, thickness);
}
inline void drawCircle_(float x, float y, float radius, core::color_ c) {
    window::ensureFrame("graphics2d.drawCircle()");
    DrawCircleV(::Vector2{x, y}, radius, rl(c));
}
inline void drawCircle_(core::vec2_ p, float radius, core::color_ c) { drawCircle_(p.x_, p.y_, radius, c); }
inline void drawCircleOutline_(float x, float y, float radius, core::color_ c) {
    window::ensureFrame("graphics2d.drawCircleOutline()");
    DrawCircleLinesV(::Vector2{x, y}, radius, rl(c));
}
inline void drawLine_(float x1, float y1, float x2, float y2, core::color_ c, float thickness) {
    window::ensureFrame("graphics2d.drawLine()");
    DrawLineEx(::Vector2{x1, y1}, ::Vector2{x2, y2}, thickness, rl(c));
}
inline void drawLine_(core::vec2_ a, core::vec2_ b, core::color_ c, float thickness) { drawLine_(a.x_, a.y_, b.x_, b.y_, c, thickness); }
inline void drawTriangle_(core::vec2_ a, core::vec2_ b, core::vec2_ c, core::color_ fill) {
    window::ensureFrame("graphics2d.drawTriangle()");
    // raylib only draws triangles whose corners go counter-clockwise, so fix the order if needed.
    float cross = (b.x_ - a.x_) * (c.y_ - a.y_) - (b.y_ - a.y_) * (c.x_ - a.x_);
    if (cross < 0)
        DrawTriangle(rl(a), rl(b), rl(c), rl(fill));
    else
        DrawTriangle(rl(a), rl(c), rl(b), rl(fill));
}
inline void drawText_(const std::string& text, float x, float y, int32_t size, core::color_ c) {
    window::ensureFrame("graphics2d.drawText()");
    DrawText(text.c_str(), (int)x, (int)y, size, rl(c));
}
inline int32_t textWidth_(const std::string& text, int32_t size) { return MeasureText(text.c_str(), size); }
// The 2D camera: drawing after setCamera is moved, zoomed and turned by it; resetCamera goes back to the screen.
inline void setCamera_(Camera2D_ c) {
    if (!(c.zoom_ > 0)) panic("graphics2d.setCamera()", "the camera's zoom must be more than 0 (1 is normal size, 2 is twice as big), but it's " + str(c.zoom_));
    window::leaveCamera2D();  // the next 2D drawing starts again with the new camera
    window::st().camera2d = c;
    window::st().camera2dSet = true;
}
inline void resetCamera_() {
    window::leaveCamera2D();
    window::st().camera2dSet = false;
}
inline Camera2D_ camera_() { return window::st().camera2d; }
inline core::vec2_ screenToWorld_(core::vec2_ p) {
    if (!window::st().camera2dSet) return p;
    ::Vector2 v = GetScreenToWorld2D(rl(p), rlCamera(window::st().camera2d));
    return {v.x, v.y};
}
inline core::vec2_ worldToScreen_(core::vec2_ p) {
    if (!window::st().camera2dSet) return p;
    ::Vector2 v = GetWorldToScreen2D(rl(p), rlCamera(window::st().camera2d));
    return {v.x, v.y};
}

inline void drawFPS_(float x, float y) {
    window::ensureFrame("graphics2d.drawFPS()");
    DrawFPS((int)x, (int)y);
}
}  // namespace graphics2d

}  // namespace frg
