#include "checker.hpp"

#include "builtins.hpp"
#include "checker_util.hpp"
#include "types.hpp"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <limits>
#include <set>

namespace fs = std::filesystem;

namespace {

using u128 = unsigned __int128;

bool isArith(const std::string& op) { return op == "+" || op == "-" || op == "*" || op == "/" || op == "%"; }
bool isCompare(const std::string& op) { return op == "<" || op == ">" || op == "<=" || op == ">="; }
bool isEquality(const std::string& op) { return op == "==" || op == "!="; }
bool isLogic(const std::string& op) { return op == "&&" || op == "||"; }
bool isBitwise(const std::string& op) { return op == "&" || op == "|" || op == "^"; }

std::string opName(const std::string& op) {
    if (op == "&&") return "and";
    if (op == "||") return "or";
    return op;
}

bool isLit(const Expr* e) { return e->litInt || e->litFloat; }

// Literal numbers like `5` or `0.5 * 2` take on the type they're used as: `int8 x = 5;` makes 5 an int8.
void coerce(Expr* e, Type* T) {
    if (T->kind == TK::Optional) T = T->elem;
    if (!isLit(e) || !isNumeric(T)) return;
    if (e->litFloat && T->kind == TK::Int) return;
    e->type = T;
    if (e->kind == EK::Unary || e->kind == EK::Binary) {
        e->opType = T;
        if (e->a) coerce(e->a.get(), T);
        if (e->b) coerce(e->b.get(), T);
    }
}

// The type a group of numbers is combined in. Literals adapt to the non-literal values around them.
Type* commonOf(const std::vector<Expr*>& xs) {
    Type* T = nullptr;
    bool anyLitFloat = false;
    for (Expr* x : xs) {
        if (isLit(x))
            anyLitFloat |= x->litFloat;
        else
            T = T ? standardCommon(T, x->type) : x->type;
    }
    if (!T) {
        for (Expr* x : xs) T = T ? standardCommon(T, x->type) : x->type;
    } else if (anyLitFloat && T->kind == TK::Int) {
        T = tFloat(32);
    }
    for (Expr* x : xs) coerce(x, T);
    return T;
}

bool literalFits(const Expr* e, const Type* T) {
    bool neg = false;
    const Expr* x = e;
    if (x->kind == EK::Unary && x->text == "-" && x->a->kind == EK::Int) {
        neg = true;
        x = x->a.get();
    }
    if (x->kind != EK::Int || T->kind != TK::Int) return true;
    u128 v = x->ival;
    if (!T->isSigned) {
        if (neg && v != 0) return false;
        return T->bits >= 128 || v <= ((u128)1 << T->bits) - 1;
    }
    u128 maxPos = ((u128)1 << (T->bits - 1)) - 1;
    return neg ? v <= maxPos + 1 : v <= maxPos;
}

bool containsBreak(const Stmt* s) {
    if (!s) return false;
    switch (s->kind) {
        case SK::Break: return true;
        case SK::Block:
            for (auto& c : s->body)
                if (containsBreak(c.get())) return true;
            return false;
        case SK::If: return containsBreak(s->then.get()) || containsBreak(s->els.get());
        default: return false;  // a break inside a nested loop belongs to that loop
    }
}

// Does every path through `s` end in a return?
bool alwaysReturns(const Stmt* s) {
    if (!s) return false;
    switch (s->kind) {
        case SK::Return: return true;
        case SK::Block:
            for (auto& c : s->body)
                if (alwaysReturns(c.get())) return true;
            return false;
        case SK::If: return s->els && alwaysReturns(s->then.get()) && alwaysReturns(s->els.get());
        case SK::While: {
            bool forever = s->expr->kind == EK::Bool && s->expr->boolVal;
            return forever && !containsBreak(s->then.get());
        }
        case SK::For: return !s->expr && !containsBreak(s->then.get());
        case SK::Switch: {
            bool hasDefault = false;
            for (auto& c : s->cases) {
                hasDefault |= c.isDefault;
                if (!alwaysReturns(c.body.get())) return false;
            }
            return hasDefault;
        }
        default: return false;
    }
}

std::string folderDisplay(const Folder* f) {
    std::string s = f->qualName;
    for (char& c : s)
        if (c == '.') c = '/';
    return s;
}

std::string kindWord(const TypeDecl* d) {
    return d->kind == DK::Struct ? "struct" : d->kind == DK::Class ? "class" : "enum";
}

bool isVec(const Type* t) {
    return t->kind == TK::Struct && t->decl->builtin && (t->name == "vec2" || t->name == "vec3" || t->name == "vec4");
}

bool isMapKey(const Type* t) {
    return t->kind == TK::Int || t->kind == TK::String || t->kind == TK::Enum || t->kind == TK::Bool;
}

// Strips Pointer<T> and T? so `p.x` works on the thing they point at.
Type* derefType(Type* t) {
    if (t->kind == TK::Pointer) t = t->elem;
    if (t->kind == TK::Optional) t = t->elem;
    if (t->kind == TK::Pointer) t = t->elem;
    return t;
}

// ---- Methods built into lists, maps and text ----

struct BMethod {
    TK on;
    const char* name;
    std::vector<std::string> params;
    std::string ret;
    bool sortable = false;  // only for lists of numbers or text
};

const std::vector<BMethod>& bmethods() {
    static const std::vector<BMethod> t = {
        {TK::List, "add", {"E"}, "void"},
        {TK::List, "insert", {"int", "E"}, "void"},
        {TK::List, "removeAt", {"int"}, "E"},
        {TK::List, "remove", {"E"}, "bool"},
        {TK::List, "contains", {"E"}, "bool"},
        {TK::List, "indexOf", {"E"}, "int"},
        {TK::List, "clear", {}, "void"},
        {TK::List, "isEmpty", {}, "bool"},
        {TK::List, "first", {}, "E"},
        {TK::List, "last", {}, "E"},
        {TK::List, "pop", {}, "E"},
        {TK::List, "reverse", {}, "void"},
        {TK::List, "sort", {}, "void", true},
        {TK::List, "copy", {}, "list<E>"},
        {TK::Map, "has", {"K"}, "bool"},
        {TK::Map, "remove", {"K"}, "bool"},
        {TK::Map, "get", {"K", "V"}, "V"},
        {TK::Map, "keys", {}, "list<K>"},
        {TK::Map, "values", {}, "list<V>"},
        {TK::Map, "clear", {}, "void"},
        {TK::Map, "isEmpty", {}, "bool"},
        {TK::String, "contains", {"string"}, "bool"},
        {TK::String, "indexOf", {"string"}, "int"},
        {TK::String, "startsWith", {"string"}, "bool"},
        {TK::String, "endsWith", {"string"}, "bool"},
        {TK::String, "upper", {}, "string"},
        {TK::String, "lower", {}, "string"},
        {TK::String, "trim", {}, "string"},
        {TK::String, "replace", {"string", "string"}, "string"},
        {TK::String, "split", {"string"}, "list<string>"},
        {TK::String, "substring", {"int", "int"}, "string"},
        {TK::String, "charAt", {"int"}, "string"},
        {TK::String, "repeat", {"int"}, "string"},
        {TK::String, "isEmpty", {}, "bool"},
        {TK::Task, "isDone", {}, "bool"},
        {TK::Task, "stop", {}, "void"},
    };
    return t;
}

Type* bmType(const std::string& pat, Type* obj) {
    if (pat == "E" || pat == "V") return obj->elem;
    if (pat == "K") return obj->key;
    if (pat == "int") return tInt(32, true);
    if (pat == "string") return tString();
    if (pat == "bool") return tBool();
    if (pat == "list<E>" || pat == "list<V>") return tList(obj->elem);
    if (pat == "list<K>") return tList(obj->key);
    if (pat == "list<string>") return tList(tString());
    return tVoid();
}

std::vector<std::string> memberNamesOf(TK kind) {
    std::vector<std::string> names = {"length"};
    for (auto& m : bmethods())
        if (m.on == kind) names.push_back(m.name);
    return names;
}

class Checker {
public:
    explicit Checker(Project& p) : P(p) {}
    bool run();

private:
    Project& P;
    Module* cur = nullptr;
    FuncDecl* fn = nullptr;
    TypeDecl* curType = nullptr;
    std::vector<std::map<std::string, VarDecl*>> scopes;
    std::vector<char> breakables;  // 'l' for a loop, 's' for a switch

    std::vector<Module*> everyModule() {
        std::vector<Module*> all;
        for (auto& m : P.preludeStore) all.push_back(m.get());
        for (Module* m : P.allModules) all.push_back(m);
        return all;
    }

    // Declarations
    void registerDecls(Module* m);
    void resolveSignatures(Module* m);
    void resolveTypeDecl(TypeDecl* d);
    void resolveFunc(FuncDecl* f);
    void checkOverrides(TypeDecl* d);
    void checkStructCycles();
    Type* resolveType(const TypeRef& r, bool allowVoid);
    TypeDecl* findTypeDecl(const std::string& name);
    std::string unknownTypeHint(const std::string& name);
    bool checkNewName(const std::string& n, const Loc& loc);

    // Scopes
    VarDecl* findLocal(const std::string& n);
    void declareLocal(VarDecl* v);

    // Conversions
    bool canAssignQuiet(Expr* e, Type* T);
    bool assignable(Expr* e, Type* T, const std::string& dest);
    Type* checkInto(Expr* e, Type* T, const std::string& dest);
    bool isLvalue(Expr* e);
    bool checkAssignTarget(Expr* e);

    // Expressions
    Type* expr(Expr* e, Type* expected = nullptr, bool allowNonValue = false);
    Type* value(Expr* e, Type* expected = nullptr) { return expr(e, expected, false); }
    Type* intLit(Expr* e);
    Type* nameExpr(Expr* e, bool allowNonValue);
    Type* memberExpr(Expr* e, bool allowNonValue);
    void valueMember(Expr* e, Expr* o);
    Type* finishRef(Expr* e, bool allowNonValue);
    Type* callExpr(Expr* e, Type* expected);
    FuncDecl* resolveCall(Expr* e, const std::vector<FuncDecl*>& candidates, const std::string& name);
    bool checkArgs(Expr* e, std::vector<ExprPtr>& args, FuncDecl* f, const std::string& name);
    Type* construct(Expr* e, std::vector<ExprPtr>& args, TypeDecl* d);
    Type* builtinCall(Expr* e, const Builtin* b);
    Type* bmethodCall(Expr* e);
    Type* indexExpr(Expr* e);
    Type* listLit(Expr* e, Type* expected);
    Type* mapLit(Expr* e, Type* expected);
    Type* braceLit(Expr* e, Type* expected);
    Type* ternaryExpr(Expr* e, Type* expected);
    Type* unaryExpr(Expr* e);
    Type* binaryExpr(Expr* e, Type* expected);
    Type* equalityOp(Expr* e);
    Type* castExpr(Expr* e);
    Type* strExpr(Expr* e);
    Type* isAsExpr(Expr* e);
    bool privateBlocked(bool isPrivate, TypeDecl* owner, Module* module, const Loc& loc, const std::string& name);
    std::string unknownHint(const std::string& n);
    void checkRunCallbacks(const Loc& loc);

