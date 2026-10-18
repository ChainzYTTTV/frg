// Forge runtime: <time> - the clock, the date, and timers.
#pragma once

#include "forge.hpp"

#include <ctime>

namespace frg::time {

inline std::chrono::steady_clock::time_point startTime() {
    static const auto t = std::chrono::steady_clock::now();
    return t;
}
inline const bool frg$started = (startTime(), true);  // start counting when the game starts

inline double now_() { return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime()).count(); }
inline void sleep_(float seconds) {
    if (seconds > 0) std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}

inline std::tm localNow() {
    std::time_t t = std::time(nullptr);
    std::tm out{};
    localtime_s(&out, &t);
    return out;
}
inline std::string two(int v) { return (v < 10 ? "0" : "") + std::to_string(v); }
inline std::string timeOfDay_() {
    std::tm t = localNow();
    return two(t.tm_hour) + ":" + two(t.tm_min) + ":" + two(t.tm_sec);
}
inline std::string date_() {
    std::tm t = localNow();
    return std::to_string(t.tm_year + 1900) + "-" + two(t.tm_mon + 1) + "-" + two(t.tm_mday);
}
inline int32_t year_() { return localNow().tm_year + 1900; }
inline int32_t month_() { return localNow().tm_mon + 1; }
inline int32_t day_() { return localNow().tm_mday; }
inline int32_t hour_() { return localNow().tm_hour; }
inline int32_t minute_() { return localNow().tm_min; }
inline int32_t second_() { return localNow().tm_sec; }

// A countdown: Timer(0.5) is done half a second after it starts (or restarts).
struct Timer_ : std::enable_shared_from_this<Timer_> {
    double length = 0;
    double startedAt = 0;
    Timer_() { startedAt = now_(); }
    virtual ~Timer_() = default;
    void frg$init(float seconds) {
        length = seconds;
        startedAt = now_();
    }
    static std::shared_ptr<Timer_> frg$new(float seconds) {
        auto t = std::make_shared<Timer_>();
        t->frg$init(seconds);
        return t;
    }
    virtual bool isDone_() { return now_() - startedAt >= length; }
    virtual float left_() { return (float)std::max(0.0, length - (now_() - startedAt)); }
    virtual float elapsed_() { return (float)(now_() - startedAt); }
    virtual void restart_() { startedAt = now_(); }
};

inline std::string frg$str(const std::shared_ptr<Timer_>& t) {
    return t ? "Timer(" + str(t->left_()) + " seconds left)" : "null";
}

}  // namespace frg::time
