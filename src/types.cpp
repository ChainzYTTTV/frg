#include "types.hpp"

#include "ast.hpp"

#include <algorithm>
#include <map>
#include <memory>

static Type T_error{TK::Error, 0, true, "<error>", "void"};
static Type T_void{TK::Void, 0, true, "void", "void"};
static Type T_null{TK::Null, 0, true, "null", "::std::nullptr_t"};
static Type T_bool{TK::Bool, 0, true, "bool", "bool"};
static Type T_string{TK::String, 0, true, "string", "::std::string"};
static Type T_task{TK::Task, 0, true, "task", "::frg::Task"};

static Type T_i8{TK::Int, 8, true, "int8", "int8_t"};
static Type T_i16{TK::Int, 16, true, "int16", "int16_t"};
static Type T_i32{TK::Int, 32, true, "int", "int32_t"};
static Type T_i64{TK::Int, 64, true, "int64", "int64_t"};
static Type T_i128{TK::Int, 128, true, "int128", "__int128"};
static Type T_u8{TK::Int, 8, false, "uint8", "uint8_t"};
static Type T_u16{TK::Int, 16, false, "uint16", "uint16_t"};
static Type T_u32{TK::Int, 32, false, "uint", "uint32_t"};
static Type T_u64{TK::Int, 64, false, "uint64", "uint64_t"};
static Type T_u128{TK::Int, 128, false, "uint128", "unsigned __int128"};

static Type T_f32{TK::Float, 32, true, "float", "float"};
static Type T_f64{TK::Float, 64, true, "double", "double"};

Type* tError() { return &T_error; }
Type* tVoid() { return &T_void; }
Type* tNull() { return &T_null; }
Type* tBool() { return &T_bool; }
Type* tString() { return &T_string; }
Type* tTask() { return &T_task; }

Type* tInt(int bits, bool isSigned) {
    switch (bits) {
        case 8: return isSigned ? &T_i8 : &T_u8;
        case 16: return isSigned ? &T_i16 : &T_u16;
        case 32: return isSigned ? &T_i32 : &T_u32;
        case 64: return isSigned ? &T_i64 : &T_u64;
        default: return isSigned ? &T_i128 : &T_u128;
    }
}

Type* tFloat(int bits) { return bits == 64 ? &T_f64 : &T_f32; }

// Composite types are created once and shared, so two `list<int>` types are the same pointer.
static std::map<std::string, std::unique_ptr<Type>>& internTable() {
    static std::map<std::string, std::unique_ptr<Type>> table;
    return table;
}

static Type* intern(const std::string& key, Type proto) {
    auto& table = internTable();
    auto it = table.find(key);
    if (it != table.end()) return it->second.get();
    auto t = std::make_unique<Type>(std::move(proto));
    Type* raw = t.get();
    table[key] = std::move(t);
    return raw;
}

static std::string idOf(const void* t) {
    return std::to_string(reinterpret_cast<uintptr_t>(t));
}

Type* tList(Type* elem) {
    Type t{};
    t.kind = TK::List;
    t.name = "list<" + elem->name + ">";
    t.cpp = "::frg::List<" + elem->cpp + ">";
    t.elem = elem;
    return intern("list:" + idOf(elem), t);
}

Type* tMap(Type* key, Type* value) {
    Type t{};
    t.kind = TK::Map;
    t.name = "map<" + key->name + ", " + value->name + ">";
    t.cpp = "::frg::Map<" + key->cpp + ", " + value->cpp + ">";
    t.key = key;
    t.elem = value;
    return intern("map:" + idOf(key) + ":" + idOf(value), t);
}

Type* tOptional(Type* inner) {
    if (inner->kind == TK::Optional || inner->kind == TK::Error) return inner;
    Type t{};
    t.kind = TK::Optional;
    t.name = inner->name + "?";
    // Classes and pointers can already be empty, so they don't need a wrapper.
    t.cpp = isNullableRef(inner) ? inner->cpp
            : inner->kind == TK::TypeParam ? "::frg::Opt<" + inner->cpp + ">"  // T could be a class, which is already nullable
                                           : "::std::optional<" + inner->cpp + ">";
    t.elem = inner;
    return intern("opt:" + idOf(inner), t);
}

