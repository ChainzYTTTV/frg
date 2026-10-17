// Forge runtime: <physics>, simple 2D physics for platformers.
//
// A Body is an axis-aligned box (it never turns), a circle (circle = true: a ball as wide as the box), or, for
// static bodies, a ramp (slope = 1 rises to the right, -1 to the left). Each step, moving bodies fall with gravity,
// move by their velocity, and get pushed out of static bodies (floors, walls, platforms, ramps), then out of each
// other. Bodies with solid = false (coins, triggers) never push or get pushed - check them with overlaps().
// Static bodies with oneWay = true are platforms you can jump up through and land on.
// Layers: every body has a layer (0-31); physics.collide(a, b, false) makes layers a and b pass through each other.
// bounce (0-1): how much of its speed a body keeps when it hits something (0 stops, 1 bounces back as fast).
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
    int32_t layer_ = 0;     // 0-31: which layer it's on (see physics.collide)
    bool circle_ = false;   // a circle as wide as the box (the smaller of width and height), in its middle
    int32_t slope_ = 0;     // static bodies: 1 = a ramp rising to the right, -1 rising to the left, 0 = a box
    float bounce_ = 0;      // 0-1: how much speed is kept when it hits something
    std::weak_ptr<Body_> standingOn;   // the ramp it stood on in the last step (to follow it downhill)

    float radius() const { return std::min(size_.x_, size_.y_) / 2; }
    core::vec2_ middle() const { return {position_.x_ + size_.x_ / 2, position_.y_ + size_.y_ / 2}; }

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
    virtual bool overlaps_(std::shared_ptr<Body_> other);
};