    // Statements
    void stmt(Stmt* s);
    void varStmt(VarDecl* v);
    void assignStmt(Stmt* s);
    void superCall(Stmt* s);
    void requireBool(Expr* e, const std::string& what);
    void checkBody(FuncDecl* f, TypeDecl* owner);
};

// ---------------------------------------------------------------- declarations

bool Checker::checkNewName(const std::string& n, const Loc& loc) {
    if (cur && cur->isPrelude) return true;
    if (isBuiltinTypeName(n)) {
        reportError(loc, "'" + n + "' is a built-in type name, so it can't be used as a name");
        return false;
    }
    if (findBuiltin("stdio", n) || findBuiltin("core", n)) {
        reportError(loc, "'" + n + "' is the name of a built-in function", "pick a different name");
        return false;
    }
    if (isStdLib(n) || isPlannedLib(n)) {
        reportError(loc, "'" + n + "' is the name of a Forge library", "pick a different name");
        return false;
    }
    if (P.rootFolder.modules.count(n)) {
        reportError(loc, "'" + n + "' is already the name of the file " + n + ".frg",
                    "pick a different name, so " + n + ".something always means the file");
        return false;
    }
    if (P.rootFolder.folders.count(n)) {
        reportError(loc, "'" + n + "' is already the name of a folder", "pick a different name");
        return false;
    }
    return true;
}

void Checker::registerDecls(Module* m) {
    cur = m;
    auto lineOf = [&](const std::string& n) -> int {
        if (m->funcs.count(n)) return m->funcs[n]->loc.line;
        if (m->globals.count(n)) return m->globals[n]->loc.line;
        if (m->types.count(n)) return m->types[n]->loc.line;
        return 0;
    };
    auto duplicate = [&](const std::string& n, const Loc& loc) {
        int line = lineOf(n);
        if (!line) return false;
        reportError(loc, "'" + n + "' is defined twice in this file", "the first one is on line " + std::to_string(line));
        return true;
    };

    for (auto& d : m->ast.types) {
        d->module = m;
        d->builtin = m->isPrelude;
        d->cpp = "::" + m->cppNs + "::" + d->name + "_";
        checkNewName(d->name, d->loc);
        if (!m->isPrelude) {
            for (const std::string& lib : m->libs) {
                Module* lm = P.libModules.count(lib) ? P.libModules[lib] : nullptr;
                if (lm && lm->types.count(d->name))
                    reportError(d->loc, "'" + d->name + "' is already a type in the <" + lib + "> library",
                                "pick a different name");
            }
        }
        if (!duplicate(d->name, d->loc)) m->types[d->name] = d.get();
        tNamed(d.get());
        std::set<std::string> seen;
        for (size_t i = 0; i < d->enumValues.size(); i++)
            if (!seen.insert(d->enumValues[i]).second)
                reportError(d->enumLocs[i], "'" + d->enumValues[i] + "' is in this enum twice");
    }
    for (auto& g : m->ast.globals) {
        g->module = m;
        g->isGlobal = true;
        checkNewName(g->name, g->loc);
        if (!duplicate(g->name, g->loc)) m->globals[g->name] = g.get();
    }
    for (auto& f : m->ast.funcs) {
        f->module = m;
        if (m->isPrelude) {
            m->overloads[f->name].push_back(f.get());
            if (!m->funcs.count(f->name)) m->funcs[f->name] = f.get();
            continue;
        }
        if (f->name != "main") checkNewName(f->name, f->loc);
        if (!duplicate(f->name, f->loc)) m->funcs[f->name] = f.get();
    }
}

TypeDecl* Checker::findTypeDecl(const std::string& name) {
    if (auto it = cur->types.find(name); it != cur->types.end()) return it->second;
    if (auto it = P.core->types.find(name); it != P.core->types.end()) return it->second;
    for (const std::string& lib : cur->libs) {
        auto lm = P.libModules.find(lib);
        if (lm == P.libModules.end()) continue;
        if (auto it = lm->second->types.find(name); it != lm->second->types.end()) return it->second;
    }
    return nullptr;
}

std::string Checker::unknownTypeHint(const std::string& name) {
    for (Module* m : P.allModules)
        if (m != cur && m->types.count(name))
            return "'" + name + "' is in " + m->src->displayPath + " - use " + m->qualName + "." + name;
    for (auto& lm : P.preludeStore)  // in library order, so the engine's libraries are suggested before the toolkits
        if (lm->types.count(name) && !cur->libs.count(lm->name))
            return "'" + name + "' is part of the " + lm->name + " library - add #include <" + lm->name + "> at the top of " +
                   cur->src->displayPath;
    std::vector<std::string> names = builtinTypeNames();
    for (auto& [n, _] : cur->types) names.push_back(n);
    std::string guess = suggestName(name, names);
    if (!guess.empty()) return "did you mean '" + guess + "'?";
    return "";
}

Type* Checker::resolveType(const TypeRef& r, bool allowVoid) {
    if (r.empty()) return tVoid();
    Type* base = nullptr;
    auto argCount = [&](size_t n, const std::string& example) {
        if (r.args.size() == n) return true;
        reportError(r.loc, "'" + r.path[0] + "' needs " + std::to_string(n) + " type" + (n == 1 ? "" : "s") + " inside < >",
                    "for example: " + example);
        return false;
    };

    if (r.path.size() == 1) {
        const std::string& name = r.path[0];
        if (Type* prim = primitiveTypeByName(name)) {
            if (!r.args.empty()) reportError(r.loc, "'" + name + "' doesn't take types inside < >");
            base = prim;
        } else if (name == "list") {
            if (!argCount(1, "list<int>")) return tError();
            Type* elem = resolveType(r.args[0], false);
            if (elem == tError()) return tError();
            base = tList(elem);
        } else if (name == "map") {
            if (!argCount(2, "map<string, int>")) return tError();
            Type* k = resolveType(r.args[0], false);
            Type* v = resolveType(r.args[1], false);
            if (k == tError() || v == tError()) return tError();
            if (!isMapKey(k)) {
                reportError(r.args[0].loc, "map keys have to be numbers, text, bools or enums - not " + aType(k));
                return tError();
            }
            base = tMap(k, v);
        } else if (name == "Pointer") {
            if (!argCount(1, "Pointer<int>")) return tError();
            Type* target = resolveType(r.args[0], false);
            if (target == tError()) return tError();
            base = tPointer(target);
        } else if (TypeDecl* d = findTypeDecl(name)) {
            if (!r.args.empty()) reportError(r.loc, "'" + name + "' doesn't take types inside < >");
            base = tNamed(d);
        } else {
            reportError(r.loc, "unknown type '" + name + "'", unknownTypeHint(name));
            return tError();
        }
    } else {
        // file.Type, folder.file.Type, or library.Type
        const std::string& first = r.path[0];
        Module* m = nullptr;
        Folder* f = nullptr;
        if (P.libModules.count(first) && cur->libs.count(first))
            m = P.libModules[first];
        else if (P.rootFolder.modules.count(first))
            m = P.rootFolder.modules[first].get();
        else if (P.rootFolder.folders.count(first))
            f = P.rootFolder.folders[first].get();
        for (size_t i = 1; i + 1 < r.path.size() && (m || f); i++) {
            if (m) {
                m = nullptr;
                break;
            }
            if (f->modules.count(r.path[i]))
                m = f->modules[r.path[i]].get(), f = nullptr;
            else if (f->folders.count(r.path[i]))
                f = f->folders[r.path[i]].get();
            else
                f = nullptr;
        }
        TypeDecl* d = nullptr;
        if (m && m->types.count(r.path.back())) d = m->types[r.path.back()];
        if (!d) {
            reportError(r.loc, "unknown type '" + r.display() + "'");
            return tError();
        }
        if (privateBlocked(d->isPrivate, nullptr, m, r.loc, d->name)) return tError();
        if (!r.args.empty()) reportError(r.loc, "'" + d->name + "' doesn't take types inside < >");
        base = tNamed(d);
    }

    if (base == tVoid() && (!allowVoid || r.optional)) {
        reportError(r.loc, "void means 'nothing', so a variable can't be void", "use a type like int, float, bool or string");
        return tError();
    }
    return r.optional ? tOptional(base) : base;
}

void Checker::resolveFunc(FuncDecl* f) {
    f->ret = f->isCtor ? tVoid() : resolveType(f->retRef, true);
    std::set<std::string> seen;
    bool sawDefault = false;
    for (auto& p : f->params) {
        p->module = f->module;
        p->type = resolveType(p->typeRef, false);
        if (!seen.insert(p->name).second) reportError(p->loc, "two parameters are called '" + p->name + "'");
        if (p->init) sawDefault = true;
        else if (sawDefault)
            reportError(p->loc, "'" + p->name + "' needs a default value too",
                        "once a parameter has a default value, the ones after it need one as well");
    }
}

void Checker::resolveTypeDecl(TypeDecl* d) {
    if (d->kind == DK::Enum) return;
    if (!d->baseRef.empty()) {
        Type* bt = resolveType(d->baseRef, false);
        if (bt != tError()) {
            if (bt->kind != TK::Class) {
                reportError(d->baseRef.loc, "a class can only inherit from another class, but '" + bt->name + "' is " +
                                                (bt->decl ? "a " + kindWord(bt->decl) : aType(bt)));
            } else if (inheritsFrom(bt->decl, d)) {
                reportError(d->baseRef.loc, "'" + d->name + "' can't inherit from itself");
            } else {
                d->base = bt->decl;
            }
        }
    }
    for (auto& f : d->fields) {
        f->owner = d;
        f->module = d->module;
        f->isField = true;
        f->type = resolveType(f->typeRef, false);
        if (d->fieldMap.count(f->name)) reportError(f->loc, "'" + f->name + "' is in " + d->name + " twice");
        d->fieldMap[f->name] = f.get();
    }
    for (auto& m : d->methods) {
        m->owner = d;
        m->module = d->module;
        resolveFunc(m.get());
        if (d->methodMap.count(m->name) || d->fieldMap.count(m->name))
            reportError(m->loc, "'" + m->name + "' is in " + d->name + " twice",
                        "fields and methods each need their own name");
        d->methodMap[m->name] = m.get();
    }
    for (size_t i = 0; i < d->ctors.size(); i++) {
        FuncDecl* c = d->ctors[i].get();
        c->owner = d;
        c->module = d->module;
        resolveFunc(c);
        for (size_t j = 0; j < i; j++) {
            FuncDecl* o = d->ctors[j].get();
            bool same = o->params.size() == c->params.size();
            for (size_t k = 0; same && k < c->params.size(); k++) same = o->params[k]->type == c->params[k]->type;
            if (same)
                reportError(c->loc, d->name + " already has a constructor that takes these same types",
                            "each constructor needs different types of values, so Forge can tell which one you mean");
        }
    }
}

void Checker::checkOverrides(TypeDecl* d) {
    if (d->kind != DK::Class || !d->base) return;
    for (auto& f : d->fields)
        if (VarDecl* bf = findField(d->base, f->name))
            reportError(f->loc, "'" + f->name + "' is already a field of " + bf->owner->name,
                        "a class gets all the fields of the class it inherits from");
    for (auto& m : d->methods) {
        if (findField(d->base, m->name)) reportError(m->loc, "'" + m->name + "' is already a field of a base class");
        FuncDecl* bm = findMethod(d->base, m->name);
        if (!bm) continue;
        bool same = bm->ret == m->ret && bm->params.size() == m->params.size();
        for (size_t i = 0; same && i < m->params.size(); i++) same = bm->params[i]->type == m->params[i]->type;
        if (!same)
            reportError(m->loc, "'" + m->name + "' replaces the method in " + bm->owner->name + ", so it has to match it",
                        "write it as: " + signatureOf(bm));
        m->overrides = bm;
    }
    for (auto& f : d->fields)
        if (findMethod(d->base, f->name)) reportError(f->loc, "'" + f->name + "' is already a method of a base class");
}

// A struct is stored inside whatever holds it, so a struct can't contain itself (it would be infinitely big).
void Checker::checkStructCycles() {
    std::set<TypeDecl*> done, active;
    std::function<bool(TypeDecl*)> visit = [&](TypeDecl* d) -> bool {
        if (done.count(d)) return false;
        if (active.count(d)) return true;
        active.insert(d);
        for (auto& f : d->fields) {
            if (f->isStatic || !f->type) continue;
            Type* t = f->type->kind == TK::Optional ? f->type->elem : f->type;
            if (t->kind == TK::Struct && visit(t->decl)) {
                if (!d->builtin)
                    reportError(f->loc, "struct '" + d->name + "' ends up containing itself through '" + f->name + "'",
                                "a struct is stored inside whatever holds it, so it can't contain itself - use a class instead");
                active.erase(d);
                done.insert(d);
                return false;
            }
        }
        active.erase(d);
        done.insert(d);
        return false;
    };
    for (Module* m : P.allModules)
        for (auto& d : m->ast.types)
            if (d->kind == DK::Struct) visit(d.get());
}

// ---------------------------------------------------------------- scopes

VarDecl* Checker::findLocal(const std::string& n) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(n);
        if (found != it->end()) return found->second;
    }
    return nullptr;
}

