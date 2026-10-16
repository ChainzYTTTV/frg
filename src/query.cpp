// frg query: answers an editor's questions about the code at one position, for the VS Code extension:
// where something is defined, where it's used, what it is (hover), what could be typed next (autocomplete),
// how to rename it, and the outline of a file. The project is read and checked exactly as for a build (with the
// file's unsaved text from the editor, if given), and the answer is printed as JSON on standard output.
#include "query.hpp"

#include "builtins.hpp"
#include "checker.hpp"
#include "checker_util.hpp"
#include "common.hpp"
#include "lexer.hpp"
#include "project.hpp"
#include "types.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace {

// ---------------------------------------------------------------- JSON output

std::string json(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
    return out + "\"";
}

// ---------------------------------------------------------------- what a name refers to

enum class TK2 { None, Var, Func, Type, EnumValue, Module, Folder };

struct Target {
    TK2 kind = TK2::None;
    VarDecl* var = nullptr;
    FuncDecl* func = nullptr;
    TypeDecl* type = nullptr;
    Module* module = nullptr;
    Folder* folder = nullptr;
    int index = 0;

    bool operator==(const Target& o) const {
        return kind == o.kind && var == o.var && func == o.func && type == o.type && module == o.module && folder == o.folder &&
               index == o.index;
    }
};

// One place a name is written: where it's declared or where it's used.
struct Ref {
    const SourceFile* file = nullptr;
    int line = 0, col = 0, len = 0;
    Target target;
    bool isDecl = false;
};

struct Query {
    Project& P;
    std::vector<Ref> refs;
    Module* cur = nullptr;  // the file being walked

    explicit Query(Project& p) : P(p) {}

    void add(const Loc& loc, const std::string& name, const Target& t, bool isDecl) {
        if (!loc.file || t.kind == TK2::None || name.empty()) return;
        refs.push_back({loc.file, loc.line, loc.col, (int)name.size(), t, isDecl});
    }

    Module* findModule(const std::string& qualName) {
        for (Module* m : P.allModules)
            if (m->qualName == qualName) return m;
        return nullptr;
    }

    // The struct, class, enum or interface a written type names, looked up the way the checker does.
    TypeDecl* typeNamed(const std::vector<std::string>& path, Module* in) {
        if (path.empty()) return nullptr;
        const std::string& last = path.back();
        if (path.size() == 1) {
            if (in) {
                auto it = in->types.find(last);
                if (it != in->types.end()) return it->second;
                for (const std::string& lib : in->libs)
                    if (P.libModules.count(lib)) {
                        auto& types = P.libModules[lib]->types;
                        if (types.count(last)) return types[last];
                    }
            }
            if (P.core && P.core->types.count(last)) return P.core->types[last];
            return nullptr;
        }
        std::string qual;
        for (size_t i = 0; i + 1 < path.size(); i++) qual += (i ? "." : "") + path[i];
        Module* m = findModule(qual);
        if (m && m->types.count(last)) return m->types[last];
        return nullptr;
    }

    void typeRef(const TypeRef& tr) {
        if (tr.empty()) return;
        for (const TypeRef& a : tr.args) typeRef(a);
        if (tr.isFunction) return;
        TypeDecl* d = typeNamed(tr.path, cur);
        if (!d) return;
        Loc at = tr.loc;
        for (size_t i = 0; i + 1 < tr.path.size(); i++) at.col += (int)tr.path[i].size() + 1;  // a.b.Type: point at Type
        Target t;
        t.kind = TK2::Type;
        t.type = d;
        add(at, tr.path.back(), t, false);
    }

    void varDecl(VarDecl* v) {
        if (!v) return;
        Target t;
        t.kind = TK2::Var;
        t.var = v;
        add(v->loc, v->name, t, true);
        typeRef(v->typeRef);
        expr(v->init.get());
    }

    void funcDecl(FuncDecl* f) {
        if (!f) return;
        Target t;
        t.kind = TK2::Func;
        t.func = f;
        if (!f->isCtor) add(f->loc, f->name, t, true);
        typeRef(f->retRef);
        for (auto& p : f->params) varDecl(p.get());
        stmt(f->body.get());
    }

