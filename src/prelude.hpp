#pragma once
#include <string>
#include <vector>

// Forge's built-in types and libraries, declared in Forge syntax. Each one becomes a Module whose
// C++ code lives in the runtime (runtime/*.hpp) instead of being generated.
struct PreludeLib {
    std::string name;    // "core" is always available; the rest need #include <name>
    std::string source;
    bool needsRaylib;    // games that include it get linked with raylib
    std::string header;  // runtime header to #include, or "" if it's in forge.hpp
};

const std::vector<PreludeLib>& preludeLibs();
