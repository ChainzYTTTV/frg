// Forge runtime: the C++ library every compiled Forge game is built with.
// Names the generated code uses end in '_' (Forge names) or contain '$' (Forge internals).
#pragma once

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <coroutine>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);

namespace frg {

using u128 = unsigned __int128;

#ifdef FRG_NO_CONSOLE
}  // namespace frg
// Built without a console (--release or --no-console): there's nowhere to print errors, so they're shown in a box.
// Declared by hand, exactly as windows.h does, because windows.h can't be mixed with raylib.h.
struct HWND__;
extern "C" __declspec(dllimport) int __stdcall MessageBoxA(HWND__* window, const char* text, const char* caption, unsigned int type);
namespace frg {
#endif

[[noreturn]] inline void panic(const char* where, const std::string& msg) {
    std::fflush(stdout);
    std::fprintf(stderr, "\nRuntime error in %s: %s\n", where, msg.c_str());
#ifdef FRG_NO_CONSOLE
    std::string text = std::string("Runtime error in ") + where + ":\n\n" + msg;
    MessageBoxA(nullptr, text.c_str(), "The game stopped", 0x10 /* MB_ICONERROR */);
#endif
    std::exit(1);
}

// Libraries register cleanup to run when main returns (newest first), and work to do once per frame.
inline std::vector<void (*)()>& shutdownHooks() {
    static std::vector<void (*)()> hooks;
    return hooks;
}
inline std::vector<void (*)()>& frameHooks() {
    static std::vector<void (*)()> hooks;
    return hooks;
}
inline void addHook(std::vector<void (*)()>& hooks, void (*fn)()) {
    if (std::find(hooks.begin(), hooks.end(), fn) == hooks.end()) hooks.push_back(fn);
}
inline void init() { SetConsoleOutputCP(65001); }
inline void shutdown() {
    auto& hooks = shutdownHooks();
    for (auto it = hooks.rbegin(); it != hooks.rend(); ++it) (*it)();
    hooks.clear();
    std::fflush(stdout);
}

// ---------------------------------------------------------------- text

inline std::string str(const std::string& s) { return s; }
inline std::string str(bool b) { return b ? "true" : "false"; }

template <class T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
std::string str(T v) {
    bool neg = false;
    u128 mag;
    if constexpr (std::is_signed_v<T>) {
        neg = v < 0;
        mag = neg ? u128(0) - u128(v) : u128(v);
    } else {
        mag = u128(v);
    }
    char buf[48];
    int i = sizeof buf;
    do {
        buf[--i] = char('0' + int(mag % 10));
        mag /= 10;
    } while (mag);
    if (neg) buf[--i] = '-';
    return std::string(buf + i, buf + sizeof buf);
}

template <class F>
std::string floatStr(F v) {
    char buf[64];
    auto r = std::to_chars(buf, buf + sizeof buf, v);
    std::string s(buf, r.ptr);
    if (s.find_first_of(".en") == std::string::npos) s += ".0";  // 5 -> "5.0", but leave 1e+20, inf, nan alone
    return s;
}
inline std::string str(float v) { return floatStr(v); }
inline std::string str(double v) { return floatStr(v); }

// ---------------------------------------------------------------- stdio

inline void out(const std::string& s) { std::fwrite(s.data(), 1, s.size(), stdout); }
inline void print() { out("\n"); }
inline void print(const std::string& s) { out(s + "\n"); }
inline void write(const std::string& s) { out(s); }

inline std::string readInput() {
    std::fflush(stdout);
    std::string line;
    if (!std::getline(std::cin, line)) return "";
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}
inline std::string readInput(const std::string& prompt) {
    write(prompt);
    return readInput();
}

// ---------------------------------------------------------------- text -> numbers

inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Reads a number from text. Returns 0 if it worked, 1 if the text isn't a number, 2 if the number is too big.
template <class T>
int parseInto(const std::string& text, T& out) {
    std::string s = trim(text);
    if constexpr (std::is_floating_point_v<T>) {
        auto r = std::from_chars(s.data(), s.data() + s.size(), out);
        return s.empty() || r.ec != std::errc() || r.ptr != s.data() + s.size() ? 1 : 0;
    } else {
        size_t i = 0;
        bool neg = false;
        if (i < s.size() && (s[i] == '-' || s[i] == '+')) neg = s[i++] == '-';
        if (i >= s.size()) return 1;
        u128 limit;
        if constexpr (std::is_signed_v<T>)
            limit = neg ? u128(std::numeric_limits<T>::max()) + 1 : u128(std::numeric_limits<T>::max());
        else
            limit = neg ? 0 : u128(std::numeric_limits<T>::max());
        u128 mag = 0;
        bool tooBig = false;
        for (; i < s.size(); i++) {
            if (s[i] < '0' || s[i] > '9') return 1;
            unsigned d = unsigned(s[i] - '0');
            if (mag > limit / 10 || (mag == limit / 10 && d > limit % 10)) tooBig = true;
            if (!tooBig) mag = mag * 10 + d;
        }
        if (tooBig) return 2;
        out = neg ? T(u128(0) - mag) : T(mag);
        return 0;
    }
}

template <class T>
T parse(const std::string& text, const char* typeName, const char* where) {
    T v{};
    int r = parseInto(text, v);
    if (r != 0) {
        std::string err = "couldn't turn \"" + text + "\" into " + (typeName[0] == 'i' ? "an " : "a ") + typeName;
        panic(where, r == 2 ? err + " (it's too big)" : err);
    }
    return v;
}

// ---------------------------------------------------------------- math

template <class T>
T idiv(T a, T b, const char* where) {
    if (b == 0) panic(where, "division by zero");
    if constexpr (std::is_signed_v<T>)
        if (b == T(-1)) return T(T(0) - a);
    return a / b;
}

template <class T>
T imod(T a, T b, const char* where) {
    if (b == 0) panic(where, "division by zero (in %)");
    if constexpr (std::is_signed_v<T>)
        if (b == T(-1)) return T(0);
    return a % b;
}

template <class T>
T abs_(T v) {
    if constexpr (std::is_unsigned_v<T>)
        return v;
    else
        return v < 0 ? T(-v) : v;
}
template <class T>
T sign_(T v) { return v > 0 ? T(1) : v < 0 ? T(-1) : T(0); }
template <class T>
T min_(T a, T b) { return b < a ? b : a; }
template <class T>
T max_(T a, T b) { return a < b ? b : a; }
template <class T>
T clamp_(T v, T lo, T hi) { return v < lo ? lo : (hi < v ? hi : v); }
template <class T>
T lerp_(T a, T b, T t) { return a + (b - a) * t; }

namespace math {
constexpr double PI_ = 3.14159265358979323846;
constexpr double TAU_ = 6.28318530717958647692;
constexpr double E_ = 2.71828182845904523536;
}  // namespace math

// ---------------------------------------------------------------- tasks: functions that can wait()
//
// A Forge function that returns `task` becomes a C++ coroutine. Calling it runs it right away, up to its
// first wait(); after that the scheduler resumes it once enough game time has passed. With a window, the
// scheduler moves forward by itself every frame; without one, call tasks.update(dt) or tasks.runAll().

namespace tasks {
struct State {
    std::coroutine_handle<> handle;
    double wakeAt = 0;
    std::shared_ptr<State> waitingOn;  // wait(otherTask)
    bool done = false;
    bool running = false;
    bool stopRequested = false;
};
struct Scheduler {
    double now = 0;
    std::vector<std::shared_ptr<State>> waiting;
};
inline Scheduler& sched() {
    static Scheduler s;
    return s;
}
}  // namespace tasks

struct Task {
    std::shared_ptr<tasks::State> st;

