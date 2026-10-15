#include "builtins.hpp"

#include "prelude.hpp"

#include <algorithm>

// stdio and math functions that need special type rules (print takes anything, math.min works on any number).
// Everything else built in is declared in Forge syntax in prelude.cpp.
static const Builtin kBuiltins[] = {
    {"core", "wait", BSig::Wait, "::frg::tasks::Wait"},
    {"stdio", "print", BSig::Print, "::frg::print"},
    {"stdio", "write", BSig::Write, "::frg::write"},
    {"stdio", "input", BSig::Input, "::frg::readInput"},

    {"math", "sqrt", BSig::Float1, "::std::sqrt"},
    {"math", "sin", BSig::Float1, "::std::sin"},
    {"math", "cos", BSig::Float1, "::std::cos"},
    {"math", "tan", BSig::Float1, "::std::tan"},
    {"math", "asin", BSig::Float1, "::std::asin"},
    {"math", "acos", BSig::Float1, "::std::acos"},
    {"math", "atan", BSig::Float1, "::std::atan"},
    {"math", "exp", BSig::Float1, "::std::exp"},
    {"math", "log", BSig::Float1, "::std::log"},
    {"math", "floor", BSig::Float1, "::std::floor"},
    {"math", "ceil", BSig::Float1, "::std::ceil"},
    {"math", "round", BSig::Float1, "::std::round"},
    {"math", "pow", BSig::Float2, "::std::pow"},
    {"math", "atan2", BSig::Float2, "::std::atan2"},
    {"math", "lerp", BSig::Float3, "::frg::lerp_"},
    {"math", "abs", BSig::Generic1, "::frg::abs_"},
    {"math", "sign", BSig::Generic1, "::frg::sign_"},
    {"math", "min", BSig::Generic2, "::frg::min_"},
    {"math", "max", BSig::Generic2, "::frg::max_"},
    {"math", "clamp", BSig::Generic3, "::frg::clamp_"},
};

static const BuiltinConst kConsts[] = {
    {"math", "PI", "::frg::math::PI_"},
    {"math", "TAU", "::frg::math::TAU_"},
    {"math", "E", "::frg::math::E_"},
};

static const char* kPlannedLibs[] = {
    "Vulkan",
};

const Builtin* findBuiltin(const std::string& lib, const std::string& name) {
    for (const Builtin& b : kBuiltins)
        if (lib == b.lib && name == b.name) return &b;
    return nullptr;
}

const BuiltinConst* findBuiltinConst(const std::string& lib, const std::string& name) {
    for (const BuiltinConst& c : kConsts)
        if (lib == c.lib && name == c.name) return &c;
    return nullptr;
}

std::vector<std::string> builtinMemberNames(const std::string& lib) {
    std::vector<std::string> names;
    for (const Builtin& b : kBuiltins)
        if (lib == b.lib) names.push_back(b.name);
    for (const BuiltinConst& c : kConsts)
        if (lib == c.lib) names.push_back(c.name);
    return names;
}

bool isStdLib(const std::string& name) {
    if (name == "stdio" || name == "math") return true;
    for (const PreludeLib& lib : preludeLibs())
        if (lib.name == name && name != "core") return true;
    return false;
}

std::vector<std::string> stdLibNames() {
    std::vector<std::string> names = {"stdio", "math"};
    for (const PreludeLib& lib : preludeLibs())
        if (lib.name != "core") names.push_back(lib.name);
    return names;
}

bool isPlannedLib(const std::string& name) {
    return std::any_of(std::begin(kPlannedLibs), std::end(kPlannedLibs), [&](const char* s) { return name == s; });
}
