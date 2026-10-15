// Forge runtime: <tween> - smooth changes over time (menus sliding in, things bouncing and fading).
//
// A Tween goes from one number to another in a set time. Each frame: tw.update(dt), then use tw.value().
// The easing decides the shape of the change: steady (LINEAR), starting slowly (IN), ending slowly (OUT),
// both (IN_OUT), or bouncing at the end like a dropped ball (BOUNCE).
#pragma once

#include "forge.hpp"

namespace frg::tween {

// Keep the order in sync with Ease in src/prelude.cpp.
enum class Ease_ : int32_t { LINEAR_, IN_, OUT_, IN_OUT_, BOUNCE_ };

inline float bounceOut(float t) {
    const float n = 7.5625f, d = 2.75f;
    if (t < 1 / d) return n * t * t;
    if (t < 2 / d) return t -= 1.5f / d, n * t * t + 0.75f;
    if (t < 2.5f / d) return t -= 2.25f / d, n * t * t + 0.9375f;
    return t -= 2.625f / d, n * t * t + 0.984375f;
}

// t from 0 to 1 -> the eased amount, also from 0 to 1.
inline float ease_(Ease_ e, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    switch (e) {
        case Ease_::LINEAR_: return t;
        case Ease_::IN_: return t * t * t;
        case Ease_::OUT_: return 1 - (1 - t) * (1 - t) * (1 - t);
        case Ease_::IN_OUT_: return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3.0f) / 2;
        case Ease_::BOUNCE_: return bounceOut(t);
    }
    return t;
}

struct Tween_ : std::enable_shared_from_this<Tween_> {
    float from = 0, to = 1, seconds = 1;
    Ease_ easing = Ease_::LINEAR_;
    float time = 0;
    bool looping_ = false;  // starts again when it reaches the end
    bool pingPong_ = false; // goes back and forth (with looping, forever)
    bool backwards = false;

    Tween_() = default;
    virtual ~Tween_() = default;
    void frg$init(float a, float b, float s, Ease_ e) {
        if (s < 0) panic("Tween(...)", "the time can't be negative, but it's " + str(s) + " seconds");
        from = a, to = b, seconds = s, easing = e;
    }
    static std::shared_ptr<Tween_> frg$new(float a, float b, float s, Ease_ e) {
        auto o = std::make_shared<Tween_>();
        o->frg$init(a, b, s, e);
        return o;
    }

    virtual void update_(float dt) {
        if (dt <= 0) return;
        time += dt;
        if (time < seconds) return;
        if (pingPong_ && !backwards) {
            backwards = true;
            time = seconds > 0 ? std::fmod(time - seconds, seconds) : 0;
            if (!looping_ && time >= seconds) time = seconds;
        } else if (looping_) {
            backwards = false;
            time = seconds > 0 ? std::fmod(time - seconds, seconds) : 0;
        } else {
            time = seconds;
        }
    }
    virtual float progress_() {
        float t = seconds > 0 ? time / seconds : 1;
        if (backwards) t = 1 - t;
        return ease_(easing, t);
    }
    virtual float value_() { return from + (to - from) * progress_(); }
    virtual bool isDone_() { return !looping_ && time >= seconds && (!pingPong_ || backwards); }
    virtual void restart_() {
        time = 0;
        backwards = false;
    }
    // Plays it the other way: from where it is now back to where it started (a menu sliding back out).
    virtual void reverse_() {
        float now = value_();
        std::swap(from, to);
        time = 0;
        backwards = false;
        from = now;
    }
};

inline std::string frg$str(const std::shared_ptr<Tween_>& t) {
    return t ? "Tween(" + str(t->value_()) + (t->isDone_() ? ", done)" : ")") : "null";
}
inline std::string frg$str(Ease_ e) {
    static const char* names[] = {"LINEAR", "IN", "OUT", "IN_OUT", "BOUNCE"};
    return names[(int)e];
}

}  // namespace frg::tween
