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
// FRG_AUTOTEST_INPUT=RIGHT:10-60,SPACE:30-31   hold keys during those frames (and MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE, MOUSE_BACK, MOUSE_FORWARD).
//                              CLOSE:N-N clicks the window's X button at frame N.
//                              MOUSE@x/y:N-M puts the pointer at (x, y) from frame N on (once any MOUSE@ is given,
//                              the real mouse is ignored). TURN@dx/dy:N-M makes mouseDelta() (dx, dy) during those
//                              frames, and (0, 0) on the others.
// FRG_AUTOTEST_HIDDEN=1        the window isn't shown, and sounds are silent (for tests that check what the game prints)
namespace autotest {
struct Hold {
    int key;
    long from, to;
};
struct Move {  // MOUSE@ (where the pointer is) or TURN@ (how far it moves each frame)
    bool turn;
    float x, y;
    long from, to;
};
struct State {
    long frames = -1;
    std::string shot = "autotest.png";
    std::vector<Hold> holds;
    std::vector<Move> moves;
};
inline State& st() {
    static State s;
    return s;
}
const int kClose = 2000;  // CLOSE: the window's X button
inline bool held(int key, long frame) {
    for (auto& h : st().holds)
        if (h.key == key && frame >= h.from && frame <= h.to) return true;
    return false;
}
// Whether scripted input clicks the window's X at the start of this frame.
inline bool closeRequested(long frame) { return held(kClose, frame) && !held(kClose, frame - 1); }
// Scripted pointer position: the latest MOUSE@ that has started by this frame. Gives false if none is scripted.
inline bool scriptedPointer(long frame, float& x, float& y) {
    bool any = false, found = false;
    long latest = -1;
    for (auto& m : st().moves) {
        if (m.turn) continue;
        any = true;
        if (m.from <= frame && m.from >= latest) {
            latest = m.from;
            x = m.x;
            y = m.y;
            found = true;
        }
    }
    if (any && !found) x = y = 0;
    return any;
}
// Scripted mouse movement this frame. Gives false if no TURN@ is scripted at all.
inline bool scriptedTurn(long frame, float& dx, float& dy) {
    bool any = false;
    dx = dy = 0;
    for (auto& m : st().moves) {
        if (!m.turn) continue;
        any = true;
        if (frame >= m.from && frame <= m.to) {
            dx += m.x;
            dy += m.y;
        }
    }
    return any;
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
    bool closed = false;  // window.close() was called (or the player closed it), so drawing can say so
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
    s.closed = true;
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
    if (!st().open && st().closed)
        panic(what, "the window has been closed (by window.close(), or by the player), so there's nothing to draw on - "
                    "check window.shouldClose() before drawing, or stop drawing after closing it");
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
    // FRG_AUTOTEST_HIDDEN=1: tests run the game without its window showing up on the screen.
    if (std::getenv("FRG_AUTOTEST_HIDDEN")) SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(width, height, title.c_str());
    SetExitKey(KEY_NULL);  // Escape is a normal key (for pause menus): only the X button or window.close() close the window
    SetTargetFPS(60);
    // icon.png in the game's folder becomes the window's icon (frg build also puts it into the .exe).
    std::string icon = findGameFile("icon.png");
    if (!icon.empty()) {
        ::Image img = LoadImage(icon.c_str());
        if (img.data) {
            ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            SetWindowIcon(img);
            UnloadImage(img);
        }
    }
    s.open = true;
    s.closed = false;
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
    if (WindowShouldClose() || autotest::closeRequested(s.frame) || (at.frames >= 0 && s.frame >= at.frames)) return true;  // exactly N frames
    beginFrame();
    return false;
}

template <class Update, class Draw>
void run_(Update update, Draw draw) {
    if (!st().open && st().closed)
        panic("window.run()", "the window has been closed, so there's no game to run - open it again with window.open(width, height, \"title\") first");
    if (!st().open) panic("window.run()", "the window isn't open yet - call window.open(width, height, \"title\") first");
    while (!shouldClose_()) {
        update(GetFrameTime());
        if (!st().open) break;  // update called window.close(): there's no window left to draw on
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
enum class MouseButton_ : int32_t { LEFT_, RIGHT_, MIDDLE_, BACK_, FORWARD_ };
// Named like an Xbox controller. Keep the order in sync with GamepadButton in src/prelude.cpp.
enum class GamepadButton_ : int32_t {
    A_, B_, X_, Y_, LEFT_BUMPER_, RIGHT_BUMPER_, LEFT_TRIGGER_, RIGHT_TRIGGER_,
    BACK_, START_, LEFT_STICK_, RIGHT_STICK_, UP_, DOWN_, LEFT_, RIGHT_,
};

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

// BACK and FORWARD are the two side (thumb) buttons: raylib calls them SIDE and EXTRA (Windows' XBUTTON1 / 2).
inline int rlButton(MouseButton_ b) {
    switch (b) {
        case MouseButton_::LEFT_: return MOUSE_BUTTON_LEFT;
        case MouseButton_::RIGHT_: return MOUSE_BUTTON_RIGHT;
        case MouseButton_::MIDDLE_: return MOUSE_BUTTON_MIDDLE;
        case MouseButton_::BACK_: return MOUSE_BUTTON_SIDE;
        default: return MOUSE_BUTTON_EXTRA;
    }
}

// Keys held down by FRG_AUTOTEST_INPUT: returns -1 if the key isn't scripted, otherwise 0/1 for `frameOffset` frames ago.
// Scripted mouse buttons are stored after the keys, as 1000 + the button.
inline int scripted(int k, long frameOffset) {
    long f = window::st().frame - frameOffset;
    int result = -1;
    for (auto& h : autotest::st().holds) {
        if (h.key != k) continue;
        if (result < 0) result = 0;
        if (f >= h.from && f <= h.to) result = 1;
    }
    return result;
}

inline bool keyDown_(Key_ k) {
    int s = scripted((int)k, 0);
    return s >= 0 ? s == 1 : IsKeyDown(rlKey(k));
}
inline bool keyPressed_(Key_ k) {
    int s = scripted((int)k, 0);
    return s >= 0 ? (s == 1 && scripted((int)k, 1) == 0) : IsKeyPressed(rlKey(k));
}
inline bool keyReleased_(Key_ k) {
    int s = scripted((int)k, 0);
    return s >= 0 ? (s == 0 && scripted((int)k, 1) == 1) : IsKeyReleased(rlKey(k));
}
inline bool mouseDown_(MouseButton_ b) {
    int s = scripted(1000 + (int)b, 0);
    return s >= 0 ? s == 1 : IsMouseButtonDown(rlButton(b));
}
inline bool mousePressed_(MouseButton_ b) {
    int s = scripted(1000 + (int)b, 0);
    return s >= 0 ? (s == 1 && scripted(1000 + (int)b, 1) == 0) : IsMouseButtonPressed(rlButton(b));
}
inline bool mouseReleased_(MouseButton_ b) {
    int s = scripted(1000 + (int)b, 0);
    return s >= 0 ? (s == 0 && scripted(1000 + (int)b, 1) == 1) : IsMouseButtonReleased(rlButton(b));
}
inline core::vec2_ mousePosition_() {
    float x, y;
    if (autotest::scriptedPointer(window::st().frame, x, y)) return {x, y};
    ::Vector2 p = GetMousePosition();
    return {p.x, p.y};
}
inline float mouseX_() { return mousePosition_().x_; }
inline float mouseY_() { return mousePosition_().y_; }
inline float mouseWheel_() { return GetMouseWheelMove(); }

// How far the mouse moved since the last frame, in pixels: for looking around in 3D.
inline core::vec2_ mouseDelta_() {
    float dx, dy;
    if (autotest::scriptedTurn(window::st().frame, dx, dy)) return {dx, dy};
    ::Vector2 d = GetMouseDelta();
    return {d.x, d.y};
}
// The mouse pointer (the cursor) belongs to the window, so these need it open.
inline void needWindow(const char* what) {
    if (!window::st().open && window::st().closed)
        panic(what, "the window has been closed, so there's no mouse pointer to change");
    if (!window::st().open) panic(what, "the window isn't open yet - call window.open(width, height, \"title\") first");
}
inline bool& mouseLocked() {
    static bool locked = false;
    return locked;
}

// A locked mouse is hidden and can't leave the window, and keeps giving mouseDelta() however far it's moved
// (like in first-person games). Unlocked, it's the normal pointer again (shown, even if hideCursor() hid it before).
inline void lockMouse_(bool locked) {
    needWindow("input.lockMouse()");
    if (locked && !mouseLocked()) DisableCursor();
    if (!locked && mouseLocked()) EnableCursor();
    mouseLocked() = locked;
}
inline bool isMouseLocked_() { return mouseLocked(); }

// A hidden pointer still moves, and can leave the window (unlike a locked one). For drawing your own pointer,
// or hiding it while the player uses a controller.
inline void hideCursor_() {
    needWindow("input.hideCursor()");
    HideCursor();
}
inline void showCursor_() {
    needWindow("input.showCursor()");
    if (mouseLocked()) {  // showing it lets go of it too: a locked pointer has to be hidden
        EnableCursor();
        mouseLocked() = false;
    }
    ShowCursor();
}
inline bool isCursorHidden_() { return window::st().open && IsCursorHidden(); }

enum class Cursor_ : int32_t { ARROW_, HAND_, TEXT_, CROSSHAIR_, RESIZE_ALL_, RESIZE_LEFT_RIGHT_, RESIZE_UP_DOWN_, NOT_ALLOWED_ };
inline void setCursor_(Cursor_ c) {
    needWindow("input.setCursor()");
    static const int shapes[] = {MOUSE_CURSOR_ARROW,      MOUSE_CURSOR_POINTING_HAND, MOUSE_CURSOR_IBEAM,     MOUSE_CURSOR_CROSSHAIR,
                                 MOUSE_CURSOR_RESIZE_ALL, MOUSE_CURSOR_RESIZE_EW,     MOUSE_CURSOR_RESIZE_NS, MOUSE_CURSOR_NOT_ALLOWED};
    SetMouseCursor(shapes[(int)c]);
}
inline std::string frg$str(Cursor_ c) {
    static const char* names[] = {"ARROW", "HAND", "TEXT", "CROSSHAIR", "RESIZE_ALL", "RESIZE_LEFT_RIGHT", "RESIZE_UP_DOWN", "NOT_ALLOWED"};
    return names[(int)c];
}

// Moves the pointer to a place in the window (0, 0 is the top left).
inline void setMousePosition_(float x, float y) {
    needWindow("input.setMousePosition()");
    SetMousePosition((int)x, (int)y);
}
inline void setMousePosition_(core::vec2_ p) { setMousePosition_(p.x_, p.y_); }
inline bool isMouseInWindow_() {
    float x, y;
    if (autotest::scriptedPointer(window::st().frame, x, y)) return window::st().open;
    return window::st().open && IsCursorOnScreen();
}

// ---- gamepads (controllers). gamepad is 0 for the first one plugged in, 1 for the second...

inline int rlGamepadButton(GamepadButton_ b) {
    static const int map[] = {
        GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GAMEPAD_BUTTON_RIGHT_FACE_UP,
        GAMEPAD_BUTTON_LEFT_TRIGGER_1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, GAMEPAD_BUTTON_LEFT_TRIGGER_2, GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
        GAMEPAD_BUTTON_MIDDLE_LEFT, GAMEPAD_BUTTON_MIDDLE_RIGHT, GAMEPAD_BUTTON_LEFT_THUMB, GAMEPAD_BUTTON_RIGHT_THUMB,
        GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_DOWN, GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    };
    return map[(int)b];
}
inline bool gamepadConnected_(int32_t pad) { return IsGamepadAvailable(pad); }
inline std::string gamepadName_(int32_t pad) {
    const char* n = IsGamepadAvailable(pad) ? GetGamepadName(pad) : nullptr;
    return n ? n : "";
}
inline bool gamepadDown_(GamepadButton_ b, int32_t pad) { return IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, rlGamepadButton(b)); }
inline bool gamepadPressed_(GamepadButton_ b, int32_t pad) { return IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, rlGamepadButton(b)); }
inline bool gamepadReleased_(GamepadButton_ b, int32_t pad) { return IsGamepadAvailable(pad) && IsGamepadButtonReleased(pad, rlGamepadButton(b)); }

// A stick, from (-1, -1) (up and left) to (1, 1) (down and right). Sticks never rest exactly in the middle, so small
// movements (the "dead zone") count as (0, 0), and the rest is stretched to still reach 1.
inline core::vec2_ stick(int32_t pad, int axisX, int axisY) {
    if (!IsGamepadAvailable(pad)) return {0, 0};
    float x = GetGamepadAxisMovement(pad, axisX), y = GetGamepadAxisMovement(pad, axisY);
    float len = std::sqrt(x * x + y * y);
    const float dead = 0.2f;
    if (len < dead) return {0, 0};
    float scale = std::min(1.0f, (len - dead) / (1 - dead)) / len;
    return {x * scale, y * scale};
}
inline core::vec2_ leftStick_(int32_t pad) { return stick(pad, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y); }
inline core::vec2_ rightStick_(int32_t pad) { return stick(pad, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_AXIS_RIGHT_Y); }
// A trigger, from 0 (let go) to 1 (pressed all the way). raylib gives -1 to 1.
inline float trigger(int32_t pad, int axis) {
    if (!IsGamepadAvailable(pad)) return 0;
    float v = (GetGamepadAxisMovement(pad, axis) + 1) / 2;
    return v < 0.05f ? 0 : std::min(1.0f, v);
}
inline float leftTrigger_(int32_t pad) { return trigger(pad, GAMEPAD_AXIS_LEFT_TRIGGER); }
inline float rightTrigger_(int32_t pad) { return trigger(pad, GAMEPAD_AXIS_RIGHT_TRIGGER); }

inline std::string frg$str(GamepadButton_ b) {
    static const char* names[] = {"A", "B", "X", "Y", "LEFT_BUMPER", "RIGHT_BUMPER", "LEFT_TRIGGER", "RIGHT_TRIGGER",
                                  "BACK", "START", "LEFT_STICK", "RIGHT_STICK", "UP", "DOWN", "LEFT", "RIGHT"};
    return names[(int)b];
}

inline std::string frg$str(Key_ k) { return keyNames()[(int)k]; }
inline std::string frg$str(MouseButton_ b) {
    static const char* names[] = {"LEFT", "RIGHT", "MIDDLE", "BACK", "FORWARD"};
    int i = (int)b;
    return i >= 0 && i < 5 ? names[i] : "MouseButton(?)";
}
}  // namespace input