struct World {
    std::vector<std::shared_ptr<Body_>> bodies;
    core::vec2_ gravity{0, 980};
    bool pairs[32][32];   // which layers collide with which (all of them, to start with)
    World() {
        for (auto& row : pairs)
            for (bool& p : row) p = true;
    }
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

inline int32_t checkLayer(int32_t layer, const char* what) {
    if (layer < 0 || layer > 31) panic(what, "layers go from 0 to 31, but this one is " + std::to_string(layer));
    return layer;
}
// physics.collide(a, b, false): bodies on layers a and b pass through each other (a and b can be the same layer:
// collide(2, 2, false) and bullets on layer 2 don't hit each other). true makes them collide again.
inline void collide_(int32_t a, int32_t b, bool on) {
    checkLayer(a, "physics.collide()");
    checkLayer(b, "physics.collide()");
    world().pairs[a][b] = world().pairs[b][a] = on;
}
inline bool collides_(int32_t a, int32_t b) {
    checkLayer(a, "physics.collides()");
    checkLayer(b, "physics.collides()");
    return world().pairs[a][b];
}
inline bool pairCollides(const Body_& a, const Body_& b) {
    return world().pairs[std::clamp(a.layer_, 0, 31)][std::clamp(b.layer_, 0, 31)];
}

// ---- shapes

// The height (y) of a ramp's top at x.
inline float slopeTop(const Body_& o, float x) {
    float t = o.size_.x_ > 0 ? std::clamp((x - o.position_.x_) / o.size_.x_, 0.0f, 1.0f) : 0.0f;
    return o.slope_ > 0 ? o.position_.y_ + o.size_.y_ * (1 - t) : o.position_.y_ + o.size_.y_ * t;
}

// Where a box rests on a ramp: the highest point of the ramp's top under the box (as a real box would).
inline float rampTopUnder(const Body_& o, const Body_& b) {
    float x0 = std::clamp(b.position_.x_, o.position_.x_, o.position_.x_ + o.size_.x_);
    float x1 = std::clamp(b.position_.x_ + b.size_.x_, o.position_.x_, o.position_.x_ + o.size_.x_);
    return std::min(slopeTop(o, x0), slopeTop(o, x1));
}

inline core::vec2_ closestOnSegment(core::vec2_ p, core::vec2_ a, core::vec2_ b) {
    core::vec2_ ab{b.x_ - a.x_, b.y_ - a.y_};
    float len2 = ab.x_ * ab.x_ + ab.y_ * ab.y_;
    float t = len2 > 0 ? std::clamp(((p.x_ - a.x_) * ab.x_ + (p.y_ - a.y_) * ab.y_) / len2, 0.0f, 1.0f) : 0.0f;
    return {a.x_ + ab.x_ * t, a.y_ + ab.y_ * t};
}

// The point of a static body's shape nearest to p, and whether p is inside it.
inline core::vec2_ closestPoint(const Body_& o, core::vec2_ p, bool& inside) {
    float x0 = o.position_.x_, y0 = o.position_.y_, x1 = x0 + o.size_.x_, y1 = y0 + o.size_.y_;
    if (o.circle_) {
        core::vec2_ c = o.middle();
        float dx = p.x_ - c.x_, dy = p.y_ - c.y_, len = std::sqrt(dx * dx + dy * dy), r = o.radius();
        inside = len < r;
        if (len < 1e-6f) return {c.x_, c.y_ - r};
        return {c.x_ + dx / len * r, c.y_ + dy / len * r};
    }
    if (o.slope_ == 0) {
        core::vec2_ q{std::clamp(p.x_, x0, x1), std::clamp(p.y_, y0, y1)};
        inside = p.x_ > x0 && p.x_ < x1 && p.y_ > y0 && p.y_ < y1;
        if (!inside) return q;
        // inside: the nearest edge
        float dl = p.x_ - x0, dr = x1 - p.x_, dt = p.y_ - y0, db = y1 - p.y_, m = std::min({dl, dr, dt, db});
        if (m == dt) return {p.x_, y0};
        if (m == db) return {p.x_, y1};
        if (m == dl) return {x0, p.y_};
        return {x1, p.y_};
    }
    // a ramp: the triangle under its slanted top
    core::vec2_ a = o.slope_ > 0 ? core::vec2_{x0, y1} : core::vec2_{x0, y0};
    core::vec2_ b = o.slope_ > 0 ? core::vec2_{x1, y0} : core::vec2_{x1, y1};
    core::vec2_ c = o.slope_ > 0 ? core::vec2_{x1, y1} : core::vec2_{x0, y1};
    inside = p.x_ > x0 && p.x_ < x1 && p.y_ < y1 && p.y_ > slopeTop(o, p.x_);
    core::vec2_ best = closestOnSegment(p, a, b);
    auto d2 = [&](core::vec2_ q) { return (q.x_ - p.x_) * (q.x_ - p.x_) + (q.y_ - p.y_) * (q.y_ - p.y_); };
    for (core::vec2_ q : {closestOnSegment(p, b, c), closestOnSegment(p, c, a)})
        if (d2(q) < d2(best)) best = q;
    return best;
}

inline bool overlapsShape(const Body_& a, const Body_& b) {
    if (!a.circle_ && !b.circle_) {
        return a.position_.x_ < b.position_.x_ + b.size_.x_ && b.position_.x_ < a.position_.x_ + a.size_.x_ &&
               a.position_.y_ < b.position_.y_ + b.size_.y_ && b.position_.y_ < a.position_.y_ + a.size_.y_;
    }
    const Body_& c = a.circle_ ? a : b;
    const Body_& o = a.circle_ ? b : a;
    core::vec2_ m = c.middle();
    if (o.circle_) {
        core::vec2_ n = o.middle();
        float dx = m.x_ - n.x_, dy = m.y_ - n.y_, r = c.radius() + o.radius();
        return dx * dx + dy * dy < r * r;
    }
    bool inside = false;
    core::vec2_ q = closestPoint(o, m, inside);
    float dx = m.x_ - q.x_, dy = m.y_ - q.y_;
    return inside || dx * dx + dy * dy < c.radius() * c.radius();
}
inline bool Body_::overlaps_(std::shared_ptr<Body_> other) { return other && overlapsShape(*this, *other); }

// The speed after hitting something: bounced back (if it's fast enough to bounce), or stopped.
inline float bounced(float v, float bounce) { return bounce > 0 && std::fabs(v) > 40 ? -v * bounce : 0; }

inline bool touching(const Body_& a, const Body_& b) {
    return a.position_.x_ < b.position_.x_ + b.size_.x_ && b.position_.x_ < a.position_.x_ + a.size_.x_ &&
           a.position_.y_ < b.position_.y_ + b.size_.y_ && b.position_.y_ < a.position_.y_ + a.size_.y_;
}

// Two moving balls: pushed apart along the line between their middles, and they bounce off each other.
inline void separateCircles(Body_& a, Body_& b) {
    core::vec2_ ca = a.middle(), cb = b.middle();
    float dx = cb.x_ - ca.x_, dy = cb.y_ - ca.y_, dist = std::sqrt(dx * dx + dy * dy), r = a.radius() + b.radius();
    if (dist >= r) return;
    float nx = dist > 1e-6f ? dx / dist : 0, ny = dist > 1e-6f ? dy / dist : 1;
    float push = (r - dist) / 2;
    a.position_.x_ -= nx * push;
    a.position_.y_ -= ny * push;
    b.position_.x_ += nx * push;
    b.position_.y_ += ny * push;
    float rel = (b.velocity_.x_ - a.velocity_.x_) * nx + (b.velocity_.y_ - a.velocity_.y_) * ny;
    if (rel < 0) {
        float j = -(1 + std::max(a.bounce_, b.bounce_)) * rel / 2;
        a.velocity_.x_ -= nx * j;
        a.velocity_.y_ -= ny * j;
        b.velocity_.x_ += nx * j;
        b.velocity_.y_ += ny * j;
    }
    if (ny > 0.5f) {
        a.onGround_ = true;
        b.onCeiling_ = true;
    } else if (ny < -0.5f) {
        b.onGround_ = true;
        a.onCeiling_ = true;
    } else {
        a.onWall_ = b.onWall_ = true;
    }
}

// A moving ball against the static bodies: pushed out along the shortest way, and its speed into the surface is
// bounced or stopped (it keeps sliding along it, so it rolls down ramps).
inline void circleAgainstStatic(Body_& b, const std::vector<std::shared_ptr<Body_>>& bodies, float bottomBefore) {
    for (auto& op : bodies) {
        Body_& o = *op;
        if (&o == &b || !o.isStatic_ || !o.solid_ || !pairCollides(b, o)) continue;
        core::vec2_ m = b.middle();
        float r = b.radius();
        bool inside = false;
        core::vec2_ q = closestPoint(o, m, inside);
        float dx = m.x_ - q.x_, dy = m.y_ - q.y_, dist = std::sqrt(dx * dx + dy * dy);
        if (!inside && dist >= r) continue;
        float nx, ny;
        if (dist > 1e-6f) {
            nx = dx / dist;
            ny = dy / dist;
            if (inside) {
                nx = -nx;
                ny = -ny;
            }
        } else {
            nx = 0;
            ny = -1;
        }
        // a one-way platform only holds a ball falling onto its top
        if (o.oneWay_ && (ny > -0.5f || b.velocity_.y_ < 0 || bottomBefore > o.position_.y_ + 0.01f)) continue;
        float depth = inside ? r + dist : r - dist;
        b.position_.x_ += nx * depth;
        b.position_.y_ += ny * depth;
        float vn = b.velocity_.x_ * nx + b.velocity_.y_ * ny;
        if (vn < 0) {
            float keep = -vn > 40 ? b.bounce_ : 0;
            b.velocity_.x_ -= nx * vn * (1 + keep);
            b.velocity_.y_ -= ny * vn * (1 + keep);
        }
        if (ny < -0.5f)
            b.onGround_ = true;
        else if (ny > 0.5f)
            b.onCeiling_ = true;
        else
            b.onWall_ = true;
    }
}

// Pushes overlapping moving bodies apart along whichever direction they overlap least.
// A body resting on the ground doesn't get pushed down; the one on top is pushed up instead.
inline void separateMovingBodies(const std::vector<std::shared_ptr<Body_>>& bodies) {
    for (size_t i = 0; i < bodies.size(); i++) {
        Body_& a = *bodies[i];
        if (a.isStatic_ || !a.solid_) continue;
        for (size_t j = i + 1; j < bodies.size(); j++) {
            Body_& b = *bodies[j];
            if (b.isStatic_ || !b.solid_ || !pairCollides(a, b) || !touching(a, b)) continue;
            if (a.circle_ && b.circle_) {
                separateCircles(a, b);
                continue;
            }
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
            if (b.circle_) {
                float bottomBefore = b.position_.y_ + b.size_.y_;
                b.position_ += b.velocity_ * part;
                if (b.solid_) circleAgainstStatic(b, bodies, bottomBefore);
                continue;
            }
            float stepX = std::fabs(b.velocity_.x_ * part);
            b.position_.x_ += b.velocity_.x_ * part;
            for (auto& op : bodies) {
                Body_& o = *op;
                if (&o == &b || !o.isStatic_ || o.oneWay_ || !b.solid_ || !o.solid_ || !pairCollides(b, o) || !touching(b, o)) continue;
                if (o.slope_ != 0) {
                    // walking up a ramp: the vertical part below lifts it. Only its tall side is a wall.
                    float climb = b.position_.y_ + b.size_.y_ - rampTopUnder(o, b);
                    if (climb <= stepX * 2 + 1) continue;
                }
                if (b.velocity_.x_ > 0)
                    b.position_.x_ = o.position_.x_ - b.size_.x_;
                else if (b.velocity_.x_ < 0)
                    b.position_.x_ = o.position_.x_ + o.size_.x_;
                b.velocity_.x_ = bounced(b.velocity_.x_, b.bounce_);
                b.onWall_ = true;
            }
            float bottomBefore = b.position_.y_ + b.size_.y_, topBefore = b.position_.y_;
            b.position_.y_ += b.velocity_.y_ * part;
            bool landed = false;
            for (auto& op : bodies) {
                Body_& o = *op;
                if (&o == &b || !o.isStatic_ || !b.solid_ || !o.solid_ || !pairCollides(b, o) || !touching(b, o)) continue;
                if (o.slope_ != 0) {
                    // a ramp: its top is slanted (a box rests on its highest point under it), its underside flat
                    float top = rampTopUnder(o, b);
                    float bottom = b.position_.y_ + b.size_.y_;
                    if (b.velocity_.y_ >= 0 && bottom > top && bottomBefore <= top + stepX * 2 + 1) {
                        b.position_.y_ = top - b.size_.y_;
                        b.velocity_.y_ = bounced(b.velocity_.y_, b.bounce_);
                        b.onGround_ = landed = true;
                        b.standingOn = op;
                    } else if (b.velocity_.y_ < 0 && topBefore >= o.position_.y_ + o.size_.y_ - 0.01f) {
                        b.position_.y_ = o.position_.y_ + o.size_.y_;
                        b.velocity_.y_ = bounced(b.velocity_.y_, b.bounce_);
                        b.onCeiling_ = true;
                    }
                    continue;
                }
                // A one-way platform only stops a body that's falling onto it from above.
                if (o.oneWay_ && (b.velocity_.y_ <= 0 || bottomBefore > o.position_.y_ + 0.01f)) continue;
                if (b.velocity_.y_ > 0) {
                    b.position_.y_ = o.position_.y_ - b.size_.y_;
                    b.onGround_ = landed = true;
                    b.standingOn.reset();
                } else if (b.velocity_.y_ < 0) {
                    b.position_.y_ = o.position_.y_ + o.size_.y_;
                    b.onCeiling_ = true;
                }
                b.velocity_.y_ = bounced(b.velocity_.y_, b.bounce_);
            }
            // walking down a ramp: stay on it instead of skipping down the slope in small falls
            if (auto ramp = b.standingOn.lock(); ramp && !landed) {
                const Body_& o = *ramp;
                float gap = rampTopUnder(o, b) - (b.position_.y_ + b.size_.y_);
                bool over = b.position_.x_ < o.position_.x_ + o.size_.x_ && b.position_.x_ + b.size_.x_ > o.position_.x_;
                if (over && b.velocity_.y_ >= 0 && gap >= 0 && gap <= stepX * 2 + 1) {
                    b.position_.y_ += gap;
                    b.velocity_.y_ = 0;
                    b.onGround_ = true;
                } else {
                    b.standingOn.reset();
                }
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
