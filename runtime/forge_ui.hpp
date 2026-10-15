// Forge runtime: <ui> - buttons, checkboxes, sliders and labels.
//
// These are drawn every frame, like everything else: `if (ui.button(area, "Play")) { ... }`
// draws the button and is true on the frame it gets clicked.
#pragma once

#include "forge_window.hpp"

namespace frg::ui {

struct Theme {
    core::color_ background{52, 58, 82, 255};
    core::color_ accent{98, 160, 255, 255};
    core::color_ text{240, 240, 245, 255};
};
inline Theme& theme() {
    static Theme t;
    return t;
}

inline core::color_ shade(core::color_ c, float amount) {
    auto f = [&](uint8_t v) { return uint8_t(std::clamp(v + 255 * amount, 0.0f, 255.0f)); };
    return {f(c.r_), f(c.g_), f(c.b_), c.a_};
}

inline bool hovering(core::rect_ area) {
    ::Vector2 m = GetMousePosition();
    return CheckCollisionPointRec(m, rl(area));
}

inline void centeredText(const std::string& text, core::rect_ area, int size, core::color_ c) {
    int w = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), (int)(area.x_ + (area.width_ - w) / 2), (int)(area.y_ + (area.height_ - size) / 2), size, rl(c));
}

inline int fontSizeFor(core::rect_ area) { return std::clamp((int)(area.height_ * 0.5f), 10, 40); }

