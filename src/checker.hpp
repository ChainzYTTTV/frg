#pragma once
#include "project.hpp"
#include "types.hpp"

#include <string>
#include <vector>

// Pass 1 learns every function and global variable in every file, so things can be used
// before (or after) the place they're written. Pass 2 checks each function body.
bool checkProject(Project& proj);

// A method built into lists, maps, text, tasks, pointers and weak references (and `length`), for editors.
struct BuiltinMethodInfo {
    std::string name;       // "add"
    std::string signature;  // "void add(Enemy)"
    Type* ret = nullptr;    // what calling it gives back
    bool isMethod = true;   // false for `length`, which is used without ()
};
std::vector<BuiltinMethodInfo> builtinMethodsOf(Type* t);
