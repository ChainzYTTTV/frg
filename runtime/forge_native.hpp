// Forge runtime: the plain Windows window shared by the low-level toolkits (<OpenGL>, <DirectX>, <Vulkan>).
//
// The toolkits don't use Forge's engine (raylib). Each opens this window itself and draws into it with its
// own graphics API. This header only declares things: the code is in native_window.inl, which is compiled
// as part of the toolkit's own .cpp file (Windows' headers can't be mixed with raylib's in one file).
#pragma once

#include "forge.hpp"

namespace frg::native {

// Keep in sync with the Key enum in src/prelude.cpp (the same list as <input>).
enum class Key_ : int32_t {
    A_, B_, C_, D_, E_, F_, G_, H_, I_, J_, K_, L_, M_, N_, O_, P_, Q_, R_, S_, T_, U_, V_, W_, X_, Y_, Z_,
    NUM0_, NUM1_, NUM2_, NUM3_, NUM4_, NUM5_, NUM6_, NUM7_, NUM8_, NUM9_,
    SPACE_, ENTER_, ESCAPE_, BACKSPACE_, TAB_, LEFT_, RIGHT_, UP_, DOWN_,
    LEFT_SHIFT_, RIGHT_SHIFT_, LEFT_CONTROL_, RIGHT_CONTROL_, LEFT_ALT_, RIGHT_ALT_,
    F1_, F2_, F3_, F4_, F5_, F6_, F7_, F8_, F9_, F10_, F11_, F12_,
};
enum class MouseButton_ : int32_t { LEFT_, RIGHT_, MIDDLE_ };

std::string keyName(Key_ k);
std::string buttonName(MouseButton_ b);

// Window (the toolkit supplies how to show a frame and take a screenshot).
void open(int32_t width, int32_t height, const std::string& title, void (*present)(), void (*screenshot)(const std::string&));
bool shouldClose();
void close();
bool isOpen();
void* handle();  // the HWND
int32_t width();
int32_t height();
float deltaTime();
double time();
void setTitle(const std::string& title);

bool keyDown(Key_ k);
bool keyPressed(Key_ k);
bool keyReleased(Key_ k);
bool mouseDown(MouseButton_ b);
bool mousePressed(MouseButton_ b);
bool mouseReleased(MouseButton_ b);
core::vec2_ mousePosition();
float mouseWheel();

[[noreturn]] void fail(const char* where, const std::string& msg);

// A file the game uses: where the game was started from, next to the .exe, or one folder up. "" if missing.
std::string findGameFile(const std::string& path);

// True while drawing the frame that FRG_AUTOTEST_SHOT will capture.
bool screenshotDue();

// Saves RGBA pixels (top row first) as a .png, for screenshots.
bool savePng(const std::string& path, const unsigned char* rgba, int width, int height);

}  // namespace frg::native