inline bool button_(core::rect_ area, const std::string& label) {
    window::ensureFrame("ui.button()");
    bool over = hovering(area);
    bool down = over && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    core::color_ bg = down ? shade(theme().accent, -0.15f) : over ? theme().accent : theme().background;
    DrawRectangleRounded(rl(area), 0.25f, 6, rl(bg));
    DrawRectangleRoundedLinesEx(rl(area), 0.25f, 6, 2, rl(shade(theme().accent, over ? 0.15f : 0)));
    centeredText(label, area, fontSizeFor(area), theme().text);
    return over && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

inline void label_(const std::string& text, float x, float y, int32_t size, core::color_ c) {
    window::ensureFrame("ui.label()");
    DrawText(text.c_str(), (int)x, (int)y, size, rl(c));
}

inline bool checkbox_(core::rect_ area, const std::string& label, bool checked) {
    window::ensureFrame("ui.checkbox()");
    bool over = hovering(area);
    if (over && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) checked = !checked;
    float box = area.height_;
    ::Rectangle b{area.x_, area.y_, box, box};
    DrawRectangleRounded(b, 0.2f, 4, rl(over ? shade(theme().background, 0.08f) : theme().background));
    DrawRectangleRoundedLinesEx(b, 0.2f, 4, 2, rl(theme().accent));
    if (checked) DrawRectangleRounded(::Rectangle{b.x + box * 0.22f, b.y + box * 0.22f, box * 0.56f, box * 0.56f}, 0.2f, 4, rl(theme().accent));
    int size = fontSizeFor(area);
    DrawText(label.c_str(), (int)(area.x_ + box + 10), (int)(area.y_ + (area.height_ - size) / 2), size, rl(theme().text));
    return checked;
}

inline float slider_(core::rect_ area, float value, float lo, float hi) {
    window::ensureFrame("ui.slider()");
    if (hi <= lo) return lo;
    bool over = hovering(area);
    if (over && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float t = std::clamp((GetMousePosition().x - area.x_) / area.width_, 0.0f, 1.0f);
        value = lo + t * (hi - lo);
    }
    value = std::clamp(value, lo, hi);
    float t = (value - lo) / (hi - lo);
    float trackH = std::max(4.0f, area.height_ * 0.25f);
    ::Rectangle track{area.x_, area.y_ + (area.height_ - trackH) / 2, area.width_, trackH};
    DrawRectangleRounded(track, 1, 6, rl(theme().background));
    DrawRectangleRounded(::Rectangle{track.x, track.y, track.width * t, track.height}, 1, 6, rl(theme().accent));
    DrawCircleV(::Vector2{area.x_ + area.width_ * t, area.y_ + area.height_ / 2}, area.height_ * 0.4f,
                rl(over ? shade(theme().text, 0) : shade(theme().text, -0.1f)));
    return value;
}

// ---- text boxes. Only one box is being typed into at a time; it's known by its rectangle.
struct TextFocus {
    bool active = false;
    core::rect_ area{};
    long frame = -1;  // the frame it was last drawn in, so a box that stops being drawn loses the focus
};
inline TextFocus& textFocus() {
    static TextFocus f;
    return f;
}
inline bool sameArea(core::rect_ a, core::rect_ b) {
    return a.x_ == b.x_ && a.y_ == b.y_ && a.width_ == b.width_ && a.height_ == b.height_;
}
inline size_t charCount(const std::string& s) {
    size_t n = 0;
    for (unsigned char c : s) n += (c & 0xC0) != 0x80;
    return n;
}
inline void appendUtf8(std::string& s, int cp) {
    if (cp < 0x80) {
        s += (char)cp;
    } else if (cp < 0x800) {
        s += (char)(0xC0 | (cp >> 6)), s += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        s += (char)(0xE0 | (cp >> 12)), s += (char)(0x80 | ((cp >> 6) & 0x3F)), s += (char)(0x80 | (cp & 0x3F));
    } else {
        s += (char)(0xF0 | (cp >> 18)), s += (char)(0x80 | ((cp >> 12) & 0x3F)), s += (char)(0x80 | ((cp >> 6) & 0x3F)),
            s += (char)(0x80 | (cp & 0x3F));
    }
}

// True while the player is typing in a text box, so the game can ignore its own key shortcuts meanwhile.
inline bool isTyping_() {
    TextFocus& f = textFocus();
    return f.active && f.frame >= window::st().frame - 1;
}

inline std::string textBox_(core::rect_ area, std::string text, int32_t maxLength) {
    window::ensureFrame("ui.textBox()");
    TextFocus& f = textFocus();
    long frame = window::st().frame;
    if (f.active && f.frame < frame - 1) f.active = false;  // the focused box wasn't drawn last frame
    bool mine = f.active && sameArea(f.area, area);
    bool over = hovering(area);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (over) {
            f.active = true;
            f.area = area;
            mine = true;
        } else if (mine) {
            f.active = mine = false;  // a click somewhere else
        }
    }
    if (mine) {
        f.frame = frame;
        for (int cp = GetCharPressed(); cp > 0; cp = GetCharPressed())
            if (cp >= 32 && (maxLength <= 0 || charCount(text) < (size_t)maxLength)) appendUtf8(text, cp);
        // Keys come from raylib's queue, so even a very quick tap (pressed and let go within one frame) counts.
        int backspaces = IsKeyPressedRepeat(KEY_BACKSPACE) ? 1 : 0;
        bool enter = false;
        for (int k = GetKeyPressed(); k != 0; k = GetKeyPressed()) {
            if (k == KEY_BACKSPACE) backspaces++;
            if (k == KEY_ENTER || k == KEY_KP_ENTER) enter = true;
        }
        for (; backspaces > 0 && !text.empty(); backspaces--) {
            size_t end = text.size() - 1;
            while (end > 0 && ((unsigned char)text[end] & 0xC0) == 0x80) end--;  // a whole letter, even é
            text.erase(end);
        }
        if (enter) f.active = mine = false;
    }

    DrawRectangleRounded(rl(area), 0.2f, 6, rl(mine ? shade(theme().background, 0.08f) : theme().background));
    DrawRectangleRoundedLinesEx(rl(area), 0.2f, 6, 2, rl(mine ? theme().accent : shade(theme().background, over ? 0.25f : 0.15f)));
    int size = fontSizeFor(area);
    float pad = std::max(6.0f, area.height_ * 0.2f);
    std::string shown = text;
    // Too long to fit: show the end, which is where the typing happens.
    while (!shown.empty() && MeasureText(shown.c_str(), size) > area.width_ - pad * 2 - 4) {
        size_t cut = 1;
        while (cut < shown.size() && ((unsigned char)shown[cut] & 0xC0) == 0x80) cut++;
        shown.erase(0, cut);
    }
    int y = (int)(area.y_ + (area.height_ - size) / 2);
    DrawText(shown.c_str(), (int)(area.x_ + pad), y, size, rl(theme().text));
    if (mine && std::fmod(GetTime(), 1.0) < 0.6) {
        int x = (int)(area.x_ + pad) + MeasureText(shown.c_str(), size) + 2;
        DrawRectangle(x, y, 2, size, rl(theme().accent));
    }
    return text;
}

inline void panel_(core::rect_ area) {
    window::ensureFrame("ui.panel()");
    DrawRectangleRounded(rl(area), 0.06f, 8, rl(core::color_{theme().background.r_, theme().background.g_, theme().background.b_, 230}));
    DrawRectangleRoundedLinesEx(rl(area), 0.06f, 8, 2, rl(shade(theme().background, 0.12f)));
}

inline void setTheme_(core::color_ background, core::color_ accent, core::color_ text) { theme() = {background, accent, text}; }

}  // namespace frg::ui
