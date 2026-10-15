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

inline void panel_(core::rect_ area) {
    window::ensureFrame("ui.panel()");
    DrawRectangleRounded(rl(area), 0.06f, 8, rl(core::color_{theme().background.r_, theme().background.g_, theme().background.b_, 230}));
    DrawRectangleRoundedLinesEx(rl(area), 0.06f, 8, 2, rl(shade(theme().background, 0.12f)));
}

inline void setTheme_(core::color_ background, core::color_ accent, core::color_ text) { theme() = {background, accent, text}; }

}  // namespace frg::ui
