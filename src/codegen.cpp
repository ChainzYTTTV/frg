#include "codegen.hpp"

#include "builtins.hpp"
#include "checker_util.hpp"
#include "prelude.hpp"
#include "types.hpp"

#include <algorithm>
#include <cstdio>
#include <functional>
#include <limits>
#include <set>

namespace {

std::string cppString(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\%03o", c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
    return out + "\"";
}

bool isNarrowInt(const Type* t) { return t && t->kind == TK::Int && t->bits < 32; }

std::string u128ToString(unsigned __int128 v) {
    std::string s;
    do {
        s.insert(s.begin(), char('0' + int(v % 10)));
        v /= 10;
    } while (v);
    return s;
}

// C++ has no 128-bit literals, so big ones are built from two 64-bit halves.
std::string intLiteral(unsigned __int128 v, const Type* T) {
    const auto u64max = std::numeric_limits<uint64_t>::max();
    if (T->bits == 128) {
        if (v <= u64max) return "static_cast<" + T->cpp + ">(" + u128ToString(v) + "ULL)";
        return "static_cast<" + T->cpp + ">((static_cast<unsigned __int128>(" + u128ToString(v >> 64) + "ULL) << 64) | " +
               u128ToString(v & u64max) + "ULL)";
    }
    std::string digits = u128ToString(v);
    if (v > (unsigned __int128)std::numeric_limits<int64_t>::max()) return digits + "ULL";
    if (v > (unsigned __int128)std::numeric_limits<int32_t>::max()) return digits + "LL";
    return digits;
}

std::string where(const Loc& l) {
    return cppString((l.file ? l.file->displayPath : std::string("?")) + ":" + std::to_string(l.line));
}

const char* bmethodPrefix(TK kind) {
    return kind == TK::List ? "::frg::list_" : kind == TK::Map ? "::frg::map_" : kind == TK::Task ? "::frg::task_" : kind == TK::Weak ? "::frg::weak_" : "::frg::str_";
}

Type* derefType(Type* t) {
    if (t->kind == TK::Optional) t = t->elem;
    return t;
}

class Gen {
public:
    Gen(Project& p, bool debug, bool tests) : P(p), debug(debug), tests(tests) {}
    GeneratedCpp run();

private:
    Project& P;
    std::string out;
    int ind = 0;
    FuncDecl* fn = nullptr;
    TypeDecl* curType = nullptr;
    int tmp = 0;
    bool globalScope = false;  // writing a global's or static field's starting value, outside any function
    bool debug = false;        // a debug build: each statement says which .frg line it came from (#line)
    bool framed = false;       // the code being written has a ::frg::Frame frg$frame (for runtime errors' "called from")
    bool tests = false;        // frg test: main() runs the tests instead of the game

    // Runtime errors list the calls that led to them, so each function keeps a Frame saying where it's up to.
    void frameStart(const std::string& name, const Loc& l) {
        if (!framed) return;
        line("::frg::Frame frg$frame(" + cppString(name) + ", " + cppString(l.file ? l.file->displayPath : "?") + ", " +
             std::to_string(l.line) + ");");
    }
    // A condition that's worked out again and again (while, else if): it also moves the Frame to its line.
    std::string atLine(const Loc& l, const std::string& cond) {
        if (!framed) return cond;
        return "(frg$frame.line = " + std::to_string(l.line) + ", " + cond + ")";
    }

    // #line tells the C++ compiler (and so the debugger) that the code after it comes from this line of a .frg file.
    void sourceLine(const Loc& l) {
        if (!debug || !l.file || l.file->fullPath.empty()) return;
        std::string path;
        for (char c : l.file->fullPath) path += c == '\\' ? std::string("\\\\") : c == '"' ? std::string("\\\"") : std::string(1, c);
        out += "#line " + std::to_string(l.line) + " \"" + path + "\"\n";
    }
    // After a function, Forge's own helper code isn't from any line of a .frg file.
    void generatedLines() {
        if (debug) out += "#line 1 \"<code made by Forge>\"\n";
    }

    void line(const std::string& s) {
        out.append(ind * 4, ' ');
        out += s;
        out += '\n';
    }
    static std::string id(const std::string& n) { return n + "_"; }
    static std::string qual(const Module* m, const std::string& n) { return "::" + m->cppNs + "::" + id(n); }

    // Declarations
    std::string params(const FuncDecl* f, bool withNames = true);
    std::string fieldParams(TypeDecl* d);
    bool hasAutoInit(TypeDecl* d);
    std::vector<TypeDecl*> typeOrder();
    void emitTypeDef(TypeDecl* d);
    void emitTypeFuncs(TypeDecl* d);
    std::string strFuncProto(TypeDecl* d);
    std::string tmplHead(const TypeDecl* d);
    std::string targs(const TypeDecl* d);
    std::string classOf(const Type* t);

    // Expressions
    std::string ex(const Expr* e);
    std::string conv(const Expr* e, const Type* T);
    std::string globalValue(const VarDecl* v);
    std::string unwrap(const std::string& code, const Type* opt, const Loc& l, const std::string& what = "");
    std::string objRef(const Expr* o, std::string& sep);
    std::string objValue(const Expr* o);
    std::string strCode(const std::string& code, const Type* t, bool shallow = false);
    std::string ref(const Expr* e);
    std::vector<std::string> argList(const Expr* e, const std::vector<ExprPtr>& given, const FuncDecl* f);
    bool effects(const Expr* e);
    bool awaits(const Expr* e);
    std::string ordered(const std::vector<const Expr*>& exprs, const std::vector<std::string>& codes,
                        const std::function<std::string(const std::vector<std::string>&)>& build);
    std::string call(const Expr* e);
    std::string testingCall(const Expr* e, const FuncDecl* f);
    std::string construct(const Expr* e, const std::vector<ExprPtr>& given);
    std::string binary(const Expr* e);

