#pragma once
#include <string>
#include <vector>

// What a library is part of. A game can use the built-in engine or one low-level toolkit, not both.
enum class LibKind {
    Plain,    // usable anywhere (math, physics, file, audio, ...)
    Engine,   // Forge's built-in engine (window, graphics2d, ...)
    Toolkit,  // a separate low-level graphics toolkit (OpenGL, DirectX, Vulkan)
};

// Forge's built-in types and libraries, declared in Forge syntax. Each one becomes a Module whose
// C++ code lives in the runtime (runtime/*.hpp) instead of being generated.
struct PreludeLib {
    std::string name;    // "core" is always available; the rest need #include <name>
    std::string source;
    bool needsRaylib;    // games that include it get linked with raylib
    std::string header;  // runtime header to #include, or "" if it's in forge.hpp
    LibKind kind = LibKind::Plain;
    std::vector<std::string> sources;    // extra runtime .cpp files compiled into the game
    std::vector<std::string> linkFlags;  // extra libraries to link
    std::vector<std::string> uses;       // other libraries whose types its declarations use (tilemap uses Body)
};

const std::vector<PreludeLib>& preludeLibs();
const PreludeLib* findPreludeLib(const std::string& name);