void Checker::declareLocal(VarDecl* v) {
    checkNewName(v->name, v->loc);
    if (findLocal(v->name)) {
        reportError(v->loc, "there's already a variable called '" + v->name + "' here",
                    "pick a different name - or to change the existing one, leave off the type: " + v->name + " = ...;");
        return;
    }
    if (cur->funcs.count(v->name)) {
        reportError(v->loc, "'" + v->name + "' is already the name of a function in this file", "pick a different name");
        return;
    }
    if (cur->globals.count(v->name)) {
        reportError(v->loc, "there's already a global variable called '" + v->name + "' in this file",
                    "to change it, leave off the type: " + v->name + " = ...;");
        return;
    }
    if (cur->types.count(v->name) || findTypeDecl(v->name)) {
        reportError(v->loc, "'" + v->name + "' is already the name of a type", "pick a different name");
        return;
    }
    scopes.back()[v->name] = v;
}

// ---------------------------------------------------------------- conversions

bool Checker::canAssignQuiet(Expr* e, Type* T) {
    Type* S = e->type;
    if (!S || S == tError() || T == tError()) return true;
    if (S->kind == TK::Void) return false;
    Type* inner = T->kind == TK::Optional ? T->elem : T;
    if (e->litInt && isNumeric(inner)) return literalFits(e, inner);
    if (e->litFloat && inner->kind == TK::Float) return true;
    if (implicitlyConverts(S, T)) return true;
    // An optional value can be used as a plain one; it's checked for null when the game runs.
    if (S->kind == TK::Optional && T->kind != TK::Optional && implicitlyConverts(S->elem, T)) return true;
    // Pointer<int> p = score;  points p at the variable `score`.
    if (T->kind == TK::Pointer && S == T->elem && isLvalue(e)) return true;
    return false;
}

bool Checker::assignable(Expr* e, Type* T, const std::string& dest) {
    Type* S = e->type;
    if (!S || S == tError() || T == tError()) return true;
    if (S->kind == TK::Void) {
        reportError(e->loc, "this doesn't give back a value",
                    e->kind == EK::Call ? "it's a void function, so it has nothing to give back" : "");
        return false;
    }
    Type* inner = T->kind == TK::Optional ? T->elem : T;
    if (e->litInt && isNumeric(inner)) {
        if (!literalFits(e, inner)) {
            reportError(e->loc, "this number doesn't fit in " + aType(inner),
                        !inner->isSigned ? inner->name + " can only hold 0 and positive whole numbers"
                                         : "use a bigger type, like int64");
            return false;
        }
        coerce(e, inner);
        return true;
    }
    if (e->litFloat && inner->kind == TK::Float) {
        coerce(e, inner);
        return true;
    }
    if (canAssignQuiet(e, T)) return true;

    std::string hint;
    if (S->kind == TK::Null)
        hint = "null can only go in an optional type - add ? to the type, like " + T->name + "?";
    else if (S->kind == TK::Float && T->kind == TK::Int)
        hint = "use " + T->name + "(...) to turn it into a whole number (the decimals get cut off)";
    else if (S->kind == TK::String && isNumeric(T))
        hint = "use " + T->name + "(...) to turn text into a number";
    else if (T->kind == TK::String)
        hint = "use string(...) or \"{...}\" to turn it into text";
    else if (S->kind == TK::Int && T->kind == TK::Int)
        hint = T->name + " can't hold every " + S->name + " value - use " + T->name + "(...) to convert it anyway";
    else if (S->kind == TK::Float && T->kind == TK::Float)
        hint = "use float(...) to convert it (it loses some precision)";
    else if (T->kind == TK::Pointer && S == T->elem)
        hint = "a Pointer has to point at a variable, a field or a list item";
    else if (S->kind == TK::Class && T->kind == TK::Class)
        hint = "a " + S->name + " isn't a kind of " + T->name;
    else if (S->kind == TK::List && T->kind == TK::List)
        hint = "the items in the list have to be exactly " + T->elem->name;
    reportError(e->loc, dest + " needs " + aType(T) + ", but this is " + aType(S), hint);
    return false;
}

Type* Checker::checkInto(Expr* e, Type* T, const std::string& dest) {
    Type* S = value(e, T);
    if (S != tError() && T != tError()) assignable(e, T, dest);
    return S;
}

bool Checker::isLvalue(Expr* e) {
    switch (e->kind) {
        case EK::Name: return e->res.kind == RK::Local || e->res.kind == RK::Global || e->res.kind == RK::Field;
        case EK::Member:
            if (e->res.kind == RK::PointerValue) return true;
            if (e->res.kind == RK::Global) return true;
            if (e->res.kind == RK::Field) {
                Expr* o = e->a.get();
                if (o->kind == EK::This || o->kind == EK::Super) return true;
                if (o->type && (o->type->kind == TK::Class || o->type->kind == TK::Pointer ||
                                (o->type->kind == TK::Optional && isNullableRef(o->type->elem))))
                    return true;
                return isLvalue(o);
            }
            return false;
        case EK::Index: return true;
        case EK::This: return curType && curType->kind == DK::Struct;
        default: return false;
    }
}

bool Checker::checkAssignTarget(Expr* e) {
    if (e->type == tError()) return false;
    VarDecl* v = e->res.var;
    if ((e->res.kind == RK::Local || e->res.kind == RK::Global) && v->isConst) {
        reportError(e->loc, "'" + v->name + "' is const, so it can't be changed");
        return false;
    }
    if (e->res.kind == RK::TypeStatic) {
        reportError(e->loc, "built-in constants like this can't be changed");
        return false;
    }
    if (e->kind == EK::Member && e->res.kind == RK::BuiltinMethod && e->text == "length") {
        reportError(e->loc, "length can't be changed directly", "use add, remove or clear instead");
        return false;
    }
    if (!isLvalue(e)) {
        reportError(e->loc, "only variables, fields and list items can be changed",
                    e->kind == EK::Member && e->res.kind == RK::Field
                        ? "this struct is a temporary copy - store it in a variable first"
                        : "");
        return false;
    }
    if (e->kind == EK::Index) e->isWrite = true;
    return true;
}

// ---------------------------------------------------------------- expressions

Type* Checker::expr(Expr* e, Type* expected, bool allowNonValue) {
    switch (e->kind) {
        case EK::Int: return intLit(e);
        case EK::Float: e->litFloat = true; return e->type = tFloat(32);
        case EK::Bool: return e->type = tBool();
        case EK::Null: return e->type = tNull();
        case EK::Str: return strExpr(e);
        case EK::Name: return nameExpr(e, allowNonValue);
        case EK::Member: return memberExpr(e, allowNonValue);
        case EK::Call: return callExpr(e, expected);
        case EK::Unary: return unaryExpr(e);
        case EK::Binary: return binaryExpr(e, expected);
        case EK::Cast: return castExpr(e);
        case EK::Index: return indexExpr(e);
        case EK::ListLit: return listLit(e, expected);
        case EK::MapLit: return mapLit(e, expected);
        case EK::BraceLit: return braceLit(e, expected);
        case EK::Ternary: return ternaryExpr(e, expected);
        case EK::This:
            if (!curType) {
                reportError(e->loc, "'this' only works inside the methods of a struct or class");
                return e->type = tError();
            }
            return e->type = tNamed(curType);
        case EK::Super:
            reportError(e->loc, "super is used like super.method()");
            return e->type = tError();
        case EK::Is:
        case EK::As: return isAsExpr(e);
    }
    return e->type = tError();
}

// `enemy is Goblin` and `enemy as Goblin`: checking what kind of class an object really is.
Type* Checker::isAsExpr(Expr* e) {
    const char* word = e->kind == EK::Is ? "is" : "as";
    Type* S = value(e->a.get());
    Type* T = resolveType(e->typeRef, false);
    if (S == tError() || T == tError()) return e->type = tError();
    if (T->kind == TK::Optional) T = T->elem;
    Type* sc = S->kind == TK::Optional ? S->elem : S;
    if (sc->kind != TK::Class || T->kind != TK::Class) {
        reportError(e->loc, std::string("'") + word + "' checks what kind of class an object is, but this is " + aType(S) +
                                (T->kind != TK::Class ? " and " + T->name + " isn't a class" : ""));
        return e->type = tError();
    }
    if (!inheritsFrom(T->decl, sc->decl) && !inheritsFrom(sc->decl, T->decl)) {
        reportError(e->loc, aType(sc) + " can never be " + aType(T), T->name + " doesn't inherit from " + sc->name + ", or the other way around");
        return e->type = tError();
    }
    e->opType = T;
    return e->type = e->kind == EK::Is ? tBool() : tOptional(T);
}