    void expr(Expr* e) {
        if (!e) return;
        if (e->kind == EK::Name || e->kind == EK::Member) {
            Target t;
            const Resolved& r = e->res;
            switch (r.kind) {
                case RK::Local:
                case RK::Global:
                case RK::Field:
                case RK::TypeStatic:
                    t.kind = TK2::Var;
                    t.var = r.var;
                    break;
                case RK::Func:
                case RK::Method:
                    t.kind = TK2::Func;
                    t.func = r.func;
                    break;
                case RK::Type:
                    t.kind = TK2::Type;
                    t.type = r.tdecl;
                    break;
                case RK::EnumValue:
                    t.kind = TK2::EnumValue;
                    t.type = r.tdecl;
                    t.index = r.index;
                    break;
                case RK::Module:
                    t.kind = TK2::Module;
                    t.module = r.module;
                    break;
                case RK::Folder:
                    t.kind = TK2::Folder;
                    t.folder = r.folder;
                    break;
                default: break;
            }
            add(e->loc, e->text, t, false);
        }
        if (e->kind == EK::Is || e->kind == EK::As) typeRef(e->typeRef);
        if (e->kind == EK::Lambda && e->lambda) {
            for (auto& p : e->lambda->params) varDecl(p.get());
            stmt(e->lambda->body.get());
        }
        expr(e->a.get());
        expr(e->b.get());
        expr(e->c.get());
        for (auto& x : e->args) expr(x.get());
        for (auto& x : e->keys) expr(x.get());
    }

    void stmt(Stmt* s) {
        if (!s) return;
        if (s->var) varDecl(s->var.get());
        expr(s->expr.get());
        expr(s->value.get());
        stmt(s->init.get());
        stmt(s->step.get());
        stmt(s->then.get());
        stmt(s->els.get());
        for (auto& b : s->body) stmt(b.get());
        for (auto& c : s->cases) {
            for (auto& v : c.values) expr(v.get());
            stmt(c.body.get());
        }
        for (auto& a : s->args) expr(a.get());
    }

    void typeDecl(TypeDecl* d) {
        Target t;
        t.kind = TK2::Type;
        t.type = d;
        add(d->loc, d->name, t, true);
        typeRef(d->baseRef);
        for (const TypeRef& r : d->moreBaseRefs) typeRef(r);
        for (size_t i = 0; i < d->enumValues.size() && i < d->enumLocs.size(); i++) {
            Target v;
            v.kind = TK2::EnumValue;
            v.type = d;
            v.index = (int)i;
            add(d->enumLocs[i], d->enumValues[i], v, true);
        }
        for (auto& f : d->fields) varDecl(f.get());
        for (auto& c : d->ctors) funcDecl(c.get());
        for (auto& m : d->methods) funcDecl(m.get());
    }

    void indexAll() {
        for (Module* m : P.allModules) {
            cur = m;
            for (auto& d : m->ast.types) typeDecl(d.get());
            for (auto& g : m->ast.globals) varDecl(g.get());
            for (auto& f : m->ast.funcs) funcDecl(f.get());
        }
    }

    const Ref* at(const SourceFile* file, int line, int col) {
        const Ref* best = nullptr;
        for (const Ref& r : refs)
            if (r.file == file && r.line == line && col >= r.col && col <= r.col + r.len) {
                if (!best || r.len < best->len) best = &r;
            }
        return best;
    }
};

// Where a target is declared.
Loc declLoc(const Target& t) {
    switch (t.kind) {
        case TK2::Var: return t.var->loc;
        case TK2::Func: return t.func->loc;
        case TK2::Type: return t.type->loc;
        case TK2::EnumValue: return t.index < (int)t.type->enumLocs.size() ? t.type->enumLocs[t.index] : t.type->loc;
        case TK2::Module: return Loc{t.module->src.get(), 1, 1, 0};
        default: return Loc{};
    }
}

std::string targetName(const Target& t) {
    switch (t.kind) {
        case TK2::Var: return t.var->name;
        case TK2::Func: return t.func->name;
        case TK2::Type: return t.type->name;
        case TK2::EnumValue: return t.type->enumValues[t.index];
        case TK2::Module: return t.module->name;
        case TK2::Folder: return t.folder->name;
        default: return "";
    }
}

std::string typeName(const Type* t, const TypeRef& written) {
    if (t && t->kind != TK::Error) return t->name;
    return written.display();
}

std::string kindWord(const TypeDecl* d) {
    switch (d->kind) {
        case DK::Struct: return "struct";
        case DK::Class: return d->isAbstract ? "abstract class" : "class";
        case DK::Enum: return "enum";
        case DK::Interface: return "interface";
    }
    return "";
}

