// Forge runtime: <physics>, simple 2D box physics for platformers.
//
// Every Body is an axis-aligned box. Each step, moving bodies fall with gravity, move by their velocity,
// and get pushed out of static bodies (floors, walls, platforms). Moving bodies don't collide with
// each other - use body.overlaps(other) to check for that yourself.
#pragma once

#include "forge.hpp"

namespace frg::physics {

struct Body_ : std::enable_shared_from_this<Body_> {
    core::vec2_ position_{};
    core::vec2_ size_{};
    core::vec2_ velocity_{};
    bool isStatic_ = false;
    float gravityScale_ = 1;
    bool onGround_ = false;
    bool onCeiling_ = false;
    bool onWall_ = false;

    Body_() = default;
    virtual ~Body_() = default;
    void frg$init(float x, float y, float w, float h) {
        position_ = {x, y};
        size_ = {w, h};
    }
    static std::shared_ptr<Body_> frg$new(float x, float y, float w, float h) {
        auto o = std::make_shared<Body_>();
        o->frg$init(x, y, w, h);
        return o;
    }
    virtual core::rect_ bounds_() { return {position_.x_, position_.y_, size_.x_, size_.y_}; }
    virtual core::vec2_ center_() { return {position_.x_ + size_.x_ / 2, position_.y_ + size_.y_ / 2}; }
    virtual bool overlaps_(std::shared_ptr<Body_> other) { return other && bounds_().overlaps_(other->bounds_()); }
};

struct World {
    std::vector<std::shared_ptr<Body_>> bodies;
    core::vec2_ gravity{0, 980};
};
inline World& world() {
    static World w;
    return w;
}

inline void add_(std::shared_ptr<Body_> b) {
    auto& v = world().bodies;
    if (b && std::find(v.begin(), v.end(), b) == v.end()) v.push_back(b);
}
inline void remove_(std::shared_ptr<Body_> b) {
    auto& v = world().bodies;
    v.erase(std::remove(v.begin(), v.end(), b), v.end());
}
inline void clear_() { world().bodies.clear(); }
inline void setGravity_(core::vec2_ g) { world().gravity = g; }
inline core::vec2_ gravity_() { return world().gravity; }
inline int32_t count_() { return (int32_t)world().bodies.size(); }

inline bool touching(const Body_& a, const Body_& b) {
    return a.position_.x_ < b.position_.x_ + b.size_.x_ && b.position_.x_ < a.position_.x_ + a.size_.x_ &&
           a.position_.y_ < b.position_.y_ + b.size_.y_ && b.position_.y_ < a.position_.y_ + a.size_.y_;
}

inline void step_(float dt) {
    if (dt <= 0) return;
    if (dt > 0.05f) dt = 0.05f;  // after a long pause, don't jump everything forward at once
    auto bodies = world().bodies;  // a copy, in case the list changes while stepping
    for (auto& bp : bodies) {
        Body_& b = *bp;
        if (b.isStatic_) continue;
        b.onGround_ = b.onCeiling_ = b.onWall_ = false;
        b.velocity_ += world().gravity * (b.gravityScale_ * dt);

        // Move in small steps so fast bodies can't pass straight through thin platforms.
        core::vec2_ move = b.velocity_ * dt;
        float sx = b.size_.x_ > 1 ? b.size_.x_ / 2 : 1, sy = b.size_.y_ > 1 ? b.size_.y_ / 2 : 1;
        int steps = (int)std::ceil(std::max(std::fabs(move.x_) / sx, std::fabs(move.y_) / sy));
        steps = std::clamp(steps, 1, 64);
        float part = dt / steps;

        for (int s = 0; s < steps; s++) {
            b.position_.x_ += b.velocity_.x_ * part;
            for (auto& op : bodies) {
                Body_& o = *op;
                if (&o == &b || !o.isStatic_ || !touching(b, o)) continue;
                if (b.velocity_.x_ > 0)
                    b.position_.x_ = o.position_.x_ - b.size_.x_;
                else if (b.velocity_.x_ < 0)
                    b.position_.x_ = o.position_.x_ + o.size_.x_;
                b.velocity_.x_ = 0;
                b.onWall_ = true;
            }
            b.position_.y_ += b.velocity_.y_ * part;
            for (auto& op : bodies) {
                Body_& o = *op;
                if (&o == &b || !o.isStatic_ || !touching(b, o)) continue;
                if (b.velocity_.y_ > 0) {
                    b.position_.y_ = o.position_.y_ - b.size_.y_;
                    b.onGround_ = true;
                } else if (b.velocity_.y_ < 0) {
                    b.position_.y_ = o.position_.y_ + o.size_.y_;
                    b.onCeiling_ = true;
                }
                b.velocity_.y_ = 0;
            }
        }
    }
}

inline std::string frg$str(const std::shared_ptr<Body_>& b) {
    if (!b) return "null";
    return "Body(position: " + core::frg$str(b->position_) + ", size: " + core::frg$str(b->size_) +
           ", velocity: " + core::frg$str(b->velocity_) + ", onGround: " + str(b->onGround_) + ")";
}

}  // namespace frg::physics