bool Checker::privateBlocked(bool isPrivate, TypeDecl* owner, Module* module, const Loc& loc, const std::string& name) {
    if (!isPrivate) return false;
    if (owner) {
        if (curType && inheritsFrom(curType, owner)) return false;
        reportError(loc, "'" + name + "' is private to " + owner->name,
                    "only " + owner->name + "'s own methods (and classes that inherit from it) can use it");
        return true;
    }
    if (module == cur) return false;
    reportError(loc, "'" + name + "' is private to " + module->src->displayPath, "only code in that file can use it");
    return true;
}

Type* Checker::intLit(Expr* e) {
    const std::string& t = e->text;
    int base = 10;
    size_t start = 0;
    if (t.size() > 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) base = 16, start = 2;
    if (t.size() > 2 && t[0] == '0' && (t[1] == 'b' || t[1] == 'B')) base = 2, start = 2;
    const u128 maxU128 = ~(u128)0;
    u128 v = 0;
    for (size_t i = start; i < t.size(); i++) {
        char c = t[i];
        unsigned d = c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10;
        if (v > (maxU128 - d) / base) {
            reportError(e->loc, "this number is too big", "the biggest whole number a uint128 can hold is 2^128 - 1");
            return e->type = tError();
        }
        v = v * base + d;
    }
    e->ival = v;
    e->litInt = true;
    if (v <= (u128)std::numeric_limits<int32_t>::max()) return e->type = tInt(32, true);
    if (v <= (u128)std::numeric_limits<int64_t>::max()) return e->type = tInt(64, true);
    if (v <= (u128)std::numeric_limits<uint64_t>::max()) return e->type = tInt(64, false);
    if (v <= maxU128 >> 1) return e->type = tInt(128, true);
    return e->type = tInt(128, false);
}

Type* Checker::strExpr(Expr* e) {
    for (auto& a : e->args) {
        Type* t = value(a.get());
        if (t->kind == TK::Void) reportError(a->loc, "this doesn't give back a value to show");
    }
    return e->type = tString();
}

Type* Checker::finishRef(Expr* e, bool allowNonValue) {
    const std::string& n = e->text;
    switch (e->res.kind) {
        case RK::Local:
        case RK::Global:
        case RK::Field:
        case RK::TypeStatic: return e->type = e->res.var->type ? e->res.var->type : tError();
        case RK::EnumValue: return e->type = tNamed(e->res.tdecl);
        case RK::StdConst: e->litFloat = true; return e->type = tFloat(32);
        case RK::PointerValue: return e->type;
        case RK::None: return e->type = tError();
        case RK::BuiltinMethod:
            if (n == "length") return e->type = tInt(32, true);
            break;
        default: break;
    }
    if (allowNonValue) return e->type = nullptr;
    switch (e->res.kind) {
        case RK::Func:
        case RK::Builtin:
        case RK::Method:
        case RK::BuiltinMethod:
            reportError(e->loc, "'" + n + "' is a function", "to use it, call it: " + n + "()");
            break;
        case RK::Module:
            reportError(e->loc, "'" + n + "' is a file, not a value", "use it to reach things inside it, like " + n + ".something");
            break;
        case RK::Folder:
            reportError(e->loc, "'" + n + "' is a folder, not a value", "use it to reach files inside it, like " + n + ".file");
            break;
        case RK::StdLib:
            reportError(e->loc, "'" + n + "' is a library, not a value", "use it to reach things inside it, like " + n + ".something");
            break;
        case RK::Type:
            reportError(e->loc, "'" + n + "' is a type, not a value",
                        e->res.tdecl->kind == DK::Enum ? "pick one of its values, like " + n + "." + e->res.tdecl->enumValues[0]
                                                       : "to make one, call it: " + n + "(...)");
            break;
        default: break;
    }
    return e->type = tError();
}

std::string Checker::unknownHint(const std::string& n) {
    for (Module* m : P.allModules)
        if (m != cur && (m->funcs.count(n) || m->globals.count(n)))
            return "'" + n + "' is in " + m->src->displayPath + " - use " + m->qualName + "." + n;

    std::vector<std::string> names;
    for (auto& scope : scopes)
        for (auto& [k, _] : scope) names.push_back(k);
    for (TypeDecl* d = curType; d; d = d->base) {
        for (auto& [k, _] : d->fieldMap) names.push_back(k);
        for (auto& [k, _] : d->methodMap) names.push_back(k);
    }
    for (auto& [k, _] : cur->funcs) names.push_back(k);
    for (auto& [k, _] : cur->globals) names.push_back(k);
    for (auto& [k, _] : cur->types) names.push_back(k);
    for (auto& [k, _] : P.rootFolder.modules) names.push_back(k);
    for (auto& [k, _] : P.rootFolder.folders) names.push_back(k);
    for (const std::string& lib : cur->libs) {
        names.push_back(lib);
        if (lib == "stdio")
            for (auto& b : builtinMemberNames("stdio")) names.push_back(b);
    }
    std::string guess = suggestName(n, names);
    if (!guess.empty()) return "did you mean '" + guess + "'?";
    for (const std::string& lib : stdLibNames()) {
        if (lib == "stdio") continue;
        bool has = findBuiltin(lib, n) || findBuiltinConst(lib, n) ||
                   (P.libModules.count(lib) && P.libModules[lib]->funcs.count(n));
        if (has) return "there's a '" + n + "' in the " + lib + " library: #include <" + lib + "> and use " + lib + "." + n;
    }
    for (auto& lm : P.preludeStore)
        if (lm->types.count(n) && !cur->libs.count(lm->name))
            return "'" + n + "' is part of the " + lm->name + " library - add #include <" + lm->name + ">";
    return "";
}

Type* Checker::nameExpr(Expr* e, bool allowNonValue) {
    const std::string& n = e->text;
    Resolved& r = e->res;
    if (VarDecl* v = findLocal(n)) {
        r.kind = RK::Local;
        r.var = v;
    } else if (VarDecl* f = curType ? findField(curType, n) : nullptr) {
        r.kind = RK::Field;
        r.var = f;
    } else if (FuncDecl* m = curType ? findMethod(curType, n) : nullptr) {
        r.kind = RK::Method;
        r.func = m;
    } else if (auto g = cur->globals.find(n); g != cur->globals.end()) {
        r.kind = RK::Global;
        r.var = g->second;
    } else if (auto f = cur->funcs.find(n); f != cur->funcs.end()) {
        r.kind = RK::Func;
        r.func = f->second;
        r.module = cur;
    } else if (TypeDecl* d = findTypeDecl(n)) {
        r.kind = RK::Type;
        r.tdecl = d;
    } else if (const Builtin* cb = findBuiltin("core", n)) {
        r.kind = RK::Builtin;
        r.builtin = cb;
    } else if (const Builtin* b = findBuiltin("stdio", n)) {
        if (!cur->libs.count("stdio")) {
            reportError(e->loc, "'" + n + "' is part of the stdio library, which this file doesn't include",
                        "add #include <stdio> at the top of " + cur->src->displayPath);
        } else {
            r.kind = RK::Builtin;
            r.builtin = b;
        }
    } else if (isStdLib(n)) {
        if (!cur->libs.count(n)) {
            reportError(e->loc, "this file doesn't include the " + n + " library",
                        "add #include <" + n + "> at the top of " + cur->src->displayPath);
        } else {
            r.kind = RK::StdLib;
            r.lib = n;
        }
    } else if (auto m = P.rootFolder.modules.find(n); m != P.rootFolder.modules.end()) {
        r.kind = RK::Module;
        r.module = m->second.get();
    } else if (auto fo = P.rootFolder.folders.find(n); fo != P.rootFolder.folders.end()) {
        r.kind = RK::Folder;
        r.folder = fo->second.get();
    } else if (fs::is_directory(P.root / n) && !isBuiltinTypeName(n)) {
        reportError(e->loc, "the folder '" + n + "' isn't part of the game yet",
                    "add #include <\"" + n + "\"> to a file in the top folder, like main.frg");
    } else {
        reportError(e->loc, "unknown name '" + n + "'", unknownHint(n));
    }
    return finishRef(e, allowNonValue);
}

// `obj.name` where obj is a value: a field, a method, or a built-in member of a list/map/text.
void Checker::valueMember(Expr* e, Expr* o) {
    const std::string& n = e->text;
    Resolved& r = e->res;
    Type* t = o->type;
    if (t->kind == TK::Pointer && n == "value") {
        r.kind = RK::PointerValue;
        e->type = t->elem;
        return;
    }
    Type* d = derefType(t);
    if (d->kind == TK::Struct || d->kind == TK::Class) {
        if (VarDecl* f = findField(d->decl, n)) {
            r.kind = RK::Field;
            r.var = f;
            privateBlocked(f->isPrivate, f->owner, nullptr, e->loc, n);
        } else if (FuncDecl* m = findMethod(d->decl, n)) {
            r.kind = RK::Method;
            r.func = m;
            privateBlocked(m->isPrivate, m->owner, nullptr, e->loc, n);
        } else {
            std::vector<std::string> names;
            for (TypeDecl* x = d->decl; x; x = x->base) {
                for (auto& [k, v] : x->fieldMap)
                    if (!v->isStatic) names.push_back(k);
                for (auto& [k, _] : x->methodMap) names.push_back(k);
            }
            std::string guess = suggestName(n, names);
            reportError(e->loc, d->name + " has no field or method called '" + n + "'",
                        guess.empty() ? "" : "did you mean '" + guess + "'?");
        }
        return;
    }
    if (d->kind == TK::List || d->kind == TK::Map || d->kind == TK::String || d->kind == TK::Task) {
        if (n == "length" && d->kind != TK::Task) {
            r.kind = RK::BuiltinMethod;
            r.index = -1;
            return;
        }
        auto& tbl = bmethods();
        for (size_t i = 0; i < tbl.size(); i++) {
            if (tbl[i].on == d->kind && n == tbl[i].name) {
                r.kind = RK::BuiltinMethod;
                r.index = (int)i;
                return;
            }
        }
        std::string what = d->kind == TK::List ? "lists" : d->kind == TK::Map ? "maps" : d->kind == TK::Task ? "tasks" : "text";
        std::string guess = suggestName(n, memberNamesOf(d->kind));
        reportError(e->loc, what + " don't have anything called '" + n + "'", guess.empty() ? "" : "did you mean '" + guess + "'?");
        return;
    }
    if (t->kind == TK::Pointer)
        reportError(e->loc, "a Pointer<" + t->elem->name + "> only has '.value'", "use p.value to read or change what it points at");
    else
        reportError(e->loc, "'" + o->text + "' is " + aType(t) + ", which has nothing inside it to reach with '.'");
}

