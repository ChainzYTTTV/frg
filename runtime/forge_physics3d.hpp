// Forge runtime: <physics3d>, simple 3D physics: boxes and spheres that fall, move and collide.
//
// The same ideas as <physics> (2D), in 3D world space (y up, meters, like <graphics3d>). A Body3D is a box that
// never turns, or a sphere (sphere = true: a ball as wide as the smallest side of its box). `position` is the
// middle. Each step, moving bodies fall with gravity, move by their velocity (in small parts, so fast ones can't
// pass through thin walls), get pushed out of static bodies, then out of each other. A moving box with a
// stepHeight climbs steps that low by itself (stairs). raycast() finds the first body a line hits.
// Layers: every body has one (0-31); physics3d.collide(a, b, false) makes two layers pass through each other.
#pragma once

#include "forge.hpp"

namespace frg::physics3d {

struct Body3D_ : std::enable_shared_from_this<Body3D_> {
    core::vec3_ position_{};   // the middle
    core::vec3_ size_{1, 1, 1};
    core::vec3_ velocity_{};
    bool isStatic_ = false;
    bool solid_ = true;
    float gravityScale_ = 1;
    float bounce_ = 0;
    int32_t layer_ = 0;
    bool sphere_ = false;
    float stepHeight_ = 0;     // moving boxes: climb steps up to this high by themselves
    bool onGround_ = false;
    bool onCeiling_ = false;
    bool onWall_ = false;
    bool grounded = false;     // standing on something in the last small step (for climbing steps)