    struct promise_type {
        std::shared_ptr<tasks::State> state = std::make_shared<tasks::State>();
        Task get_return_object() {
            state->handle = std::coroutine_handle<promise_type>::from_promise(*this);
            state->running = true;
            return Task{state};
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {
            state->done = true;
            state->running = false;
        }
        void unhandled_exception() { std::terminate(); }
    };
};

namespace tasks {
// Shared by wait(seconds) and wait(otherTask): parks the task in the scheduler.
inline void park(std::coroutine_handle<Task::promise_type> h, double seconds, std::shared_ptr<State> on) {
    auto st = h.promise().state;
    st->running = false;
    if (st->stopRequested) {  // it called stop() on itself
        st->done = true;
        h.destroy();
        return;
    }
    st->wakeAt = sched().now + (seconds > 0 ? seconds : 0);
    st->waitingOn = std::move(on);
    sched().waiting.push_back(st);
}

struct Wait {
    double seconds;
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<Task::promise_type> h) { park(h, seconds, nullptr); }
    void await_resume() const noexcept {}
};

struct WaitFor {
    Task other;
    bool await_ready() const noexcept { return !other.st || other.st->done; }
    void await_suspend(std::coroutine_handle<Task::promise_type> h) { park(h, 0, other.st); }
    void await_resume() const noexcept {}
};

inline void update_(float dt) {
    Scheduler& s = sched();
    s.now += dt > 0 ? dt : 0;
    std::vector<std::shared_ptr<State>> ready, still;
    for (auto& t : s.waiting) {
        if (t->done) continue;
        bool wake = t->waitingOn ? t->waitingOn->done : t->wakeAt <= s.now + 1e-9;
        (wake ? ready : still).push_back(t);
    }
    s.waiting = std::move(still);
    for (auto& t : ready) {
        if (t->done) continue;
        t->running = true;
        t->handle.resume();
    }
}

inline int32_t count_() {
    int32_t n = 0;
    for (auto& t : sched().waiting)
        if (!t->done) n++;
    return n;
}

inline void stop(const std::shared_ptr<State>& st) {
    if (!st || st->done) return;
    if (st->running) {  // stopping itself: it stops at its next wait()
        st->stopRequested = true;
        return;
    }
    auto& w = sched().waiting;
    w.erase(std::remove(w.begin(), w.end(), st), w.end());
    st->done = true;
    st->handle.destroy();
}

inline void stopAll_() {
    auto waiting = sched().waiting;
    for (auto& t : waiting) stop(t);
}

// For programs without a window: keeps going (really waiting) until every task has finished.
inline void runAll_() {
    while (true) {
        Scheduler& s = sched();
        double next = -1;
        bool any = false;
        for (auto& t : s.waiting) {
            if (t->done) continue;
            any = true;
            if (!t->waitingOn && (next < 0 || t->wakeAt < next)) next = t->wakeAt;
        }
        if (!any) return;
        if (next < 0) {  // only tasks waiting for other tasks are left
            size_t before = s.waiting.size();
            update_(0);
            if (sched().waiting.size() >= before) return;  // nothing can move any more
            continue;
        }
        double dt = next - s.now;
        if (dt > 0) std::this_thread::sleep_for(std::chrono::duration<double>(dt));
        update_((float)(dt > 0 ? dt : 0));
    }
}
}  // namespace tasks

inline bool task_isDone(const Task& t, const char*) { return !t.st || t.st->done; }
inline void task_stop(Task& t, const char*) { tasks::stop(t.st); }
inline std::string str(const Task& t) { return task_isDone(t, "") ? "task (done)" : "task (running)"; }

// ---------------------------------------------------------------- optionals and null

template <class T>
bool isNull(const std::optional<T>& o) { return !o.has_value(); }
template <class T>
bool isNull(const std::shared_ptr<T>& p) { return p == nullptr; }
template <class T>
bool isNull(T* p) { return p == nullptr; }

// Using a T? as a T: stops the game with a clear message if it's null.
template <class O>
decltype(auto) unwrap(O&& o, const char* where, const char* what) {
    if (!o.has_value()) panic(where, std::string(what) + " is null");
    return *std::forward<O>(o);
}
template <class P>
P&& nn(P&& p, const char* where, const char* what) {
    if (p == nullptr) panic(where, std::string(what) + " is null");
    return std::forward<P>(p);
}

// Pointer<T>: p.remove() makes it point at nothing; using it after that stops the game with a clear message.
template <class T>
T* live(T* p, const char* where) {
    if (p == nullptr)
        panic(where, "this pointer was removed (with remove()), so it doesn't point at anything any more - point it at "
                     "something again first (p = someVariable;), or check p.isRemoved()");
    return p;
}
template <class T>
void ptr_remove(T*& p, const char*) { p = nullptr; }
template <class T>
bool ptr_isRemoved(T* p, const char*) { return p == nullptr; }
template <class T, class F>
std::string strPtr(T* p, F&& show) { return p ? show(*p) : std::string("removed pointer"); }

template <class T, class B>
T orElse(const std::optional<T>& o, B&& b) { return o ? *o : T(std::forward<B>(b)); }
template <class T, class B>
std::shared_ptr<T> orElse(const std::shared_ptr<T>& o, B&& b) { return o ? o : std::shared_ptr<T>(std::forward<B>(b)); }
template <class T, class B>
T* orElse(T* o, B&& b) { return o ? o : static_cast<T*>(b); }
template <class T>
std::optional<T> orElseOpt(const std::optional<T>& o, const std::optional<T>& b) { return o ? o : b; }

template <class T, class F>
std::string strOpt(const std::optional<T>& o, F f) { return o ? f(*o) : "null"; }
template <class T, class F>
std::string strOpt(const std::shared_ptr<T>& p, F f) { return p ? f(p) : "null"; }
template <class T, class F>
std::string strOpt(T* p, F f) { return p ? f(p) : "null"; }

// ---------------------------------------------------------------- lists

// std::vector<bool> is special in C++ (its items aren't real bools), so lists of bools use this instead.
struct Bool {
    bool v = false;
    Bool() = default;
    Bool(bool b) : v(b) {}
    operator bool&() { return v; }
    operator bool() const { return v; }
    bool operator==(const Bool& o) const { return v == o.v; }
};
template <class T>
struct ListItem { using type = T; };
template <>
struct ListItem<bool> { using type = Bool; };

template <class T>
using List = std::vector<typename ListItem<T>::type>;
template <class K, class V>
using Map = std::map<K, V>;

template <class L>
decltype(auto) at(L&& list, int64_t i, const char* where) {
    if (i < 0 || i >= (int64_t)list.size())
        panic(where, "position " + str(i) + " is outside the list (it has " + str((int64_t)list.size()) + " items, at positions 0 to " +
                         str((int64_t)list.size() - 1) + ")");
    return std::forward<L>(list)[(size_t)i];
}

template <class L, class X>
void list_add(L& l, X&& x, const char*) { l.push_back(std::forward<X>(x)); }
template <class L, class M>
void list_addAll(L& l, const M& more, const char*) {
    auto copy = more;  // safe even when adding a list to itself
    l.insert(l.end(), copy.begin(), copy.end());
}
template <class L, class X>
void list_insert(L& l, int64_t i, X&& x, const char* where) {
    if (i < 0 || i > (int64_t)l.size()) panic(where, "can't insert at position " + str(i) + " (the list has " + str((int64_t)l.size()) + " items)");
    l.insert(l.begin() + i, std::forward<X>(x));
}
template <class L>
auto list_removeAt(L& l, int64_t i, const char* where) {
    auto item = at(l, i, where);
    l.erase(l.begin() + i);
    return item;
}
template <class L, class X>
bool list_remove(L& l, const X& x, const char*) {
    auto it = std::find(l.begin(), l.end(), x);
    if (it == l.end()) return false;
    l.erase(it);
    return true;
}
template <class L, class X>
bool list_contains(const L& l, const X& x, const char*) { return std::find(l.begin(), l.end(), x) != l.end(); }
template <class L, class X>
int32_t list_indexOf(const L& l, const X& x, const char*) {
    auto it = std::find(l.begin(), l.end(), x);
    return it == l.end() ? -1 : int32_t(it - l.begin());
}
template <class L>
void list_clear(L& l, const char*) { l.clear(); }
template <class L>
bool list_isEmpty(const L& l, const char*) { return l.empty(); }
template <class L>
auto list_first(const L& l, const char* where) {
    if (l.empty()) panic(where, "first() was used on an empty list");
    return l.front();
}
template <class L>
auto list_last(const L& l, const char* where) {
    if (l.empty()) panic(where, "last() was used on an empty list");
    return l.back();
}
template <class L>
auto list_pop(L& l, const char* where) {
    if (l.empty()) panic(where, "pop() was used on an empty list");
    auto item = l.back();
    l.pop_back();
    return item;
}
template <class L>
void list_reverse(L& l, const char*) { std::reverse(l.begin(), l.end()); }
template <class L>
void list_sort(L& l, const char*) { std::sort(l.begin(), l.end()); }
template <class L>
std::decay_t<L> list_copy(const L& l, const char*) { return l; }

template <class L, class F>
std::string strList(const L& l, F f) {
    std::string s = "[";
    for (size_t i = 0; i < l.size(); i++) {
        if (i) s += ", ";
        s += f(l[i]);
    }
    return s + "]";
}

// ---------------------------------------------------------------- maps

template <class M, class K>
decltype(auto) mapGet(M&& m, const K& k, const char* where) {
    auto it = m.find(k);
    if (it == m.end()) panic(where, "the key isn't in the map (use .has(key) to check first, or .get(key, backup))");
    return (it->second);
}
template <class M, class K>
bool map_has(const M& m, const K& k, const char*) { return m.count(k) != 0; }
template <class M, class K>
bool map_remove(M& m, const K& k, const char*) { return m.erase(k) != 0; }
template <class M, class K, class V>
typename std::decay_t<M>::mapped_type map_get(const M& m, const K& k, V&& backup, const char*) {
    auto it = m.find(k);
    return it == m.end() ? typename std::decay_t<M>::mapped_type(std::forward<V>(backup)) : it->second;
}
template <class M>
auto map_keys(const M& m, const char*) {
    List<typename std::decay_t<M>::key_type> keys;
    for (auto& [k, v] : m) keys.push_back(k);
    return keys;
}
template <class M>
auto map_values(const M& m, const char*) {
    List<typename std::decay_t<M>::mapped_type> values;
    for (auto& [k, v] : m) values.push_back(v);
    return values;
}
template <class M>
void map_clear(M& m, const char*) { m.clear(); }
template <class M>
bool map_isEmpty(const M& m, const char*) { return m.empty(); }

template <class M, class FK, class FV>
std::string strMap(const M& m, FK fk, FV fv) {
    std::string s = "{";
    bool first = true;
    for (auto& [k, v] : m) {
        if (!first) s += ", ";
        first = false;
        s += fk(k) + ": " + fv(v);
    }
    return s + "}";
}

// ---------------------------------------------------------------- text methods

inline bool str_contains(const std::string& s, const std::string& x, const char*) { return s.find(x) != std::string::npos; }
inline int32_t str_indexOf(const std::string& s, const std::string& x, const char*) {
    size_t i = s.find(x);
    return i == std::string::npos ? -1 : int32_t(i);
}
inline bool str_startsWith(const std::string& s, const std::string& x, const char*) { return s.rfind(x, 0) == 0; }
inline bool str_endsWith(const std::string& s, const std::string& x, const char*) {
    return s.size() >= x.size() && s.compare(s.size() - x.size(), x.size(), x) == 0;
}
inline std::string str_upper(std::string s, const char*) {
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}
inline std::string str_lower(std::string s, const char*) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
inline std::string str_trim(const std::string& s, const char*) { return trim(s); }
inline std::string str_replace(std::string s, const std::string& from, const std::string& to, const char*) {
    if (from.empty()) return s;
    for (size_t i = 0; (i = s.find(from, i)) != std::string::npos; i += to.size()) s.replace(i, from.size(), to);
    return s;
}
inline List<std::string> str_split(const std::string& s, const std::string& sep, const char*) {
    List<std::string> parts;
    if (sep.empty()) {
        for (char c : s) parts.push_back(std::string(1, c));
        return parts;
    }
    size_t start = 0, i;
    while ((i = s.find(sep, start)) != std::string::npos) {
        parts.push_back(s.substr(start, i - start));
        start = i + sep.size();
    }
    parts.push_back(s.substr(start));
    return parts;
}
inline std::string str_substring(const std::string& s, int64_t start, int64_t count, const char* where) {
    if (start < 0 || count < 0 || start + count > (int64_t)s.size())
        panic(where, "substring(" + str(start) + ", " + str(count) + ") goes outside the text (it has " + str((int64_t)s.size()) + " characters)");
    return s.substr((size_t)start, (size_t)count);
}
inline std::string str_charAt(const std::string& s, int64_t i, const char* where) {
    if (i < 0 || i >= (int64_t)s.size())
        panic(where, "position " + str(i) + " is outside the text (it has " + str((int64_t)s.size()) + " characters)");
    return std::string(1, s[(size_t)i]);
}
inline std::string str_repeat(const std::string& s, int64_t n, const char*) {
    std::string r;
    for (int64_t i = 0; i < n; i++) r += s;
    return r;
}
inline bool str_isEmpty(const std::string& s, const char*) { return s.empty(); }
// True when int(s) / float(s) would work.
inline bool str_isInt(const std::string& s, const char*) {
    int32_t v;
    return parseInto(s, v) == 0;
}
inline bool str_isFloat(const std::string& s, const char*) {
    float v;
    return parseInto(s, v) == 0;
}
// "{value:.2}": a decimal number shown with a fixed number of decimals.
inline std::string fixed(double v, int decimals) {
    char buf[512];
    std::snprintf(buf, sizeof buf, "%.*f", decimals, v);
    return buf;
}

// ---------------------------------------------------------------- core types: vec2, vec3, vec4, color, rect

namespace core {

struct vec2_ {
    float x_ = 0, y_ = 0;
    static vec2_ frg$make() { return {}; }
    static vec2_ frg$make(float x, float y) { return {x, y}; }
    float length_() const { return std::sqrt(x_ * x_ + y_ * y_); }
    float lengthSquared_() const { return x_ * x_ + y_ * y_; }
    vec2_ normalized_() const {
        float l = length_();
        return l > 0 ? vec2_{x_ / l, y_ / l} : vec2_{};
    }
    float dot_(vec2_ o) const { return x_ * o.x_ + y_ * o.y_; }
    float distance_(vec2_ o) const { return vec2_{x_ - o.x_, y_ - o.y_}.length_(); }
    float angle_() const { return std::atan2(y_, x_); }
    vec2_ rotated_(float r) const {
        float c = std::cos(r), s = std::sin(r);
        return {x_ * c - y_ * s, x_ * s + y_ * c};
    }
    vec2_ lerp_(vec2_ t, float k) const { return {x_ + (t.x_ - x_) * k, y_ + (t.y_ - y_) * k}; }
    vec2_ operator+(vec2_ o) const { return {x_ + o.x_, y_ + o.y_}; }
    vec2_ operator-(vec2_ o) const { return {x_ - o.x_, y_ - o.y_}; }
    vec2_ operator*(float s) const { return {x_ * s, y_ * s}; }
    vec2_ operator/(float s) const { return {x_ / s, y_ / s}; }
    vec2_ operator-() const { return {-x_, -y_}; }
    vec2_& operator+=(vec2_ o) { return *this = *this + o; }
    vec2_& operator-=(vec2_ o) { return *this = *this - o; }
    vec2_& operator*=(float s) { return *this = *this * s; }
    vec2_& operator/=(float s) { return *this = *this / s; }
    bool operator==(const vec2_&) const = default;
    static const vec2_ ZERO_, ONE_, UP_, DOWN_, LEFT_, RIGHT_;
};
inline vec2_ operator*(float s, vec2_ v) { return v * s; }
inline const vec2_ vec2_::ZERO_{0, 0}, vec2_::ONE_{1, 1}, vec2_::UP_{0, -1}, vec2_::DOWN_{0, 1}, vec2_::LEFT_{-1, 0}, vec2_::RIGHT_{1, 0};

struct vec3_ {
    float x_ = 0, y_ = 0, z_ = 0;
    static vec3_ frg$make() { return {}; }
    static vec3_ frg$make(float x, float y, float z) { return {x, y, z}; }
    float length_() const { return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_); }
    float lengthSquared_() const { return x_ * x_ + y_ * y_ + z_ * z_; }
    vec3_ normalized_() const {
        float l = length_();
        return l > 0 ? vec3_{x_ / l, y_ / l, z_ / l} : vec3_{};
    }
    float dot_(vec3_ o) const { return x_ * o.x_ + y_ * o.y_ + z_ * o.z_; }
    vec3_ cross_(vec3_ o) const { return {y_ * o.z_ - z_ * o.y_, z_ * o.x_ - x_ * o.z_, x_ * o.y_ - y_ * o.x_}; }
    float distance_(vec3_ o) const { return vec3_{x_ - o.x_, y_ - o.y_, z_ - o.z_}.length_(); }
    vec3_ lerp_(vec3_ t, float k) const { return {x_ + (t.x_ - x_) * k, y_ + (t.y_ - y_) * k, z_ + (t.z_ - z_) * k}; }
    vec3_ operator+(vec3_ o) const { return {x_ + o.x_, y_ + o.y_, z_ + o.z_}; }
    vec3_ operator-(vec3_ o) const { return {x_ - o.x_, y_ - o.y_, z_ - o.z_}; }
    vec3_ operator*(float s) const { return {x_ * s, y_ * s, z_ * s}; }
    vec3_ operator/(float s) const { return {x_ / s, y_ / s, z_ / s}; }
    vec3_ operator-() const { return {-x_, -y_, -z_}; }
    vec3_& operator+=(vec3_ o) { return *this = *this + o; }
    vec3_& operator-=(vec3_ o) { return *this = *this - o; }
    vec3_& operator*=(float s) { return *this = *this * s; }
    vec3_& operator/=(float s) { return *this = *this / s; }
    bool operator==(const vec3_&) const = default;
    static const vec3_ ZERO_, ONE_, UP_, FORWARD_, RIGHT_;
};
inline vec3_ operator*(float s, vec3_ v) { return v * s; }
inline const vec3_ vec3_::ZERO_{0, 0, 0}, vec3_::ONE_{1, 1, 1}, vec3_::UP_{0, 1, 0}, vec3_::FORWARD_{0, 0, -1}, vec3_::RIGHT_{1, 0, 0};

struct vec4_ {
    float x_ = 0, y_ = 0, z_ = 0, w_ = 0;
    static vec4_ frg$make() { return {}; }
    static vec4_ frg$make(float x, float y, float z, float w) { return {x, y, z, w}; }
    float length_() const { return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_); }
    vec4_ normalized_() const {
        float l = length_();
        return l > 0 ? vec4_{x_ / l, y_ / l, z_ / l, w_ / l} : vec4_{};
    }
    float dot_(vec4_ o) const { return x_ * o.x_ + y_ * o.y_ + z_ * o.z_ + w_ * o.w_; }
    vec4_ operator+(vec4_ o) const { return {x_ + o.x_, y_ + o.y_, z_ + o.z_, w_ + o.w_}; }
    vec4_ operator-(vec4_ o) const { return {x_ - o.x_, y_ - o.y_, z_ - o.z_, w_ - o.w_}; }
    vec4_ operator*(float s) const { return {x_ * s, y_ * s, z_ * s, w_ * s}; }
    vec4_ operator/(float s) const { return {x_ / s, y_ / s, z_ / s, w_ / s}; }
    vec4_ operator-() const { return {-x_, -y_, -z_, -w_}; }
    vec4_& operator+=(vec4_ o) { return *this = *this + o; }
    vec4_& operator-=(vec4_ o) { return *this = *this - o; }
    vec4_& operator*=(float s) { return *this = *this * s; }
    vec4_& operator/=(float s) { return *this = *this / s; }
    bool operator==(const vec4_&) const = default;
    static const vec4_ ZERO_, ONE_;
};
inline vec4_ operator*(float s, vec4_ v) { return v * s; }
inline const vec4_ vec4_::ZERO_{0, 0, 0, 0}, vec4_::ONE_{1, 1, 1, 1};

