#pragma once
#include "project.hpp"

#include <string>
#include <vector>

struct GeneratedCpp {
    std::string code;
    bool needsRaylib = false;  // the game uses a window, so it gets linked with raylib
};

// Turns a checked project into one C++ file. Every user name gets a trailing '_' so it can
// never collide with a C++ keyword or macro, and every file becomes its own namespace.
// Forge's own helpers use '$' in their names, which Forge code can't, so they never collide either.
GeneratedCpp generateCpp(Project& proj);
