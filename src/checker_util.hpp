#pragma once
// Small lookups over structs and classes, shared by the checker and the code generator.
#include "ast.hpp"
#include "types.hpp"

#include <string>
#include <vector>

// A field of `d` or of a class it inherits from (not built-in static constants).
inline VarDecl* findField(TypeDecl* d, const std::string& n) {
    for (; d; d = d->base) {
        auto it = d->fieldMap.find(n);
        if (it != d->fieldMap.end() && !it->second->isStatic) return it->second;
    }
    return nullptr;
}

inline FuncDecl* findMethod(TypeDecl* d, const std::string& n) {
    for (; d; d = d->base) {
        auto it = d->methodMap.find(n);
        if (it != d->methodMap.end()) return it->second;
    }
    return nullptr;
}

// The constructors used to build `d`: its own, or (for classes) the nearest ones it inherits.
// None means the type is built field by field.
inline std::vector<FuncDecl*> findCtors(TypeDecl* d) {
    std::vector<FuncDecl*> out;
    for (; d; d = d->base) {
        for (auto& c : d->ctors) out.push_back(c.get());
        if (!out.empty() || d->kind != DK::Class) break;
    }
    return out;
}

// Every field, inherited ones first.
inline std::vector<VarDecl*> allFields(TypeDecl* d) {
    std::vector<VarDecl*> out;
    if (d->base) out = allFields(d->base);
    for (auto& f : d->fields)
        if (!f->isStatic) out.push_back(f.get());
    return out;
}

inline std::string joinNames(const std::vector<std::string>& names) {
    std::string s;
    for (size_t i = 0; i < names.size(); i++) s += (i ? ", " : "") + names[i];
    return s;
}

inline std::string signatureOf(const FuncDecl* f) {
    std::string s = f->isCtor ? f->name + "(" : f->retRef.display() + " " + f->name + "(";
    for (size_t i = 0; i < f->params.size(); i++) {
        if (i) s += ", ";
        s += f->params[i]->typeRef.display() + " " + f->params[i]->name;
        if (f->params[i]->init) s += " = ...";
    }
    return s + ")";
}