struct color_ {
    uint8_t r_ = 0, g_ = 0, b_ = 0, a_ = 255;
    static color_ frg$make() { return {}; }
    // Each part is kept between 0 and 255: color(300, -5, 0) is the same as color(255, 0, 0).
    static color_ frg$make(int32_t r, int32_t g, int32_t b, int32_t a) {
        auto part = [](int32_t v) { return uint8_t(std::clamp(v, 0, 255)); };
        return {part(r), part(g), part(b), part(a)};
    }
    color_ fade_(float alpha) const { return {r_, g_, b_, uint8_t(std::clamp(alpha, 0.0f, 1.0f) * 255)}; }
    color_ lerp_(color_ t, float k) const {
        auto mix = [&](uint8_t a, uint8_t b) { return uint8_t(std::clamp(a + (b - a) * k, 0.0f, 255.0f)); };
        return {mix(r_, t.r_), mix(g_, t.g_), mix(b_, t.b_), mix(a_, t.a_)};
    }
    bool operator==(const color_&) const = default;
    static const color_ WHITE_, BLACK_, BLANK_, LIGHTGRAY_, GRAY_, DARKGRAY_, RED_, MAROON_, ORANGE_, YELLOW_, GOLD_, GREEN_,
        LIME_, DARKGREEN_, SKYBLUE_, BLUE_, DARKBLUE_, PURPLE_, VIOLET_, PINK_, MAGENTA_, BROWN_, BEIGE_;
};
inline const color_ color_::WHITE_{255, 255, 255, 255}, color_::BLACK_{0, 0, 0, 255}, color_::BLANK_{0, 0, 0, 0},
    color_::LIGHTGRAY_{200, 200, 200, 255}, color_::GRAY_{130, 130, 130, 255}, color_::DARKGRAY_{80, 80, 80, 255},
    color_::RED_{230, 41, 55, 255}, color_::MAROON_{190, 33, 55, 255}, color_::ORANGE_{255, 161, 0, 255},
    color_::YELLOW_{253, 249, 0, 255}, color_::GOLD_{255, 203, 0, 255}, color_::GREEN_{0, 228, 48, 255},
    color_::LIME_{0, 158, 47, 255}, color_::DARKGREEN_{0, 117, 44, 255}, color_::SKYBLUE_{102, 191, 255, 255},
    color_::BLUE_{0, 121, 241, 255}, color_::DARKBLUE_{0, 82, 172, 255}, color_::PURPLE_{200, 122, 255, 255},
    color_::VIOLET_{135, 60, 190, 255}, color_::PINK_{255, 109, 194, 255}, color_::MAGENTA_{255, 0, 255, 255},
    color_::BROWN_{127, 106, 79, 255}, color_::BEIGE_{211, 176, 131, 255};

