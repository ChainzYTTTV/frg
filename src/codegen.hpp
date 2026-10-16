#pragma once
#include "project.hpp"

#include <string>
#include <vector>

// One .frg file's C++: its function, method and constructor bodies.
struct GeneratedModule {
    std::string path;  // the .frg file, like "world/dungeon.frg": the build uses it to choose which part it goes in
    std::string code;
};

// The C++ for a whole game, in pieces, so the build can compile it as one file or as several parts at once:
//   header     every type, function and global variable's declaration, which every part includes. It includes the
//              runtime headers it needs too. Generic types (Box<T>) are templates, so their code is in here as well.
//   modules    each .frg file's code
//   mainCode   the global variables (with their starting values, in the order Forge sets them) and C++'s main()
// A part is: #include "forge_pch.hpp" (the precompiled runtime the build makes), #include the header, then code.
struct GeneratedCpp {
    std::string header;
    std::vector<GeneratedModule> modules;
    std::string mainCode;
    std::vector<std::string> headers;      // the runtime headers it uses
    bool needsRaylib = false;              // the game uses raylib (the engine, or images/sound), so it's linked with it
    std::vector<std::string> sources;      // extra runtime .cpp files to compile with it (toolkits)
    std::vector<std::string> linkFlags;    // extra libraries to link
};

// Turns a checked project into C++. Every user name gets a trailing '_' so it can never collide with a C++ keyword or
// macro, and every file becomes its own namespace. Forge's own helpers use '$' in their names, which Forge code can't,
// so they never collide either.
GeneratedCpp generateCpp(Project& proj);