Type* Checker::memberExpr(Expr* e, bool allowNonValue) {
    Expr* o = e->a.get();
    const std::string& n = e->text;
    Resolved& r = e->res;

    if (o->kind == EK::Super) {
        if (!curType || curType->kind != DK::Class || !curType->base) {
            reportError(o->loc, "super only works inside a class that inherits from another class");
            return e->type = tError();
        }
        o->type = tNamed(curType->base);
        if (FuncDecl* m = findMethod(curType->base, n)) {
            r.kind = RK::Method;
            r.func = m;
        } else if (VarDecl* f = findField(curType->base, n)) {
            r.kind = RK::Field;
            r.var = f;
        } else {
            reportError(e->loc, curType->base->name + " has no method called '" + n + "'");
        }
        return finishRef(e, allowNonValue);
    }

    // `input` is both a stdio function and a library; in `input.keyDown(...)` it's the library.
    if (o->kind == EK::Name && o->text == "input" && cur->libs.count("input") && !findLocal("input")) {
        o->res.kind = RK::StdLib;
        o->res.lib = "input";
        o->type = nullptr;
    } else {
        expr(o, nullptr, true);
    }

    switch (o->res.kind) {
        case RK::Module: {
            Module* m = o->res.module;
            if (auto f = m->funcs.find(n); f != m->funcs.end()) {
                r.kind = RK::Func;
                r.func = f->second;
                r.module = m;
                privateBlocked(f->second->isPrivate, nullptr, m, e->loc, n);
            } else if (auto g = m->globals.find(n); g != m->globals.end()) {
                r.kind = RK::Global;
                r.var = g->second;
                privateBlocked(g->second->isPrivate, nullptr, m, e->loc, n);
            } else if (auto t = m->types.find(n); t != m->types.end()) {
                r.kind = RK::Type;
                r.tdecl = t->second;
                privateBlocked(t->second->isPrivate, nullptr, m, e->loc, n);
            } else {
                std::vector<std::string> names;
                for (auto& [k, _] : m->funcs) names.push_back(k);
                for (auto& [k, _] : m->globals) names.push_back(k);
                for (auto& [k, _] : m->types) names.push_back(k);
                std::string guess = suggestName(n, names);
                reportError(e->loc, m->src->displayPath + " has nothing called '" + n + "'",
                            guess.empty() ? "" : "did you mean '" + guess + "'?");
            }
            break;
        }
        case RK::Folder: {
            Folder* F = o->res.folder;
            if (auto m = F->modules.find(n); m != F->modules.end()) {
                r.kind = RK::Module;
                r.module = m->second.get();
            } else if (auto sub = F->folders.find(n); sub != F->folders.end()) {
                r.kind = RK::Folder;
                r.folder = sub->second.get();
            } else if (fs::is_directory(F->dir / n)) {
                reportError(e->loc, "the folder '" + folderDisplay(F) + "/" + n + "' isn't part of the game yet",
                            "add #include <\"" + n + "\"> to a file inside " + folderDisplay(F));
            } else {
                reportError(e->loc, "the folder '" + folderDisplay(F) + "' has no file called " + n + ".frg");
            }
            break;
        }
        case RK::StdLib: {
            const std::string& lib = o->res.lib;
            Module* lm = P.libModules.count(lib) ? P.libModules[lib] : nullptr;
            if (const Builtin* b = findBuiltin(lib, n)) {
                r.kind = RK::Builtin;
                r.builtin = b;
            } else if (const BuiltinConst* c = findBuiltinConst(lib, n)) {
                r.kind = RK::StdConst;
                r.constant = c;
            } else if (lm && lm->funcs.count(n)) {
                r.kind = RK::Func;
                r.func = lm->funcs[n];
                r.module = lm;
            } else if (lm && lm->globals.count(n)) {  // library constants, like OpenGL.TRIANGLES
                r.kind = RK::Global;
                r.var = lm->globals[n];
            } else if (lm && lm->types.count(n)) {
                r.kind = RK::Type;
                r.tdecl = lm->types[n];
            } else {
                std::vector<std::string> names = builtinMemberNames(lib);
                if (lm) {
                    for (auto& [k, _] : lm->globals) names.push_back(k);
                    for (auto& [k, _] : lm->funcs) names.push_back(k);
                    for (auto& [k, _] : lm->types) names.push_back(k);
                }
                std::string guess = suggestName(n, names);
                reportError(e->loc, "the " + lib + " library has nothing called '" + n + "'",
                            guess.empty() ? "" : "did you mean '" + guess + "'?");
            }
            break;
        }
        case RK::Type: {
            TypeDecl* d = o->res.tdecl;
            if (d->kind == DK::Enum) {
                auto it = std::find(d->enumValues.begin(), d->enumValues.end(), n);
                if (it != d->enumValues.end()) {
                    r.kind = RK::EnumValue;
                    r.tdecl = d;
                    r.index = (int)(it - d->enumValues.begin());
                } else {
                    std::string guess = suggestName(n, d->enumValues);
                    reportError(e->loc, "the enum " + d->name + " has no value called '" + n + "'",
                                guess.empty() ? "its values are: " + joinNames(d->enumValues) : "did you mean '" + guess + "'?");
                }
            } else if (auto it = d->fieldMap.find(n); it != d->fieldMap.end() && it->second->isStatic) {
                r.kind = RK::TypeStatic;
                r.var = it->second;
                r.tdecl = d;
            } else {
                std::vector<std::string> statics;
                for (auto& [k, v] : d->fieldMap)
                    if (v->isStatic) statics.push_back(k);
                std::string guess = suggestName(n, statics);
                reportError(e->loc, "'" + n + "' isn't something " + d->name + " has by itself",
                            guess.empty() ? "to use a field or method, make a " + d->name + " first and use that"
                                          : "did you mean '" + guess + "'?");
            }
            break;
        }
        case RK::Func:
        case RK::Builtin:
        case RK::Method:
        case RK::BuiltinMethod: reportError(e->loc, "'" + o->text + "' is a function, so '.' can't be used on it"); break;
        case RK::None:
            if (o->type && o->type != tError()) valueMember(e, o);
            break;
        default:
            if (o->type && o->type != tError()) valueMember(e, o);
            break;
    }
    return finishRef(e, allowNonValue);
}

void Checker::checkRunCallbacks(const Loc& loc) {
    Module* mm = P.mainModule;
    auto upd = mm->funcs.find("update");
    auto drw = mm->funcs.find("draw");
    std::string example = "\n\n      void update(float dt){\n          // move things here\n      }\n\n      void draw(){\n          // draw things here\n      }\n";
    if (upd == mm->funcs.end() || drw == mm->funcs.end()) {
        reportError(loc, "window.run() calls update(float dt) and draw() every frame, so main.frg needs both", "add them to main.frg:" + example);
        return;
    }
    FuncDecl* u = upd->second;
    if (u->params.size() != 1 || u->params[0]->type != tFloat(32) || u->ret != tVoid())
        reportError(u->loc, "for window.run(), update has to be written as: void update(float dt)");
    FuncDecl* d = drw->second;
    if (!d->params.empty() || d->ret != tVoid())
        reportError(d->loc, "for window.run(), draw has to be written as: void draw()");
}

bool Checker::checkArgs(Expr* e, std::vector<ExprPtr>& args, FuncDecl* f, const std::string& name) {
    size_t total = f->params.size(), required = f->requiredParams();
    if (args.size() < required || args.size() > total) {
        for (auto& a : args) value(a.get());
        std::string want = required == total ? std::to_string(total) : std::to_string(required) + " to " + std::to_string(total);
        reportError(e->loc,
                    name + "() needs " + want + (total == 1 ? " value" : " values") + ", but got " + std::to_string(args.size()),
                    "it's written as: " + signatureOf(f));
        return false;
    }
    for (size_t i = 0; i < args.size(); i++) checkInto(args[i].get(), f->params[i]->type, "'" + f->params[i]->name + "' in " + name + "()");
    for (size_t i = args.size(); i < total; i++) e->defaultArgs.push_back(f->params[i]->init.get());
    return true;
}

FuncDecl* Checker::resolveCall(Expr* e, const std::vector<FuncDecl*>& candidates, const std::string& name) {
    if (candidates.size() == 1) {
        checkArgs(e, e->args, candidates[0], name);
        return candidates[0];
    }
    bool bad = false;
    for (auto& a : e->args) bad |= value(a.get()) == tError();
    if (bad) return candidates[0];
    for (FuncDecl* f : candidates) {
        if (e->args.size() < f->requiredParams() || e->args.size() > f->params.size()) continue;
        bool fits = true;
        for (size_t i = 0; fits && i < e->args.size(); i++) fits = canAssignQuiet(e->args[i].get(), f->params[i]->type);
        if (!fits) continue;
        for (size_t i = 0; i < e->args.size(); i++) assignable(e->args[i].get(), f->params[i]->type, "");
        for (size_t i = e->args.size(); i < f->params.size(); i++) e->defaultArgs.push_back(f->params[i]->init.get());
        return f;
    }
    std::string sigs;
    for (FuncDecl* f : candidates) sigs += "\n      " + signatureOf(f);
    reportError(e->loc, "none of the versions of " + name + "() take these values", "it can be used as:" + sigs);
    return nullptr;
}

Type* Checker::construct(Expr* e, std::vector<ExprPtr>& args, TypeDecl* d) {
    e->builds = d;
    if (d->kind == DK::Enum) {
        if (args.size() == 1) {
            Type* t = value(args[0].get());
            if (t == tError() || t->kind == TK::Int) {
                coerce(args[0].get(), tInt(32, true));
                return e->type = tNamed(d);
            }
        }
        reportError(e->loc, "pick one of the values of " + d->name + ", like " + d->name + "." + d->enumValues[0],
                    "to turn a number into a " + d->name + ", use " + d->name + "(number)");
        return e->type = tError();
    }
    std::vector<FuncDecl*> ctors = findCtors(d);
    if (!ctors.empty()) {
        FuncDecl* ctor = resolveCall(e, ctors, d->name);
        if (!ctor) return e->type = tError();
        e->ctor = ctor;
        if (ctor->isPrivate && !(curType && inheritsFrom(curType, ctor->owner)))
            reportError(e->loc, "this constructor of " + ctor->owner->name + " is private",
                        "only " + ctor->owner->name + "'s own methods can use it");
    } else {
        std::vector<VarDecl*> fields = allFields(d);
        if (!args.empty() && args.size() != fields.size()) {
            for (auto& a : args) value(a.get());
            std::vector<std::string> names;
            for (VarDecl* f : fields) names.push_back(f->typeRef.display() + " " + f->name);
            reportError(e->loc, d->name + "(...) takes either nothing or all " + std::to_string(fields.size()) + " of its fields, but got " +
                                    std::to_string(args.size()),
                        "its fields, in order, are: " + joinNames(names) + " - or give " + d->name + " a constructor");
        } else {
            for (size_t i = 0; i < args.size(); i++) checkInto(args[i].get(), fields[i]->type, "'" + fields[i]->name + "' in " + d->name + "()");
        }
    }
    return e->type = tNamed(d);
}

