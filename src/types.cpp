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

static std::string idOf(const Type* t) {
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
    t.cpp = isNullableRef(inner) ? inner->cpp : "::std::optional<" + inner->cpp + ">";
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

Type* tNamed(TypeDecl* decl) {
    if (decl->type) return decl->type;
    Type t{};
    t.kind = decl->kind == DK::Class ? TK::Class : decl->kind == DK::Enum ? TK::Enum : TK::Struct;
    t.name = decl->name;
    t.cpp = decl->kind == DK::Class ? "::std::shared_ptr<" + decl->cpp + ">" : decl->cpp;
    t.decl = decl;
    decl->type = intern("decl:" + std::to_string(reinterpret_cast<uintptr_t>(decl)), t);
    return decl->type;
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

static const char* kOtherBuiltinTypes[] = {"list", "map", "Pointer", "vec2", "vec3", "vec4", "color", "rect"};

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
    for (const TypeDecl* d = derived; d; d = d->base)
        if (d == base) return true;
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
    if (from->kind == TK::Class && to->kind == TK::Class) return inheritsFrom(from->decl, to->decl);
    if (to->kind == TK::Optional) {
        if (from->kind == TK::Null) return true;
        if (from->kind == TK::Optional) return implicitlyConverts(from->elem, to->elem) && from->elem->kind == to->elem->kind;
        return implicitlyConverts(from, to->elem);
    }
    return false;
}