Type* tPointer(Type* target) {
    Type t{};
    t.kind = TK::Pointer;
    t.name = "Pointer<" + target->name + ">";
    t.cpp = target->cpp + "*";
    t.elem = target;
    return intern("ptr:" + idOf(target), t);
}

Type* tFunction(Type* ret, const std::vector<Type*>& params) {
    Type t{};
    t.kind = TK::Function;
    std::string names, cpps, key = "fn:" + idOf(ret);
    for (size_t i = 0; i < params.size(); i++) {
        names += (i ? ", " : "") + params[i]->name;
        cpps += (i ? ", " : "") + params[i]->cpp;
        key += ":" + idOf(params[i]);
    }
    t.name = "Function<" + ret->name + "(" + names + ")>";
    t.cpp = "::std::function<" + ret->cpp + "(" + cpps + ")>";
    t.elem = ret;
    t.params = params;
    return intern(key, t);
}

Type* tNamed(TypeDecl* decl) {
    if (decl->type) return decl->type;
    if (!decl->typeParams.empty()) {
        // Inside class Box<T>, the type is Box<T>, with T standing for whatever type each Box is made with.
        if (decl->paramTypes.empty())
            for (size_t i = 0; i < decl->typeParams.size(); i++) {
                Type t{};
                t.kind = TK::TypeParam;
                t.name = decl->typeParams[i];
                t.cpp = decl->typeParams[i] + "_";
                t.decl = decl;
                decl->paramTypes.push_back(intern("tparam:" + idOf(decl) + ":" + std::to_string(i), t));
            }
        decl->type = tGeneric(decl, decl->paramTypes);
        return decl->type;
    }
    Type t{};
    // An interface is used like a class: it's shared, can be null as Name?, and holds objects of the classes that have it.
    bool shared = decl->kind == DK::Class || decl->kind == DK::Interface;
    t.kind = shared ? TK::Class : decl->kind == DK::Enum ? TK::Enum : TK::Struct;
    t.name = decl->name;
    t.cpp = shared ? "::std::shared_ptr<" + decl->cpp + ">" : decl->cpp;
    t.decl = decl;
    decl->type = intern("decl:" + std::to_string(reinterpret_cast<uintptr_t>(decl)), t);
    return decl->type;
}

Type* tGeneric(TypeDecl* decl, const std::vector<Type*>& args) {
    Type t{};
    bool shared = decl->kind == DK::Class;
    t.kind = shared ? TK::Class : TK::Struct;
    std::string names, cpps, key = "gen:" + idOf(decl);
    for (size_t i = 0; i < args.size(); i++) {
        names += (i ? ", " : "") + args[i]->name;
        cpps += (i ? ", " : "") + args[i]->cpp;
        key += ":" + idOf(args[i]);
    }
    t.name = decl->name + "<" + names + ">";
    std::string inner = decl->cpp + "<" + cpps + ">";
    t.cpp = shared ? "::std::shared_ptr<" + inner + ">" : inner;
    t.decl = decl;
    t.params = args;
    return intern(key, t);
}

bool isGeneric(const Type* t) {
    return (t->kind == TK::Class || t->kind == TK::Struct) && t->decl && !t->decl->typeParams.empty();
}

Type* substType(Type* t, const std::vector<Type*>& from, const std::vector<Type*>& to) {
    if (!t || from.empty()) return t;
    switch (t->kind) {
        case TK::TypeParam:
            for (size_t i = 0; i < from.size() && i < to.size(); i++)
                if (from[i] == t) return to[i];
            return t;
        case TK::List: return tList(substType(t->elem, from, to));
        case TK::Map: return tMap(substType(t->key, from, to), substType(t->elem, from, to));
        case TK::Optional: return tOptional(substType(t->elem, from, to));
        case TK::Pointer: return tPointer(substType(t->elem, from, to));
        case TK::Function: {
            std::vector<Type*> ps;
            for (Type* p : t->params) ps.push_back(substType(p, from, to));
            return tFunction(substType(t->elem, from, to), ps);
        }
        case TK::Struct:
        case TK::Class:
            if (isGeneric(t)) {
                std::vector<Type*> args;
                for (Type* a : t->params) args.push_back(substType(a, from, to));
                return tGeneric(t->decl, args);
            }
            return t;
        default: return t;
    }
}

