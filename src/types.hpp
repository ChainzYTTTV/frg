#pragma once
#include <string>
#include <vector>

struct TypeDecl;

enum class TK { Error, Void, Null, Bool, Int, Float, String, Task, Struct, Class, Enum, List, Map, Optional, Pointer, Function, TypeParam, Weak };

struct Type {
    TK kind;
    int bits = 0;              // Int: 8..128, Float: 32 or 64
    bool isSigned = true;      // Int only
    std::string name;          // name shown to the user ("int", "list<Enemy>", "Player?")
    std::string cpp;           // C++ spelling used by the code generator
    Type* elem = nullptr;      // List element, Map value, Optional inner type, Pointer / Weak target
    Type* key = nullptr;       // Map key
    std::vector<Type*> params; // Function: the parameter types (elem is the return type). Generic struct/class: the types it's made with
    TypeDecl* decl = nullptr;  // Struct / Class / Enum, or the generic type a TypeParam belongs to
};

Type* tError();
Type* tVoid();
Type* tNull();
Type* tBool();
Type* tString();
Type* tTask();  // a running coroutine: functions that return 'task' can wait()
Type* tInt(int bits, bool isSigned);
Type* tFloat(int bits);
Type* tList(Type* elem);
Type* tMap(Type* key, Type* value);
Type* tOptional(Type* inner);
Type* tPointer(Type* target);
Type* tWeak(Type* target);  // Weak<Player>: points at a class object without keeping it alive
Type* tFunction(Type* ret, const std::vector<Type*>& params);  // Function<ret(params)>
Type* tNamed(TypeDecl* decl);  // the type of a struct, class or enum (a generic one with its own type names: Box<T>)
Type* tGeneric(TypeDecl* decl, const std::vector<Type*>& args);  // Box<int>
// The type `t` with each of `from` (a generic type's T, U, ...) replaced by the matching one of `to`.
Type* substType(Type* t, const std::vector<Type*>& from, const std::vector<Type*>& to);
// True for Box<...> (a struct or class written with types inside < >).
bool isGeneric(const Type* t);

// Primitive types by name (int, float, string, ...). nullptr if `name` isn't one.
Type* primitiveTypeByName(const std::string& name);
// Names no user declaration may take: primitives, list/map/Pointer, and the core types (vec2, color, ...).
bool isBuiltinTypeName(const std::string& name);
std::vector<std::string> builtinTypeNames();

inline bool isNumeric(const Type* t) { return t->kind == TK::Int || t->kind == TK::Float; }
inline bool isClass(const Type* t) { return t->kind == TK::Class; }
// Types that are always shared by reference (copying the variable doesn't copy the object).
inline bool isNullableRef(const Type* t) { return t->kind == TK::Class || t->kind == TK::Pointer; }

// "an int", "a float" - for error messages.
std::string aType(const Type* t);

// The type two numbers become when combined, e.g. int + float -> float.
Type* standardCommon(Type* a, Type* b);

// True if `derived` is `base` or inherits from it.
bool inheritsFrom(const TypeDecl* derived, const TypeDecl* base);

// True if a value of type `from` can be stored in `to` without an explicit conversion
// (and without a runtime check).
bool implicitlyConverts(Type* from, Type* to);
