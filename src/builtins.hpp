#pragma once
#include <string>
#include <vector>

// How a built-in function's arguments are checked and its result typed.
enum class BSig {
    Print,     // print(any) / print()
    Write,     // write(any)
    Input,     // input() / input(string)
    Float1,    // one number -> float (or double if given a double)
    Float2,    // two numbers -> float (or double)
    Float3,    // three numbers -> float (or double)
    Generic1,  // one number -> same type
    Generic2,  // two numbers -> their common type
    Generic3,  // three numbers -> their common type
};

struct Builtin {
    const char* lib;
    const char* name;
    BSig sig;
    const char* cpp;
};

struct BuiltinConst {
    const char* lib;
    const char* name;
    const char* cpp;
};

const Builtin* findBuiltin(const std::string& lib, const std::string& name);
const BuiltinConst* findBuiltinConst(const std::string& lib, const std::string& name);
std::vector<std::string> builtinMemberNames(const std::string& lib);

// Standard libraries that exist today (#include <stdio>).
bool isStdLib(const std::string& name);
std::vector<std::string> stdLibNames();
// Libraries that are planned but not built yet (#include <window>).
bool isPlannedLib(const std::string& name);
