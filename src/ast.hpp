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
    bool isFunction = false;        // Function<ret(params)>: args[0] is the return type, the rest the parameters
    Loc loc;

    bool empty() const { return path.empty(); }
    std::string display() const {
        std::string s;
        for (size_t i = 0; i < path.size(); i++) s += (i ? "." : "") + path[i];
        if (isFunction) {
            s += "<" + args[0].display() + "(";
            for (size_t i = 1; i < args.size(); i++) s += (i > 1 ? ", " : "") + args[i].display();
            s += ")>";
        } else if (!args.empty()) {
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
    Lambda,    // (Enemy e) { return e.hp <= 0; } - a function value
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
    Describe,      // obj.describe: built-in info about any struct or class object
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
    std::vector<int> decimals;       // Str: for each interpolation, the decimals asked for with {value:.2}, or -1
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
    FuncDecl* opMethod = nullptr;        // Binary on your own type: the method it calls (a + b is a.plus(b)); list.sort(): less
    bool opSwap = false;                 // a > b is b.less(a)
    bool opNot = false;                  // a >= b is !a.less(b), a != b is !a.equals(b)
    TypeDecl* builds = nullptr;          // Call/BraceLit that builds an object: the type built
    std::vector<Expr*> defaultArgs;      // Call: default values filled in for missing arguments
    std::vector<Type*> paramTypes;       // Call on a generic type (Box<int>): each parameter's type with T filled in
    std::shared_ptr<FuncDecl> lambda;    // Lambda: its parameters and body (ret is filled in by the checker)

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
    bool isStatic = false;   // static fields: one shared by the whole type (Enemy.count), and built-in constants like color.RED
    bool isPrivate = false;  // only usable inside its class (fields) or its file (globals)
    Type* type = nullptr;
    Module* module = nullptr;
    TypeDecl* owner = nullptr;  // fields
};
using VarPtr = std::unique_ptr<VarDecl>;

enum class SK { Block, Var, ExprStmt, Assign, IncDec, If, While, DoWhile, For, ForEach, Switch, Return, Break, Continue, SuperCall, Try, Throw };

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
    Loc end;                    // Block: its closing '}' (line 0 if it was never closed)
    std::vector<StmtPtr> body;  // Block
    VarPtr var;                 // Var, ForEach loop variable, Try: the Error in catch (Error e)
    ExprPtr expr;               // ExprStmt, Return value, If/While/For condition, Assign/IncDec target, ForEach list, Switch subject
    ExprPtr value;              // Assign right-hand side; ForEach over a range (0..10): the end, with expr the start
    std::string op;             // Assign ("=", "+=", ...), IncDec ("++", "--")
    StmtPtr then, els;          // If branches; While/DoWhile/For/ForEach body is `then`; Try: then = try { }, els = catch { }
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
    bool isStatic = false;      // a static method: belongs to the type, not to one object
    bool isAbstract = false;    // an abstract method (or an interface's): no code, every class below must write it
    FuncDecl* overrides = nullptr;

    size_t requiredParams() const {
        size_t n = 0;
        while (n < params.size() && !params[n]->init) n++;
        return n;
    }
};
using FuncPtr = std::unique_ptr<FuncDecl>;

enum class DK { Struct, Class, Enum, Interface };

struct TypeDecl {
    DK kind;
    std::string name;
    Loc loc;
    Module* module = nullptr;
    bool builtin = false;
    bool isPrivate = false;  // only usable inside its own file
    std::string cpp;  // C++ name of the struct/class/enum itself, e.g. "::m_main::Player_"
    TypeRef baseRef;
    std::vector<TypeRef> moreBaseRefs;      // class Goblin : Enemy, Attacker - the names after the first
    TypeDecl* base = nullptr;               // the class it inherits from
    std::vector<TypeDecl*> interfaces;      // the interfaces it has
    bool isAbstract = false;                // abstract class: can't be made directly
    std::vector<std::string> typeParams;    // class Box<T>: {"T"}
    std::vector<Loc> typeParamLocs;
    std::vector<Type*> paramTypes;          // the type each of typeParams stands for inside the type
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