struct rect_ {
    float x_ = 0, y_ = 0, width_ = 0, height_ = 0;
    static rect_ frg$make() { return {}; }
    static rect_ frg$make(float x, float y, float w, float h) { return {x, y, w, h}; }
    bool overlaps_(rect_ o) const { return x_ < o.x_ + o.width_ && o.x_ < x_ + width_ && y_ < o.y_ + o.height_ && o.y_ < y_ + height_; }
    bool contains_(vec2_ p) const { return p.x_ >= x_ && p.x_ < x_ + width_ && p.y_ >= y_ && p.y_ < y_ + height_; }
    vec2_ center_() const { return {x_ + width_ / 2, y_ + height_ / 2}; }
    vec2_ position_() const { return {x_, y_}; }
    vec2_ size_() const { return {width_, height_}; }
    bool operator==(const rect_&) const = default;
};

inline std::string frg$str(const vec2_& v) { return "vec2(" + str(v.x_) + ", " + str(v.y_) + ")"; }
inline std::string frg$str(const vec3_& v) { return "vec3(" + str(v.x_) + ", " + str(v.y_) + ", " + str(v.z_) + ")"; }
inline std::string frg$str(const vec4_& v) { return "vec4(" + str(v.x_) + ", " + str(v.y_) + ", " + str(v.z_) + ", " + str(v.w_) + ")"; }
inline std::string frg$str(const color_& c) { return "color(" + str(c.r_) + ", " + str(c.g_) + ", " + str(c.b_) + ", " + str(c.a_) + ")"; }
inline std::string frg$str(const rect_& r) {
    return "rect(" + str(r.x_) + ", " + str(r.y_) + ", " + str(r.width_) + ", " + str(r.height_) + ")";
}

}  // namespace core

// ---------------------------------------------------------------- random

namespace random {
inline std::mt19937& engine() {
    static std::mt19937 e{std::random_device{}()};
    return e;
}
inline void seed_(int32_t v) { engine().seed((uint32_t)v); }
inline int32_t range_(int32_t lo, int32_t hi) {
    if (hi < lo) std::swap(lo, hi);
    return std::uniform_int_distribution<int32_t>(lo, hi)(engine());
}
inline float range_(float lo, float hi) {
    if (hi < lo) std::swap(lo, hi);
    return std::uniform_real_distribution<float>(lo, hi)(engine());
}
inline float value_() { return std::uniform_real_distribution<float>(0.0f, 1.0f)(engine()); }
inline bool chance_(float p) { return value_() < p; }
// A random solid color.
inline core::color_ color_() { return core::color_::frg$make(range_(0, 255), range_(0, 255), range_(0, 255), 255); }
}  // namespace random

}  // namespace frg