inline void window::parseAutotestInput(const char* spec) {
    std::string s = spec;
    size_t start = 0;
    while (start < s.size()) {
        size_t end = s.find(',', start);
        if (end == std::string::npos) end = s.size();
        std::string item = s.substr(start, end - start);
        size_t colon = item.find(':');
        size_t dash = colon == std::string::npos ? std::string::npos : item.find('-', colon);  // after the :, since TURN@-5/0 has a - too
        if (colon != std::string::npos && dash != std::string::npos) {
            std::string name = item.substr(0, colon);
            for (int i = 0; input::keyNames()[i]; i++)
                if (name == input::keyNames()[i])
                    autotest::st().holds.push_back({i, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
            if (name == "CLOSE")
                autotest::st().holds.push_back({autotest::kClose, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
            // MOUSE@x/y and TURN@dx/dy: the numbers are between the @ and the :, split by /
            size_t at = name.find('@'), slash = name.find('/');
            if (at != std::string::npos && slash != std::string::npos && slash > at) {
                std::string what = name.substr(0, at);
                float x = (float)std::atof(name.c_str() + at + 1), y = (float)std::atof(name.c_str() + slash + 1);
                if (what == "MOUSE" || what == "TURN")
                    autotest::st().moves.push_back({what == "TURN", x, y, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
            }
            const char* buttons[] = {"MOUSE_LEFT", "MOUSE_RIGHT", "MOUSE_MIDDLE", "MOUSE_BACK", "MOUSE_FORWARD"};
            for (int b = 0; b < 5; b++)
                if (name == buttons[b])
                    autotest::st().holds.push_back({1000 + b, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
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

// ---------------------------------------------------------------- sprite effects
// beginFlash / beginOutline / beginGlow: textures and animations drawn after one of them get that effect, until
// endEffect() (or the next begin...). Shapes and text don't. Everything a texture draws goes through drawSprite().
extern "C" void rlDrawRenderBatchActive(void);

enum class Fx { None, Flash, Outline, Glow };
struct Effects {
    Fx kind = Fx::None;
    ::Color color{255, 255, 255, 255};
    float amount = 1;      // flash: how far toward the color; outline: thickness; glow: size (pixels)
    ::Shader flash{}, outline{};
    bool loaded = false;
};
inline Effects& fx() {
    static Effects e;
    return e;
}

inline void unloadEffects() {
    Effects& e = fx();
    if (!e.loaded) return;
    UnloadShader(e.flash);
    UnloadShader(e.outline);
    e.loaded = false;
}

inline void loadEffects() {
    Effects& e = fx();
    if (e.loaded) return;
    static const char* flashFs = R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 fxColor;
uniform float fxAmount;
out vec4 finalColor;
void main() {
    vec4 t = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    finalColor = vec4(mix(t.rgb, fxColor.rgb, fxAmount), t.a * mix(1.0, fxColor.a, fxAmount));
}
)GLSL";
    static const char* outlineFs = R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 fxColor;
uniform float fxAmount;
uniform vec2 texel;
out vec4 finalColor;
void main() {
    vec4 t = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    if (t.a > 0.5) {
        finalColor = t;
        return;
    }
    float near = 0.0;
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            if (float(dx * dx + dy * dy) > fxAmount * fxAmount + 0.5) continue;
            near = max(near, texture(texture0, fragTexCoord + vec2(float(dx), float(dy)) * texel).a);
        }
    }
    finalColor = near > 0.5 ? vec4(fxColor.rgb, fxColor.a * fragColor.a) : t;
}
)GLSL";
    e.flash = LoadShaderFromMemory(nullptr, flashFs);
    e.outline = LoadShaderFromMemory(nullptr, outlineFs);
    e.loaded = true;
    addHook(window::beforeCloseHooks(), &unloadEffects);
}

inline void fxUniforms(::Shader& s, ::Color c, float amount) {
    float col[4] = {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
    SetShaderValue(s, GetShaderLocation(s, "fxColor"), col, SHADER_UNIFORM_VEC4);
    SetShaderValue(s, GetShaderLocation(s, "fxAmount"), &amount, SHADER_UNIFORM_FLOAT);
}

// Draws (part of) a texture, with the current effect.
inline void drawSprite(const ::Texture2D& tex, ::Rectangle src, ::Rectangle dst, ::Vector2 origin, float degrees, ::Color tint) {
    Effects& e = fx();
    if (e.kind == Fx::None) {
        DrawTexturePro(tex, src, dst, origin, degrees, tint);
        return;
    }
    loadEffects();
    rlDrawRenderBatchActive();   // what's waiting is drawn first, with the shader it was meant for
    if (e.kind == Fx::Flash) {
        BeginShaderMode(e.flash);
        fxUniforms(e.flash, e.color, e.amount);
        DrawTexturePro(tex, src, dst, origin, degrees, tint);
        EndShaderMode();
        return;
    }
    if (e.kind == Fx::Outline) {
        BeginShaderMode(e.outline);
        fxUniforms(e.outline, e.color, e.amount);
        float texel[2] = {1.0f / std::max(1, tex.width), 1.0f / std::max(1, tex.height)};
        SetShaderValue(e.outline, GetShaderLocation(e.outline, "texel"), texel, SHADER_UNIFORM_VEC2);
        DrawTexturePro(tex, src, dst, origin, degrees, tint);
        EndShaderMode();
        return;
    }
    // glow: soft copies in the glow's color around it (added light), then the sprite itself
    BeginShaderMode(e.flash);
    fxUniforms(e.flash, e.color, 1);
    BeginBlendMode(BLEND_ADDITIVE);
    for (int ring = 1; ring <= 2; ring++) {
        float r = e.amount * ring / 2;
        for (int i = 0; i < 12; i++) {
            float a = i * 6.2831853f / 12;
            ::Rectangle d{dst.x + std::cos(a) * r, dst.y + std::sin(a) * r, dst.width, dst.height};
            DrawTexturePro(tex, src, d, origin, degrees, ::Color{255, 255, 255, (unsigned char)(e.color.a / 6)});
        }
    }
    EndBlendMode();
    EndShaderMode();
    DrawTexturePro(tex, src, dst, origin, degrees, tint);
}

inline void beginFlash_(core::color_ c, float amount) {
    if (amount < 0 || amount > 1) panic("graphics2d.beginFlash()", "the amount goes from 0 (no flash) to 1 (all one color), but it's " + str(amount));
    fx() = Effects{Fx::Flash, rl(c), amount, fx().flash, fx().outline, fx().loaded};
}
inline void beginOutline_(core::color_ c, float thickness) {
    if (thickness < 1 || thickness > 4) panic("graphics2d.beginOutline()", "the outline is 1 to 4 pixels thick, but this is " + str(thickness));
    fx() = Effects{Fx::Outline, rl(c), thickness, fx().flash, fx().outline, fx().loaded};
}
inline void beginGlow_(core::color_ c, float size) {
    if (size < 1 || size > 32) panic("graphics2d.beginGlow()", "the glow is 1 to 32 pixels wide, but this is " + str(size));
    fx() = Effects{Fx::Glow, rl(c), size, fx().flash, fx().outline, fx().loaded};
}
inline void endEffect_() { fx().kind = Fx::None; }
}  // namespace graphics2d

}  // namespace frg