// What hovering over a name shows: a line of Forge, and a short note.
std::string hoverText(const Target& t) {
    switch (t.kind) {
        case TK2::Var: {
            VarDecl* v = t.var;
            std::string what = v->isParam ? "parameter" : v->isField ? (v->isStatic ? "static field" : "field") : v->isGlobal ? "global variable" : "variable";
            std::string where;
            if (v->owner) where = " of " + v->owner->name;
            else if (v->isGlobal && v->module && !v->module->isPrelude) where = " in " + v->module->src->displayPath;
            std::string code = std::string(v->isConst ? "const " : "") + typeName(v->type, v->typeRef) + " " + v->name;
            return "```forge\n" + code + "\n```\n(" + what + where + ")";
        }
        case TK2::Func: {
            FuncDecl* f = t.func;
            std::string sig = signatureOf(f);
            if (f->owner) sig = (f->isStatic ? "static " : "") + sig;
            std::string where;
            if (f->owner) where = "method of " + f->owner->name;
            else if (f->module && f->module->isPrelude) where = "from <" + f->module->name + ">";
            else if (f->module) where = "function in " + f->module->src->displayPath;
            return "```forge\n" + sig + "\n```\n(" + where + ")";
        }
        case TK2::Type: {
            TypeDecl* d = t.type;
            std::string head = kindWord(d) + " " + d->name;
            if (!d->typeParams.empty()) head += "<" + joinNames(d->typeParams) + ">";
            std::vector<std::string> bases;
            if (d->base) bases.push_back(d->base->name);
            for (TypeDecl* i : d->interfaces) bases.push_back(i->name);
            if (!bases.empty()) head += " : " + joinNames(bases);
            if (d->kind == DK::Enum) head += " { " + joinNames(d->enumValues) + " }";
            std::string where = d->module && !d->module->isPrelude ? "in " + d->module->src->displayPath
                                : d->module ? "from <" + d->module->name + ">"
                                            : "";
            return "```forge\n" + head + "\n```\n" + (where.empty() ? "" : "(" + where + ")");
        }
        case TK2::EnumValue: return "```forge\n" + t.type->name + "." + t.type->enumValues[t.index] + "\n```\n(a value of the enum " + t.type->name + ")";
        case TK2::Module: return "```forge\n" + t.module->qualName + "\n```\n(the file " + t.module->src->displayPath + ")";
        case TK2::Folder: return "```forge\n" + t.folder->qualName + "\n```\n(the folder " + t.folder->qualName + ")";
        default: return "";
    }
}

std::string locJson(const Loc& l, int len) {
    return "{\"file\":" + json(l.file ? l.file->fullPath : "") + ",\"line\":" + std::to_string(l.line) + ",\"col\":" +
           std::to_string(l.col) + ",\"len\":" + std::to_string(len) + "}";
}

// ---------------------------------------------------------------- autocomplete

struct Item {
    std::string label, kind, detail;
};

// What the code before a '.' is: a value of some type, a file, a folder, a type (for its statics), or a library.
struct Ctx {
    enum What { Unknown, Value, ModuleRef, FolderRef, TypeRef_, Library } what = Unknown;
    Type* type = nullptr;
    Module* module = nullptr;
    Folder* folder = nullptr;
    TypeDecl* tdecl = nullptr;
};

// One step of a chain like player.body.velocity: a name, and the calls and [ ] after it.
struct Segment {
    std::string name;
    std::vector<char> suffixes;  // '(' for a call, '[' for an index
};

bool isIdentChar(char c) { return std::isalnum((unsigned char)c) || c == '_'; }

