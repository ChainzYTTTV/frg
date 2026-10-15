// Forge runtime: <particles> - dust, sparks, smoke and explosions, drawn in 2D.
//
// An Emitter makes particles at its position: a steady stream (rate, per second) and/or all at once (burst).
// Each particle flies off at a random angle within `spread` around `direction`, falls with `gravity`, changes
// from startColor/startSize to endColor/endSize, and disappears after `lifetime` seconds.
#pragma once

#include "forge_window.hpp"

namespace frg::particles {

struct Particle {
    core::vec2_ pos, vel;
    float age = 0, life = 1;
};

struct Emitter_ : std::enable_shared_from_this<Emitter_> {
    core::vec2_ position_{};
    float rate_ = 0;          // particles per second; 0 means only burst() makes them
    float lifetime_ = 1;      // seconds
    float speed_ = 100;       // pixels per second
    float direction_ = -90;   // degrees: 0 is right, -90 is up, 90 is down
    float spread_ = 360;      // degrees around the direction: 360 is every way, 30 a narrow cone
    core::color_ startColor_{255, 255, 255, 255};
    core::color_ endColor_{255, 255, 255, 0};
    float startSize_ = 8;
    float endSize_ = 2;
    core::vec2_ gravity_{0, 0};
    std::vector<Particle> live;
    float owed = 0;  // part-particles carried over between frames, so a rate of 2.5 really makes 2.5 a second

    static constexpr size_t kMax = 5000;

    Emitter_() = default;
    virtual ~Emitter_() = default;
    void frg$init(core::vec2_ p) { position_ = p; }
    static std::shared_ptr<Emitter_> frg$new(core::vec2_ p) {
        auto o = std::make_shared<Emitter_>();
        o->frg$init(p);
        return o;
    }

    void spawn() {
        if (live.size() >= kMax) return;
        auto& e = random::engine();
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);
        float angle = (direction_ + (unit(e) - 0.5f) * spread_) * 3.14159265f / 180.0f;
        float speed = speed_ * (0.7f + 0.6f * unit(e));  // a little different each, so it looks natural
        Particle p;
        p.pos = position_;
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.life = std::max(0.01f, lifetime_ * (0.8f + 0.4f * unit(e)));
        live.push_back(p);
    }

    virtual void burst_(int32_t n) {
        for (int32_t i = 0; i < n; i++) spawn();
    }
    virtual void update_(float dt) {
        if (dt <= 0) return;
        if (rate_ > 0) {
            owed += rate_ * dt;
            for (; owed >= 1; owed -= 1) spawn();
        }
        for (Particle& p : live) {
            p.vel += gravity_ * dt;
            p.pos += p.vel * dt;
            p.age += dt;
        }
        live.erase(std::remove_if(live.begin(), live.end(), [](const Particle& p) { return p.age >= p.life; }), live.end());
    }
    virtual void draw_() {
        window::ensureFrame("emitter.draw()");
        for (const Particle& p : live) {
            float t = std::clamp(p.age / p.life, 0.0f, 1.0f);
            float size = startSize_ + (endSize_ - startSize_) * t;
            if (size <= 0) continue;
            DrawCircleV(rl(p.pos), size / 2, rl(startColor_.lerp_(endColor_, t)));
        }
    }
    virtual int32_t count_() { return (int32_t)live.size(); }
    virtual void clear_() {
        live.clear();
        owed = 0;
    }
};

inline std::string frg$str(const std::shared_ptr<Emitter_>& e) {
    return e ? "Emitter(" + str(e->count_()) + " particles)" : "null";
}

}  // namespace frg::particles
