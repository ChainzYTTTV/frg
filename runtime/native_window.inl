// Forge runtime: the Windows window behind <OpenGL>, <DirectX> and <Vulkan>.
// Included by exactly one toolkit .cpp file per game (windows.h must already be included).

#include <cstring>
#include <filesystem>
#include <vector>

// raylib's image saver (raylib is linked anyway). Declared by hand: raylib.h can't be included next to windows.h.
extern "C" {
struct FrgRlImage {
    void* data;
    int width, height, mipmaps, format;
};
bool ExportImage(FrgRlImage image, const char* fileName);
void SetTraceLogLevel(int logLevel);
}

// Only raylib's warnings, not its info messages (it's only used here for loading and saving images).
static const bool frg$quietRaylib = (SetTraceLogLevel(4 /* LOG_WARNING */), true);

namespace frg::native {

namespace {

struct Hold {
    int key;
    long from, to;
};
const int kScriptedClose = 2000;  // FRG_AUTOTEST_INPUT's CLOSE: the window's X button

struct Win {
    HWND hwnd = nullptr;
    bool open = false;
    bool closing = false;
    bool inFrame = false;
    int w = 0, h = 0;
    bool keys[256] = {}, prevKeys[256] = {};
    bool mouse[3] = {}, prevMouse[3] = {};
    float mx = 0, my = 0, wheel = 0, wheelAcc = 0;
    LARGE_INTEGER freq{}, start{}, last{};
    double dt = 0;
    long frame = 0;
    void (*present)() = nullptr;
    void (*screenshot)(const std::string&) = nullptr;
    long autotestFrames = -1;
    std::string autotestShot = "autotest.png";
    std::vector<Hold> holds;
};

Win& win() {
    static Win w;
    return w;
}

const char* const kKeyNames[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9",
    "SPACE", "ENTER", "ESCAPE", "BACKSPACE", "TAB", "LEFT", "RIGHT", "UP", "DOWN",
    "LEFT_SHIFT", "RIGHT_SHIFT", "LEFT_CONTROL", "RIGHT_CONTROL", "LEFT_ALT", "RIGHT_ALT",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
};
const int kKeyCount = sizeof kKeyNames / sizeof kKeyNames[0];

int virtualKey(int k) {
    if (k <= 25) return 'A' + k;
    if (k <= 35) return '0' + (k - 26);
    static const int rest[] = {VK_SPACE, VK_RETURN, VK_ESCAPE, VK_BACK, VK_TAB, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
                               VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU};
    if (k <= 50) return rest[k - 36];
    return VK_F1 + (k - 51);
}

// Left/right Shift, Ctrl and Alt arrive as one key code; work out which side it was.
int sidedKey(WPARAM wp, LPARAM lp) {
    int vk = (int)wp;
    bool extended = (lp >> 24) & 1;
    if (vk == VK_SHIFT) return (int)MapVirtualKeyW((lp >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
    if (vk == VK_CONTROL) return extended ? VK_RCONTROL : VK_LCONTROL;
    if (vk == VK_MENU) return extended ? VK_RMENU : VK_LMENU;
    return vk;
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Win& s = win();
    switch (msg) {
        case WM_CLOSE: s.closing = true; return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            // Every key is a normal key, Escape too: no single key closes the window. Alt+F4 goes on to Windows,
            // which turns it into WM_CLOSE, like the X button.
            s.keys[sidedKey(wp, lp) & 0xff] = true;
            if (msg == WM_SYSKEYDOWN && wp == VK_F4) break;
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP: s.keys[sidedKey(wp, lp) & 0xff] = false; return 0;
        case WM_LBUTTONDOWN: s.mouse[0] = true; return 0;
        case WM_LBUTTONUP: s.mouse[0] = false; return 0;
        case WM_RBUTTONDOWN: s.mouse[1] = true; return 0;
        case WM_RBUTTONUP: s.mouse[1] = false; return 0;
        case WM_MBUTTONDOWN: s.mouse[2] = true; return 0;
        case WM_MBUTTONUP: s.mouse[2] = false; return 0;
        case WM_MOUSEMOVE:
            s.mx = (float)(short)LOWORD(lp);
            s.my = (float)(short)HIWORD(lp);
            return 0;
        case WM_MOUSEWHEEL: s.wheelAcc += (float)GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA; return 0;
        case WM_KILLFOCUS:
            std::memset(s.keys, 0, sizeof s.keys);
            std::memset(s.mouse, 0, sizeof s.mouse);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

std::wstring wide(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
    return w;
}

void parseAutotest() {
    Win& s = win();
    if (const char* f = std::getenv("FRG_AUTOTEST_FRAMES")) s.autotestFrames = std::atol(f);
    if (const char* f = std::getenv("FRG_AUTOTEST_SHOT")) s.autotestShot = f;
    if (const char* f = std::getenv("FRG_AUTOTEST_INPUT")) {
        std::string spec = f;
        size_t start = 0;
        while (start < spec.size()) {
            size_t end = spec.find(',', start);
            if (end == std::string::npos) end = spec.size();
            std::string item = spec.substr(start, end - start);
            size_t colon = item.find(':'), dash = item.find('-');
            if (colon != std::string::npos && dash != std::string::npos) {
                std::string name = item.substr(0, colon);
                for (int i = 0; i < kKeyCount; i++)
                    if (name == kKeyNames[i])
                        s.holds.push_back({i, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
                if (name == "CLOSE")  // the window's X button
                    s.holds.push_back({kScriptedClose, std::atol(item.c_str() + colon + 1), std::atol(item.c_str() + dash + 1)});
            }
            start = end + 1;
        }
    }
}

// Keys held by FRG_AUTOTEST_INPUT: -1 if the key isn't scripted, otherwise 0/1 for `ago` frames ago.
int scripted(int k, long ago) {
    long f = win().frame - ago;
    int result = -1;
    for (auto& h : win().holds) {
        if (h.key != k) continue;
        if (result < 0) result = 0;
        if (f >= h.from && f <= h.to) result = 1;
    }
    return result;
}

}  // namespace

std::string keyName(Key_ k) {
    int i = (int)k;
    return i >= 0 && i < kKeyCount ? kKeyNames[i] : "Key(?)";
}
std::string buttonName(MouseButton_ b) {
    return b == MouseButton_::LEFT_ ? "LEFT" : b == MouseButton_::RIGHT_ ? "RIGHT" : "MIDDLE";
}

void fail(const char* where, const std::string& msg) { panic(where, msg); }

std::string findGameFile(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(path, ec)) return path;
    wchar_t buf[4096];
    DWORD n = GetModuleFileNameW(nullptr, buf, 4096);
    fs::path exeDir = fs::path(std::wstring(buf, n)).parent_path();
    for (fs::path candidate : {exeDir / path, exeDir.parent_path() / path})
        if (fs::exists(candidate, ec)) return candidate.string();
    return "";
}

void open(int32_t width, int32_t height, const std::string& title, void (*present)(), void (*screenshot)(const std::string&)) {
    Win& s = win();
    if (s.open) fail("openWindow()", "the window is already open");
    HINSTANCE inst = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
    wc.lpszClassName = L"ForgeToolkitWindow";
    RegisterClassExW(&wc);

    // A fixed-size window: the toolkits don't resize their drawing surfaces.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r{0, 0, width, height};
    AdjustWindowRect(&r, style, FALSE);
    s.hwnd = CreateWindowExW(0, wc.lpszClassName, wide(title).c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left,
                             r.bottom - r.top, nullptr, nullptr, inst, nullptr);
    if (!s.hwnd) fail("openWindow()", "Windows couldn't create the window");
    s.w = width;
    s.h = height;
    s.open = true;
    s.present = present;
    s.screenshot = screenshot;
    QueryPerformanceFrequency(&s.freq);
    QueryPerformanceCounter(&s.start);
    s.last = s.start;
    parseAutotest();
    // FRG_AUTOTEST_HIDDEN=1: tests run the game without its window showing up on the screen.
    if (!std::getenv("FRG_AUTOTEST_HIDDEN")) ShowWindow(s.hwnd, SW_SHOW);
    UpdateWindow(s.hwnd);
}

bool isOpen() { return win().open; }
void* handle() { return win().hwnd; }
int32_t width() { return win().w; }
int32_t height() { return win().h; }
float deltaTime() { return (float)win().dt; }
double time() {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return double(now.QuadPart - win().start.QuadPart) / double(win().freq.QuadPart);
}
void setTitle(const std::string& title) {
    if (win().hwnd) SetWindowTextW(win().hwnd, wide(title).c_str());
}

bool shouldClose() {
    Win& s = win();
    if (!s.open) return true;
    if (s.inFrame) {
        if (s.autotestFrames >= 0 && s.frame + 1 == s.autotestFrames && s.screenshot) s.screenshot(s.autotestShot);
        if (s.present) s.present();
        s.frame++;
        for (auto fn : frameHooks()) fn();  // e.g. keeping <audio> music playing
    }
    std::memcpy(s.prevKeys, s.keys, sizeof s.keys);
    std::memcpy(s.prevMouse, s.mouse, sizeof s.mouse);
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    s.wheel = s.wheelAcc;
    s.wheelAcc = 0;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    s.dt = double(now.QuadPart - s.last.QuadPart) / double(s.freq.QuadPart);
    if (s.dt > 0.25) s.dt = 0.25;
    s.last = now;
    if (scripted(kScriptedClose, 0) == 1 && scripted(kScriptedClose, 1) == 0) s.closing = true;
    if (s.closing || (s.autotestFrames >= 0 && s.frame >= s.autotestFrames)) return true;
    s.inFrame = true;
    tasks::update_((float)s.dt);
    return false;
}

void close() {
    Win& s = win();
    if (!s.open) return;
    DestroyWindow(s.hwnd);
    s.hwnd = nullptr;
    s.open = false;
}

bool keyDown(Key_ k) {
    int sc = scripted((int)k, 0);
    return sc >= 0 ? sc == 1 : win().keys[virtualKey((int)k)];
}
bool keyPressed(Key_ k) {
    int sc = scripted((int)k, 0);
    if (sc >= 0) return sc == 1 && scripted((int)k, 1) == 0;
    int vk = virtualKey((int)k);
    return win().keys[vk] && !win().prevKeys[vk];
}
bool keyReleased(Key_ k) {
    int sc = scripted((int)k, 0);
    if (sc >= 0) return sc == 0 && scripted((int)k, 1) == 1;
    int vk = virtualKey((int)k);
    return !win().keys[vk] && win().prevKeys[vk];
}
bool mouseDown(MouseButton_ b) { return win().mouse[(int)b]; }
bool mousePressed(MouseButton_ b) { return win().mouse[(int)b] && !win().prevMouse[(int)b]; }
bool mouseReleased(MouseButton_ b) { return !win().mouse[(int)b] && win().prevMouse[(int)b]; }
core::vec2_ mousePosition() { return {win().mx, win().my}; }
float mouseWheel() { return win().wheel; }

bool screenshotDue() { return win().autotestFrames >= 0 && win().frame + 1 == win().autotestFrames; }

bool savePng(const std::string& path, const unsigned char* rgba, int w, int h) {
    FrgRlImage img{(void*)rgba, w, h, 1, 7 /* PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 */};
    return ExportImage(img, path.c_str());
}

}  // namespace frg::native