Type* Checker::callExpr(Expr* e, Type* expected) {
    (void)expected;
    Expr* callee = e->a.get();
    expr(callee, nullptr, true);
    switch (callee->res.kind) {
        case RK::Func: {
            Module* m = callee->res.module ? callee->res.module : callee->res.func->module;
            std::vector<FuncDecl*> candidates = m->isPrelude ? m->overloads[callee->res.func->name] : std::vector<FuncDecl*>{callee->res.func};
            std::string name = m->isPrelude ? m->name + "." + callee->res.func->name : callee->res.func->name;
            FuncDecl* f = resolveCall(e, candidates, name);
            if (!f) return e->type = tError();
            callee->res.func = f;
            if (m->isPrelude && m->name == "window" && f->name == "run") checkRunCallbacks(e->loc);
            return e->type = f->ret ? f->ret : tError();
        }
        case RK::Method: {
            FuncDecl* f = callee->res.func;
            checkArgs(e, e->args, f, f->name);
            return e->type = f->ret ? f->ret : tError();
        }
        case RK::Builtin: return builtinCall(e, callee->res.builtin);
        case RK::BuiltinMethod:
            if (callee->res.index < 0) {
                reportError(e->loc, "length isn't a function", "use it without ( ): list.length");
                return e->type = tError();
            }
            return bmethodCall(e);
        case RK::Type: return construct(e, e->args, callee->res.tdecl);
        case RK::None:
            for (auto& a : e->args) value(a.get());
            if (callee->type && callee->type != tError()) reportError(e->loc, "this is " + aType(callee->type) + ", not a function");
            return e->type = tError();
        case RK::Local:
        case RK::Global:
        case RK::Field:
        case RK::StdConst:
        case RK::TypeStatic:
            for (auto& a : e->args) value(a.get());
            reportError(e->loc, "'" + callee->text + "' is a variable, not a function");
            return e->type = tError();
        default:
            for (auto& a : e->args) value(a.get());
            reportError(e->loc, "'" + callee->text + "' isn't a function, so it can't be called");
            return e->type = tError();
    }
}

Type* Checker::bmethodCall(Expr* e) {
    Expr* callee = e->a.get();
    const BMethod& m = bmethods()[callee->res.index];
    Type* obj = derefType(callee->a->type);
    std::string name = m.name;
    if (e->args.size() != m.params.size()) {
        for (auto& a : e->args) value(a.get());
        reportError(e->loc, name + "() needs " + std::to_string(m.params.size()) + (m.params.size() == 1 ? " value" : " values") +
                                ", but got " + std::to_string(e->args.size()));
        return e->type = tError();
    }
    for (size_t i = 0; i < e->args.size(); i++) checkInto(e->args[i].get(), bmType(m.params[i], obj), "the value given to " + name + "()");
    if (m.sortable && !isNumeric(obj->elem) && obj->elem->kind != TK::String)
        reportError(e->loc, "sort() only works on lists of numbers or text");
    return e->type = bmType(m.ret, obj);
}

Type* Checker::builtinCall(Expr* e, const Builtin* b) {
    std::string lib = b->lib;
    std::string name = lib == "stdio" || lib == "core" ? b->name : lib + "." + b->name;
    std::vector<Expr*> args;
    bool bad = false;
    for (auto& a : e->args) {
        args.push_back(a.get());
        if (value(a.get()) == tError()) bad = true;
    }
    auto needCount = [&](size_t lo, size_t hi) {
        if (args.size() >= lo && args.size() <= hi) return true;
        std::string want = lo == hi ? std::to_string(lo) : std::to_string(lo) + " or " + std::to_string(hi);
        reportError(e->loc, name + "() needs " + want + (hi == 1 ? " value" : " values") + ", but got " + std::to_string(args.size()));
        return false;
    };
    auto needNumbers = [&]() {
        for (Expr* a : args) {
            if (!isNumeric(a->type)) {
                reportError(a->loc, name + "() needs numbers, but this is " + aType(a->type));
                return false;
            }
        }
        return true;
    };

    switch (b->sig) {
        case BSig::Print:
        case BSig::Write:
            if (args.empty() && b->sig == BSig::Print) return e->type = tVoid();
            if (args.size() != 1) {
                reportError(e->loc, name + "() takes one value",
                            "to show several values, put them in the text: " + name + "(\"x = {x}, y = {y}\")");
            } else if (!bad && args[0]->type->kind == TK::Void) {
                reportError(args[0]->loc, "this doesn't give back a value to show");
            }
            return e->type = tVoid();
        case BSig::Input:
            if (needCount(0, 1) && args.size() == 1 && !bad) assignable(args[0], tString(), "the prompt of input()");
            return e->type = tString();
        case BSig::Wait:
            if (!fn || fn->ret != tTask()) {
                reportError(e->loc, "wait() only works inside a task function",
                            fn && !fn->isCtor ? "change '" + fn->retRef.display() + " " + fn->name + "()' to 'task " + fn->name +
                                                    "()' - then calling it starts it, and it keeps going on its own"
                                              : "make a function like: task blink() { ... wait(0.5); ... }");
                return e->type = tVoid();
            }
            if (!needCount(0, 1) || bad || args.empty()) return e->type = tVoid();
            if (args[0]->type == tTask()) return e->type = tVoid();
            if (!isNumeric(args[0]->type)) {
                reportError(args[0]->loc, "wait() needs a number of seconds (or a task to wait for), but this is " + aType(args[0]->type));
                return e->type = tVoid();
            }
            coerce(args[0], tFloat(64));
            return e->type = tVoid();
        default: break;
    }

    size_t want = (b->sig == BSig::Float1 || b->sig == BSig::Generic1) ? 1
                  : (b->sig == BSig::Float2 || b->sig == BSig::Generic2) ? 2
                                                                         : 3;
    if (!needCount(want, want) || bad || !needNumbers()) return e->type = tError();
    Type* T = commonOf(args);
    if (b->sig == BSig::Float1 || b->sig == BSig::Float2 || b->sig == BSig::Float3) {
        T = (T->kind == TK::Float && T->bits == 64) ? tFloat(64) : tFloat(32);
        for (Expr* a : args) coerce(a, T);
    }
    return e->type = T;
}

Type* Checker::indexExpr(Expr* e) {
    Type* t = value(e->a.get());
    if (t == tError()) {
        value(e->b.get());
        return e->type = tError();
    }
    Type* d = derefType(t);
    if (d->kind == TK::List) {
        Type* it = value(e->b.get());
        if (it != tError() && it->kind != TK::Int) {
            reportError(e->b->loc, "list positions are whole numbers, but this is " + aType(it));
        } else if (e->b->litInt) {
            coerce(e->b.get(), tInt(32, true));
        }
        return e->type = d->elem;
    }
    if (d->kind == TK::Map) {
        checkInto(e->b.get(), d->key, "the key");
        return e->type = d->elem;
    }
    value(e->b.get());
    if (d->kind == TK::String)
        reportError(e->loc, "text can't be used with [ ]", "use text.charAt(i) or text.substring(start, count)");
    else
        reportError(e->loc, "[ ] only works on lists and maps, but this is " + aType(t));
    return e->type = tError();
}

Type* Checker::listLit(Expr* e, Type* expected) {
    Type* want = expected && expected->kind == TK::Optional ? expected->elem : expected;
    if (want && want->kind == TK::List) {
        for (auto& a : e->args) checkInto(a.get(), want->elem, "this list");
        return e->type = want;
    }
    if (e->args.empty()) {
        reportError(e->loc, "Forge can't tell what kind of list [] is", "say the type first, like: list<int> numbers = [];");
        return e->type = tError();
    }
    std::vector<Expr*> items;
    bool allNumbers = true;
    for (auto& a : e->args) {
        Type* t = value(a.get());
        if (t == tError()) return e->type = tError();
        allNumbers &= isNumeric(t);
        items.push_back(a.get());
    }
    Type* T = allNumbers ? commonOf(items) : items[0]->type;
    for (Expr* x : items) {
        if (!canAssignQuiet(x, T)) {
            if (canAssignQuiet(items[0], x->type)) {
                T = x->type;
                continue;
            }
            reportError(x->loc, "all the items in a list need the same type, but this is " + aType(x->type) + " and the first one is " +
                                    aType(items[0]->type));
            return e->type = tError();
        }
    }
    for (Expr* x : items) assignable(x, T, "this list");
    return e->type = tList(T);
}

Type* Checker::mapLit(Expr* e, Type* expected) {
    Type* want = expected && expected->kind == TK::Optional ? expected->elem : expected;
    if (want && want->kind == TK::Map) {
        for (size_t i = 0; i < e->keys.size(); i++) {
            checkInto(e->keys[i].get(), want->key, "a key in this map");
            checkInto(e->args[i].get(), want->elem, "a value in this map");
        }
        return e->type = want;
    }
    Type* K = value(e->keys[0].get());
    Type* V = value(e->args[0].get());
    if (K == tError() || V == tError()) return e->type = tError();
    if (!isMapKey(K)) {
        reportError(e->keys[0]->loc, "map keys have to be numbers, text, bools or enums - not " + aType(K));
        return e->type = tError();
    }
    coerce(e->keys[0].get(), K);
    coerce(e->args[0].get(), V);
    for (size_t i = 1; i < e->keys.size(); i++) {
        checkInto(e->keys[i].get(), K, "a key in this map");
        checkInto(e->args[i].get(), V, "a value in this map");
    }
    return e->type = tMap(K, V);
}

Type* Checker::braceLit(Expr* e, Type* expected) {
    Type* want = expected && expected->kind == TK::Optional ? expected->elem : expected;
    if (want && (want->kind == TK::Struct || want->kind == TK::Class)) return construct(e, e->args, want->decl);
    if (want && want->kind == TK::Map && e->args.empty()) return e->type = want;
    if (want && want->kind == TK::List) {
        reportError(e->loc, "lists use [ ] instead of { }", "for example: [1, 2, 3]");
        return e->type = tError();
    }
    for (auto& a : e->args) value(a.get());
    reportError(e->loc, "Forge can't tell what { } should make here",
                want ? "{ } builds structs, classes and maps, but " + want->name + " is none of those"
                     : "say the type, like: Point p = {1, 2};  or call it: Point(1, 2)");
    return e->type = tError();
}

Type* Checker::ternaryExpr(Expr* e, Type* expected) {
    requireBool(e->a.get(), "? :");
    Type* B = value(e->b.get(), expected);
    Type* C = value(e->c.get(), expected);
    if (B == tError() || C == tError()) return e->type = tError();
    Type* T = nullptr;
    if (B == C) {
        T = B;
    } else if (isNumeric(B) && isNumeric(C)) {
        T = commonOf({e->b.get(), e->c.get()});
    } else if (B->kind == TK::Null) {
        T = tOptional(C);
    } else if (C->kind == TK::Null) {
        T = tOptional(B);
    } else if (implicitlyConverts(B, C)) {
        T = C;
    } else if (implicitlyConverts(C, B)) {
        T = B;
    } else {
        reportError(e->loc, "both sides of ? : need the same type, but they're " + aType(B) + " and " + aType(C));
        return e->type = tError();
    }
    assignable(e->b.get(), T, "this side of ? :");
    assignable(e->c.get(), T, "this side of ? :");
    return e->type = T;
}