// Reads a chain backwards from the end of `s`: "a.b(1).c[2]" -> {a}, {b, (}, {c, [}. Empty if it isn't one.
std::vector<Segment> parseChain(std::string s) {
    std::vector<Segment> segs;
    while (true) {
        Segment seg;
        while (!s.empty() && (s.back() == ')' || s.back() == ']')) {
            char close = s.back(), open = close == ')' ? '(' : '[';
            int depth = 0;
            size_t i = s.size();
            bool found = false;
            while (i > 0) {
                char c = s[--i];
                if (c == close) depth++;
                else if (c == open && --depth == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return {};
            seg.suffixes.insert(seg.suffixes.begin(), open);
            s.erase(i);
        }
        size_t end = s.size(), start = end;
        while (start > 0 && isIdentChar(s[start - 1])) start--;
        if (start == end) return {};
        seg.name = s.substr(start, end - start);
        if (std::isdigit((unsigned char)seg.name[0])) return {};
        s.erase(start);
        segs.insert(segs.begin(), seg);
        if (s.empty() || s.back() != '.') break;
        s.pop_back();
    }
    return segs;
}

Type* unwrap(Type* t) {
    while (t && t->kind == TK::Optional) t = t->elem;
    return t;
}

struct Completer {
    Project& P;
    Module* M;
    int line, col;
    FuncDecl* fn = nullptr;        // the function or method the cursor is in
    TypeDecl* owner = nullptr;     // its type, for a method
    std::vector<VarDecl*> locals;  // its parameters and the variables made before the cursor
    std::vector<Item> items;

    Completer(Project& p, Module* m, int l, int c) : P(p), M(m), line(l), col(c) {}

    bool before(const Loc& l) const { return l.line < line || (l.line == line && l.col < col); }

    // Whether the cursor is between a block's { and }. A block that was never closed (code being typed) counts.
    bool inside(const Stmt* block) const {
        if (!block || block->kind != SK::Block) return false;
        bool afterOpen = block->loc.line < line || (block->loc.line == line && block->loc.col < col);
        if (block->end.line == 0) return afterOpen;
        bool beforeClose = block->end.line > line || (block->end.line == line && block->end.col >= col);
        return afterOpen && beforeClose;
    }

    // The variables that exist at the cursor: made before it, in a block that the cursor is inside.
    void collectLocals(Stmt* s) {
        if (!s) return;
        switch (s->kind) {
            case SK::Block:
                if (!inside(s)) return;
                for (auto& b : s->body) collectLocals(b.get());
                return;
            case SK::Var:
                if (before(s->var->loc)) locals.push_back(s->var.get());
                collectLambdaParams(s->var->init.get());
                return;
            case SK::ForEach:  // the loop variable exists inside the loop
                if (inside(s->then.get()) && s->var) locals.push_back(s->var.get());
                collectLocals(s->then.get());
                return;
            case SK::For:
                if (inside(s->then.get())) collectLocals(s->init.get());
                collectLocals(s->then.get());
                return;
            case SK::Try:  // catch (Error e): e exists inside the catch
                collectLocals(s->then.get());
                if (inside(s->els.get()) && s->var) locals.push_back(s->var.get());
                collectLocals(s->els.get());
                return;
            default:
                collectLambdaParams(s->expr.get());
                collectLambdaParams(s->value.get());
                collectLocals(s->then.get());
                collectLocals(s->els.get());
                for (auto& b : s->body) collectLocals(b.get());
                for (auto& c : s->cases) collectLocals(c.body.get());
                return;
        }
    }

    // A function value the cursor is inside: its parameters exist there.
    void collectLambdaParams(Expr* e) {
        if (!e) return;
        if (e->kind == EK::Lambda && e->lambda && inside(e->lambda->body.get())) {
            for (auto& p : e->lambda->params) locals.push_back(p.get());
            collectLocals(e->lambda->body.get());
        }
        collectLambdaParams(e->a.get());
        collectLambdaParams(e->b.get());
        collectLambdaParams(e->c.get());
        for (auto& x : e->args) collectLambdaParams(x.get());
    }

    // The function the cursor is in: the one whose { } it's inside.
    void findScope() {
        auto consider = [&](FuncDecl* f, TypeDecl* d) {
            if (!f->body || !inside(f->body.get())) return;
            if (!fn || f->loc.line > fn->loc.line) {
                fn = f;
                owner = d;
            }
        };
        for (auto& f : M->ast.funcs) consider(f.get(), nullptr);
        for (auto& d : M->ast.types) {
            for (auto& c : d->ctors) consider(c.get(), d.get());
            for (auto& m : d->methods) consider(m.get(), d.get());
        }
        if (!fn) return;
        for (auto& p : fn->params) locals.push_back(p.get());
        collectLocals(fn->body.get());
    }

    VarDecl* localNamed(const std::string& n) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it)
            if ((*it)->name == n) return *it;
        return nullptr;
    }

    bool libIncluded(const std::string& lib) { return M->libs.count(lib) && P.libModules.count(lib); }

    TypeDecl* visibleType(const std::string& n) {
        if (M->types.count(n)) return M->types[n];
        for (const std::string& lib : M->libs)
            if (P.libModules.count(lib) && P.libModules[lib]->types.count(n)) return P.libModules[lib]->types[n];
        if (P.core && P.core->types.count(n)) return P.core->types[n];
        return nullptr;
    }

    // The result of calling a function, or the function value itself when it isn't called.
    Ctx afterFunc(FuncDecl* f, const Segment& seg, size_t& suffix) {
        Ctx c;
        if (suffix < seg.suffixes.size() && seg.suffixes[suffix] == '(') {
            suffix++;
            c.what = Ctx::Value;
            c.type = f->ret;
        }
        return c;
    }

    // Applies the [ ] and ( ) after a name to a value.
    Ctx applySuffixes(Ctx c, const Segment& seg, size_t suffix) {
        for (; suffix < seg.suffixes.size() && c.what == Ctx::Value && c.type; suffix++) {
            Type* t = unwrap(c.type);
            if (!t) return Ctx{};
            if (seg.suffixes[suffix] == '[') c.type = t->kind == TK::List || t->kind == TK::Map ? t->elem : nullptr;
            else if (t->kind == TK::Function) c.type = t->elem;  // calling a function value
            else return Ctx{};
        }
        return c;
    }

    Ctx first(const Segment& seg) {
        Ctx c;
        size_t suffix = 0;
        const std::string& n = seg.name;
        if (n == "this" && owner) {
            c.what = Ctx::Value;
            c.type = owner->type;
            return applySuffixes(c, seg, 0);
        }
        if (VarDecl* v = localNamed(n)) {
            c.what = Ctx::Value;
            c.type = v->type;
            return applySuffixes(c, seg, 0);
        }
        if (owner) {
            if (VarDecl* f = findField(owner, n)) {
                c.what = Ctx::Value;
                c.type = f->type;
                return applySuffixes(c, seg, 0);
            }
            if (FuncDecl* m = findMethod(owner, n)) return applySuffixes(afterFunc(m, seg, suffix), seg, suffix);
        }
        if (M->globals.count(n)) {
            c.what = Ctx::Value;
            c.type = M->globals[n]->type;
            return applySuffixes(c, seg, 0);
        }
        if (M->funcs.count(n)) return applySuffixes(afterFunc(M->funcs[n], seg, suffix), seg, suffix);
        if (P.rootFolder.modules.count(n)) {
            c.what = Ctx::ModuleRef;
            c.module = P.rootFolder.modules[n].get();
            return c;
        }
        if (P.rootFolder.folders.count(n)) {
            c.what = Ctx::FolderRef;
            c.folder = P.rootFolder.folders[n].get();
            return c;
        }
        if (libIncluded(n)) {
            c.what = Ctx::Library;
            c.module = P.libModules[n];
            return c;
        }
        if (TypeDecl* d = visibleType(n)) {
            c.what = Ctx::TypeRef_;
            c.tdecl = d;
            return c;
        }
        return c;
    }

    Ctx step(const Ctx& c, const Segment& seg) {
        Ctx r;
        size_t suffix = 0;
        const std::string& n = seg.name;
        switch (c.what) {
            case Ctx::Value: {
                Type* t = unwrap(c.type);
                if (!t) return r;
                if ((t->kind == TK::Struct || t->kind == TK::Class) && t->decl) {
                    TypeDecl* d = t->decl;
                    auto fill = [&](Type* x) { return isGeneric(t) ? substType(x, d->paramTypes, t->params) : x; };
                    if (VarDecl* f = findField(d, n)) {
                        r.what = Ctx::Value;
                        r.type = fill(f->type);
                        return applySuffixes(r, seg, 0);
                    }
                    if (FuncDecl* m = findMethod(d, n)) {
                        r = afterFunc(m, seg, suffix);
                        if (r.type) r.type = fill(r.type);
                        return applySuffixes(r, seg, suffix);
                    }
                }
                for (const BuiltinMethodInfo& b : builtinMethodsOf(t))
                    if (b.name == n) {
                        r.what = Ctx::Value;
                        r.type = b.ret;
                        if (b.isMethod && !seg.suffixes.empty() && seg.suffixes[0] == '(') suffix = 1;
                        return applySuffixes(r, seg, suffix);
                    }
                return r;
            }
            case Ctx::ModuleRef:
            case Ctx::Library: {
                Module* m = c.module;
                if (m->globals.count(n)) {
                    r.what = Ctx::Value;
                    r.type = m->globals[n]->type;
                    return applySuffixes(r, seg, 0);
                }
                if (m->funcs.count(n)) return applySuffixes(afterFunc(m->funcs[n], seg, suffix), seg, suffix);
                if (m->types.count(n)) {
                    r.what = Ctx::TypeRef_;
                    r.tdecl = m->types[n];
                    if (!seg.suffixes.empty() && seg.suffixes[0] == '(') {  // Player(...) makes a Player
                        r.what = Ctx::Value;
                        r.type = m->types[n]->type;
                        return applySuffixes(r, seg, 1);
                    }
                }
                return r;
            }
            case Ctx::FolderRef: {
                Folder* f = c.folder;
                if (f->modules.count(n)) {
                    r.what = Ctx::ModuleRef;
                    r.module = f->modules[n].get();
                } else if (f->folders.count(n)) {
                    r.what = Ctx::FolderRef;
                    r.folder = f->folders[n].get();
                }
                return r;
            }
            case Ctx::TypeRef_: {
                TypeDecl* d = c.tdecl;
                for (size_t i = 0; i < d->enumValues.size(); i++)
                    if (d->enumValues[i] == n) {
                        r.what = Ctx::Value;
                        r.type = d->type;
                        return r;
                    }
                if (VarDecl* s = findStaticField(d, n)) {
                    r.what = Ctx::Value;
                    r.type = s->type;
                    return applySuffixes(r, seg, 0);
                }
                if (FuncDecl* m = findMethod(d, n)) return applySuffixes(afterFunc(m, seg, suffix), seg, suffix);
                return r;
            }
            default: return r;
        }
    }

    void add(const std::string& label, const std::string& kind, const std::string& detail) {
        for (const Item& i : items)
            if (i.label == label) return;
        items.push_back({label, kind, detail});
    }

    std::string typeKind(const TypeDecl* d) {
        return d->kind == DK::Enum ? "enum" : d->kind == DK::Interface ? "interface" : d->kind == DK::Struct ? "struct" : "class";
    }

    void addModuleMembers(Module* m, bool samePlace) {
        for (auto& [name, v] : m->globals)
            if (samePlace || !v->isPrivate) add(name, v->isConst ? "constant" : "variable", typeName(v->type, v->typeRef));
        for (auto& [name, f] : m->funcs)
            if (samePlace || !f->isPrivate) add(name, "function", signatureOf(f));
        for (auto& [name, d] : m->types)
            if (samePlace || !d->isPrivate) add(name, typeKind(d), kindWord(d) + " " + name);
    }

    void addTypeMembers(TypeDecl* d, Type* t, bool statics) {
        auto fill = [&](Type* x) { return t && isGeneric(t) ? substType(x, d->paramTypes, t->params) : x; };
        for (TypeDecl* x = d; x; x = x->base) {
            // private fields and methods can only be used inside their own class
            bool mine = owner && (owner == x || inheritsFrom(owner, x));
            for (auto& f : x->fields)
                if (f->isStatic == statics && (!f->isPrivate || mine))
                    add(f->name, statics && x->builtin ? "constant" : "field", typeName(fill(f->type), f->typeRef));
            for (auto& m : x->methods)
                if (m->isStatic == statics && (!m->isPrivate || mine)) add(m->name, "method", signatureOf(m.get()));
        }
        if (statics)
            for (const std::string& v : d->enumValues) add(v, "enumMember", d->name + "." + v);
    }

    void members(const Ctx& c) {
        switch (c.what) {
            case Ctx::Value: {
                Type* t = unwrap(c.type);
                if (!t) return;
                if ((t->kind == TK::Struct || t->kind == TK::Class) && t->decl) {
                    addTypeMembers(t->decl, t, false);
                    if (!t->decl->builtin) add("describe", "property", "built-in info about the object: describe.name, .parent, .fields, .values");
                }
                for (const BuiltinMethodInfo& b : builtinMethodsOf(t)) add(b.name, b.isMethod ? "method" : "property", b.signature);
                return;
            }
            case Ctx::ModuleRef: addModuleMembers(c.module, c.module == M); return;
            case Ctx::Library: {
                addModuleMembers(c.module, false);
                for (const std::string& n : builtinMemberNames(c.module->name)) add(n, "function", c.module->name + "." + n);
                return;
            }
            case Ctx::FolderRef:
                for (auto& [name, m] : c.folder->modules) add(name, "module", "the file " + m->src->displayPath);
                for (auto& [name, f] : c.folder->folders) add(name, "folder", "the folder " + f->qualName);
                return;
            case Ctx::TypeRef_: addTypeMembers(c.tdecl, nullptr, true); return;
            default: return;
        }
    }

    void everything() {
        for (VarDecl* v : locals) add(v->name, "variable", typeName(v->type, v->typeRef));
        if (owner) {
            addTypeMembers(owner, owner->type, false);
            add("this", "keyword", "this " + owner->name);
        }
        addModuleMembers(M, true);
        for (auto& [name, m] : P.rootFolder.modules) add(name, "module", "the file " + m->src->displayPath);
        for (auto& [name, f] : P.rootFolder.folders) add(name, "folder", "the folder " + f->qualName);
        for (const std::string& lib : M->libs) {
            if (!P.libModules.count(lib)) continue;
            if (lib != "stdio") add(lib, "module", "the <" + lib + "> library");
            for (auto& [name, d] : P.libModules[lib]->types) add(name, typeKind(d), kindWord(d) + " " + name + " (from <" + lib + ">)");
        }
        if (M->libs.count("stdio"))
            for (const char* f : {"print", "write", "input"}) add(f, "function", std::string(f) + " (from <stdio>)");
        if (P.core)
            for (auto& [name, d] : P.core->types) add(name, typeKind(d), "built-in " + kindWord(d));
        for (const std::string& t : builtinTypeNames()) add(t, "keyword", "built-in type");
        for (const char* k : {"if", "else", "while", "for", "in", "do", "switch", "case", "default", "return", "break", "continue",
                              "struct", "class", "enum", "interface", "abstract", "static", "private", "const", "task", "wait",
                              "true", "false", "null", "and", "or", "not", "is", "as", "try", "catch", "throw", "super"})
            add(k, "keyword", "");
    }
};

