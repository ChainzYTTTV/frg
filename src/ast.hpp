#pragma once
#include "common.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Type;
struct Module;
struct Folder;
struct FuncDecl;
struct VarDecl;
struct TypeDecl;
struct Builtin;
struct BuiltinConst;

// A type as written in the source: `int`, `list<Enemy>`, `entities.Player?`, `Pointer<int>`.
struct TypeRef {
    std::vector<std::string> path;  // {"entities", "Player"}; empty means "no type"
    std::vector<TypeRef> args;      // generic arguments
    bool optional = false;
    Loc loc;

    bool empty() const { return path.empty(); }
    std::string display() const {
        std::string s;
        for (size_t i = 0; i < path.size(); i++) s += (i ? "." : "") + path[i];
        if (!args.empty()) {
            s += "<";
            for (size_t i = 0; i < args.size(); i++) s += (i ? ", " : "") + args[i].display();
            s += ">";
        }
        return optional ? s + "?" : s;
    }
};

enum class EK {
    Int, Float, Bool, Null, Str, Name, Member, Call, Unary, Binary, Cast,
    Index,     // a[i]
    ListLit,   // [1, 2, 3]
    MapLit,    // {"a": 1}
    BraceLit,  // {1, 2} - builds the expected struct/class
    Ternary,   // cond ? a : b
    This,
    Super,
    Is,        // obj is Type
    As,        // obj as Type (null if it isn't one)
};

// What a name (or a.b.c path) refers to. Filled in by the checker.
enum class RK {
    None,
    Local, Global, Func, Builtin, Module, Folder, StdLib, StdConst,
    Field,         // a field of a struct/class (var = the field)
    Method,        // a method (func = the method)
    Type,          // a struct/class/enum name (tdecl)
    EnumValue,     // State.Idle (tdecl, index)
    TypeStatic,    // color.RED (var = the static)
    BuiltinMethod, // list.add, map.keys, text.upper (name in `lib`)
    PointerValue,  // p.value
};

struct Resolved {
    RK kind = RK::None;
    VarDecl* var = nullptr;
    FuncDecl* func = nullptr;
    Module* module = nullptr;
    Folder* folder = nullptr;
    TypeDecl* tdecl = nullptr;
    const Builtin* builtin = nullptr;
    const BuiltinConst* constant = nullptr;
    std::string lib;
    int index = 0;
};

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct Expr {
    EK kind;
    Loc loc;
    std::string text;                // literal text, name, member name, operator, or cast type name
    bool boolVal = false;
    std::vector<std::string> parts;  // Str: literal pieces around the {interpolations}
    std::vector<ExprPtr> args;       // Str interpolations, Call arguments, list items, map values, brace items
    std::vector<ExprPtr> keys;       // MapLit keys
    ExprPtr a, b, c;                 // operands / object / callee / index (b) / ternary branches (b, c)
    TypeRef typeRef;                 // Is / As: the type checked for
    std::string opWord;              // Binary: the operator as written, when it was a word ("plus", "greater or equal")

    // Filled in by the checker.
    Type* type = nullptr;
    Type* opType = nullptr;  // Binary/Unary: the type both operands are converted to first
    Resolved res;
    unsigned __int128 ival = 0;  // Int literal value
    bool litInt = false;     // a constant built only from integer literals, so it can adapt to any number type
    bool litFloat = false;   // a constant built from literals including a decimal one
    bool isWrite = false;    // Index: the target of an assignment (map[key] = ... creates the key)
    FuncDecl* ctor = nullptr;            // Call/BraceLit that builds an object: the constructor used (or null = field-by-field)
    TypeDecl* builds = nullptr;          // Call/BraceLit that builds an object: the type built
    std::vector<Expr*> defaultArgs;      // Call: default values filled in for missing arguments

    Expr(EK k, Loc l) : kind(k), loc(l) {}
};

struct VarDecl {
    std::string name;
    Loc loc;
    TypeRef typeRef;
    ExprPtr init;            // starting value / field default / parameter default
    bool isConst = false;
    bool isGlobal = false;
    bool isParam = false;
    bool isField = false;
    bool isStatic = false;   // built-in constants like color.RED
    bool isPrivate = false;  // only usable inside its class (fields) or its file (globals)
    Type* type = nullptr;
    Module* module = nullptr;
    TypeDecl* owner = nullptr;  // fields
};
using VarPtr = std::unique_ptr<VarDecl>;

enum class SK { Block, Var, ExprStmt, Assign, IncDec, If, While, For, ForEach, Switch, Return, Break, Continue, SuperCall };

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct SwitchCase {
    std::vector<ExprPtr> values;  // empty for default
    StmtPtr body;                 // a Block
    bool isDefault = false;
    Loc loc;
};

struct Stmt {
    SK kind;
    Loc loc;
    std::vector<StmtPtr> body;  // Block
    VarPtr var;                 // Var, ForEach loop variable
    ExprPtr expr;               // ExprStmt, Return value, If/While/For condition, Assign/IncDec target, ForEach list, Switch subject
    ExprPtr value;              // Assign right-hand side
    std::string op;             // Assign ("=", "+=", ...), IncDec ("++", "--")
    StmtPtr then, els;          // If branches; While/For/ForEach body is `then`
    StmtPtr init, step;         // For
    std::vector<SwitchCase> cases;
    std::vector<ExprPtr> args;  // SuperCall arguments
    FuncDecl* target = nullptr; // SuperCall: the base constructor

    Stmt(SK k, Loc l) : kind(k), loc(l) {}
};

struct FuncDecl {
    std::string name;
    Loc loc;
    TypeRef retRef;
    Type* ret = nullptr;
    std::vector<VarPtr> params;
    StmtPtr body;               // null for built-in declarations
    Module* module = nullptr;
    TypeDecl* owner = nullptr;  // methods and constructors
    bool isCtor = false;
    bool isPrivate = false;     // only usable inside its class (methods) or its file (functions)
    FuncDecl* overrides = nullptr;

    size_t requiredParams() const {
        size_t n = 0;
        while (n < params.size() && !params[n]->init) n++;
        return n;
    }
};
using FuncPtr = std::unique_ptr<FuncDecl>;

enum class DK { Struct, Class, Enum };

struct TypeDecl {
    DK kind;
    std::string name;
    Loc loc;
    Module* module = nullptr;
    bool builtin = false;
    bool isPrivate = false;  // only usable inside its own file
    std::string cpp;  // C++ name of the struct/class/enum itself, e.g. "::m_main::Player_"
    TypeRef baseRef;
    TypeDecl* base = nullptr;
    std::vector<VarPtr> fields;     // includes `static` built-in constants
    std::vector<FuncPtr> methods;
    std::vector<FuncPtr> ctors;
    std::vector<std::string> enumValues;
    std::vector<Loc> enumLocs;
    Type* type = nullptr;

    std::map<std::string, VarDecl*> fieldMap;   // own fields only
    std::map<std::string, FuncDecl*> methodMap; // own methods only
};
using TypeDeclPtr = std::unique_ptr<TypeDecl>;

struct Include {
    bool isFolder = false;  // #include <"folder"> vs #include <library>
    std::string name;
    Loc loc;
};

struct FileAST {
    bool ok = true;
    std::vector<Include> includes;
    std::vector<FuncPtr> funcs;
    std::vector<VarPtr> globals;
    std::vector<TypeDeclPtr> types;
};
