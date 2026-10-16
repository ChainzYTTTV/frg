#pragma once
#include "ast.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// One .frg file (or one built-in library). Other files reach what's inside it with its name: player.jump()
struct Module {
    std::string name;      // "goblin"
    std::string qualName;  // "enemies.goblin"
    std::string cppNs;     // "m_enemies__goblin" (or "frg::window" for built-in libraries)
    std::unique_ptr<SourceFile> src;
    FileAST ast;
    struct Folder* folder = nullptr;
    bool isPrelude = false;
    std::set<std::string> libs;  // standard libraries this file includes
    std::map<std::string, FuncDecl*> funcs;
    std::map<std::string, std::vector<FuncDecl*>> overloads;  // built-in libraries can overload functions
    std::map<std::string, VarDecl*> globals;
    std::map<std::string, TypeDecl*> types;
};

// A folder of .frg files. The project root is a Folder; subfolders join the game with #include <"name">.
struct Folder {
    std::string name;
    std::string qualName;  // "" for the root, "enemies", "enemies.bosses"
    std::filesystem::path dir;
    Folder* parent = nullptr;
    std::map<std::string, std::unique_ptr<Module>> modules;
    std::map<std::string, std::unique_ptr<Folder>> folders;
};

struct Project {
    std::filesystem::path root;
    std::string name;
    Folder rootFolder;
    Module* mainModule = nullptr;
    std::vector<Module*> allModules;  // the game's own files, main.frg first

    std::vector<std::unique_ptr<Module>> preludeStore;
    std::map<std::string, Module*> libModules;  // "core", "window", "physics", ...
    Module* core = nullptr;
};

// Reads and parses main.frg, the other files next to it, every included folder, and the built-in libraries.
bool loadProject(Project& proj, const std::filesystem::path& root);

// Makes loadProject use `text` for this file instead of what's on disk (an editor's unsaved changes).
void setSourceOverride(const std::filesystem::path& path, const std::string& text);
// A path in a form that's the same for every spelling of it (absolute, lowercase, backslashes), for comparing.
std::string normalPath(const std::filesystem::path& p);