static const std::map<std::string, Type*>& primitiveTable() {
    static const std::map<std::string, Type*> table = {
        {"int8", &T_i8},     {"int16", &T_i16},   {"int", &T_i32},     {"int32", &T_i32},
        {"int64", &T_i64},   {"int128", &T_i128}, {"uint8", &T_u8},    {"uint16", &T_u16},
        {"uint", &T_u32},    {"uint32", &T_u32},  {"uint64", &T_u64},  {"uint128", &T_u128},
        {"float", &T_f32},   {"double", &T_f64},  {"bool", &T_bool},   {"string", &T_string},
        {"void", &T_void},    {"task", &T_task},
    };
    return table;
}

Type* primitiveTypeByName(const std::string& name) {
    auto it = primitiveTable().find(name);
    return it == primitiveTable().end() ? nullptr : it->second;
}

static const char* kOtherBuiltinTypes[] = {"list", "map", "Pointer", "Function", "vec2", "vec3", "vec4", "color", "rect"};

bool isBuiltinTypeName(const std::string& name) {
    if (primitiveTypeByName(name)) return true;
    return std::any_of(std::begin(kOtherBuiltinTypes), std::end(kOtherBuiltinTypes),
                       [&](const char* s) { return name == s; });
}

std::vector<std::string> builtinTypeNames() {
    std::vector<std::string> names;
    for (auto& [name, _] : primitiveTable()) names.push_back(name);
    for (const char* s : kOtherBuiltinTypes) names.push_back(s);
    return names;
}

std::string aType(const Type* t) {
    char c = t->name.empty() ? 'x' : t->name[0];
    bool vowel = c == 'i' || c == 'e' || c == 'a' || c == 'o' || c == 'E' || c == 'A' || c == 'I' || c == 'O';
    return (vowel ? "an " : "a ") + t->name;
}

static Type* commonInt(Type* a, Type* b) {
    if (a->isSigned == b->isSigned) return a->bits >= b->bits ? a : b;
    Type* s = a->isSigned ? a : b;
    Type* u = a->isSigned ? b : a;
    if (s->bits > u->bits) return s;
    // Mixing signed and unsigned of the same size: use a signed type big enough for both.
    return tInt(std::min(128, u->bits * 2), true);
}

Type* standardCommon(Type* a, Type* b) {
    if (a->kind == TK::Float || b->kind == TK::Float) {
        if (a->kind == TK::Float && b->kind == TK::Float) return a->bits >= b->bits ? a : b;
        return a->kind == TK::Float ? a : b;
    }
    return commonInt(a, b);
}

bool inheritsFrom(const TypeDecl* derived, const TypeDecl* base) {
    for (const TypeDecl* d = derived; d; d = d->base) {
        if (d == base) return true;
        for (const TypeDecl* i : d->interfaces)
            if (i == base) return true;
    }
    return false;
}

bool implicitlyConverts(Type* from, Type* to) {
    if (from == to) return true;
    if (from->kind == TK::Int && to->kind == TK::Int) {
        if (from->isSigned == to->isSigned) return to->bits > from->bits;
        if (!from->isSigned && to->isSigned) return to->bits > from->bits;
        return false;
    }
    if (from->kind == TK::Int && to->kind == TK::Float) return true;
    if (from->kind == TK::Float && to->kind == TK::Float) return to->bits > from->bits;
    if (from->kind == TK::Class && to->kind == TK::Class) return !isGeneric(from) && !isGeneric(to) && inheritsFrom(from->decl, to->decl);
    if (to->kind == TK::Optional) {
        if (from->kind == TK::Null) return true;
        if (from->kind == TK::Optional) return implicitlyConverts(from->elem, to->elem) && from->elem->kind == to->elem->kind;
        return implicitlyConverts(from, to->elem);
    }
    return false;
}