// For autocomplete, the unfinished word at the cursor (and the '.' before it) is taken out of the text before it's
// read, so the line still makes sense: "player.ju|" is read as "player;" and "pla|" as "".
std::string patchForCompletion(const std::string& text, int line, int col, std::string& beforeOut) {
    std::vector<size_t> starts = {0};
    for (size_t i = 0; i < text.size(); i++)
        if (text[i] == '\n') starts.push_back(i + 1);
    if (line < 1 || line > (int)starts.size()) return text;
    size_t ls = starts[line - 1];
    size_t le = text.find('\n', ls);
    if (le == std::string::npos) le = text.size();
    std::string lineText = text.substr(ls, le - ls);
    if (!lineText.empty() && lineText.back() == '\r') lineText.pop_back();
    size_t cut = std::min<size_t>(std::max(col - 1, 0), lineText.size());
    std::string before = lineText.substr(0, cut), after = lineText.substr(cut);
    beforeOut = before;
    size_t p = before.size();
    while (p > 0 && isIdentChar(before[p - 1])) p--;
    std::string kept = before.substr(0, p);
    bool dot = !kept.empty() && kept.back() == '.';
    if (dot) kept.pop_back();
    size_t restWord = 0;
    while (restWord < after.size() && isIdentChar(after[restWord])) restWord++;
    std::string rest = after.substr(restWord);
    std::string trimmedRest = rest;
    trimmedRest.erase(0, trimmedRest.find_first_not_of(" \t"));
    std::string newLine = kept + rest;
    if (dot && trimmedRest.empty()) {
        std::string t = kept;
        t.erase(t.find_last_not_of(" \t") + 1);
        if (!t.empty() && std::string(";{},(").find(t.back()) == std::string::npos) newLine = kept + ";";
    }
    return text.substr(0, ls) + newLine + text.substr(le);
}