    // Statements
    void stmt(const Stmt* s);
    void children(const Stmt* block);
    std::string simple(const Stmt* s);
    void emitBody(const FuncDecl* f, TypeDecl* owner, const std::string& head);
};

// ---------------------------------------------------------------- expressions

std::string Gen::unwrap(const std::string& code, const Type* opt, const Loc& l, const std::string& what) {
    std::string desc = what.empty() ? "this value" : "'" + what + "'";
    return std::string(isNullableRef(opt->elem) ? "::frg::nn(" : "::frg::unwrap(") + code + ", " + where(l) + ", " + cppString(desc) + ")";
}

// The starting value of a global variable or a static field. It's worked out before main() starts, so anything that
// could stop with a runtime error (all but plain values) goes through initGlobal, which reports it like one in main.
std::string Gen::globalValue(const VarDecl* v) {
    const Expr* e = v->init.get();
    if (e->kind == EK::Int || e->kind == EK::Float || e->kind == EK::Bool || (e->kind == EK::Str && e->args.empty()))
        return conv(e, v->type);
    return "::frg::initGlobal([]() -> " + v->type->cpp + " { return " + conv(e, v->type) + "; })";
}

std::string Gen::conv(const Expr* e, const Type* T) {
    const Type* S = e->type;
    if (!T || !S || S == T) return ex(e);
    if (S->kind == TK::Null) {
        if (T->kind == TK::Optional && !isNullableRef(T->elem)) return T->cpp + "()";
        return "static_cast<" + T->cpp + ">(nullptr)";
    }
    if (T->kind == TK::Optional) {
        if (S->kind == TK::Optional) return "static_cast<" + T->cpp + ">(" + ex(e) + ")";
        std::string inner = conv(e, T->elem);
        return isNullableRef(T->elem) ? inner : "static_cast<" + T->cpp + ">(" + inner + ")";
    }
    if (S->kind == TK::Optional) {
        std::string v = unwrap(ex(e), S, e->loc, e->kind == EK::Name || e->kind == EK::Member ? e->text : "");
        return S->elem == T ? v : "static_cast<" + T->cpp + ">(" + v + ")";
    }
    if (T->kind == TK::String && S->kind == TK::Struct && S->decl && S->decl->builtin && S->decl->name == "Description")
        return "::frg::core::frg$str(" + ex(e) + ")";
    if (S->kind == TK::Class && T->kind == TK::Class) return "static_cast<" + T->cpp + ">(" + ex(e) + ")";
    if (isNumeric(S) && isNumeric(T)) return "static_cast<" + T->cpp + ">(" + ex(e) + ")";
    return ex(e);
}

// The object part of `obj.field` / `obj.method()`, after following pointers and checking optionals.
std::string Gen::objRef(const Expr* o, std::string& sep) {
    if (o->kind == EK::This || o->kind == EK::Super) {
        sep = "->";
        return "this";
    }
    std::string code = ex(o);
    const Type* t = o->type;
    if (t->kind == TK::Optional) {
        code = unwrap(code, t, o->loc, o->kind == EK::Name || o->kind == EK::Member ? o->text : "");
        t = t->elem;
    }
    sep = t->kind == TK::Class ? "->" : ".";
    return "(" + code + ")";
}

// A list/map/text value, after following pointers and checking optionals.
std::string Gen::objValue(const Expr* o) {
    std::string sep;
    std::string r = objRef(o, sep);
    return r == "this" ? "(*this)" : r;
}

std::string Gen::strCode(const std::string& code, const Type* t, bool shallow) {
    switch (t->kind) {
        case TK::Int:
        case TK::Float:
        case TK::Bool:
        case TK::Task:
        case TK::String: return "::frg::str(" + code + ")";
        case TK::Null: return "::std::string(\"null\")";
        case TK::TypeParam: return "::frg::strAny(" + code + ")";  // T inside a generic type: decided per type it's made with
        case TK::Enum:
        case TK::Struct:
        case TK::Class: {
            TypeDecl* d = t->decl;
            bool hasToString = d->kind != DK::Enum && (findMethod(d, "toText") || findMethod(d, "toString"));
            if (t->kind == TK::Class && shallow && !hasToString) return "::std::string(" + cppString("<" + d->name + ">") + ")";
            return "::" + d->module->cppNs + "::frg$str(" + code + ")";
        }
        case TK::List:
            return "::frg::strList(" + code + ", [](const auto& x) { return " + strCode("x", t->elem, shallow) + "; })";
        case TK::Map:
            return "::frg::strMap(" + code + ", [](const auto& k) { return " + strCode("k", t->key, shallow) +
                   "; }, [](const auto& v) { return " + strCode("v", t->elem, shallow) + "; })";
        case TK::Optional:
            return "::frg::strOpt(" + code + ", [](const auto& x) { return " + strCode("x", t->elem, shallow) + "; })";
        case TK::Weak: return "::frg::strWeak(" + code + ", [](const auto& x) { return " + strCode("x", t->elem, shallow) + "; })";
        default: return "::std::string(\"?\")";
    }
}

std::string Gen::ref(const Expr* e) {
    switch (e->res.kind) {
        case RK::Local: return id(e->res.var->name);
        case RK::Global:
            // A built-in library's constant (OpenGL.TRIANGLES): its value is written straight in.
            if (e->res.var->module->isPrelude && e->res.var->isConst && e->res.var->init) return conv(e->res.var->init.get(), e->res.var->type);
            return qual(e->res.var->module, e->res.var->name);
        case RK::Func: {
            const FuncDecl* f = e->res.func;
            std::string q = f->owner ? f->owner->cpp + "::" + id(f->name) : qual(f->module, f->name);  // static methods have an owner
            if (e->type && e->type->kind == TK::Function && f->module->overloads.count(f->name) && f->module->overloads.at(f->name).size() > 1) {
                // A function value of one version of an overloaded function: say which one.
                std::string ps;
                for (size_t i = 0; i < f->params.size(); i++) ps += (i ? ", " : "") + f->params[i]->type->cpp;
                return "static_cast<" + f->ret->cpp + "(*)(" + ps + ")>(&" + q + ")";
            }
            return q;
        }
        case RK::Builtin: return e->res.builtin->cpp;
        case RK::StdConst: return "static_cast<" + e->type->cpp + ">(" + e->res.constant->cpp + ")";
        case RK::EnumValue: return e->res.tdecl->cpp + "::" + id(e->res.tdecl->enumValues[e->res.index]);
        case RK::TypeStatic: return e->res.tdecl->cpp + "::" + id(e->res.var->name);
        case RK::Field:
        case RK::Method:
            if (e->kind == EK::Name) return "this->" + id(e->text);
            {
                std::string sep;
                std::string obj = objRef(e->a.get(), sep);
                return obj + sep + id(e->text);
            }
        case RK::Describe: {
            std::string sep;
            std::string obj = objRef(e->a.get(), sep);
            return obj + sep + "frg$describe()";
        }
        case RK::BuiltinMethod:  // only `length` is used as a value
            return "static_cast<int32_t>(" + objValue(e->a.get()) + ".size())";
        default: return "/*?*/";
    }
}

// Each argument's code: the given values first, then the default values filled in for the rest.
std::vector<std::string> Gen::argList(const Expr* e, const std::vector<ExprPtr>& given, const FuncDecl* f) {
    std::vector<std::string> list;
    // With several versions of a function, every value gets its exact type, so C++ picks the same version Forge did.
    bool overloaded = !f->owner && f->module && !f->module->isPrelude && f->module->overloads.count(f->name) &&
                      f->module->overloads.at(f->name).size() > 1;
    for (size_t i = 0; i < given.size(); i++) {
        const Type* pt = e->paramTypes.empty() ? f->params[i]->type : e->paramTypes[i];  // with a Box<int>'s T filled in
        std::string v = conv(given[i].get(), pt);
        list.push_back(overloaded ? "static_cast<" + pt->cpp + ">(" + v + ")" : v);
    }
    for (size_t i = 0; i < e->defaultArgs.size(); i++) {
        size_t pi = given.size() + i;
        list.push_back(conv(e->defaultArgs[i], e->paramTypes.empty() ? f->params[pi]->type : e->paramTypes[pi]));
    }
    return list;
}

std::string joinArgs(const std::vector<std::string>& codes, size_t from = 0) {
    std::string s;
    for (size_t i = from; i < codes.size(); i++) s += (i > from ? ", " : "") + codes[i];
    return s;
}

// Whether working out this value could do something (call a function), so the order it happens in matters.
bool Gen::effects(const Expr* e) {
    if (!e || e->kind == EK::Lambda) return false;  // making a function value doesn't run it
    if (e->kind == EK::Call || e->opMethod) return true;
    for (auto& x : e->args)
        if (effects(x.get())) return true;
    for (auto& x : e->keys)
        if (effects(x.get())) return true;
    return effects(e->a.get()) || effects(e->b.get()) || effects(e->c.get());
}

// Whether this value contains a wait(), which is a C++ co_await: it can't be moved into a helper function.
bool Gen::awaits(const Expr* e) {
    if (!e || e->kind == EK::Lambda) return false;
    if (e->kind == EK::Call && e->a && e->a->res.kind == RK::Builtin && e->a->res.builtin->sig == BSig::Wait) return true;
    for (auto& x : e->args)
        if (awaits(x.get())) return true;
    for (auto& x : e->keys)
        if (awaits(x.get())) return true;
    return awaits(e->a.get()) || awaits(e->b.get()) || awaits(e->c.get());
}

// Forge works out the parts of a call or an operator left to right, but C++ doesn't promise any order (g++ often
// goes right to left). When two or more parts do something, they're worked out one by one into temporaries first.
// `exprs` are the parts in Forge's order, `codes` their C++ (it can have extra codes at the end, like default
// values, which are left where they are), and `build` puts the call or operator together from the codes.
std::string Gen::ordered(const std::vector<const Expr*>& exprs, const std::vector<std::string>& codes,
                         const std::function<std::string(const std::vector<std::string>&)>& build) {
    int doing = 0;
    for (const Expr* x : exprs) {
        if (awaits(x)) return build(codes);
        if (effects(x)) doing++;
    }
    if (doing < 2) return build(codes);
    // Outside any function (a global's starting value), a lambda can't capture anything, and doesn't need to.
    std::string s = std::string(globalScope ? "[]" : "[&]") + "() -> auto { ";
    std::vector<std::string> names = codes;
    for (size_t i = 0; i < exprs.size(); i++) {
        std::string t = "frg$t" + std::to_string(tmp++);
        s += "auto&& " + t + " = " + codes[i] + "; ";
        names[i] = "static_cast<decltype(" + t + ")&&>(" + t + ")";
    }
    return s + "return " + build(names) + "; }()";
}

std::string Gen::construct(const Expr* e, const std::vector<ExprPtr>& given) {
    TypeDecl* d = e->builds;
    if (d->kind == DK::Enum) return "static_cast<" + d->cpp + ">(" + ex(given[0].get()) + ")";
    std::vector<std::string> codes;
    if (e->ctor) {
        codes = argList(e, given, e->ctor);
    } else {
        std::vector<VarDecl*> fields = allFields(d);
        for (size_t i = 0; i < given.size(); i++) codes.push_back(conv(given[i].get(), e->paramTypes.empty() ? fields[i]->type : e->paramTypes[i]));
    }
    std::vector<const Expr*> exprs;
    for (auto& g : given) exprs.push_back(g.get());
    std::string head = classOf(e->type) + (d->kind == DK::Class ? "::frg$new(" : "::frg$make(");
    return ordered(exprs, codes, [&](const std::vector<std::string>& c) { return head + joinArgs(c) + ")"; });
}

// testing.check(ok) and testing.fail(): they get their place and the line as written, and a comparison like
// testing.check(h.hp == 15) also shows both of its values when it fails.
std::string Gen::testingCall(const Expr* e, const FuncDecl* f) {
    std::vector<std::string> codes = argList(e, e->args, f);
    std::string text = e->loc.file ? e->loc.file->getLine(e->loc.line) : "";
    text.erase(0, text.find_first_not_of(" \t"));
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();
    if (f->name == "fail") return "::frg::testing::failAt(" + where(e->loc) + ", " + codes[0] + ")";
    const Expr* cond = e->args[0].get();
    static const std::set<std::string> comparisons = {"==", "!=", "<", ">", "<=", ">="};
    const Type* T = cond->kind == EK::Binary ? cond->opType : nullptr;
    bool simple = T && (T->kind == TK::Int || T->kind == TK::Float || T->kind == TK::String || T->kind == TK::Bool || T->kind == TK::Enum);
    if (cond->kind == EK::Binary && comparisons.count(cond->text) && !cond->opMethod && simple && cond->a->type->kind != TK::Null &&
        cond->b->type->kind != TK::Null && !awaits(cond)) {
        std::string ok = "(frg$l " + cond->text + " frg$r)";
        // Text values are shown in quotes, so spaces and empty text can be seen.
        std::string q = T->kind == TK::String ? "\"\\\"\" + " : "";
        std::string qEnd = T->kind == TK::String ? " + \"\\\"\"" : "";
        std::string values = "::std::string(\"left side: \") + " + q + strCode("frg$l", T) + qEnd + " + \", right side: \" + " + q + strCode("frg$r", T) + qEnd;
        return std::string(globalScope ? "[]" : "[&]") + "() { auto&& frg$l = " + conv(cond->a.get(), T) + "; auto&& frg$r = " +
               conv(cond->b.get(), T) + "; ::frg::testing::checkAt(" + ok + ", " + where(e->loc) + ", " + cppString(text) + ", " +
               codes[1] + ", " + values + "); }()";
    }
    return "::frg::testing::checkAt(" + codes[0] + ", " + where(e->loc) + ", " + cppString(text) + ", " + codes[1] + ", \"\")";
}

std::string Gen::call(const Expr* e) {
    const Expr* callee = e->a.get();
    std::vector<const Expr*> argExprs;
    for (auto& x : e->args) argExprs.push_back(x.get());
    switch (callee->res.kind) {
        case RK::Func: {
            const FuncDecl* f = callee->res.func;
            if (f->module->isPrelude && f->module->name == "window" && f->name == "run") {
                return "::frg::window::run_([](float dt) { " + qual(P.mainModule, "update") + "(dt); }, []() { " +
                       qual(P.mainModule, "draw") + "(); })";
            }
            if (f->module->isPrelude && f->module->name == "testing") return testingCall(e, f);
            std::string name = ref(callee);
            return ordered(argExprs, argList(e, e->args, f), [&](const std::vector<std::string>& c) { return name + "(" + joinArgs(c) + ")"; });
        }
        case RK::Method: {
            const FuncDecl* f = callee->res.func;
            if (callee->kind == EK::Member && callee->a->kind == EK::Super) {
                std::string name = f->owner->cpp + "::" + id(f->name);
                return ordered(argExprs, argList(e, e->args, f), [&](const std::vector<std::string>& c) { return name + "(" + joinArgs(c) + ")"; });
            }
            if (callee->kind == EK::Member) {
                // obj.method(args): the object is worked out first, then the arguments.
                std::string sep;
                std::string obj = objRef(callee->a.get(), sep);
                std::vector<const Expr*> exprs = {callee->a.get()};
                exprs.insert(exprs.end(), argExprs.begin(), argExprs.end());
                std::vector<std::string> codes = {obj};
                for (auto& c : argList(e, e->args, f)) codes.push_back(c);
                return ordered(exprs, codes, [&](const std::vector<std::string>& c) { return c[0] + sep + id(callee->text) + "(" + joinArgs(c, 1) + ")"; });
            }
            std::string name = ref(callee);
            return ordered(argExprs, argList(e, e->args, f), [&](const std::vector<std::string>& c) { return name + "(" + joinArgs(c) + ")"; });
        }
        case RK::Type: return construct(e, e->args);
        case RK::None:  // fs[0](), makeHandler()(): the checker allows this only for function values
        case RK::TypeStatic:  // Waves.boss(), a static Function field
        case RK::Local:
        case RK::Global:
        case RK::Field: {  // calling a function value
            const Type* ft = callee->type->kind == TK::Optional ? callee->type->elem : callee->type;
            std::string head = "::frg::fn(" + conv(callee, ft) + ", " + where(e->loc) + ")(";
            std::vector<std::string> codes;
            for (size_t i = 0; i < e->args.size(); i++) codes.push_back(conv(e->args[i].get(), ft->params[i]));
            return ordered(argExprs, codes, [&](const std::vector<std::string>& c) { return head + joinArgs(c) + ")"; });
        }
        case RK::BuiltinMethod: {
            Type* obj = derefType(callee->a->type);
            if (callee->text == "sort" && e->opMethod) {  // a list of your own type, sorted with its less()
                std::string less = obj->elem->kind == TK::Class ? "x->" + id(e->opMethod->name) + "(y)"
                                                                : "const_cast<" + obj->elem->cpp + "&>(x)." + id(e->opMethod->name) + "(y)";
                return "::frg::list_sortWith(" + objValue(callee->a.get()) + ", [](const auto& x, const auto& y) { return " + less + "; }, " +
                       where(e->loc) + ")";
            }
            std::vector<const Expr*> exprs = {callee->a.get()};
            std::vector<std::string> codes = {objValue(callee->a.get())};
            for (auto& a : e->args) {
                // The element/key type the method expects, so literals are converted correctly.
                const Type* want = a->type;
                bool takesFunction = callee->text == "removeWhere" || callee->text == "filter" || callee->text == "any" || callee->text == "sortBy";
                if (obj->kind == TK::List && callee->text != "insert" && callee->text != "removeAt" && callee->text != "addAll" && !takesFunction)
                    want = obj->elem;
                if (obj->kind == TK::List && callee->text == "insert" && &a == &e->args[1]) want = obj->elem;
                if (obj->kind == TK::Map) want = &a == &e->args[0] ? obj->key : obj->elem;
                exprs.push_back(a.get());
                codes.push_back(conv(a.get(), want));
            }
            std::string head = bmethodPrefix(obj->kind) + callee->text + "(";
            std::string tail = ", " + where(e->loc) + ")";
            return ordered(exprs, codes, [&](const std::vector<std::string>& c) { return head + joinArgs(c) + tail; });
        }
        case RK::Builtin: {
            const Builtin* b = callee->res.builtin;
            if (b->sig == BSig::Print || b->sig == BSig::Write) {
                if (e->args.empty()) return std::string(b->cpp) + "()";
                return std::string(b->cpp) + "(" + strCode(ex(e->args[0].get()), e->args[0]->type) + ")";
            }
            if (b->sig == BSig::Weak) return e->type->cpp + "(" + ex(e->args[0].get()) + ")";
            if (b->sig == BSig::Input) return std::string(b->cpp) + "(" + (e->args.empty() ? "" : ex(e->args[0].get())) + ")";
            if (b->sig == BSig::Wait) {
                if (e->args.empty()) return "co_await ::frg::tasks::Wait{0.0}";
                if (e->args[0]->type == tTask()) return "co_await ::frg::tasks::WaitFor{" + ex(e->args[0].get()) + "}";
                return "co_await ::frg::tasks::Wait{" + conv(e->args[0].get(), tFloat(64)) + "}";
            }
            std::vector<std::string> codes;
            for (size_t i = 0; i < e->args.size(); i++) codes.push_back(conv(e->args[i].get(), e->type));
            std::string name = b->cpp;
            if (b->sig == BSig::Generic1 || b->sig == BSig::Generic2 || b->sig == BSig::Generic3 || b->sig == BSig::Float3)
                name += "<" + e->type->cpp + ">";
            return ordered(argExprs, codes, [&](const std::vector<std::string>& c) { return name + "(" + joinArgs(c) + ")"; });
        }
        default: return "/*?*/";
    }
}

std::string Gen::binary(const Expr* e) {
    const std::string& op = e->text;
    const Expr* a = e->a.get();
    const Expr* b = e->b.get();
    const Type* T = e->opType;

    if (e->opMethod) {
        // Your own type's operator method: a + b is a.plus(b), a > b is b.less(a), a != b is !a.equals(b).
        // a is still worked out before b, even when the method is called on b.
        const Expr* obj = e->opSwap ? b : a;
        const Expr* arg = e->opSwap ? a : b;
        std::string objCode = ex(obj);
        std::string argCode = conv(arg, e->paramTypes.empty() ? e->opMethod->params[0]->type : e->paramTypes[0]);
        std::string sep = obj->type->kind == TK::Class ? "->" : ".";
        std::string method = id(e->opMethod->name);
        std::vector<std::string> codes = e->opSwap ? std::vector<std::string>{argCode, objCode} : std::vector<std::string>{objCode, argCode};
        std::string call = ordered({a, b}, codes, [&](const std::vector<std::string>& c) {
            const std::string& o = e->opSwap ? c[1] : c[0];
            const std::string& v = e->opSwap ? c[0] : c[1];
            return "(" + o + ")" + sep + method + "(" + v + ")";
        });
        return e->opNot ? "(!" + call + ")" : call;
    }
    if (op == "&&" || op == "||") return "(" + ex(a) + " " + op + " " + ex(b) + ")";
    if (op == "??") {
        bool valueOpt = e->type->kind == TK::Optional && !isNullableRef(e->type->elem);
        return std::string(valueOpt ? "::frg::orElseOpt(" : "::frg::orElse(") + ex(a) + ", " + conv(b, e->type) + ")";
    }
    if (op == "==" || op == "!=") {
        if (T->kind == TK::Null) return op == "==" ? "true" : "false";
        if (a->type->kind == TK::Null || b->type->kind == TK::Null) {
            const Expr* other = a->type->kind == TK::Null ? b : a;
            return std::string(op == "==" ? "" : "!") + "::frg::isNull(" + ex(other) + ")";
        }
        return ordered({a, b}, {conv(a, T), conv(b, T)}, [&](const std::vector<std::string>& c) { return "(" + c[0] + " " + op + " " + c[1] + ")"; });
    }
    if (op == "<<" || op == ">>") {
        std::string r = ordered({a, b}, {ex(a), ex(b)}, [&](const std::vector<std::string>& c) { return "(" + c[0] + " " + op + " " + c[1] + ")"; });
        return isNarrowInt(e->type) ? "static_cast<" + e->type->cpp + ">(" + r + ")" : r;
    }
    if (T->kind == TK::Struct) {  // vector math
        auto side = [&](const Expr* x) { return isNumeric(x->type) ? conv(x, tFloat(32)) : ex(x); };
        return ordered({a, b}, {side(a), side(b)}, [&](const std::vector<std::string>& c) { return "(" + c[0] + " " + op + " " + c[1] + ")"; });
    }
    std::string r = ordered({a, b}, {conv(a, T), conv(b, T)}, [&](const std::vector<std::string>& c) {
        const std::string &A = c[0], &B = c[1];
        if (T->kind == TK::Int && (op == "/" || op == "%"))
            return std::string(op == "/" ? "::frg::idiv" : "::frg::imod") + "<" + T->cpp + ">(" + A + ", " + B + ", " + where(e->loc) + ")";
        if (T->kind == TK::Float && op == "%") return "::std::fmod(" + A + ", " + B + ")";
        return "(" + A + " " + op + " " + B + ")";
    });
    // int8/int16 math happens in C++'s int, so wrap the answer back into the smaller type.
    if (isNarrowInt(e->type)) r = "static_cast<" + e->type->cpp + ">(" + r + ")";
    return r;
}

std::string Gen::ex(const Expr* e) {
    switch (e->kind) {
        case EK::Int:
            if (e->type->kind == TK::Float) return u128ToString(e->ival) + ".0" + (e->type->bits == 32 ? "f" : "");
            return intLiteral(e->ival, e->type);
        case EK::Float: return e->text + (e->type->bits == 32 ? "f" : "");
        case EK::Bool: return e->boolVal ? "true" : "false";
        case EK::Null: return "nullptr";
        case EK::Str: {
            if (e->args.empty()) return "::std::string(" + cppString(e->parts[0]) + ")";
            // The pieces go in a { } list, which C++ always works out left to right ("a + b" has no fixed order).
            std::string s = "::frg::cat({::std::string(" + cppString(e->parts[0]) + ")";
            for (size_t i = 0; i < e->args.size(); i++) {
                if (i < e->decimals.size() && e->decimals[i] >= 0)
                    s += ", ::frg::fixed(static_cast<double>(" + ex(e->args[i].get()) + "), " + std::to_string(e->decimals[i]) + ")";
                else
                    s += ", " + strCode(ex(e->args[i].get()), e->args[i]->type);
                if (!e->parts[i + 1].empty()) s += ", ::std::string(" + cppString(e->parts[i + 1]) + ")";
            }
            return s + "})";
        }
        case EK::Name:
        case EK::Member: return ref(e);
        case EK::Call: return call(e);
        case EK::Unary: {
            if (e->text == "!") return "(!" + ex(e->a.get()) + ")";
            if (e->type->kind == TK::Struct) return "(-" + ex(e->a.get()) + ")";
            std::string r = "(" + e->text + conv(e->a.get(), e->opType) + ")";
            return isNarrowInt(e->type) ? "static_cast<" + e->type->cpp + ">" + r : r;
        }
        case EK::Binary: return binary(e);
        case EK::Cast: {
            const Type* T = e->type;
            const Type* S = e->a->type;
            if (T == S) return ex(e->a.get());
            if (T->kind == TK::String) return strCode(ex(e->a.get()), S);
            if (S->kind == TK::String) return "::frg::parse<" + T->cpp + ">(" + ex(e->a.get()) + ", " + cppString(T->name) + ", " + where(e->loc) + ")";
            if (S->kind == TK::Optional) return "static_cast<" + T->cpp + ">(" + unwrap(ex(e->a.get()), S, e->loc) + ")";
            return "static_cast<" + T->cpp + ">(" + ex(e->a.get()) + ")";
        }
        case EK::Index: {
            const Type* t = derefType(e->a->type);
            std::string obj = objValue(e->a.get());
            if (t->kind == TK::List)
                return ordered({e->a.get(), e->b.get()}, {obj, "static_cast<int64_t>(" + ex(e->b.get()) + ")"},
                               [&](const std::vector<std::string>& c) { return "::frg::at(" + c[0] + ", " + c[1] + ", " + where(e->loc) + ")"; });
            if (e->isWrite) return obj + "[" + conv(e->b.get(), t->key) + "]";
            return "::frg::mapGet(" + obj + ", " + conv(e->b.get(), t->key) + ", " + where(e->loc) + ")";
        }
        case EK::ListLit: {
            std::string s = e->type->cpp + "{";
            for (size_t i = 0; i < e->args.size(); i++) s += (i ? ", " : "") + conv(e->args[i].get(), e->type->elem);
            return s + "}";
        }
        case EK::MapLit: {
            std::string s = e->type->cpp + "{";
            for (size_t i = 0; i < e->keys.size(); i++)
                s += std::string(i ? ", " : "") + "{" + conv(e->keys[i].get(), e->type->key) + ", " + conv(e->args[i].get(), e->type->elem) + "}";
            return s + "}";
        }
        case EK::BraceLit:
            if (e->builds) return construct(e, e->args);
            return e->type->cpp + "{}";
        case EK::Ternary: return "(" + ex(e->a.get()) + " ? " + conv(e->b.get(), e->type) + " : " + conv(e->c.get(), e->type) + ")";
        case EK::This:
            if (curType->kind == DK::Struct) return "(*this)";
            return "::std::static_pointer_cast<" + curType->cpp + targs(curType) + ">(this->shared_from_this())";
        case EK::Super: return "this";
        case EK::Is: {
            std::string cast = "::std::dynamic_pointer_cast<" + classOf(e->opType) + ">(" + ex(e->a.get()) + ")";
            return "(" + cast + " != nullptr)";
        }
        case EK::As: return "::std::dynamic_pointer_cast<" + classOf(e->opType) + ">(" + ex(e->a.get()) + ")";
        case EK::Lambda: {
            // A C++ lambda that gets copies of the variables around it (and `this`, inside a method).
            FuncDecl* f = e->lambda.get();
            // A global or static field's starting value is outside every function, where C++ allows no captures.
            std::string capture = globalScope ? "[]" : curType ? "[=, this]" : "[=]";
            std::string head = capture + "(" + params(f) + ") mutable -> " + f->ret->cpp + " {\n";
            std::string savedOut = std::move(out);
            FuncDecl* savedFn = fn;
            bool savedGlobal = globalScope;
            globalScope = false;  // a function value inside this one can use this one's parameters
            bool savedFramed = framed;
            out.clear();
            ind++;
            fn = f;
            framed = true;
            frameStart("a function value" + std::string(savedFn && !savedFn->name.empty() ? " in " + savedFn->name : ""), e->loc);
            children(f->body.get());
            framed = savedFramed;
            fn = savedFn;
            globalScope = savedGlobal;
            ind--;
            std::string body = std::move(out);
            out = std::move(savedOut);
            return head + body + std::string(ind * 4, ' ') + "}";
        }
    }
    return "/*?*/";
}

// ---------------------------------------------------------------- statements

std::string Gen::simple(const Stmt* s) {
    switch (s->kind) {
        case SK::Var: {
            const VarDecl* v = s->var.get();
            std::string decl = v->type->cpp + " " + id(v->name);
            return v->init ? decl + " = " + conv(v->init.get(), v->type) : decl + "{}";
        }
        case SK::Assign: {
            const Type* T = s->expr->type;
            std::string target = ex(s->expr.get());
            if (s->op == "=") return target + " = " + conv(s->value.get(), T);
            if (s->target) {  // score += bonus on your own type: score = score.plus(bonus)
                Type* pt = s->target->params[0]->type;
                if (isGeneric(T)) pt = substType(pt, T->decl->paramTypes, T->params);
                return target + " = (" + target + ")" + (T->kind == TK::Class ? "->" : ".") + id(s->target->name) + "(" + conv(s->value.get(), pt) + ")";
            }
            char base = s->op[0];
            if (T->kind == TK::Struct) {  // vector math
                const Expr* v = s->value.get();
                return target + " " + s->op + " " + (isNumeric(v->type) ? conv(v, tFloat(32)) : ex(v));
            }
            std::string val = conv(s->value.get(), T);
            if (T->kind == TK::Int && (base == '/' || base == '%'))
                return target + " = ::frg::" + (base == '/' ? "idiv" : "imod") + "<" + T->cpp + ">(" + target + ", " + val + ", " + where(s->loc) + ")";
            if (T->kind == TK::Float && base == '%') return target + " = ::std::fmod(" + target + ", " + val + ")";
            return target + " " + s->op + " " + val;
        }
        case SK::IncDec: return s->op + ex(s->expr.get());
        case SK::ExprStmt: return ex(s->expr.get());
        default: return "";
    }
}

void Gen::children(const Stmt* block) {
    ind++;
    for (auto& c : block->body) stmt(c.get());
    ind--;
}

void Gen::stmt(const Stmt* s) {
    if (s->kind != SK::Block) {
        sourceLine(s->loc);
        if (framed) line("frg$frame.line = " + std::to_string(s->loc.line) + ";");
    }
    switch (s->kind) {
        case SK::Block:
            line("{");
            children(s);
            line("}");
            break;
        case SK::Var:
        case SK::Assign:
        case SK::IncDec:
        case SK::ExprStmt: line(simple(s) + ";"); break;
        case SK::If: {
            line("if (" + ex(s->expr.get()) + ") {");
            children(s->then.get());
            const Stmt* e = s->els.get();
            while (e && e->kind == SK::If) {
                line("} else if (" + atLine(e->loc, ex(e->expr.get())) + ") {");
                children(e->then.get());
                e = e->els.get();
            }
            if (e) {
                line("} else {");
                children(e);
            }
            line("}");
            break;
        }
        case SK::While:
            line("while (" + atLine(s->loc, ex(s->expr.get())) + ") {");
            children(s->then.get());
            line("}");
            break;
        case SK::Try:
            line("try {");
            children(s->then.get());
            line("} catch (const ::frg::core::Error_& " + id(s->var->name) + ") {");
            children(s->els.get());
            line("}");
            break;
        case SK::Throw:
            if (s->expr->type == tString())
                line("::frg::raise(" + where(s->loc) + ", " + ex(s->expr.get()) + ");");
            else
                line("throw " + ex(s->expr.get()) + ";");  // throw e; inside a catch sends the same error on
            break;
        case SK::DoWhile:
            line("do {");
            children(s->then.get());
            line("} while (" + ex(s->expr.get()) + ");");
            break;
        case SK::For:
            line("for (" + (s->init ? simple(s->init.get()) : "") + "; " + (s->expr ? ex(s->expr.get()) : "") + "; " +
                 (s->step ? simple(s->step.get()) : "") + ") {");
            children(s->then.get());
            line("}");
            break;
        case SK::ForEach: {
            if (s->value) {
                // for (int i in 0..10): both ends included. `done` is worked out before ++, so the end can be the type's biggest value.
                int n = ++tmp;
                std::string k = "frg$k" + std::to_string(n), to = "frg$to" + std::to_string(n), done = "frg$done" + std::to_string(n);
                const VarDecl* v = s->var.get();
                const std::string& T = v->type->cpp;
                line("{");
                ind++;
                line(T + " " + k + " = " + conv(s->expr.get(), v->type) + ";");
                line(T + " " + to + " = " + conv(s->value.get(), v->type) + ";");
                line("bool " + done + " = " + k + " > " + to + ";");
                line("for (; !" + done + "; " + done + " = " + k + " == " + to + ", ++" + k + ") {");
                ind++;
                line(T + " " + id(v->name) + " = " + k + ";");
                for (auto& c : s->then->body) stmt(c.get());
                ind--;
                line("}");
                ind--;
                line("}");
                break;
            }
            int n = ++tmp;
            std::string list = "frg$list" + std::to_string(n), i = "frg$i" + std::to_string(n);
            const Type* lt = s->expr->type;
            std::string src = lt->kind == TK::Optional ? unwrap(ex(s->expr.get()), lt, s->expr->loc) : ex(s->expr.get());
            const Type* elem = (lt->kind == TK::Optional ? lt->elem : lt)->elem;
            const VarDecl* v = s->var.get();
            line("{");
            ind++;
            line("auto&& " + list + " = " + src + ";");
            line("for (::std::size_t " + i + " = 0; " + i + " < " + list + ".size(); ++" + i + ") {");
            ind++;
            std::string item = list + "[" + i + "]";
            line(v->type->cpp + " " + id(v->name) + " = " + (v->type == elem ? item : "static_cast<" + v->type->cpp + ">(" + item + ")") + ";");
            for (auto& c : s->then->body) stmt(c.get());
            ind--;
            line("}");
            ind--;
            line("}");
            break;
        }
        case SK::Switch: {
            int n = ++tmp;
            std::string v = "frg$switch" + std::to_string(n);
            const Type* T = s->expr->type;
            line("{");
            ind++;
            line("auto " + v + " = " + ex(s->expr.get()) + ";");
            bool first = true;
            const SwitchCase* def = nullptr;
            for (auto& c : s->cases) {
                if (c.isDefault) {
                    def = &c;
                    continue;
                }
                std::string cond;
                for (size_t i = 0; i < c.values.size(); i++) cond += (i ? " || " : "") + v + " == " + conv(c.values[i].get(), T);
                line(std::string(first ? "if (" : "} else if (") + cond + ") {");
                children(c.body.get());
                first = false;
            }
            if (def) {
                line(first ? "{" : "} else {");
                children(def->body.get());
                line("}");
            } else if (!first) {
                line("}");
            }
            ind--;
            line("}");
            break;
        }
        case SK::Return:
            if (fn->ret == tTask())
                line("co_return;");
            else
                line(s->expr ? "return " + conv(s->expr.get(), fn->ret) + ";" : "return;");
            break;
        case SK::Break: line("break;"); break;
        case SK::Continue: line("continue;"); break;
        case SK::SuperCall: {
            TypeDecl* base = curType->base;
            std::string a;
            if (s->target) {
                for (size_t i = 0; i < s->args.size(); i++) a += (i ? ", " : "") + conv(s->args[i].get(), s->target->params[i]->type);
                for (size_t i = s->args.size(); i < s->target->params.size(); i++)
                    a += (i ? ", " : "") + conv(s->target->params[i]->init.get(), s->target->params[i]->type);
                line(base->cpp + "::frg$init(" + a + ");");
            } else if (!s->args.empty()) {
                std::vector<VarDecl*> fields = allFields(base);
                for (size_t i = 0; i < s->args.size(); i++) a += (i ? ", " : "") + conv(s->args[i].get(), fields[i]->type);
                line(base->cpp + "::frg$init(" + a + ");");
            }
            break;
        }
    }
}

// ---------------------------------------------------------------- declarations

std::string Gen::params(const FuncDecl* f, bool withNames) {
    std::string s;
    for (size_t i = 0; i < f->params.size(); i++) {
        if (i) s += ", ";
        s += f->params[i]->type->cpp;
        if (withNames) s += " " + id(f->params[i]->name);
    }
    return s;
}

std::string Gen::fieldParams(TypeDecl* d) {
    std::string s;
    std::vector<VarDecl*> fields = allFields(d);
    for (size_t i = 0; i < fields.size(); i++) s += (i ? ", " : "") + fields[i]->type->cpp + " " + id(fields[i]->name);
    return s;
}

// A type with no constructor anywhere gets one that sets every field, in order.
bool Gen::hasAutoInit(TypeDecl* d) { return findCtors(d).empty() && !allFields(d).empty(); }

// Structs are stored inside whatever holds them, so they must be defined before it. Base classes too.
std::vector<TypeDecl*> Gen::typeOrder() {
    std::vector<TypeDecl*> order;
    std::set<TypeDecl*> done, active;
    std::function<void(TypeDecl*)> visit;
    std::function<void(const Type*)> visitType = [&](const Type* t) {
        if (!t) return;
        switch (t->kind) {
            case TK::Struct:
                visit(t->decl);
                for (const Type* a : t->params) visitType(a);  // a Box<Point> field needs Point first
                break;
            case TK::Optional:
            case TK::List: visitType(t->elem); break;
            case TK::Map:
                visitType(t->key);
                visitType(t->elem);
                break;
            default: break;
        }
    };
    visit = [&](TypeDecl* d) {
        if (d->builtin || d->kind == DK::Enum || done.count(d) || active.count(d)) return;
        active.insert(d);
        if (d->base) visit(d->base);
        for (TypeDecl* i : d->interfaces) visit(i);
        for (auto& f : d->fields) visitType(f->type);
        active.erase(d);
        done.insert(d);
        order.push_back(d);
    };
    for (Module* m : P.allModules)
        for (auto& d : m->ast.types) visit(d.get());
    return order;
}

// class Box<T>: "template <typename T_> " before everything that belongs to it, and "<T_>" after its name.
std::string Gen::tmplHead(const TypeDecl* d) {
    if (d->typeParams.empty()) return "";
    std::string s = "template <";
    for (size_t i = 0; i < d->typeParams.size(); i++) s += (i ? ", typename " : "typename ") + d->typeParams[i] + "_";
    return s + "> ";
}

std::string Gen::targs(const TypeDecl* d) {
    if (d->typeParams.empty()) return "";
    std::string s = "<";
    for (size_t i = 0; i < d->typeParams.size(); i++) s += (i ? ", " : "") + d->typeParams[i] + "_";
    return s + ">";
}

// The C++ struct behind a struct or class type: ::m_main::Player_, or ::m_main::Box_<int> for a Box<int>.
std::string Gen::classOf(const Type* t) {
    if (!isGeneric(t)) return t->decl->cpp;
    if (t->kind == TK::Struct) return t->cpp;
    const std::string pre = "::std::shared_ptr<";
    return t->cpp.substr(pre.size(), t->cpp.size() - pre.size() - 1);
}

std::string Gen::strFuncProto(TypeDecl* d) {
    if (d->kind == DK::Enum) return "::std::string frg$str(" + d->cpp + " v)";
    if (d->kind == DK::Struct) return tmplHead(d) + "::std::string frg$str(const " + d->cpp + targs(d) + "& v)";
    return tmplHead(d) + "::std::string frg$str(const ::std::shared_ptr<" + d->cpp + targs(d) + ">& v)";
}

void Gen::emitTypeDef(TypeDecl* d) {
    std::string name = id(d->name);
    line("namespace " + d->module->cppNs + " {");
    if (d->kind == DK::Interface) {
        // An interface is a C++ struct with only "= 0" (pure virtual) methods.
        line("struct " + name + " {");
        ind++;
        line("virtual ~" + name + "() = default;");
        for (auto& m : d->methods) line("virtual " + m->ret->cpp + " " + id(m->name) + "(" + params(m.get()) + ") = 0;");
        line("virtual ::frg::core::Description_ frg$describe() = 0;");
        ind--;
        line("};");
        line("}");
        return;
    }
    std::string head = tmplHead(d) + "struct " + name;
    if (d->kind == DK::Class) {
        head += d->base ? " : public " + d->base->cpp : " : public ::std::enable_shared_from_this<" + name + targs(d) + ">";
        for (TypeDecl* i : d->interfaces) head += ", public " + i->cpp;
    }
    line(head + " {");
    ind++;
    for (auto& f : d->fields) line((f->isStatic ? "static " : "") + f->type->cpp + " " + id(f->name) + (f->isStatic ? ";" : "{};"));
    line(name + "();");
    if (d->kind == DK::Class && !d->base) line("virtual ~" + name + "() = default;");
    for (auto& c : d->ctors) line("void frg$init(" + params(c.get()) + ");");
    if (hasAutoInit(d)) line("void frg$init(" + fieldParams(d) + ");");

    std::vector<FuncDecl*> ctors = findCtors(d);
    std::string self = d->kind == DK::Class ? "::std::shared_ptr<" + name + ">" : name;
    std::string maker = d->kind == DK::Class ? "frg$new" : "frg$make";
    if (!ctors.empty()) {
        for (FuncDecl* c : ctors) line("static " + self + " " + maker + "(" + params(c) + ");");
    } else {
        line("static " + self + " " + maker + "();");
        if (hasAutoInit(d)) line("static " + self + " " + maker + "(" + fieldParams(d) + ");");
    }
    for (auto& m : d->methods)
        line(std::string(m->isStatic ? "static " : d->kind == DK::Class ? "virtual " : "") + m->ret->cpp + " " + id(m->name) + "(" +
             params(m.get()) + ")" + (m->isAbstract ? " = 0" : "") + ";");
    line(std::string(d->kind == DK::Class ? "virtual " : "") + "::frg::core::Description_ frg$describe();");
    if (d->kind == DK::Struct) {
        // == on a struct compares every field, unless it has its own equals(): then list.contains() and friends use that too.
        FuncDecl* eq = findMethod(d, "equals");
        if (eq && eq->params.size() == 1 && eq->params[0]->type == tNamed(d) && eq->ret == tBool())
            line("bool operator==(const " + name + "& o) const { return const_cast<" + name + "*>(this)->" + id(eq->name) + "(o); }");
        else
            line("bool operator==(const " + name + "&) const = default;");
    }
    if (d->kind == DK::Class) {
        // Lets the runtime ask an object "are you a <type name>?" (used by ECS's entity.get("Health")).
        bool chain = d->base && (!d->base->builtin || (d->base->name == "Component" && d->base->module->name == "ECS"));
        line("virtual bool frg$is(const ::std::string& n) { return n == " + cppString(d->name) +
             (chain ? " || " + d->base->cpp + "::frg$is(n)" : "") + "; }");
    }
    ind--;
    line("};");
    line("}");
}

void Gen::emitBody(const FuncDecl* f, TypeDecl* owner, const std::string& head) {
    fn = const_cast<FuncDecl*>(f);
    curType = owner;
    sourceLine(f->loc);
    line(head + " {");
    // Tasks pause and carry on later, out of order, so they don't keep a Frame (see ::frg::Frame).
    framed = f->ret != tTask();
    ind++;
    frameStart(f->isCtor && owner ? owner->name + "(...)" : owner ? owner->name + "." + f->name : f->name, f->loc);
    ind--;
    if (f->ret == tTask() && owner && owner->kind == DK::Class) {
        // A task can outlive the line that started it, so it keeps its object alive while it runs.
        line("    auto frg$self = this->shared_from_this();");
    }
    children(f->body.get());
    if (f->ret == tTask()) line("    co_return;");  // makes it a coroutine even if it never waits
    line("}");
    generatedLines();
    framed = false;
    curType = nullptr;
}

void Gen::emitTypeFuncs(TypeDecl* d) {
    std::string name = id(d->name);
    std::string full = d->cpp + targs(d);
    std::string th = tmplHead(d);   // class Box<T>: its code is a C++ template
    std::string q = name + targs(d);
    line("namespace " + d->module->cppNs + " {");

    if (d->kind == DK::Enum) {
        line(strFuncProto(d) + " {");
        ind++;
        line("switch (v) {");
        for (const std::string& v : d->enumValues) line("    case " + full + "::" + id(v) + ": return " + cppString(v) + ";");
        line("}");
        line("return " + cppString(d->name + "(?)") + ";");
        ind--;
        line("}");
        line("}");
        return;
    }
    if (d->kind == DK::Interface) {
        // Printing through an interface shows the real object: try each class that has it, most specific first.
        std::vector<TypeDecl*> classes;
        for (Module* m : P.allModules)
            for (auto& c : m->ast.types)
                if (c->kind == DK::Class && inheritsFrom(c.get(), d)) classes.push_back(c.get());
        auto depth = [](TypeDecl* t) {
            int n = 0;
            for (; t; t = t->base) n++;
            return n;
        };
        std::stable_sort(classes.begin(), classes.end(), [&](TypeDecl* a, TypeDecl* b) { return depth(a) > depth(b); });
        line(strFuncProto(d) + " {");
        ind++;
        line("if (!v) return \"null\";");
        for (TypeDecl* c : classes)
            line("if (auto x = ::std::dynamic_pointer_cast<" + c->cpp + ">(v)) return frg$str(x);");
        line("return " + cppString("<" + d->name + ">") + ";");
        ind--;
        line("}");
        line("}");
        return;
    }

    // Default constructor: field starting values.
    line(th + q + "::" + name + "() {");
    ind++;
    curType = nullptr;
    for (auto& f : d->fields)
        if (f->init && !f->isStatic) line("this->" + id(f->name) + " = " + conv(f->init.get(), f->type) + ";");
    ind--;
    line("}");

    for (auto& c : d->ctors) emitBody(c.get(), d, th + "void " + q + "::frg$init(" + params(c.get()) + ")");
    if (hasAutoInit(d)) {
        line(th + "void " + q + "::frg$init(" + fieldParams(d) + ") {");
        for (VarDecl* f : allFields(d)) line("    this->" + id(f->name) + " = " + id(f->name) + ";");
        line("}");
    }

    std::vector<FuncDecl*> ctors = findCtors(d);
    bool isClass = d->kind == DK::Class;
    std::string self = isClass ? "::std::shared_ptr<" + full + ">" : full;
    std::string maker = isClass ? "frg$new" : "frg$make";
    std::string make = isClass ? "auto o = ::std::make_shared<" + full + ">();" : full + " o;";
    std::string dot = isClass ? "->" : ".";
    auto argNames = [&](const std::vector<std::string>& names) { return joinNames(names); };
    if (d->isAbstract) {
        // An abstract class is never made by itself, so it has no frg$new (C++ couldn't make one anyway).
    } else if (!ctors.empty()) {
        for (FuncDecl* c : ctors) {
            std::vector<std::string> names;
            for (auto& p : c->params) names.push_back(id(p->name));
            line(th + self + " " + q + "::" + maker + "(" + params(c) + ") { " + make + " o" + dot + "frg$init(" + argNames(names) + "); return o; }");
        }
    } else {
        line(th + self + " " + q + "::" + maker + "() { " + make + " return o; }");
        if (hasAutoInit(d)) {
            std::vector<std::string> names;
            for (VarDecl* f : allFields(d)) names.push_back(id(f->name));
            line(th + self + " " + q + "::" + maker + "(" + fieldParams(d) + ") { " + make + " o" + dot + "frg$init(" + argNames(names) + "); return o; }");
        }
    }

    for (auto& m : d->methods)
        if (!m->isAbstract) emitBody(m.get(), d, th + m->ret->cpp + " " + q + "::" + id(m->name) + "(" + params(m.get()) + ")");

    // obj.describe: the type's name, its parent's, and each field (not private ones) with its value as text.
    line(th + "::frg::core::Description_ " + q + "::frg$describe() {");
    ind++;
    {
        std::string parent = d->base ? d->base->name : "";
        char c = parent.empty() ? 'x' : parent[0];
        bool vowel = std::string("AEIOUaeiou").find(c) != std::string::npos;
        std::string head = d->name + (parent.empty() ? "" : std::string(" (") + (vowel ? "an " : "a ") + parent + ")");
        line("::frg::core::Description_ r;");
        line("r.name_ = " + cppString(d->name) + ";");
        line("r.parent_ = " + cppString(parent) + ";");
        std::vector<VarDecl*> shown;
        for (VarDecl* f : allFields(d))
            if (!f->isPrivate && !f->isStatic) shown.push_back(f);
        std::string text = "::std::string(" + cppString(head + (shown.empty() ? "" : ": ")) + ")";
        for (size_t i = 0; i < shown.size(); i++) {
            VarDecl* f = shown[i];
            std::string v = strCode("this->" + id(f->name), f->type, true);
            line("r.fields_.push_back(" + cppString(f->name) + ");");
            line("r.values_[" + cppString(f->name) + "] = " + v + ";");
            std::string val = "r.values_[" + cppString(f->name) + "]";
            if (f->type->kind == TK::String) val = "\"\\\"\" + " + val + " + \"\\\"\"";
            text += " + " + cppString((i ? ", " : "") + f->name + " = ") + " + " + val;
        }
        line("r.text_ = " + text + ";");
        line("return r;");
    }
    ind--;
    line("}");

    // Text form, used by print() and "{...}".
    line(strFuncProto(d) + " {");
    ind++;
    FuncDecl* toString = findMethod(d, "toText");  // toText() chooses what print and "{x}" show (toString is its old name)
    if (!toString || !toString->params.empty() || toString->ret != tString()) toString = findMethod(d, "toString");
    if (toString && toString->params.empty() && toString->ret == tString()) {
        std::string m = id(toString->name) + "()";
        line(isClass ? "return v->" + m + ";" : "return const_cast<" + q + "&>(v)." + m + ";");
    } else {
        std::string acc = isClass ? "v->" : "v.";
        std::string s = "::std::string(" + cppString(d->name + "(") + ")";
        std::vector<VarDecl*> fields = allFields(d);
        for (size_t i = 0; i < fields.size(); i++) {
            s += " + " + cppString((i ? ", " : "") + fields[i]->name + ": ") + " + " +
                 strCode(acc + id(fields[i]->name), fields[i]->type, isClass);
        }
        line("return " + s + " + \")\";");
    }
    ind--;
    line("}");
    line("}");
}

GeneratedCpp Gen::run() {
    GeneratedCpp result;
    std::set<std::string> headers;
    for (Module* m : P.allModules)
        for (const std::string& lib : m->libs)
            for (const PreludeLib& pl : preludeLibs())
                if (pl.name == lib) {
                    if (!pl.header.empty()) headers.insert(pl.header);
                    result.needsRaylib |= pl.needsRaylib;
                    for (const std::string& s : pl.sources)
                        if (std::find(result.sources.begin(), result.sources.end(), s) == result.sources.end()) result.sources.push_back(s);
                    for (const std::string& f : pl.linkFlags)
                        if (std::find(result.linkFlags.begin(), result.linkFlags.end(), f) == result.linkFlags.end()) result.linkFlags.push_back(f);
                }

    out = "// Generated by Forge from your .frg files: every declaration, shared by all the parts of the game's C++.\n"
          "// Edit your .frg files instead - this file is rebuilt every time.\n#pragma once\n";
    result.headers.push_back("forge.hpp");
    for (const std::string& h : headers) result.headers.push_back(h);
    // physics.drawDebug() is drawn with raylib, so its code is only added to games that also use <graphics2d>.
    if (headers.count("forge_physics.hpp") && headers.count("forge_window.hpp")) result.headers.push_back("forge_physics_draw.hpp");
    for (const std::string& h : result.headers) out += "#include \"" + h + "\"\n";
    out += "\n";

    // 1. Every type's name, and the full enums.
    for (Module* m : P.allModules) {
        if (m->ast.types.empty()) continue;
        line("namespace " + m->cppNs + " {");
        ind++;
        for (auto& d : m->ast.types) {
            if (d->kind == DK::Enum) {
                std::string vals;
                for (size_t i = 0; i < d->enumValues.size(); i++) vals += (i ? ", " : "") + id(d->enumValues[i]);
                line("enum class " + id(d->name) + " : int32_t { " + vals + " };");
            } else {
                line(tmplHead(d.get()) + "struct " + id(d->name) + ";");
            }
        }
        ind--;
        line("}");
    }

    // 2. Every function and global variable, so anything can use anything no matter where it's written.
    for (Module* m : P.allModules) {
        line("namespace " + m->cppNs + " {  // " + m->src->displayPath);
        ind++;
        for (auto& d : m->ast.types) line(strFuncProto(d.get()) + ";");
        for (auto& f : m->ast.funcs) line(f->ret->cpp + " " + id(f->name) + "(" + params(f.get()) + ");");
        for (auto& g : m->ast.globals) line("extern " + g->type->cpp + " " + id(g->name) + ";");
        ind--;
        line("}");
    }
    out += "\n";

    // 3. Structs and classes, each after the ones it contains or inherits from.
    for (TypeDecl* d : typeOrder()) {
        emitTypeDef(d);
        out += "\n";
    }

    // 4. The static fields of structs and classes ("inline", so every part can include them).
    globalScope = true;
    for (TypeDecl* d : typeOrder())
        for (auto& f : d->fields) {
            if (!f->isStatic) continue;
            // d->cpp starts with "::", which C++ would join to the type before it, so it's left off here.
            std::string decl = "inline " + f->type->cpp + " " + d->cpp.substr(2) + "::" + id(f->name);
            line(f->init ? decl + " = " + globalValue(f.get()) + ";" : decl + "{};");
        }

    globalScope = false;

    // 5. Generic types' code: C++ templates, which have to be in the header that every part includes.
    for (Module* m : P.allModules)
        for (auto& d : m->ast.types)
            if (!d->typeParams.empty()) {
                emitTypeFuncs(d.get());
                out += "\n";
            }
    result.header = std::move(out);

    // 6. Each file's function, method and constructor bodies.
    for (Module* m : P.allModules) {
        out.clear();
        for (auto& d : m->ast.types) {
            if (!d->typeParams.empty()) continue;
            emitTypeFuncs(d.get());
            out += "\n";
        }
        if (!m->ast.funcs.empty()) {
            line("namespace " + m->cppNs + " {");
            ind++;
            for (auto& f : m->ast.funcs) emitBody(f.get(), nullptr, f->ret->cpp + " " + id(f->name) + "(" + params(f.get()) + ")");
            ind--;
            line("}");
        }
        if (!out.empty()) result.modules.push_back({m->src->displayPath, std::move(out)});
    }

    // 7. Global variables, all in one place: C++ sets them in the order they're written within one file, but in no
    // fixed order between files. Then C++'s main().
    out.clear();
    globalScope = true;
    for (Module* m : P.allModules) {
        if (m->ast.globals.empty()) continue;
        line("namespace " + m->cppNs + " {");
        ind++;
        for (auto& g : m->ast.globals) {
            std::string decl = g->type->cpp + " " + id(g->name);
            line(g->init ? decl + " = " + globalValue(g.get()) + ";" : decl + "{};");
        }
        ind--;
        line("}");
        out += "\n";
    }
    globalScope = false;
    if (tests) {
        // frg test: every function in the tests folder whose name starts with "test" (and that takes nothing and
        // gives nothing back) is run, one after another. A failed check or a runtime error fails only that test.
        std::vector<std::string> calls;
        for (Module* m : P.allModules) {
            if (!m->folder || (m->folder->qualName != "tests" && m->folder->qualName.rfind("tests.", 0) != 0)) continue;
            for (auto& f : m->ast.funcs)
                if (f->name.rfind("test", 0) == 0 && f->params.empty() && f->ret == tVoid())
                    calls.push_back("::frg::testing::run(" + cppString(m->src->displayPath) + ", " + cppString(f->name) + ", &" +
                                    qual(m, f->name) + ");");
        }
        line("int main() {");
        line("    ::frg::init();");
        for (const std::string& c : calls) line("    " + c);
        line("    int code = ::frg::testing::summary(" + std::to_string(calls.size()) + ");");
        line("    ::frg::shutdown();");
        line("    return code;");
        line("}");
        result.mainCode = std::move(out);
        return result;
    }
    line("int main() {");
    line("    ::frg::init();");
    line("    try {");
    line("        int code = static_cast<int>(" + qual(P.mainModule, "main") + "());");
    line("        ::frg::shutdown();");
    line("        return code;");
    line("    } catch (const ::frg::core::Error_& e) {");
    line("        ::frg::fatal(e);  // a runtime error that no try { } caught: stop the game with its message");
    line("    }");
    line("}");
    result.mainCode = std::move(out);
    return result;
}

}  // namespace

GeneratedCpp generateCpp(Project& proj, bool debug, bool tests) {
    Gen g(proj, debug, tests);
    return g.run();
}
