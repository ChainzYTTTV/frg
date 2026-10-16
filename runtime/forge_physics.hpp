// Forge runtime: <physics>, simple 2D box physics for platformers.
//
// Every Body is an axis-aligned box. Each step, moving bodies fall with gravity, move by their velocity,
// and get pushed out of static bodies (floors, walls, platforms), then out of each other.
// Bodies with solid = false (coins, triggers) never push or get pushed - check them with overlaps().
// Static bodies with oneWay = true are platforms you can jump up through and land on.
#pragma once

#include "forge.hpp"

namespace frg::physics {

struct Body_ : std::enable_shared_from_this<Body_> {
    core::vec2_ position_{};
    core::vec2_ size_{};
    core::vec2_ velocity_{};
    bool isStatic_ = false;
    bool solid_ = true;
    bool oneWay_ = false;  // static bodies only: moving bodies pass through from below and the sides, and land on top
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

// Pushes overlapping moving bodies apart along whichever direction they overlap least.
// A body resting on the ground doesn't get pushed down; the one on top is pushed up instead.
inline void separateMovingBodies(const std::vector<std::shared_ptr<Body_>>& bodies) {
    for (size_t i = 0; i < bodies.size(); i++) {
        Body_& a = *bodies[i];
        if (a.isStatic_ || !a.solid_) continue;
        for (size_t j = i + 1; j < bodies.size(); j++) {
            Body_& b = *bodies[j];
            if (b.isStatic_ || !b.solid_ || !touching(a, b)) continue;
            float ox = std::min(a.position_.x_ + a.size_.x_, b.position_.x_ + b.size_.x_) - std::max(a.position_.x_, b.position_.x_);
            float oy = std::min(a.position_.y_ + a.size_.y_, b.position_.y_ + b.size_.y_) - std::max(a.position_.y_, b.position_.y_);
            if (oy <= ox) {
                Body_& top = a.position_.y_ < b.position_.y_ ? a : b;
                Body_& bottom = &top == &a ? b : a;
                float topShare = bottom.onGround_ ? 1.0f : 0.5f;
                top.position_.y_ -= oy * topShare;
                bottom.position_.y_ += oy * (1 - topShare);
                if (top.velocity_.y_ > bottom.velocity_.y_) top.velocity_.y_ = bottom.velocity_.y_;
                top.onGround_ = true;
                bottom.onCeiling_ = true;
            } else {
                Body_& left = a.position_.x_ < b.position_.x_ ? a : b;
                Body_& right = &left == &a ? b : a;
                left.position_.x_ -= ox / 2;
                right.position_.x_ += ox / 2;
                left.onWall_ = right.onWall_ = true;
            }
        }
    }
}

// One small step of the world (see step_ below).
inline void tick(float dt) {
    auto bodies = world().bodies;  // a copy, in case the list changes while stepping
    for (auto& bp : bodies) {
        Body_& b = *bp;
        if (b.isStatic_) continue;
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
                if (&o == &b || !o.isStatic_ || o.oneWay_ || !b.solid_ || !o.solid_ || !touching(b, o)) continue;
                if (b.velocity_.x_ > 0)
                    b.position_.x_ = o.position_.x_ - b.size_.x_;
                else if (b.velocity_.x_ < 0)
                    b.position_.x_ = o.position_.x_ + o.size_.x_;
                b.velocity_.x_ = 0;
                b.onWall_ = true;
            }
            float bottomBefore = b.position_.y_ + b.size_.y_;
            b.position_.y_ += b.velocity_.y_ * part;
            for (auto& op : bodies) {
                Body_& o = *op;
                if (&o == &b || !o.isStatic_ || !b.solid_ || !o.solid_ || !touching(b, o)) continue;
                // A one-way platform only stops a body that's falling onto it from above.
                if (o.oneWay_ && (b.velocity_.y_ <= 0 || bottomBefore > o.position_.y_ + 0.01f)) continue;
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
    separateMovingBodies(bodies);
}

// physics.step(dt): the frame is split into equal parts of at most 1/240 s, so a jump goes as high at 30 FPS as at
// 144 FPS (the frame time only changes how many parts there are). onGround, onCeiling and onWall describe the
// whole frame: true if it happened in any part.
inline void step_(float dt) {
    if (dt <= 0) return;
    if (dt > 0.05f) dt = 0.05f;  // after a long pause, don't jump everything forward at once
    int parts = std::max(1, (int)std::ceil(dt * 240.0f - 1e-4f));
    for (auto& b : world().bodies)
        if (!b->isStatic_) b->onGround_ = b->onCeiling_ = b->onWall_ = false;
    for (int i = 0; i < parts; i++) tick(dt / parts);
}

inline std::string frg$str(const std::shared_ptr<Body_>& b) {
    if (!b) return "null";
    return "Body(position: " + core::frg$str(b->position_) + ", size: " + core::frg$str(b->size_) +
           ", velocity: " + core::frg$str(b->velocity_) + ", onGround: " + str(b->onGround_) + ")";
}

}  // namespace frg::physics