Module* moduleForFile(Project& P, const fs::path& file) {
    std::string want = normalPath(file);
    for (Module* m : P.allModules)
        if (normalPath(m->src->fullPath) == want) return m;
    return nullptr;
}

bool validName(const std::string& n) {
    if (n.empty() || std::isdigit((unsigned char)n[0])) return false;
    return std::all_of(n.begin(), n.end(), [](char c) { return isIdentChar(c); });
}

}  // namespace

int runQuery(const std::string& what, const fs::path& folder, const fs::path& file, int line, int col, bool fromStdin,
             const std::string& newName) {
    setQuietOutput(true);
    std::string text;
    if (fromStdin) {
        std::cin >> std::noskipws;
        text.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
        text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    } else {
        std::ifstream in(file, std::ios::binary);
        text.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    }
    std::string before;
    if (what == "complete") text = patchForCompletion(text, line, col, before);
    setSourceOverride(file, text);

    Project P;
    try {
        loadProject(P, folder, true);
        if (P.mainModule) checkProject(P);
    } catch (...) {
    }
    Module* M = moduleForFile(P, file);
    if (!M) {
        std::printf("{\"error\":%s}\n", json("this file isn't part of the game (is it in a folder that's #included?)").c_str());
        return 0;
    }

    if (what == "complete") {
        Completer c(P, M, line, col);
        c.findScope();
        size_t p = before.size();
        while (p > 0 && isIdentChar(before[p - 1])) p--;
        std::string chainText = before.substr(0, p);
        if (!chainText.empty() && chainText.back() == '.') {
            chainText.pop_back();
            std::vector<Segment> segs = parseChain(chainText);
            if (!segs.empty()) {
                Ctx ctx = c.first(segs[0]);
                for (size_t i = 1; i < segs.size() && ctx.what != Ctx::Unknown; i++) ctx = c.step(ctx, segs[i]);
                c.members(ctx);
            }
        } else {
            c.everything();
        }
        std::string out = "{\"items\":[";
        for (size_t i = 0; i < c.items.size(); i++) {
            const Item& it = c.items[i];
            out += std::string(i ? "," : "") + "{\"label\":" + json(it.label) + ",\"kind\":" + json(it.kind) + ",\"detail\":" + json(it.detail) + "}";
        }
        std::printf("%s]}\n", out.c_str());
        return 0;
    }

    Query q(P);
    q.indexAll();

    if (what == "symbols") {
        // The outline: types (with their members), functions and global variables, in the order they're written.
        struct Sym {
            std::string name, kind, detail;
            Loc loc;
            std::vector<Sym> children;
        };
        std::vector<Sym> syms;
        for (auto& d : M->ast.types) {
            Sym s{d->name, kindWord(d.get()), "", d->loc, {}};
            for (size_t i = 0; i < d->enumValues.size() && i < d->enumLocs.size(); i++) s.children.push_back({d->enumValues[i], "enumMember", "", d->enumLocs[i], {}});
            for (auto& f : d->fields) s.children.push_back({f->name, "field", typeName(f->type, f->typeRef), f->loc, {}});
            for (auto& c : d->ctors) s.children.push_back({d->name, "constructor", signatureOf(c.get()), c->loc, {}});
            for (auto& m : d->methods) s.children.push_back({m->name, "method", signatureOf(m.get()), m->loc, {}});
            syms.push_back(s);
        }
        for (auto& f : M->ast.funcs) syms.push_back({f->name, "function", signatureOf(f.get()), f->loc, {}});
        for (auto& g : M->ast.globals) syms.push_back({g->name, g->isConst ? "constant" : "variable", typeName(g->type, g->typeRef), g->loc, {}});
        std::function<std::string(const Sym&)> one = [&](const Sym& s) {
            std::string r = "{\"name\":" + json(s.name) + ",\"kind\":" + json(s.kind) + ",\"detail\":" + json(s.detail) +
                            ",\"line\":" + std::to_string(s.loc.line) + ",\"col\":" + std::to_string(s.loc.col) + ",\"children\":[";
            for (size_t i = 0; i < s.children.size(); i++) r += (i ? "," : "") + one(s.children[i]);
            return r + "]}";
        };
        std::string out = "{\"symbols\":[";
        for (size_t i = 0; i < syms.size(); i++) out += (i ? "," : "") + one(syms[i]);
        std::printf("%s]}\n", out.c_str());
        return 0;
    }

    const Ref* r = q.at(M->src.get(), line, col);
    if (!r) {
        std::printf("{}\n");
        return 0;
    }
    const Target& t = r->target;

    if (what == "definition") {
        Loc l = declLoc(t);
        if (!l.file || l.file->fullPath.empty()) {
            std::printf("{}\n");  // something built in: there's no file to go to
            return 0;
        }
        std::printf("{\"location\":%s}\n", locJson(l, (int)targetName(t).size()).c_str());
        return 0;
    }
    if (what == "hover") {
        bool builtIn = false;
        if (t.kind == TK2::Func && t.func->module && t.func->module->isPrelude) builtIn = true;
        if (t.kind == TK2::Type && t.type->builtin) builtIn = true;
        if (t.kind == TK2::Var && t.var->module && t.var->module->isPrelude) builtIn = true;
        std::printf("{\"text\":%s,\"builtin\":%s}\n", json(hoverText(t)).c_str(), builtIn ? "true" : "false");
        return 0;
    }
    if (what == "references" || what == "rename") {
        std::vector<const Ref*> same;
        for (const Ref& x : q.refs)
            if (x.target == t && x.file && !x.file->fullPath.empty()) {
                bool dup = false;
                for (const Ref* y : same)
                    if (y->file == x.file && y->line == x.line && y->col == x.col) dup = true;
                if (!dup) same.push_back(&x);
            }
        if (what == "rename") {
            std::string why;
            Loc d = declLoc(t);
            if (t.kind == TK2::Module || t.kind == TK2::Folder)
                why = "a file or folder's name comes from the file itself: rename it in VS Code's file explorer";
            else if (!d.file || d.file->fullPath.empty())
                why = "'" + targetName(t) + "' is built into Forge, so it can't be renamed";
            else if (t.kind == TK2::Func && t.func->overrides)
                why = "this method replaces one from the class it inherits from, so rename that one instead";
            else if (!validName(newName))
                why = "'" + newName + "' can't be a name: names use letters, numbers and _, and can't start with a number";
            else if (isKeyword(newName) || isBuiltinTypeName(newName))
                why = "'" + newName + "' is a Forge keyword or type name";
            if (!why.empty()) {
                std::printf("{\"error\":%s}\n", json(why).c_str());
                return 0;
            }
        }
        std::string out = what == "rename" ? "{\"edits\":[" : "{\"locations\":[";
        for (size_t i = 0; i < same.size(); i++) {
            const Ref* x = same[i];
            out += (i ? "," : "") + locJson(Loc{x->file, x->line, x->col, x->len}, x->len);
        }
        std::printf("%s]}\n", out.c_str());
        return 0;
    }
    std::printf("{\"error\":%s}\n", json("unknown question: " + what).c_str());
    return 0;
}