Type* Checker::unaryExpr(Expr* e) {
    Type* t = value(e->a.get());
    if (t == tError()) return e->type = tError();
    if (e->text == "-") {
        if (isVec(t)) {
            e->opType = t;
            return e->type = t;
        }
        if (!isNumeric(t)) {
            reportError(e->loc, "'-' only works on numbers, but this is " + aType(t));
            return e->type = tError();
        }
        e->litInt = e->a->litInt;
        e->litFloat = e->a->litFloat;
        e->opType = t;
        return e->type = t;
    }
    if (e->text == "!") {
        if (t->kind != TK::Bool) {
            reportError(e->loc, "'not' only works on true/false values, but this is " + aType(t),
                        isNumeric(t) ? "compare it instead, like (x == 0)" : t->kind == TK::Optional ? "compare it with null: (x == null)" : "");
            return e->type = tError();
        }
        return e->type = tBool();
    }
    // "~"
    if (t->kind != TK::Int) {
        reportError(e->loc, "'~' only works on whole numbers, but this is " + aType(t));
        return e->type = tError();
    }
    if (e->a->litInt) coerce(e->a.get(), tInt(32, true));
    e->opType = e->a->type;
    return e->type = e->a->type;
}

Type* Checker::equalityOp(Expr* e) {
    Expr* a = e->a.get();
    Expr* b = e->b.get();
    Type* L = a->type;
    Type* R = b->type;
    auto ok = [&](Type* opType) {
        e->opType = opType;
        return e->type = tBool();
    };
    if (L->kind == TK::Null && R->kind == TK::Null) return ok(tNull());
    if (L->kind == TK::Null || R->kind == TK::Null) {
        Type* other = L->kind == TK::Null ? R : L;
        if (other->kind == TK::Optional) return ok(other);
        reportError(e->loc, aType(other) + " can never be null", "only optional types (like " + other->name + "?) can be null");
        return e->type = tError();
    }
    if (isNumeric(L) && isNumeric(R)) return ok(commonOf({a, b}));
    if (L->kind == TK::Optional || R->kind == TK::Optional) {
        Type* opt = L->kind == TK::Optional ? L : R;
        Expr* other = L->kind == TK::Optional ? b : a;
        if (canAssignQuiet(other, opt)) {
            assignable(other, opt, "");
            return ok(opt);
        }
    }
    if (L == R && L->kind != TK::Void) return ok(L);
    if (L->kind == TK::Class && R->kind == TK::Class) {
        if (inheritsFrom(L->decl, R->decl)) return ok(R);
        if (inheritsFrom(R->decl, L->decl)) return ok(L);
    }
    reportError(e->loc, "can't compare " + aType(L) + " with " + aType(R));
    return e->type = tError();
}

Type* Checker::binaryExpr(Expr* e, Type* expected) {
    const std::string& op = e->text;
    std::string shown = e->opWord.empty() ? opName(op) : e->opWord;  // as the user wrote it
    Expr* a = e->a.get();
    Expr* b = e->b.get();
    auto fail = [&](const std::string& msg, const std::string& hint = "") {
        reportError(e->loc, msg, hint);
        return e->type = tError();
    };

    if (op == "??") {
        Type* L = value(a, expected ? tOptional(expected) : nullptr);
        if (L == tError()) {
            value(b);
            return e->type = tError();
        }
        if (L->kind != TK::Optional) {
            value(b);
            return fail("the ?? operator gives a backup value for something that might be null, but this " + std::string(L->kind == TK::Null ? "is always null" : "is " + aType(L) + ", which is never null"));
        }
        Type* R = value(b, L->elem);
        if (R == tError()) return e->type = tError();
        Type* T = R->kind == TK::Optional || R->kind == TK::Null ? L : L->elem;
        assignable(b, T, "the backup value after ??");
        e->opType = L;
        return e->type = T;
    }

    Type* L = value(a);
    Type* R = value(b);
    if (L == tError() || R == tError()) return e->type = tError();

    if (isArith(op)) {
        if (op == "+" && L->kind == TK::String && R->kind == TK::String) {
            e->opType = tString();
            return e->type = tString();
        }
        if (op == "+" && (L->kind == TK::String || R->kind == TK::String))
            return fail("can't add text and " + aType(L->kind == TK::String ? R : L) + " with " + shown,
                        "put the value inside the text instead, like \"Score: {score}\"");
        if (isVec(L) || isVec(R)) {
            if ((op == "+" || op == "-") && L == R) {
                e->opType = L;
                return e->type = L;
            }
            if ((op == "*" || op == "/") && isVec(L) && isNumeric(R)) {
                coerce(b, tFloat(32));
                e->opType = L;
                return e->type = L;
            }
            if (op == "*" && isNumeric(L) && isVec(R)) {
                coerce(a, tFloat(32));
                e->opType = R;
                return e->type = R;
            }
            return fail("can't use '" + shown + "' on " + aType(L) + " and " + aType(R),
                        "vectors can be added to and subtracted from vectors, and multiplied or divided by numbers");
        }
        if (!isNumeric(L) || !isNumeric(R)) return fail("can't use '" + shown + "' on " + aType(L) + " and " + aType(R));
        Type* T = commonOf({a, b});
        e->opType = T;
        e->litInt = a->litInt && b->litInt;
        e->litFloat = isLit(a) && isLit(b) && !e->litInt;
        return e->type = T;
    }
    if (isEquality(op)) return equalityOp(e);
    if (isCompare(op)) {
        if (isNumeric(L) && isNumeric(R))
            e->opType = commonOf({a, b});
        else if (L->kind == TK::String && R->kind == TK::String)
            e->opType = tString();
        else
            return fail("can't compare " + aType(L) + " with " + aType(R) + " using '" + shown + "'");
        return e->type = tBool();
    }
    if (isLogic(op)) {
        if (L->kind != TK::Bool || R->kind != TK::Bool)
            return fail("'" + shown + "' needs true/false values on both sides, but got " + aType(L) + " and " + aType(R),
                        "compare numbers first, like (x > 0 and y > 0)");
        return e->type = tBool();
    }
    if (isBitwise(op)) {
        if (L->kind != TK::Int || R->kind != TK::Int)
            return fail("'" + shown + "' works on the bits of whole numbers, but got " + aType(L) + " and " + aType(R),
                        L->kind == TK::Bool ? "did you mean '" + std::string(op == "&" ? "and" : "or") + "'?" : "");
        Type* T = commonOf({a, b});
        e->opType = T;
        return e->type = T;
    }
    // << and >>
    if (L->kind != TK::Int || R->kind != TK::Int)
        return fail("'" + shown + "' shifts the bits of whole numbers, but got " + aType(L) + " and " + aType(R));
    if (a->litInt) coerce(a, tInt(32, true));
    e->opType = a->type;
    return e->type = a->type;
}

Type* Checker::castExpr(Expr* e) {
    Type* T = primitiveTypeByName(e->text);
    Type* S = value(e->a.get());
    e->type = T;
    if (S == tError()) return T;
    if (T == tVoid() || T == tTask()) {
        reportError(e->loc, "can't convert to " + T->name);
        return e->type = tError();
    }
    if (S->kind == TK::Void) {
        reportError(e->a->loc, "this doesn't give back a value to convert");
        return e->type = tError();
    }
    if (T->kind == TK::String) return T;
    if (isNumeric(T)) {
        if (isNumeric(S)) {
            // A written-out number becomes the new type directly - unless it doesn't fit, like int8(300),
            // which has to be converted (and wrap around) when the game runs.
            if (!e->a->litInt || literalFits(e->a.get(), T)) coerce(e->a.get(), T);
            return T;
        }
        if (S->kind == TK::String || (S->kind == TK::Enum && T->kind == TK::Int)) return T;
        if (S->kind == TK::Optional && isNumeric(S->elem)) return T;
        reportError(e->loc, "can't turn " + aType(S) + " into a number", "use an if to pick a number instead");
        return e->type = tError();
    }
    // bool
    if (S->kind != TK::Bool) {
        reportError(e->loc, "can't turn " + aType(S) + " into true/false", "compare it instead, like (x != 0)");
        return e->type = tError();
    }
    return T;
}

// ---------------------------------------------------------------- statements

void Checker::requireBool(Expr* e, const std::string& what) {
    Type* t = value(e);
    if (t != tError() && t->kind != TK::Bool)
        reportError(e->loc, "the condition of '" + what + "' must be true or false, but this is " + aType(t),
                    isNumeric(t) ? "compare it, like (x > 0)" : t->kind == TK::Optional ? "compare it with null, like (x != null)" : "");
}

void Checker::varStmt(VarDecl* v) {
    v->type = resolveType(v->typeRef, false);
    if (v->init) {
        checkInto(v->init.get(), v->type, "'" + v->name + "'");
    } else if (v->isConst) {
        reportError(v->loc, "a const variable needs a value", "for example: const int MAX_HP = 100;");
    } else if (v->type->kind == TK::Class || v->type->kind == TK::Pointer) {
        reportError(v->loc, "'" + v->name + "' needs a starting value",
                    v->type->kind == TK::Class ? "make one, like: " + v->type->name + " " + v->name + " = " + v->type->name +
                                                     "(...);  - or if it can be empty, make it optional: " + v->type->name + "? " +
                                                     v->name + " = null;"
                                               : "point it at a variable, like: Pointer<int> p = score;");
    }
    declareLocal(v);
}

void Checker::assignStmt(Stmt* s) {
    Expr* target = s->expr.get();
    Expr* val = s->value.get();
    Type* T = value(target);
    if (T == tError() || !checkAssignTarget(target)) {
        value(val);
        return;
    }
    std::string dest = "'" + target->text + "'";
    if (target->kind == EK::Index) dest = "this item";
    if (s->op == "=") {
        checkInto(val, T, dest);
        return;
    }
    Type* S = value(val, T);
    if (S == tError()) return;
    if (T->kind == TK::String) {
        if (s->op == "+=")
            assignable(val, T, dest);
        else
            reportError(s->loc, "'" + s->op + "' doesn't work on text", "only += works on text");
        return;
    }
    if (isVec(T)) {
        if ((s->op == "+=" || s->op == "-=") && S == T) return;
        if ((s->op == "*=" || s->op == "/=") && isNumeric(S)) {
            coerce(val, tFloat(32));
            return;
        }
        reportError(s->loc, "'" + s->op + "' can't use " + aType(S) + " on " + aType(T),
                    "add or subtract vectors, multiply or divide by numbers");
        return;
    }
    if (!isNumeric(T)) {
        reportError(s->loc, "'" + s->op + "' only works on numbers, but " + dest + " is " + aType(T));
        return;
    }
    assignable(val, T, dest);
}

