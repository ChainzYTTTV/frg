// Forge runtime: <testing> - checks for frg test.
//
// frg test builds the game with its tests/ folder, and instead of the game's main() runs every function in there
// whose name starts with "test". testing.check(ok) records a failure without stopping the test, so one run shows
// every check that failed; a runtime error ends that test (as a failure) and the next one runs.
#pragma once

#include "forge.hpp"

#include <cstdio>

namespace frg::testing {

struct State {
    std::vector<std::string> failures;  // in the test that's running
    int passed = 0;
    int failed = 0;
    int checks = 0;
};

inline State& state() {
    static State s;
    return s;
}

inline void fail_(const std::string& message) { state().failures.push_back(message.empty() ? "testing.fail() was called" : message); }

// testing.check(ok, message) when the line isn't known (called through a function value).
inline void check_(bool ok, const std::string& message) {
    state().checks++;
    if (!ok) fail_(message.empty() ? "a check failed" : message);
}

// What a testing.check(...) line becomes: with its place, the line as written, and (for a comparison) both values.
inline void checkAt(bool ok, const char* where, const char* text, const std::string& message, const std::string& values) {
    state().checks++;
    if (ok) return;
    std::string s = std::string(where) + ": " + text;
    if (!values.empty()) s += "\n          " + values;
    if (!message.empty()) s += "\n          " + message;
    state().failures.push_back(s);
}

inline void failAt(const char* where, const std::string& message) {
    state().failures.push_back(std::string(where) + ": " + (message.empty() ? "testing.fail() was called" : message));
}

// Runs one test function, and prints PASS or FAIL with what went wrong.
inline void run(const char* file, const char* name, void (*test)()) {
    State& s = state();
    s.failures.clear();
    try {
        test();
    } catch (const core::Error_& e) {
        s.failures.push_back("a runtime error stopped this test: " + e.where_ + ": " + e.message_ +
                             (e.trace_.empty() ? "" : "\n" + e.trace_.substr(0, e.trace_.size() - 1)));
    }
    std::fflush(stdout);
    if (s.failures.empty()) {
        s.passed++;
        std::printf("  PASS  %s  (%s)\n", name, file);
    } else {
        s.failed++;
        std::printf("  FAIL  %s  (%s)\n", name, file);
        for (const std::string& f : s.failures) std::printf("        %s\n", f.c_str());
    }
    std::fflush(stdout);
}

// The last line of frg test, and the exit code: 1 if anything failed.
inline int summary(int tests) {
    State& s = state();
    if (tests == 0) {
        std::printf("No tests found: write functions whose names start with test (like void testJump()) in the tests folder.\n");
        return 1;
    }
    std::printf("\n%d test%s passed, %d failed (%d check%s).\n", s.passed, s.passed == 1 ? "" : "s", s.failed, s.checks,
                s.checks == 1 ? "" : "s");
    return s.failed ? 1 : 0;
}

}  // namespace frg::testing
