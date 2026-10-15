// Forge runtime: <scenes> - switching between parts of a game (menu, level, game over...).
//
// Each scene is a class that inherits from Scene and fills in enter/update/draw/exit.
// scenes.change(next) switches at the start of the next scenes.update(dt).
#pragma once

#include "forge.hpp"

namespace frg::scenes {

struct Scene_ : std::enable_shared_from_this<Scene_> {
    Scene_() = default;
    virtual ~Scene_() = default;
    static std::shared_ptr<Scene_> frg$new() { return std::make_shared<Scene_>(); }
    virtual void enter_() {}
    virtual void update_(float) {}
    virtual void draw_() {}
    virtual void exit_() {}
};

struct State {
    std::shared_ptr<Scene_> current;
    std::shared_ptr<Scene_> next;
};
inline State& st() {
    static State s;
    return s;
}

inline void switchNow() {
    State& s = st();
    if (!s.next) return;
    auto old = s.current;
    s.current = s.next;
    s.next = nullptr;
    if (old) old->exit_();
    s.current->enter_();
}

inline void change_(std::shared_ptr<Scene_> next) {
    State& s = st();
    s.next = next;
    if (!s.current) switchNow();  // the very first scene starts right away
}

inline void update_(float dt) {
    switchNow();
    if (st().current) st().current->update_(dt);
}

inline void draw_() {
    if (st().current) st().current->draw_();
}

inline std::shared_ptr<Scene_> current_() { return st().current; }

inline std::string frg$str(const std::shared_ptr<Scene_>& s) { return s ? "Scene" : "null"; }

}  // namespace frg::scenes