void Checker::superCall(Stmt* s) {
    if (!fn || !fn->isCtor || !curType || curType->kind != DK::Class || !curType->base) {
        for (auto& a : s->args) value(a.get());
        reportError(s->loc, "super(...) can only be used in the constructor of a class that inherits from another class");
        return;
    }
    TypeDecl* base = curType->base;
    std::vector<FuncDecl*> ctors = findCtors(base);
    if (!ctors.empty()) {
        // The code generator fills in default values for missing arguments itself.
        Expr tmp(EK::Call, s->loc);
        tmp.args = std::move(s->args);
        s->target = resolveCall(&tmp, ctors, "super");
        s->args = std::move(tmp.args);
    } else {
        std::vector<VarDecl*> fields = allFields(base);
        if (!s->args.empty() && s->args.size() != fields.size()) {
            for (auto& a : s->args) value(a.get());
            reportError(s->loc, "super(...) takes either nothing or all " + std::to_string(fields.size()) + " fields of " + base->name);
            return;
        }
        for (size_t i = 0; i < s->args.size(); i++) checkInto(s->args[i].get(), fields[i]->type, "'" + fields[i]->name + "'");
    }
}

void Checker::stmt(Stmt* s) {
    switch (s->kind) {
        case SK::Block:
            scopes.emplace_back();
            for (auto& c : s->body) stmt(c.get());
            scopes.pop_back();
            break;
        case SK::Var: varStmt(s->var.get()); break;
        case SK::ExprStmt: {
            Expr* e = s->expr.get();
            Type* t = value(e);
            if (e->kind == EK::Call || t == tError()) break;
            if (e->kind == EK::Binary && e->text == "==")
                reportError(e->loc, "this compares two values but doesn't use the answer", "to set a variable, use '='");
            else
                reportError(e->loc, "this line doesn't do anything", "a line should call a function or change a variable");
            break;
        }
        case SK::Assign: assignStmt(s); break;
        case SK::IncDec: {
            Type* t = value(s->expr.get());
            if (t == tError() || !checkAssignTarget(s->expr.get())) break;
            if (!isNumeric(t)) reportError(s->loc, "'" + s->op + "' only works on numbers, but this is " + aType(t));
            break;
        }
        case SK::If:
            requireBool(s->expr.get(), "if");
            stmt(s->then.get());
            if (s->els) stmt(s->els.get());
            break;
        case SK::While:
            requireBool(s->expr.get(), "while");
            breakables.push_back('l');
            stmt(s->then.get());
            breakables.pop_back();
            break;
        case SK::For:
            scopes.emplace_back();
            if (s->init) stmt(s->init.get());
            if (s->expr) requireBool(s->expr.get(), "for");
            if (s->step) stmt(s->step.get());
            breakables.push_back('l');
            stmt(s->then.get());
            breakables.pop_back();
            scopes.pop_back();
            break;
        case SK::ForEach: {
            Type* lt = value(s->expr.get());
            VarDecl* v = s->var.get();
            v->type = resolveType(v->typeRef, false);
            if (lt != tError()) {
                Type* d = lt->kind == TK::Optional ? lt->elem : lt;
                if (d->kind != TK::List) {
                    reportError(s->expr->loc, "for (... in ...) goes through a list, but this is " + aType(lt),
                                d->kind == TK::Map ? "go through its keys: for (K key in m.keys())" : "");
                } else if (v->type != tError() && !implicitlyConverts(d->elem, v->type)) {
                    reportError(v->typeRef.loc, "the items in this list are " + d->elem->name + ", not " + v->type->name,
                                "write it as: for (" + d->elem->name + " " + v->name + " in ...)");
                }
            }
            scopes.emplace_back();
            declareLocal(v);
            breakables.push_back('l');
            stmt(s->then.get());
            breakables.pop_back();
            scopes.pop_back();
            break;
        }
        case SK::Switch: {
            Type* t = value(s->expr.get());
            bool okType = t == tError() || t->kind == TK::Int || t->kind == TK::Enum || t->kind == TK::String || t->kind == TK::Bool;
            if (!okType) reportError(s->expr->loc, "switch works on whole numbers, text and enums, but this is " + aType(t));
            bool sawDefault = false;
            for (auto& c : s->cases) {
                if (c.isDefault) {
                    if (sawDefault) reportError(c.loc, "this switch already has a default");
                    sawDefault = true;
                }
                for (auto& v : c.values) {
                    if (okType && t != tError())
                        checkInto(v.get(), t, "this case");
                    else
                        value(v.get());
                }
                breakables.push_back('s');
                stmt(c.body.get());
                breakables.pop_back();
            }
            break;
        }
        case SK::Return: {
            Type* ret = fn->ret ? fn->ret : tError();
            if (ret == tTask()) ret = tVoid();  // a task just stops when it returns
            if (s->expr) {
                if (ret == tVoid()) {
                    value(s->expr.get());
                    reportError(s->expr->loc,
                                fn->isCtor ? "a constructor can't return a value"
                                : fn->ret == tTask() ? "a task can't return a value"
                                                     : "'" + fn->name + "' is a void function, so it can't return a value",
                                fn->isCtor || fn->ret == tTask() ? "use 'return;' to stop early"
                                                                 : "remove the value, or change 'void' to the type it should return");
                } else {
                    checkInto(s->expr.get(), ret, "the return value of " + fn->name + "()");
                }
            } else if (ret != tVoid() && ret != tError()) {
                reportError(s->loc, "'" + fn->name + "' has to return " + aType(ret), "for example: return 0;");
            }
            break;
        }
        case SK::Break:
            if (breakables.empty())
                reportError(s->loc, "'break' can only be used inside a loop");
            else if (breakables.back() == 's')
                reportError(s->loc, "a switch doesn't need 'break'",
                            "in Forge, each case stops by itself at the next case - remove this break");
            break;
        case SK::Continue:
            if (std::find(breakables.begin(), breakables.end(), 'l') == breakables.end())
                reportError(s->loc, "'continue' can only be used inside a loop");
            break;
        case SK::SuperCall: superCall(s); break;
    }
}

void Checker::checkBody(FuncDecl* f, TypeDecl* owner) {
    if (!f->body) return;
    cur = f->module;
    fn = f;
    curType = owner;
    breakables.clear();
    scopes.clear();
    scopes.emplace_back();
    for (auto& p : f->params) {
        if (owner) {
            // Parameters may share a name with a field (use this.x for the field).
            if (findLocal(p->name)) continue;
            scopes.back()[p->name] = p.get();
        } else {
            declareLocal(p.get());
        }
    }
    if (owner && owner->kind == DK::Struct && f->ret == tTask())
        reportError(f->loc, "a struct's methods can't be tasks", "make " + owner->name + " a class, or move the task to a normal function");
    for (auto& s : f->body->body) stmt(s.get());
    if (f->ret && f->ret != tVoid() && f->ret != tTask() && f->ret != tError() && !alwaysReturns(f->body.get())) {
        std::string example = f->ret->kind == TK::String ? "\"\"" : f->ret->kind == TK::Bool ? "false" : isNumeric(f->ret) ? "0" : "...";
        reportError(f->loc, "'" + f->name + "' doesn't always return a value", "add a return at the end, like 'return " + example + ";'");
    }
    curType = nullptr;
}

bool Checker::run() {
    std::vector<Module*> modules = everyModule();

    // Pass 1: learn every type, function and global variable in every file.
    for (Module* m : modules) registerDecls(m);
    for (Module* m : modules) {
        cur = m;
        for (auto& d : m->ast.types) resolveTypeDecl(d.get());
        for (auto& g : m->ast.globals) g->type = resolveType(g->typeRef, false);
        for (auto& f : m->ast.funcs) resolveFunc(f.get());
    }
    for (Module* m : modules)
        for (auto& d : m->ast.types) checkOverrides(d.get());
    checkStructCycles();

    // Pass 2: check values and bodies.
    for (Module* m : modules) {
        cur = m;
        fn = nullptr;
        curType = nullptr;
        scopes.clear();
        auto checkDefaults = [&](FuncDecl* f) {
            for (auto& p : f->params)
                if (p->init && p->type) checkInto(p->init.get(), p->type, "the default value of '" + p->name + "'");
        };
        for (auto& f : m->ast.funcs) checkDefaults(f.get());
        for (auto& d : m->ast.types) {
            for (auto& meth : d->methods) checkDefaults(meth.get());
            for (auto& c : d->ctors) checkDefaults(c.get());
            for (auto& f : d->fields) {
                if (f->isStatic) continue;
                if (f->init) {
                    checkInto(f->init.get(), f->type, "'" + f->name + "'");
                } else if (!m->isPrelude && (f->type->kind == TK::Class || f->type->kind == TK::Pointer)) {
                    reportError(f->loc, "the field '" + f->name + "' needs a starting value",
                                f->type->kind == TK::Class ? "give it one (" + f->typeRef.display() + " " + f->name + " = " + f->typeRef.display() +
                                                                 "(...);) or make it optional (" + f->typeRef.display() + "? " + f->name + ";)"
                                                           : "make it optional: Pointer<...>? " + f->name + ";");
                }
            }
        }
        for (auto& g : m->ast.globals) {
            if (g->init) {
                checkInto(g->init.get(), g->type, "'" + g->name + "'");
            } else if (g->isConst) {
                reportError(g->loc, "a const variable needs a value", "for example: const int MAX_HP = 100;");
            } else if (g->type->kind == TK::Class || g->type->kind == TK::Pointer) {
                reportError(g->loc, "'" + g->name + "' needs a starting value", "or make it optional: " + g->type->name + "? " + g->name + " = null;");
            }
        }
    }
    for (Module* m : P.allModules) {
        for (auto& f : m->ast.funcs) checkBody(f.get(), nullptr);
        for (auto& d : m->ast.types) {
            for (auto& c : d->ctors) checkBody(c.get(), d.get());
            for (auto& meth : d->methods) checkBody(meth.get(), d.get());
        }
    }

    Module* mm = P.mainModule;
    auto it = mm->funcs.find("main");
    if (it == mm->funcs.end()) {
        reportError(Loc{mm->src.get(), 1, 1, 1}, "main.frg needs a main function",
                    "every game starts in main:\n\n      int main(){\n          return 0;\n      }\n");
    } else {
        FuncDecl* f = it->second;
        if (f->ret != tInt(32, true) && f->ret != tError())
            reportError(f->retRef.loc, "main has to return an int (0 means success, 1 means an error)", "write it as: int main()");
        if (!f->params.empty()) reportError(f->loc, "main can't take parameters", "write it as: int main()");
    }
    return errorCount() == 0;
}

}  // namespace

bool checkProject(Project& proj) {
    Checker c(proj);
    return c.run();
}