    Body3D_() = default;
    virtual ~Body3D_() = default;
    void frg$init(core::vec3_ position, core::vec3_ size) {
        position_ = position;
        size_ = size;
    }
    static std::shared_ptr<Body3D_> frg$new(core::vec3_ position, core::vec3_ size) {
        auto o = std::make_shared<Body3D_>();
        o->frg$init(position, size);
        return o;
    }
    float radius() const { return std::min({size_.x_, size_.y_, size_.z_}) / 2; }
    core::vec3_ lo() const { return {position_.x_ - size_.x_ / 2, position_.y_ - size_.y_ / 2, position_.z_ - size_.z_ / 2}; }
    core::vec3_ hi() const { return {position_.x_ + size_.x_ / 2, position_.y_ + size_.y_ / 2, position_.z_ + size_.z_ / 2}; }
    virtual core::vec3_ minCorner_() { return lo(); }
    virtual core::vec3_ maxCorner_() { return hi(); }
    virtual bool overlaps_(std::shared_ptr<Body3D_> other);
};

// What a raycast hit (hit is false when it hit nothing).
struct RayHit_ {
    bool hit_ = false;
    core::vec3_ point_{};
    core::vec3_ normal_{};      // the direction the surface faces where it was hit
    float distance_ = 0;
    std::shared_ptr<Body3D_> body_;
    static RayHit_ frg$make() { return {}; }
    bool operator==(const RayHit_&) const = default;
};
inline std::string frg$str(const RayHit_& h) {
    if (!h.hit_) return "RayHit(nothing)";
    return "RayHit(point: " + core::frg$str(h.point_) + ", distance: " + str(h.distance_) + ")";
}

struct World {
    std::vector<std::shared_ptr<Body3D_>> bodies;
    core::vec3_ gravity{0, -9.81f, 0};
    bool pairs[32][32];
    World() {
        for (auto& row : pairs)
            for (bool& p : row) p = true;
    }
};
inline World& world() {
    static World w;
    return w;
}

inline void add_(std::shared_ptr<Body3D_> b) {
    auto& v = world().bodies;
    if (b && std::find(v.begin(), v.end(), b) == v.end()) v.push_back(b);
}
inline void remove_(std::shared_ptr<Body3D_> b) {
    auto& v = world().bodies;
    v.erase(std::remove(v.begin(), v.end(), b), v.end());
}
inline void clear_() { world().bodies.clear(); }
inline void setGravity_(core::vec3_ g) { world().gravity = g; }
inline core::vec3_ gravity_() { return world().gravity; }
inline int32_t count_() { return (int32_t)world().bodies.size(); }

inline int32_t checkLayer(int32_t layer, const char* what) {
    if (layer < 0 || layer > 31) panic(what, "layers go from 0 to 31, but this one is " + std::to_string(layer));
    return layer;
}
inline void collide_(int32_t a, int32_t b, bool on) {
    checkLayer(a, "physics3d.collide()");
    checkLayer(b, "physics3d.collide()");
    world().pairs[a][b] = world().pairs[b][a] = on;
}
inline bool collides_(int32_t a, int32_t b) {
    checkLayer(a, "physics3d.collides()");
    checkLayer(b, "physics3d.collides()");
    return world().pairs[a][b];
}
inline bool pairCollides(const Body3D_& a, const Body3D_& b) {
    return world().pairs[std::clamp(a.layer_, 0, 31)][std::clamp(b.layer_, 0, 31)];
}

// ---- shapes

inline bool boxesTouch(core::vec3_ alo, core::vec3_ ahi, core::vec3_ blo, core::vec3_ bhi) {
    return alo.x_ < bhi.x_ && blo.x_ < ahi.x_ && alo.y_ < bhi.y_ && blo.y_ < ahi.y_ && alo.z_ < bhi.z_ && blo.z_ < ahi.z_;
}
inline bool boxesTouch(const Body3D_& a, const Body3D_& b) { return boxesTouch(a.lo(), a.hi(), b.lo(), b.hi()); }

inline float len(core::vec3_ v) { return std::sqrt(v.x_ * v.x_ + v.y_ * v.y_ + v.z_ * v.z_); }

// The point of a body's shape nearest to p, and whether p is inside it.
inline core::vec3_ closestPoint(const Body3D_& o, core::vec3_ p, bool& inside) {
    if (o.sphere_) {
        core::vec3_ d{p.x_ - o.position_.x_, p.y_ - o.position_.y_, p.z_ - o.position_.z_};
        float l = len(d), r = o.radius();
        inside = l < r;
        if (l < 1e-6f) return {o.position_.x_, o.position_.y_ + r, o.position_.z_};
        return {o.position_.x_ + d.x_ / l * r, o.position_.y_ + d.y_ / l * r, o.position_.z_ + d.z_ / l * r};
    }
    core::vec3_ lo = o.lo(), hi = o.hi();
    core::vec3_ q{std::clamp(p.x_, lo.x_, hi.x_), std::clamp(p.y_, lo.y_, hi.y_), std::clamp(p.z_, lo.z_, hi.z_)};
    inside = p.x_ > lo.x_ && p.x_ < hi.x_ && p.y_ > lo.y_ && p.y_ < hi.y_ && p.z_ > lo.z_ && p.z_ < hi.z_;
    if (!inside) return q;
    // inside: the nearest face
    float d[6] = {p.x_ - lo.x_, hi.x_ - p.x_, p.y_ - lo.y_, hi.y_ - p.y_, p.z_ - lo.z_, hi.z_ - p.z_};
    int k = (int)(std::min_element(d, d + 6) - d);
    if (k == 0) q.x_ = lo.x_;
    if (k == 1) q.x_ = hi.x_;
    if (k == 2) q.y_ = lo.y_;
    if (k == 3) q.y_ = hi.y_;
    if (k == 4) q.z_ = lo.z_;
    if (k == 5) q.z_ = hi.z_;
    return q;
}

inline bool overlapsShape(const Body3D_& a, const Body3D_& b) {
    if (!a.sphere_ && !b.sphere_) return boxesTouch(a, b);
    const Body3D_& s = a.sphere_ ? a : b;
    const Body3D_& o = a.sphere_ ? b : a;
    bool inside = false;
    core::vec3_ q = closestPoint(o, s.position_, inside);
    core::vec3_ d{s.position_.x_ - q.x_, s.position_.y_ - q.y_, s.position_.z_ - q.z_};
    return inside || len(d) < s.radius();
}
inline bool Body3D_::overlaps_(std::shared_ptr<Body3D_> other) { return other && overlapsShape(*this, *other); }

inline float bounced(float v, float bounce) { return bounce > 0 && std::fabs(v) > 1.5f ? -v * bounce : 0; }

// ---- moving bodies against static ones

// A moving box running into a static body sideways: can it climb onto it (a step, up to stepHeight)?
inline bool climb(Body3D_& b, const Body3D_& o, const std::vector<std::shared_ptr<Body3D_>>& bodies) {
    if (b.stepHeight_ <= 0 || !b.grounded || o.sphere_) return false;
    float rise = o.hi().y_ - b.lo().y_;
    if (rise <= 0 || rise > b.stepHeight_) return false;
    // is there room on top?
    core::vec3_ lo = b.lo(), hi = b.hi();
    lo.y_ += rise + 0.001f;
    hi.y_ += rise + 0.001f;
    for (auto& op : bodies) {
        const Body3D_& w = *op;
        if (&w == &b || !w.isStatic_ || !w.solid_ || !pairCollides(b, w)) continue;
        if (boxesTouch(lo, hi, w.lo(), w.hi())) return false;
    }
    b.position_.y_ += rise + 0.001f;
    return true;
}

// Moves a box along one axis (0 x, 1 y, 2 z) and pushes it back out of static bodies it ran into.
inline void moveBoxAxis(Body3D_& b, int axis, float d, const std::vector<std::shared_ptr<Body3D_>>& bodies) {
    float* pos = axis == 0 ? &b.position_.x_ : axis == 1 ? &b.position_.y_ : &b.position_.z_;
    float* vel = axis == 0 ? &b.velocity_.x_ : axis == 1 ? &b.velocity_.y_ : &b.velocity_.z_;
    *pos += d;
    if (!b.solid_ || d == 0) return;
    for (auto& op : bodies) {
        Body3D_& o = *op;
        if (&o == &b || !o.isStatic_ || !o.solid_ || !pairCollides(b, o) || !boxesTouch(b, o)) continue;
        if (axis != 1 && climb(b, o, bodies)) continue;
        float half = axis == 0 ? b.size_.x_ / 2 : axis == 1 ? b.size_.y_ / 2 : b.size_.z_ / 2;
        float olo = axis == 0 ? o.lo().x_ : axis == 1 ? o.lo().y_ : o.lo().z_;
        float ohi = axis == 0 ? o.hi().x_ : axis == 1 ? o.hi().y_ : o.hi().z_;
        if (d > 0) {
            *pos = olo - half;
            if (axis == 1) b.onCeiling_ = true;
        } else {
            *pos = ohi + half;
            if (axis == 1) b.onGround_ = b.grounded = true;
        }
        if (axis != 1) b.onWall_ = true;
        *vel = bounced(*vel, b.bounce_);
    }
}

// A moving ball against the static bodies: pushed out along the shortest way; its speed into the surface is
// bounced or stopped (it keeps sliding along it).
inline void sphereAgainstStatic(Body3D_& b, const std::vector<std::shared_ptr<Body3D_>>& bodies) {
    for (auto& op : bodies) {
        Body3D_& o = *op;
        if (&o == &b || !o.isStatic_ || !o.solid_ || !pairCollides(b, o)) continue;
        float r = b.radius();
        bool inside = false;
        core::vec3_ q = closestPoint(o, b.position_, inside);
        core::vec3_ d{b.position_.x_ - q.x_, b.position_.y_ - q.y_, b.position_.z_ - q.z_};
        float dist = len(d);
        if (!inside && dist >= r) continue;
        core::vec3_ n = dist > 1e-6f ? core::vec3_{d.x_ / dist, d.y_ / dist, d.z_ / dist} : core::vec3_{0, 1, 0};
        if (inside) n = {-n.x_, -n.y_, -n.z_};
        float depth = inside ? r + dist : r - dist;
        b.position_ += n * depth;
        float vn = b.velocity_.x_ * n.x_ + b.velocity_.y_ * n.y_ + b.velocity_.z_ * n.z_;
        if (vn < 0) {
            float keep = -vn > 1.5f ? b.bounce_ : 0;
            b.velocity_ -= n * (vn * (1 + keep));
        }
        if (n.y_ > 0.5f) {
            b.onGround_ = b.grounded = true;
        } else if (n.y_ < -0.5f) {
            b.onCeiling_ = true;
        } else {
            b.onWall_ = true;
        }
    }
}

// ---- moving bodies against each other

inline void separateSpheres(Body3D_& a, Body3D_& b) {
    core::vec3_ d{b.position_.x_ - a.position_.x_, b.position_.y_ - a.position_.y_, b.position_.z_ - a.position_.z_};
    float dist = len(d), r = a.radius() + b.radius();
    if (dist >= r) return;
    core::vec3_ n = dist > 1e-6f ? core::vec3_{d.x_ / dist, d.y_ / dist, d.z_ / dist} : core::vec3_{0, 1, 0};
    float push = (r - dist) / 2;
    a.position_ -= n * push;
    b.position_ += n * push;
    float rel = (b.velocity_.x_ - a.velocity_.x_) * n.x_ + (b.velocity_.y_ - a.velocity_.y_) * n.y_ + (b.velocity_.z_ - a.velocity_.z_) * n.z_;
    if (rel < 0) {
        float j = -(1 + std::max(a.bounce_, b.bounce_)) * rel / 2;
        a.velocity_ -= n * j;
        b.velocity_ += n * j;
    }
    if (n.y_ < -0.5f) {   // b is below a
        a.onGround_ = a.grounded = true;
        b.onCeiling_ = true;
    } else if (n.y_ > 0.5f) {
        b.onGround_ = b.grounded = true;
        a.onCeiling_ = true;
    } else {
        a.onWall_ = b.onWall_ = true;
    }
}

// Pushes overlapping moving bodies apart along whichever direction they overlap least. A body standing on the
// ground isn't pushed down: the one on top is pushed up instead.
inline void separateMovingBodies(const std::vector<std::shared_ptr<Body3D_>>& bodies) {
    for (size_t i = 0; i < bodies.size(); i++) {
        Body3D_& a = *bodies[i];
        if (a.isStatic_ || !a.solid_) continue;
        for (size_t j = i + 1; j < bodies.size(); j++) {
            Body3D_& b = *bodies[j];
            if (b.isStatic_ || !b.solid_ || !pairCollides(a, b) || !boxesTouch(a, b)) continue;
            if (a.sphere_ && b.sphere_) {
                separateSpheres(a, b);
                continue;
            }
            core::vec3_ alo = a.lo(), ahi = a.hi(), blo = b.lo(), bhi = b.hi();
            float ox = std::min(ahi.x_, bhi.x_) - std::max(alo.x_, blo.x_);
            float oy = std::min(ahi.y_, bhi.y_) - std::max(alo.y_, blo.y_);
            float oz = std::min(ahi.z_, bhi.z_) - std::max(alo.z_, blo.z_);
            if (oy <= ox && oy <= oz) {
                Body3D_& top = a.position_.y_ > b.position_.y_ ? a : b;
                Body3D_& bottom = &top == &a ? b : a;
                float topShare = bottom.grounded ? 1.0f : 0.5f;
                top.position_.y_ += oy * topShare;
                bottom.position_.y_ -= oy * (1 - topShare);
                if (top.velocity_.y_ < bottom.velocity_.y_) top.velocity_.y_ = bottom.velocity_.y_;
                top.onGround_ = top.grounded = true;
                bottom.onCeiling_ = true;
            } else if (ox <= oz) {
                Body3D_& left = a.position_.x_ < b.position_.x_ ? a : b;
                Body3D_& right = &left == &a ? b : a;
                left.position_.x_ -= ox / 2;
                right.position_.x_ += ox / 2;
                left.onWall_ = right.onWall_ = true;
            } else {
                Body3D_& near = a.position_.z_ < b.position_.z_ ? a : b;
                Body3D_& far = &near == &a ? b : a;
                near.position_.z_ -= oz / 2;
                far.position_.z_ += oz / 2;
                near.onWall_ = far.onWall_ = true;
            }
        }
    }
}

// One small step of the world (see step_ below).
inline void tick(float dt) {
    auto bodies = world().bodies;  // a copy, in case the list changes while stepping
    for (auto& bp : bodies) {
        Body3D_& b = *bp;
        if (b.isStatic_) continue;
        b.velocity_ += world().gravity * (b.gravityScale_ * dt);
        bool wasGrounded = b.grounded;
        b.grounded = false;
        // in small parts, so fast bodies can't pass through thin walls
        core::vec3_ move = b.velocity_ * dt;
        float part = std::max(0.02f, std::min({b.size_.x_, b.size_.y_, b.size_.z_}) / 2);
        int steps = std::clamp((int)std::ceil(std::max({std::fabs(move.x_), std::fabs(move.y_), std::fabs(move.z_)}) / part), 1, 64);
        float sdt = dt / steps;
        for (int s = 0; s < steps; s++) {
            if (b.sphere_) {
                b.position_ += b.velocity_ * sdt;
                if (b.solid_) sphereAgainstStatic(b, bodies);
                continue;
            }
            b.grounded = b.grounded || wasGrounded;   // climbing steps needs to know it was standing
            moveBoxAxis(b, 0, b.velocity_.x_ * sdt, bodies);
            moveBoxAxis(b, 2, b.velocity_.z_ * sdt, bodies);
            b.grounded = false;
            moveBoxAxis(b, 1, b.velocity_.y_ * sdt, bodies);
            wasGrounded = b.grounded;
        }
    }
    separateMovingBodies(bodies);
}

// physics3d.step(dt): the frame is split into equal parts of at most 1/240 s, so things move the same at any frame
// rate. onGround, onCeiling and onWall describe the whole frame: true if it happened in any part.
inline void step_(float dt) {
    if (dt <= 0) return;
    if (dt > 0.05f) dt = 0.05f;  // after a long pause, don't jump everything forward at once
    int parts = std::max(1, (int)std::ceil(dt * 240.0f - 1e-4f));
    for (auto& b : world().bodies)
        if (!b->isStatic_) b->onGround_ = b->onCeiling_ = b->onWall_ = false;
    for (int i = 0; i < parts; i++) tick(dt / parts);
}

// ---- rays

// The first solid body the line from `from` toward `direction` hits, up to maxDistance away. A body the line starts
// inside isn't hit (so a ray from a character's eyes doesn't hit the character).
inline RayHit_ raycast_(core::vec3_ from, core::vec3_ direction, float maxDistance) {
    float l = len(direction);
    if (l < 1e-6f) panic("physics3d.raycast()", "the direction can't be (0, 0, 0) - it's the way the ray goes, like vec3(0, -1, 0) for straight down");
    core::vec3_ d{direction.x_ / l, direction.y_ / l, direction.z_ / l};
    RayHit_ best;
    best.distance_ = maxDistance;
    for (auto& bp : world().bodies) {
        const Body3D_& o = *bp;
        if (!o.solid_) continue;
        float t = -1;
        core::vec3_ n{};
        if (o.sphere_) {
            core::vec3_ m{from.x_ - o.position_.x_, from.y_ - o.position_.y_, from.z_ - o.position_.z_};
            float r = o.radius(), bq = m.x_ * d.x_ + m.y_ * d.y_ + m.z_ * d.z_, c = m.x_ * m.x_ + m.y_ * m.y_ + m.z_ * m.z_ - r * r;
            if (c <= 0) continue;   // starts inside
            float disc = bq * bq - c;
            if (disc < 0) continue;
            t = -bq - std::sqrt(disc);
            if (t < 0) continue;
            core::vec3_ p = from + d * t;
            n = {(p.x_ - o.position_.x_) / r, (p.y_ - o.position_.y_) / r, (p.z_ - o.position_.z_) / r};
        } else {
            core::vec3_ lo = o.lo(), hi = o.hi();
            float tmin = -1e30f, tmax = 1e30f;
            int axis = -1;
            float sign = 0;
            const float o3[3] = {from.x_, from.y_, from.z_}, d3[3] = {d.x_, d.y_, d.z_}, lo3[3] = {lo.x_, lo.y_, lo.z_}, hi3[3] = {hi.x_, hi.y_, hi.z_};
            bool miss = false;
            for (int k = 0; k < 3 && !miss; k++) {
                if (std::fabs(d3[k]) < 1e-9f) {
                    if (o3[k] < lo3[k] || o3[k] > hi3[k]) miss = true;
                    continue;
                }
                float t1 = (lo3[k] - o3[k]) / d3[k], t2 = (hi3[k] - o3[k]) / d3[k];
                float s = -1;   // entering through the low face (its outside faces -axis)
                if (t1 > t2) {
                    std::swap(t1, t2);
                    s = 1;
                }
                if (t1 > tmin) {
                    tmin = t1;
                    axis = k;
                    sign = s;
                }
                tmax = std::min(tmax, t2);
                if (tmin > tmax) miss = true;
            }
            if (miss || tmin < 0 || axis < 0) continue;   // missed, or starts inside
            t = tmin;
            n = {axis == 0 ? sign : 0, axis == 1 ? sign : 0, axis == 2 ? sign : 0};
        }
        if (t >= 0 && t < best.distance_) {
            best.hit_ = true;
            best.distance_ = t;
            best.point_ = from + d * t;
            best.normal_ = n;
            best.body_ = bp;
        }
    }
    if (!best.hit_) best.distance_ = 0;
    return best;
}

inline std::string frg$str(const std::shared_ptr<Body3D_>& b) {
    if (!b) return "null";
    return "Body3D(position: " + core::frg$str(b->position_) + ", size: " + core::frg$str(b->size_) +
           ", velocity: " + core::frg$str(b->velocity_) + ", onGround: " + str(b->onGround_) + ")";
}

}  // namespace frg::physics3d
