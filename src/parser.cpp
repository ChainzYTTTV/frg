#include "parser.hpp"

#include "lexer.hpp"
#include "types.hpp"

#include <algorithm>

namespace {

class Parser {
public:
    Parser(const SourceFile& f, std::vector<Token> toks, bool prelude) : file(f), t(std::move(toks)), prelude(prelude) {}

    FileAST parseFile();
    ExprPtr parseExpr() { return parseTernary(); }
    bool atEnd() const { return t[p].kind == Tok::Eof; }
    const Token& cur() const { return t[p]; }

private:
    const SourceFile& file;
    std::vector<Token> t;
    bool prelude;
    size_t p = 0;
    // `>>` closing two generic types is split into two `>`s; this remembers how to undo it.
    std::vector<std::pair<size_t, Token>> splitLog;
    bool hadError = false;  // set when a syntax error was skipped over, so more errors can be reported

    // After a syntax error inside a block: skip to the end of that statement.
    void syncStatement() {
        int depth = 0;
        while (!atEnd()) {
            Tok k = cur().kind;
            if (k == Tok::LBrace) {
                depth++;
            } else if (k == Tok::RBrace) {
                if (depth == 0) return;
                p++;
                if (--depth == 0) return;
                continue;
            } else if (k == Tok::Semi && depth == 0) {
                p++;
                return;
            }
            p++;
        }
    }

    // After a syntax error outside any function: skip to the next thing that starts at the left edge.
    void syncTopLevel() {
        if (!atEnd()) p++;
        while (!atEnd()) {
            Tok k = cur().kind;
            bool starts = k == Tok::Ident || k == Tok::KwVoid || k == Tok::KwStruct || k == Tok::KwClass || k == Tok::KwEnum ||
                          k == Tok::KwInterface || k == Tok::KwAbstract ||
                          k == Tok::KwConst || k == Tok::Hash || k == Tok::KwPrivate || k == Tok::KwPublic;
            if (starts && cur().loc.col == 1) return;
            p++;
        }
    }

    const Token& peek(size_t n = 1) const { return t[std::min(p + n, t.size() - 1)]; }
    const Token& prev() const { return t[p ? p - 1 : 0]; }
    bool at(Tok k) const { return t[p].kind == k; }
    bool accept(Tok k) {
        if (!at(k)) return false;
        p++;
        return true;
    }
    [[noreturn]] void fail(const Loc& l, const std::string& msg, const std::string& hint = "") {
        reportError(l, msg, hint);
        throw CompileAbort{};
    }
    std::string describe(const Token& k) const {
        if (k.kind == Tok::Eof) return "the end of the file";
        if (k.kind == Tok::StrLit) return "some text";
        return "'" + k.text + "'";
    }
    const Token& expect(Tok k, const std::string& what);

    bool parseTypeImpl(TypeRef& r, bool quiet);
    TypeRef parseType() {
        TypeRef r;
        parseTypeImpl(r, false);
        return r;
    }
    bool declAhead(bool allowParen);
    bool lambdaAhead();
    bool bodyless = false;  // parsing an interface's or an abstract method: it ends with ; instead of { code }
    ExprPtr parseLambda();
    Include parseInclude();
    TypeDeclPtr parseTypeDecl(bool isAbstract = false);
    FuncPtr parseFuncRest(const TypeRef& ret, const Token& name, bool isCtor);
    VarPtr finishVar(bool isConst, const TypeRef& type, const Token& name);

    StmtPtr parseBlock();
    StmtPtr parseBody(const std::string& what);
    StmtPtr parseStmt();
    StmtPtr parseIf();
    StmtPtr parseFor();
    StmtPtr parseSwitch();
    StmtPtr parseVarStmt();
    StmtPtr parseSimple();
    ExprPtr parseCond();

    ExprPtr binary(ExprPtr l, const Token& op, ExprPtr r, const std::string& text);
    bool atWord(const char* w) const;
    StmtPtr tryWordStatement();
    ExprPtr parseTernary();
    ExprPtr parseCoalesce();
    ExprPtr parseOr();
    ExprPtr parseAnd();
    ExprPtr parseNot();
    ExprPtr parseBitOr();
    ExprPtr parseBitXor();
    ExprPtr parseBitAnd();
    ExprPtr parseEquality();
    ExprPtr parseCompare();
    ExprPtr parseShift();
    ExprPtr parseAdd();
    ExprPtr parseMul();
    ExprPtr parseUnary();
    ExprPtr parsePostfix();
    ExprPtr parsePrimary();
    ExprPtr parseString(const Token& tok);
    ExprPtr parseBraceLiteral();
};

const Token& Parser::expect(Tok k, const std::string& what) {
    if (at(k)) return t[p++];
    if (k == Tok::Semi && p > 0) {
        Loc l = prev().loc;
        l.col += l.len;
        l.len = 1;
        fail(l, "missing ';' at the end of this line");
    }
    fail(cur().loc, "expected " + what + ", but found " + describe(cur()));
}

// Parses `name`, `a.b.Name`, `list<int>`, `map<string, list<int>>`, `Player?`.
bool Parser::parseTypeImpl(TypeRef& r, bool quiet) {
    r.loc = cur().loc;
    auto bail = [&](const std::string& what) {
        if (!quiet) fail(cur().loc, "expected " + what + ", but found " + describe(cur()));
        return false;
    };
    if (accept(Tok::KwVoid)) {
        r.path = {"void"};
    } else if (at(Tok::Ident)) {
        r.path.push_back(t[p++].text);
        while (at(Tok::Dot) && peek().kind == Tok::Ident) {
            p++;
            r.path.push_back(t[p++].text);
        }
    } else {
        return bail("a type");
    }
    if (r.path.size() == 1 && r.path[0] == "Function" && at(Tok::Lt)) {
        // Function<bool(Enemy, int)>: the return type, then the parameter types in ( ).
        p++;
        r.isFunction = true;
        TypeRef ret;
        if (!parseTypeImpl(ret, quiet)) return false;
        r.args.push_back(std::move(ret));
        if (!accept(Tok::LParen)) return bail("'(' and the parameter types, like Function<bool(Enemy)>");
        if (!at(Tok::RParen)) {
            do {
                TypeRef arg;
                if (!parseTypeImpl(arg, quiet)) return false;
                r.args.push_back(std::move(arg));
            } while (accept(Tok::Comma));
        }
        if (!accept(Tok::RParen)) return bail("')'");
        if (at(Tok::Shr)) {
            splitLog.push_back({p, t[p]});
            t[p].kind = Tok::Gt;
            t[p].text = ">";
            t[p].loc.col++;
            t[p].loc.len = 1;
        } else if (!accept(Tok::Gt)) {
            return bail("'>'");
        }
    } else if (at(Tok::Lt)) {
        p++;
        do {
            TypeRef arg;
            if (!parseTypeImpl(arg, quiet)) return false;
            r.args.push_back(std::move(arg));
        } while (accept(Tok::Comma));
        if (at(Tok::Shr)) {
            splitLog.push_back({p, t[p]});
            t[p].kind = Tok::Gt;
            t[p].text = ">";
            t[p].loc.col++;
            t[p].loc.len = 1;
        } else if (!accept(Tok::Gt)) {
            return bail("'>'");
        }
    }
    if (accept(Tok::Question)) r.optional = true;
    if (prev().loc.line == r.loc.line) r.loc.len = std::max(1, prev().loc.col + prev().loc.len - r.loc.col);
    return true;
}

// Is a declaration (`Type name ...`) coming up? Looks ahead without consuming anything.
// Is a function value like `(Enemy e) { ... }` or `() { ... }` coming up? Looks ahead without consuming anything.
bool Parser::lambdaAhead() {
    size_t save = p, splits = splitLog.size();
    bool result = false;
    p++;  // (
    if (at(Tok::RParen)) {
        result = peek().kind == Tok::LBrace;
    } else {
        for (;;) {
            TypeRef tmp;
            if (!parseTypeImpl(tmp, true) || !at(Tok::Ident)) break;
            p++;
            if (accept(Tok::Comma)) continue;
            if (at(Tok::RParen)) result = peek().kind == Tok::LBrace;
            break;
        }
    }
    while (splitLog.size() > splits) {
        t[splitLog.back().first] = splitLog.back().second;
        splitLog.pop_back();
    }
    p = save;
    return result;
}

ExprPtr Parser::parseLambda() {
    auto e = std::make_unique<Expr>(EK::Lambda, cur().loc);
    auto f = std::make_shared<FuncDecl>();
    f->name = "function value";
    f->loc = cur().loc;
    expect(Tok::LParen, "'('");
    if (!at(Tok::RParen)) {
        do {
            auto v = std::make_unique<VarDecl>();
            v->typeRef = parseType();
            const Token& pn = expect(Tok::Ident, "a parameter name");
            v->name = pn.text;
            v->loc = pn.loc;
            v->isParam = true;
            f->params.push_back(std::move(v));
        } while (accept(Tok::Comma));
    }
    expect(Tok::RParen, "')'");
    f->body = parseBlock();
    e->lambda = f;
    return e;
}

bool Parser::declAhead(bool allowParen) {
    if (!at(Tok::Ident) && !at(Tok::KwVoid)) return false;
    size_t save = p, splits = splitLog.size();
    TypeRef tmp;
    bool result = false;
    if (parseTypeImpl(tmp, true) && at(Tok::Ident)) {
        Tok next = peek().kind;
        // `x ? y : z` looks like the start of `x? y` - a ':' after the name means it's an expression.
        result = !(tmp.optional && next == Tok::Colon) && (next != Tok::LParen || allowParen);
        if (!allowParen && next == Tok::LParen) result = false;
    } else if (!tmp.empty() && p < t.size() && isKeyword(cur().text) && (peek().kind == Tok::Assign || peek().kind == Tok::Semi)) {
        // `int do = 5;` - a reserved word used as a name.
        fail(cur().loc, "'" + cur().text + "' is a reserved word in Forge, so it can't be the name of a variable",
             "pick a different name - the reserved words are listed in the docs (Reserved words)");
    }
    while (splitLog.size() > splits) {
        t[splitLog.back().first] = splitLog.back().second;
        splitLog.pop_back();
    }
    p = save;
    return result;
}

FileAST Parser::parseFile() {
    FileAST ast;
    while (!atEnd()) {
        try {
            if (at(Tok::Hash)) {
                ast.includes.push_back(parseInclude());
                continue;
            }
            if (accept(Tok::Semi)) continue;
            Loc modLoc = cur().loc;
            bool isPrivate = at(Tok::KwPrivate);
            bool hasModifier = accept(Tok::KwPrivate) || accept(Tok::KwPublic);
            if (at(Tok::KwAbstract)) {
                Loc l = cur().loc;
                p++;
                if (!at(Tok::KwClass)) fail(l, "only a class can be abstract", "write it as: abstract class Enemy { ... }");
                ast.types.push_back(parseTypeDecl(true));
                ast.types.back()->isPrivate = isPrivate;
                continue;
            }
            if (at(Tok::KwStruct) || at(Tok::KwClass) || at(Tok::KwEnum) || at(Tok::KwInterface)) {
                ast.types.push_back(parseTypeDecl());
                ast.types.back()->isPrivate = isPrivate;
                continue;
            }
            if (at(Tok::KwConst) || declAhead(true)) {
                Loc constLoc = cur().loc;
                bool isConst = accept(Tok::KwConst);
                TypeRef type = parseType();
                const Token& name = expect(Tok::Ident, "a name");
                if (at(Tok::LParen)) {
                    if (isConst) fail(constLoc, "functions can't be const");
                    ast.funcs.push_back(parseFuncRest(type, name, false));
                    ast.funcs.back()->isPrivate = isPrivate;
                } else {
                    ast.globals.push_back(finishVar(isConst, type, name));
                    ast.globals.back()->isPrivate = isPrivate;
                    expect(Tok::Semi, "';'");
                }
                continue;
            }
            if (hasModifier) fail(modLoc, "'" + std::string(isPrivate ? "private" : "public") + "' has to come right before a function, a variable or a type");
            if (at(Tok::Ident) && peek().kind == Tok::LParen)
                fail(cur().loc, "this code has to go inside a function",
                     "put it inside main(){ ... } - or, if you're making a new function, give it a return type like 'void " +
                         cur().text + "()'");
            fail(cur().loc, "expected a function, a variable, a type or an #include, but found " + describe(cur()));
        } catch (CompileAbort&) {
            ast.ok = false;
            if (atEnd()) break;
            syncTopLevel();  // keep going, to find more errors
        }
    }
    if (hadError) ast.ok = false;
    return ast;
}

Include Parser::parseInclude() {
    p++;  // '#'
    if (!at(Tok::Ident) || cur().text != "include")
        fail(cur().loc, "unknown directive after '#'", "the only one is #include");
    p++;
    Include inc;
    if (at(Tok::StrLit))
        fail(cur().loc, "includes look like #include <stdio> or #include <\"folder\">",
             "files in the same folder are already included automatically");
    expect(Tok::Lt, "'<'");
    inc.loc = cur().loc;
    if (at(Tok::StrLit)) {
        inc.isFolder = true;
        inc.name = cur().text;
        p++;
    } else if (at(Tok::Ident)) {
        inc.name = cur().text;
        p++;
        if (at(Tok::Dot)) fail(cur().loc, "leave off the file extension", "write #include <" + inc.name + ">");
    } else {
        fail(cur().loc, "expected a library like <stdio> or a folder like <\"enemies\">");
    }
    expect(Tok::Gt, "'>'");
    return inc;
}

TypeDeclPtr Parser::parseTypeDecl(bool isAbstract) {
    auto d = std::make_unique<TypeDecl>();
    d->isAbstract = isAbstract;
    Tok kw = cur().kind;
    std::string kwName = cur().text;
    d->kind = kw == Tok::KwStruct ? DK::Struct : kw == Tok::KwClass ? DK::Class : kw == Tok::KwInterface ? DK::Interface : DK::Enum;
    p++;
    const Token& name = expect(Tok::Ident, "a name for the " + kwName);
    d->name = name.text;
    d->loc = name.loc;

    if (at(Tok::Lt)) {
        // class Box<T>, struct Pair<A, B>: a generic type, with a type name for each type it's made with.
        if (d->kind == DK::Enum || d->kind == DK::Interface)
            fail(cur().loc, "an " + kwName + " can't be generic", "only structs and classes can have types inside < >, like class Box<T>");
        p++;
        do {
            const Token& tp = expect(Tok::Ident, "a name for the type inside < >, like T");
            d->typeParams.push_back(tp.text);
            d->typeParamLocs.push_back(tp.loc);
        } while (accept(Tok::Comma));
        expect(Tok::Gt, "'>'");
    }

    if (d->kind == DK::Enum) {
        const Token& open = expect(Tok::LBrace, "'{'");
        while (!at(Tok::RBrace)) {
            if (atEnd()) fail(open.loc, "this '{' is never closed", "add a matching '}'");
            const Token& v = expect(Tok::Ident, "a name for an enum value");
            d->enumValues.push_back(v.text);
            d->enumLocs.push_back(v.loc);
            if (!accept(Tok::Comma)) break;
        }
        expect(Tok::RBrace, "'}'");
        accept(Tok::Semi);
        if (d->enumValues.empty()) fail(d->loc, "an enum needs at least one value", "for example: enum State { Idle, Running }");
        return d;
    }

    if (at(Tok::Colon)) {
        if (d->kind == DK::Struct)
            fail(cur().loc, "structs can't inherit from other types", "use a class if you need inheritance");
        if (d->kind == DK::Interface)
            fail(cur().loc, "an interface can't inherit from other types", "a class can have several interfaces: class Goblin : Enemy, Attacker, Flyer");
        p++;
        d->baseRef = parseType();
        if (d->baseRef.optional) fail(d->baseRef.loc, "a class can't inherit from an optional type");
        while (accept(Tok::Comma)) {
            d->moreBaseRefs.push_back(parseType());
            if (d->moreBaseRefs.back().optional) fail(d->moreBaseRefs.back().loc, "a class can't inherit from an optional type");
        }
    }
    const Token& open = expect(Tok::LBrace, "'{'");
    while (!at(Tok::RBrace)) {
        if (atEnd()) fail(open.loc, "this '{' is never closed", "add a matching '}'");
        if (accept(Tok::Semi)) continue;
        bool isPrivate = at(Tok::KwPrivate);
        if (accept(Tok::KwPrivate) || accept(Tok::KwPublic)) {
            if (at(Tok::KwPrivate) || at(Tok::KwPublic)) fail(cur().loc, "only one of private or public can be used");
        }
        if (d->kind == DK::Interface) {
            // An interface only lists methods, without their code: void attack(Enemy target);
            if (at(Tok::KwStatic) || at(Tok::KwAbstract) || isPrivate)
                fail(cur().loc, "an interface only lists methods, like: void attack(Enemy target);", "everything in an interface is public, and nothing is static");
            if (at(Tok::Ident) && cur().text == d->name && peek().kind == Tok::LParen)
                fail(cur().loc, "an interface can't have a constructor", "interfaces are never made directly - classes that have them are");
            TypeRef type = parseType();
            const Token& mname = expect(Tok::Ident, "a method name");
            if (!at(Tok::LParen)) fail(mname.loc, "an interface can't have fields, only methods", "for example: int power();");
            bodyless = true;
            d->methods.push_back(parseFuncRest(type, mname, false));
            bodyless = false;
            d->methods.back()->isAbstract = true;
            if (d->methods.back()->body)
                fail(mname.loc, "a method in an interface has no code - just end it with ;", "the classes that have the interface write the code");
            continue;
        }
        if (at(Tok::KwAbstract)) {
            // abstract void attack();  - only in an abstract class; the classes below write the code.
            Loc l = cur().loc;
            p++;
            if (!d->isAbstract) fail(l, "only an abstract class can have abstract methods", "write it as: abstract class " + d->name);
            TypeRef type = parseType();
            const Token& mname = expect(Tok::Ident, "a method name");
            if (!at(Tok::LParen)) fail(mname.loc, "only methods can be abstract, not fields");
            bodyless = true;
            d->methods.push_back(parseFuncRest(type, mname, false));
            bodyless = false;
            d->methods.back()->isAbstract = true;
            d->methods.back()->isPrivate = isPrivate;
            if (d->methods.back()->body)
                fail(mname.loc, "an abstract method has no code - end it with ;", "the classes that inherit from " + d->name + " write the code");
            continue;
        }
        if (at(Tok::KwStatic)) {
            // static: a field or method that belongs to the type itself (Enemy.count), not to each object.
            p++;
            if (d->kind == DK::Enum) fail(cur().loc, "an enum can't have static fields or methods");
            TypeRef type = parseType();
            const Token& fname = expect(Tok::Ident, "a name");
            if (at(Tok::LParen)) {
                d->methods.push_back(parseFuncRest(type, fname, false));
                d->methods.back()->isStatic = true;
                d->methods.back()->isPrivate = isPrivate;
                continue;
            }
            auto v = finishVar(false, type, fname);
            v->isStatic = true;
            v->isPrivate = isPrivate;
            expect(Tok::Semi, "';'");
            d->fields.push_back(std::move(v));
            continue;
        }
        if (at(Tok::Ident) && cur().text == d->name && peek().kind == Tok::LParen) {
            const Token& ctorName = t[p++];
            TypeRef none;
            d->ctors.push_back(parseFuncRest(none, ctorName, true));
            d->ctors.back()->isPrivate = isPrivate;
            continue;
        }
        if (at(Tok::KwConst)) fail(cur().loc, "fields can't be const yet", "use a const global variable instead");
        TypeRef type = parseType();
        const Token& mname = expect(Tok::Ident, "a field or method name");
        if (at(Tok::LParen)) {
            d->methods.push_back(parseFuncRest(type, mname, false));
            d->methods.back()->isPrivate = isPrivate;
        } else {
            d->fields.push_back(finishVar(false, type, mname));
            d->fields.back()->isPrivate = isPrivate;
            expect(Tok::Semi, "';'");
        }
    }
    p++;
    accept(Tok::Semi);
    return d;
}

FuncPtr Parser::parseFuncRest(const TypeRef& ret, const Token& name, bool isCtor) {
    auto f = std::make_unique<FuncDecl>();
    f->name = name.text;
    f->loc = name.loc;
    f->retRef = ret;
    f->isCtor = isCtor;
    expect(Tok::LParen, "'('");
    if (!at(Tok::RParen)) {
        do {
            auto v = std::make_unique<VarDecl>();
            v->typeRef = parseType();
            const Token& pn = expect(Tok::Ident, "a parameter name");
            v->name = pn.text;
            v->loc = pn.loc;
            v->isParam = true;
            if (accept(Tok::Assign)) v->init = parseExpr();
            f->params.push_back(std::move(v));
        } while (accept(Tok::Comma));
    }
    expect(Tok::RParen, "')'");
    if ((prelude || bodyless) && accept(Tok::Semi)) return f;
    if (at(Tok::Semi))
        fail(cur().loc, "Forge doesn't need function declarations - just write the function with its { body }",
             "functions can be used before or after the place they're written");
    f->body = parseBlock();
    return f;
}

VarPtr Parser::finishVar(bool isConst, const TypeRef& type, const Token& name) {
    auto v = std::make_unique<VarDecl>();
    v->name = name.text;
    v->loc = name.loc;
    v->typeRef = type;
    v->isConst = isConst;
    if (accept(Tok::Assign)) v->init = parseExpr();
    if (at(Tok::Comma)) fail(cur().loc, "make one variable per line", "for example: int a = 1;  int b = 2;");
    return v;
}

StmtPtr Parser::parseBlock() {
    const Token& open = expect(Tok::LBrace, "'{'");
    auto b = std::make_unique<Stmt>(SK::Block, open.loc);
    while (!at(Tok::RBrace)) {
        if (atEnd()) fail(open.loc, "this '{' is never closed", "add a matching '}'");
        try {
            b->body.push_back(parseStmt());
        } catch (CompileAbort&) {
            if (atEnd()) throw;
            hadError = true;
            syncStatement();  // skip the broken statement and keep checking the rest
        }
    }
    p++;
    return b;
}

StmtPtr Parser::parseBody(const std::string& what) {
    if (!at(Tok::LBrace))
        fail(cur().loc, "'" + what + "' needs { } around its code", "for example: " + what + " (...) { ... }");
    return parseBlock();
}

ExprPtr Parser::parseCond() {
    ExprPtr e = parseExpr();
    if (at(Tok::Assign)) fail(cur().loc, "'=' sets a variable - to compare two values, use '=='");
    return e;
}

StmtPtr Parser::parseStmt() {
    Loc l = cur().loc;
    switch (cur().kind) {
        case Tok::LBrace: return parseBlock();
        case Tok::KwIf: return parseIf();
        case Tok::KwFor: return parseFor();
        case Tok::KwSwitch: return parseSwitch();
        case Tok::KwWhile: {
            p++;
            auto s = std::make_unique<Stmt>(SK::While, l);
            expect(Tok::LParen, "'(' after while");
            s->expr = parseCond();
            expect(Tok::RParen, "')'");
            s->then = parseBody("while");
            return s;
        }
        case Tok::KwDo: {
            // do { ... } while (condition);  runs the code once, then again for as long as the condition is true.
            p++;
            auto s = std::make_unique<Stmt>(SK::DoWhile, l);
            if (!at(Tok::LBrace)) fail(cur().loc, "'do' needs { } around its code", "for example: do { ... } while (x > 0);");
            s->then = parseBlock();
            if (!at(Tok::KwWhile))
                fail(cur().loc, "a 'do { ... }' needs 'while (...)' after its closing }", "for example: do { ... } while (lives > 0);");
            p++;
            expect(Tok::LParen, "'(' after while");
            s->expr = parseCond();
            expect(Tok::RParen, "')'");
            expect(Tok::Semi, "';'");
            return s;
        }
        case Tok::KwTry: {
            // try { ... } catch (Error e) { ... }: a runtime error in the first part runs the second part instead of stopping the game.
            p++;
            auto s = std::make_unique<Stmt>(SK::Try, l);
            if (!at(Tok::LBrace)) fail(cur().loc, "'try' needs { } around its code", "for example: try { ... } catch (Error e) { ... }");
            s->then = parseBlock();
            if (!at(Tok::KwCatch))
                fail(cur().loc, "a 'try { ... }' needs 'catch (Error e) { ... }' after its closing }",
                     "the catch part runs if something goes wrong in the try part");
            p++;
            if (!at(Tok::LParen)) fail(cur().loc, "catch needs '(Error e)' after it", "for example: catch (Error e) { print(e.message); }");
            p++;
            s->var = std::make_unique<VarDecl>();
            s->var->typeRef = parseType();
            const Token& name = expect(Tok::Ident, "a name for the error, like: catch (Error e)");
            s->var->name = name.text;
            s->var->loc = name.loc;
            expect(Tok::RParen, "')'");
            if (!at(Tok::LBrace)) fail(cur().loc, "'catch' needs { } around its code", "for example: catch (Error e) { print(e.message); }");
            s->els = parseBlock();
            return s;
        }
        case Tok::KwThrow: {
            p++;
            auto s = std::make_unique<Stmt>(SK::Throw, l);
            s->expr = parseExpr();
            expect(Tok::Semi, "';'");
            return s;
        }
        case Tok::KwCatch: fail(l, "this 'catch' doesn't belong to a 'try'", "catch has to come right after the } of a try");
        case Tok::KwReturn: {
            p++;
            auto s = std::make_unique<Stmt>(SK::Return, l);
            if (!at(Tok::Semi)) s->expr = parseExpr();
            expect(Tok::Semi, "';'");
            return s;
        }
        case Tok::KwBreak:
        case Tok::KwContinue: {
            p++;
            auto s = std::make_unique<Stmt>(prev().kind == Tok::KwBreak ? SK::Break : SK::Continue, l);
            expect(Tok::Semi, "';'");
            return s;
        }
        case Tok::KwSuper:
            if (peek().kind == Tok::LParen) {
                p += 2;
                auto s = std::make_unique<Stmt>(SK::SuperCall, l);
                if (!at(Tok::RParen)) {
                    do {
                        s->args.push_back(parseExpr());
                    } while (accept(Tok::Comma));
                }
                expect(Tok::RParen, "')'");
                expect(Tok::Semi, "';'");
                return s;
            }
            break;
        case Tok::KwElse: fail(l, "this 'else' doesn't belong to an 'if'", "else has to come right after the } of an if");
        case Tok::KwCase:
        case Tok::KwDefault: fail(l, "'" + cur().text + "' can only be used inside a switch");
        case Tok::KwStruct:
        case Tok::KwClass:
        case Tok::KwEnum:
        case Tok::KwInterface:
            fail(l, cur().text + " types have to be made outside of functions", "move it to the top level of the file");
        default: break;
    }
    StmtPtr s = tryWordStatement();
    if (!s) s = (at(Tok::KwConst) || declAhead(false)) ? parseVarStmt() : parseSimple();
    expect(Tok::Semi, "';'");
    return s;
}

StmtPtr Parser::parseIf() {
    auto s = std::make_unique<Stmt>(SK::If, cur().loc);
    p++;
    expect(Tok::LParen, "'(' after if");
    s->expr = parseCond();
    expect(Tok::RParen, "')'");
    s->then = parseBody("if");
    if (accept(Tok::KwElse)) s->els = at(Tok::KwIf) ? parseIf() : parseBody("else");
    return s;
}

StmtPtr Parser::parseFor() {
    Loc l = cur().loc;
    p++;
    expect(Tok::LParen, "'(' after for");

    // for (Type name in list) { ... }
    if (declAhead(false)) {
        size_t save = p, splits = splitLog.size();
        TypeRef type = parseType();
        const Token& name = expect(Tok::Ident, "a name");
        if (accept(Tok::KwIn)) {
            auto s = std::make_unique<Stmt>(SK::ForEach, l);
            s->var = std::make_unique<VarDecl>();
            s->var->name = name.text;
            s->var->loc = name.loc;
            s->var->typeRef = type;
            s->expr = parseExpr();
            if (accept(Tok::DotDot)) s->value = parseExpr();  // for (int i in 0..10): counts 0 to 10, both included
            expect(Tok::RParen, "')'");
            s->then = parseBody("for");
            return s;
        }
        while (splitLog.size() > splits) {
            t[splitLog.back().first] = splitLog.back().second;
            splitLog.pop_back();
        }
        p = save;
    }

    auto s = std::make_unique<Stmt>(SK::For, l);
    if (!at(Tok::Semi)) {
        s->init = tryWordStatement();
        if (!s->init) s->init = (at(Tok::KwConst) || declAhead(false)) ? parseVarStmt() : parseSimple();
    }
    expect(Tok::Semi, "';'");
    if (!at(Tok::Semi)) s->expr = parseCond();
    expect(Tok::Semi, "';'");
    if (!at(Tok::RParen)) s->step = parseSimple();
    expect(Tok::RParen, "')'");
    s->then = parseBody("for");
    return s;
}

StmtPtr Parser::parseSwitch() {
    auto s = std::make_unique<Stmt>(SK::Switch, cur().loc);
    p++;
    expect(Tok::LParen, "'(' after switch");
    s->expr = parseExpr();
    expect(Tok::RParen, "')'");
    const Token& open = expect(Tok::LBrace, "'{'");
    while (!at(Tok::RBrace)) {
        if (atEnd()) fail(open.loc, "this '{' is never closed", "add a matching '}'");
        SwitchCase c;
        c.loc = cur().loc;
        if (accept(Tok::KwDefault)) {
            c.isDefault = true;
        } else if (accept(Tok::KwCase)) {
            do {
                c.values.push_back(parseExpr());
            } while (accept(Tok::Comma));
        } else {
            fail(cur().loc, "expected 'case' or 'default' inside the switch, but found " + describe(cur()));
        }
        expect(Tok::Colon, "':'");
        auto body = std::make_unique<Stmt>(SK::Block, c.loc);
        while (!at(Tok::KwCase) && !at(Tok::KwDefault) && !at(Tok::RBrace)) {
            if (atEnd()) fail(open.loc, "this '{' is never closed", "add a matching '}'");
            body->body.push_back(parseStmt());
        }
        c.body = std::move(body);
        s->cases.push_back(std::move(c));
    }
    p++;
    return s;
}

StmtPtr Parser::parseVarStmt() {
    bool isConst = accept(Tok::KwConst);
    TypeRef type = parseType();
    const Token& name = expect(Tok::Ident, "a name");
    auto s = std::make_unique<Stmt>(SK::Var, name.loc);
    s->var = finishVar(isConst, type, name);
    return s;
}

// Word versions of assignments:
//   set x to 5;          x = 5;          increase x by 2;   x += 2;      increase x;   x++;
//   multiply x by 2;     x *= 2;         decrease x by 2;   x -= 2;      decrease x;   x--;
//   divide x by 2;       x /= 2;
// Only used when the `to` / `by` is really there, so `set(...)` or a variable named `increase` still work.
StmtPtr Parser::tryWordStatement() {
    if (!at(Tok::Ident)) return nullptr;
    const std::string& w = cur().text;
    bool canBeBare = w == "increase" || w == "decrease";
    const char* joiner = w == "set" ? "to" : (canBeBare || w == "multiply" || w == "divide") ? "by" : nullptr;
    if (!joiner) return nullptr;
    if (peek().kind != Tok::Ident && peek().kind != Tok::KwThis) return nullptr;

    // Look ahead (without reporting anything) for the `to` / `by`, or the end of the statement.
    bool found = false, bare = false;
    int depth = 0;
    for (size_t k = p + 1; k < t.size(); k++) {
        Tok kind = t[k].kind;
        if (kind == Tok::LParen || kind == Tok::LBracket || kind == Tok::LBrace) {
            depth++;
        } else if (kind == Tok::RParen || kind == Tok::RBracket || kind == Tok::RBrace) {
            if (depth == 0) {
                bare = kind == Tok::RParen;  // the end of a for (...) header
                break;
            }
            depth--;
        } else if (depth == 0 && kind == Tok::Ident && t[k].text == joiner) {
            found = true;
            break;
        } else if (depth == 0 && kind == Tok::Semi) {
            bare = true;
            break;
        } else if (depth == 0 && (kind == Tok::Assign || kind == Tok::Eof)) {
            break;
        }
    }
    if (!found && !(bare && canBeBare)) return nullptr;

    Loc l = cur().loc;
    p++;
    ExprPtr target = parsePostfix();
    if (!found) {
        auto s = std::make_unique<Stmt>(SK::IncDec, l);
        s->op = w == "increase" ? "++" : "--";
        s->expr = std::move(target);
        return s;
    }
    p++;  // to / by
    auto s = std::make_unique<Stmt>(SK::Assign, l);
    s->op = w == "set" ? "=" : w == "increase" ? "+=" : w == "decrease" ? "-=" : w == "multiply" ? "*=" : "/=";
    s->expr = std::move(target);
    s->value = parseExpr();
    return s;
}

// A statement that starts with an expression: a call, an assignment, or x++ / x--.
StmtPtr Parser::parseSimple() {
    if (StmtPtr w = tryWordStatement()) return w;
    Loc l = cur().loc;
    if (at(Tok::PlusPlus) || at(Tok::MinusMinus)) {
        auto s = std::make_unique<Stmt>(SK::IncDec, l);
        s->op = cur().text;
        p++;
        s->expr = parsePostfix();
        return s;
    }
    ExprPtr e = parseExpr();
    switch (cur().kind) {
        case Tok::Assign:
        case Tok::PlusEq:
        case Tok::MinusEq:
        case Tok::StarEq:
        case Tok::SlashEq:
        case Tok::PercentEq: {
            auto s = std::make_unique<Stmt>(SK::Assign, l);
            s->op = cur().text;
            p++;
            s->expr = std::move(e);
            s->value = parseExpr();
            return s;
        }
        case Tok::PlusPlus:
        case Tok::MinusMinus: {
            auto s = std::make_unique<Stmt>(SK::IncDec, l);
            s->op = cur().text;
            p++;
            s->expr = std::move(e);
            return s;
        }
        default: {
            auto s = std::make_unique<Stmt>(SK::ExprStmt, l);
            s->expr = std::move(e);
            return s;
        }
    }
}

ExprPtr Parser::binary(ExprPtr l, const Token& op, ExprPtr r, const std::string& text) {
    auto e = std::make_unique<Expr>(EK::Binary, op.loc);
    e->text = text;
    if (op.kind == Tok::Ident || op.kind == Tok::KwNot) {
        // Remember how it was written, for error messages.
        e->opWord = op.kind == Tok::KwNot ? "not equals" : op.text;
        if ((op.text == "greater" || op.text == "less") && text.size() == 2) e->opWord += " or equal";
    }
    e->a = std::move(l);
    e->b = std::move(r);
    return e;
}

// Word versions of operators (`a plus b`, `a greater or equal b`) are names right after a value - a place
// where a plain name can never go - so they don't stop anyone from using those words as names elsewhere.
bool Parser::atWord(const char* w) const { return at(Tok::Ident) && t[p].text == w; }

ExprPtr Parser::parseTernary() {
    ExprPtr first = parseCoalesce();
    // Python-style: `a if condition else b`. The `if` has to be on the same line, so a missing ';' before
    // an if statement on the next line is still reported as a missing ';'.
    if (at(Tok::KwIf) && cur().loc.line == prev().loc.line) {
        auto e = std::make_unique<Expr>(EK::Ternary, cur().loc);
        p++;
        e->b = std::move(first);
        e->a = parseCoalesce();
        expect(Tok::KwElse, "'else' (a if condition else b picks a when the condition is true, b when it's false)");
        e->c = parseTernary();
        return e;
    }
    if (!at(Tok::Question)) return first;
    auto e = std::make_unique<Expr>(EK::Ternary, cur().loc);
    p++;
    e->a = std::move(first);
    e->b = parseExpr();
    expect(Tok::Colon, "':' (a ? b : c picks b when a is true, c when it's false)");
    e->c = parseTernary();
    return e;
}

ExprPtr Parser::parseCoalesce() {
    ExprPtr l = parseOr();
    while (at(Tok::QuestionQuestion) || atWord("otherwise")) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseOr(), "??");
    }
    return l;
}

ExprPtr Parser::parseOr() {
    ExprPtr l = parseAnd();
    while (at(Tok::OrOr) || at(Tok::KwOr)) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseAnd(), "||");
    }
    return l;
}

ExprPtr Parser::parseAnd() {
    ExprPtr l = parseNot();
    while (at(Tok::AndAnd) || at(Tok::KwAnd)) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseNot(), "&&");
    }
    return l;
}

// `not` binds looser than comparisons (like Python): `not a == b` means `not (a == b)`.
ExprPtr Parser::parseNot() {
    if (!at(Tok::KwNot)) return parseBitOr();
    auto e = std::make_unique<Expr>(EK::Unary, cur().loc);
    p++;
    e->text = "!";
    e->a = parseNot();
    return e;
}

ExprPtr Parser::parseBitOr() {
    ExprPtr l = parseBitXor();
    while (at(Tok::Pipe) || atWord("bitor")) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseBitXor(), "|");
    }
    return l;
}

ExprPtr Parser::parseBitXor() {
    ExprPtr l = parseBitAnd();
    while (at(Tok::Caret) || atWord("bitxor")) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseBitAnd(), "^");
    }
    return l;
}

ExprPtr Parser::parseBitAnd() {
    ExprPtr l = parseEquality();
    while (at(Tok::Amp) || atWord("bitand")) {
        const Token& op = t[p++];
        l = binary(std::move(l), op, parseEquality(), "&");
    }
    return l;
}

ExprPtr Parser::parseEquality() {
    ExprPtr l = parseCompare();
    while (true) {
        if (at(Tok::EqEq) || at(Tok::NotEq) || atWord("equals")) {
            const Token& op = t[p++];
            l = binary(std::move(l), op, parseCompare(), op.kind == Tok::NotEq ? "!=" : "==");
        } else if (at(Tok::KwNot) && peek().kind == Tok::Ident && peek().text == "equals") {  // a not equals b
            const Token& op = t[p];
            p += 2;
            l = binary(std::move(l), op, parseCompare(), "!=");
        } else {
            return l;
        }
    }
}

ExprPtr Parser::parseCompare() {
    ExprPtr l = parseShift();
    while (at(Tok::Lt) || at(Tok::Gt) || at(Tok::LtEq) || at(Tok::GtEq) || at(Tok::KwIs) || at(Tok::KwAs) || atWord("greater") ||
           atWord("less")) {
        if (at(Tok::Ident)) {  // greater, less, greater or equal, less or equal
            const Token& op = t[p++];
            bool orEqual = at(Tok::KwOr) && peek().kind == Tok::Ident && peek().text == "equal";
            if (orEqual) p += 2;
            std::string sym = std::string(op.text == "greater" ? ">" : "<") + (orEqual ? "=" : "");
            l = binary(std::move(l), op, parseShift(), sym);
            continue;
        }
        const Token& op = t[p++];
        if (op.kind == Tok::KwIs || op.kind == Tok::KwAs) {
            auto e = std::make_unique<Expr>(op.kind == Tok::KwIs ? EK::Is : EK::As, op.loc);
            e->a = std::move(l);
            e->typeRef = parseType();
            // In `x is Goblin ? a : b` the ? belongs to the ternary, not to the type.
            if (e->typeRef.optional && !at(Tok::RParen) && !at(Tok::Semi) && !at(Tok::Comma) && !at(Tok::RBracket) &&
                !at(Tok::RBrace) && !atEnd()) {
                p--;
                e->typeRef.optional = false;
            }
            l = std::move(e);
            continue;
        }
        l = binary(std::move(l), op, parseShift(), op.text);
    }
    return l;
}

ExprPtr Parser::parseShift() {
    ExprPtr l = parseAdd();
    while (at(Tok::Shl) || at(Tok::Shr) || atWord("shiftleft") || atWord("shiftright")) {
        const Token& op = t[p++];
        std::string sym = op.text == "shiftleft" ? "<<" : op.text == "shiftright" ? ">>" : op.text;
        l = binary(std::move(l), op, parseAdd(), sym);
    }
    return l;
}

ExprPtr Parser::parseAdd() {
    ExprPtr l = parseMul();
    while (at(Tok::Plus) || at(Tok::Minus) || atWord("plus") || atWord("minus")) {
        const Token& op = t[p++];
        std::string sym = op.text == "plus" ? "+" : op.text == "minus" ? "-" : op.text;
        l = binary(std::move(l), op, parseMul(), sym);
    }
    return l;
}

ExprPtr Parser::parseMul() {
    ExprPtr l = parseUnary();
    while (at(Tok::Star) || at(Tok::Slash) || at(Tok::Percent) || atWord("times") || atWord("div") || atWord("mod")) {
        const Token& op = t[p++];
        std::string sym = op.text == "times" ? "*" : op.text == "div" ? "/" : op.text == "mod" ? "%" : op.text;
        l = binary(std::move(l), op, parseUnary(), sym);
    }
    return l;
}

ExprPtr Parser::parseUnary() {
    if (at(Tok::Minus) || at(Tok::Bang) || at(Tok::Tilde) || at(Tok::KwNot) || at(Tok::KwNegative) || at(Tok::KwBitNot)) {
        auto e = std::make_unique<Expr>(EK::Unary, cur().loc);
        e->text = at(Tok::KwNot) ? "!" : at(Tok::KwNegative) ? "-" : at(Tok::KwBitNot) ? "~" : cur().text;
        p++;
        e->a = parseUnary();
        return e;
    }
    return parsePostfix();
}

ExprPtr Parser::parsePostfix() {
    ExprPtr e = parsePrimary();
    while (true) {
        if (at(Tok::LParen)) {
            p++;
            auto call = std::make_unique<Expr>(EK::Call, e->loc);
            call->a = std::move(e);
            if (!at(Tok::RParen)) {
                do {
                    call->args.push_back(parseExpr());
                } while (accept(Tok::Comma));
            }
            expect(Tok::RParen, "')'");
            e = std::move(call);
        } else if (at(Tok::Dot)) {
            p++;
            const Token& name = expect(Tok::Ident, "a name after '.'");
            auto m = std::make_unique<Expr>(EK::Member, name.loc);
            m->text = name.text;
            m->a = std::move(e);
            e = std::move(m);
        } else if (at(Tok::LBracket)) {
            auto idx = std::make_unique<Expr>(EK::Index, cur().loc);
            p++;
            idx->a = std::move(e);
            idx->b = parseExpr();
            expect(Tok::RBracket, "']'");
            e = std::move(idx);
        } else {
            return e;
        }
    }
}

ExprPtr Parser::parseBraceLiteral() {
    const Token& open = t[p++];
    auto e = std::make_unique<Expr>(EK::BraceLit, open.loc);
    if (accept(Tok::RBrace)) return e;
    ExprPtr first = parseExpr();
    if (accept(Tok::Colon)) {
        e->kind = EK::MapLit;
        e->keys.push_back(std::move(first));
        e->args.push_back(parseExpr());
        while (accept(Tok::Comma)) {
            if (at(Tok::RBrace)) break;
            e->keys.push_back(parseExpr());
            expect(Tok::Colon, "':' between a key and its value");
            e->args.push_back(parseExpr());
        }
    } else {
        e->args.push_back(std::move(first));
        while (accept(Tok::Comma)) {
            if (at(Tok::RBrace)) break;
            e->args.push_back(parseExpr());
        }
    }
    expect(Tok::RBrace, "'}'");
    return e;
}

ExprPtr Parser::parsePrimary() {
    const Token& k = cur();
    switch (k.kind) {
        case Tok::IntLit:
        case Tok::FloatLit: {
            p++;
            auto e = std::make_unique<Expr>(k.kind == Tok::IntLit ? EK::Int : EK::Float, k.loc);
            e->text = k.text;
            return e;
        }
        case Tok::StrLit: p++; return parseString(k);
        case Tok::RawStrLit: {
            p++;
            auto e = std::make_unique<Expr>(EK::Str, k.loc);
            e->parts.push_back(k.text);
            return e;
        }
        case Tok::KwTrue:
        case Tok::KwFalse: {
            p++;
            auto e = std::make_unique<Expr>(EK::Bool, k.loc);
            e->boolVal = k.kind == Tok::KwTrue;
            return e;
        }
        case Tok::KwNull: p++; return std::make_unique<Expr>(EK::Null, k.loc);
        case Tok::KwThis: p++; return std::make_unique<Expr>(EK::This, k.loc);
        case Tok::KwSuper:
            p++;
            if (!at(Tok::Dot)) fail(k.loc, "super is used like super.method() or super(...) in a constructor");
            return std::make_unique<Expr>(EK::Super, k.loc);
        case Tok::LParen: {
            if (lambdaAhead()) return parseLambda();
            p++;
            ExprPtr e = parseExpr();
            if (at(Tok::Assign)) fail(cur().loc, "'=' sets a variable - to compare two values, use '=='");
            expect(Tok::RParen, "')'");
            return e;
        }
        case Tok::LBracket: {
            p++;
            auto e = std::make_unique<Expr>(EK::ListLit, k.loc);
            while (!at(Tok::RBracket)) {
                e->args.push_back(parseExpr());
                if (!accept(Tok::Comma)) break;
            }
            expect(Tok::RBracket, "']'");
            return e;
        }
        case Tok::LBrace: return parseBraceLiteral();
        case Tok::Ident: {
            if (primitiveTypeByName(k.text)) {
                if (peek().kind != Tok::LParen) fail(k.loc, "'" + k.text + "' is a type, not a value");
                p += 2;
                auto e = std::make_unique<Expr>(EK::Cast, k.loc);
                e->text = k.text;
                e->a = parseExpr();
                if (at(Tok::Comma)) fail(cur().loc, k.text + "(...) converts just one value");
                expect(Tok::RParen, "')'");
                return e;
            }
            if (k.text == "list" || k.text == "map" || k.text == "Pointer")
                fail(k.loc, "'" + k.text + "' is a type, not a value",
                     k.text == "list" ? "make a list like this: list<int> numbers = [1, 2, 3];"
                     : k.text == "map" ? "make a map like this: map<string, int> ages = {\"Ana\": 12};"
                                       : "make a pointer like this: Pointer<int> p = score;");
            p++;
            auto e = std::make_unique<Expr>(EK::Name, k.loc);
            e->text = k.text;
            return e;
        }
        case Tok::KwReserved:
            fail(k.loc, "'" + k.text + "' isn't available yet", "it's saved for a future version of Forge");
        default: fail(k.loc, "expected a value, but found " + describe(k));
    }
}

// Splits "HP: {hp}/{maxHp}" into text parts and the expressions inside { }.
ExprPtr Parser::parseString(const Token& tok) {
    auto e = std::make_unique<Expr>(EK::Str, tok.loc);
    const std::string& raw = tok.text;
    int baseCol = tok.loc.col + 1;
    auto locAt = [&](size_t idx, int len) { return Loc{&file, tok.loc.line, baseCol + (int)idx, len}; };
    std::string part;
    size_t i = 0;
    while (i < raw.size()) {
        char c = raw[i];
        if (c == '\\') {
            char n = raw[i + 1];
            switch (n) {
                case 'n': part += '\n'; break;
                case 't': part += '\t'; break;
                case 'r': part += '\r'; break;
                case '\\': part += '\\'; break;
                case '"': part += '"'; break;
                case '\'': part += '\''; break;
                case '{': part += '{'; break;
                case '}': part += '}'; break;
                default:
                    fail(locAt(i, 2), std::string("unknown escape '\\") + n + "'",
                         "the escapes are \\n (new line), \\t (tab), \\\\ (backslash), \\\" (quote), \\{ and \\}");
            }
            i += 2;
        } else if (c == '{') {
            if (i + 1 < raw.size() && raw[i + 1] == '{') {
                part += '{';
                i += 2;
                continue;
            }
            size_t close = i + 1;
            for (; close < raw.size() && raw[close] != '}'; close++) {
                if (raw[close] == '\\') {
                    close++;
                    continue;
                }
                if (raw[close] != '"') continue;
                for (close++; close < raw.size() && raw[close] != '"'; close++)
                    if (raw[close] == '\\') close++;
            }
            if (close >= raw.size()) close = std::string::npos;
            if (close == std::string::npos)
                fail(locAt(i, 1), "this { is never closed", "to show a { in the text, write {{ or \\{");
            std::string inner = raw.substr(i + 1, close - i - 1);
            // Text inside the interpolation, like {int(\"42\")}, keeps its quotes escaped in the source.
            for (size_t q; (q = inner.find("\\\"")) != std::string::npos;) inner.erase(q, 1);
            // {value:.2} shows a decimal number with 2 decimals. Forge numbers never start with a dot, so ": ." can't be
            // the end of a ternary (a ? b : c), or anything else.
            int decimals = -1;
            {
                int depth = 0;
                size_t colon = std::string::npos;
                for (size_t k = 0; k < inner.size(); k++) {
                    char ch = inner[k];
                    if (ch == '"') {
                        for (k++; k < inner.size() && inner[k] != '"'; k++)
                            if (inner[k] == '\\') k++;
                    } else if (ch == '(' || ch == '[' || ch == '{') {
                        depth++;
                    } else if (ch == ')' || ch == ']' || ch == '}') {
                        depth--;
                    } else if (ch == ':' && depth == 0) {
                        colon = k;
                    }
                }
                if (colon != std::string::npos) {
                    std::string fmt = inner.substr(colon + 1);
                    size_t a = fmt.find_first_not_of(" \t"), b = fmt.find_last_not_of(" \t");
                    fmt = a == std::string::npos ? "" : fmt.substr(a, b - a + 1);
                    if (!fmt.empty() && fmt[0] == '.') {
                        if (fmt.size() != 2 || fmt[1] < '0' || fmt[1] > '9')
                            fail(locAt(i + 1 + colon, (int)(inner.size() - colon)), "':" + fmt + "' isn't a way to show a number",
                                 "write :. and how many decimals to show, from 0 to 9, like {speed:.2}");
                        decimals = fmt[1] - '0';
                        inner = inner.substr(0, colon);
                    }
                }
            }
            if (inner.find_first_not_of(" \t") == std::string::npos)
                fail(locAt(i, (int)(close - i + 1)), "nothing inside { }", "put a value inside, like {score}");
            Parser sub(file, lexFragment(file, inner, tok.loc.line, baseCol + (int)i + 1), prelude);
            ExprPtr ex = sub.parseExpr();
            if (!sub.atEnd()) fail(sub.cur().loc, "unexpected " + sub.describe(sub.cur()) + " inside { }");
            e->parts.push_back(part);
            part.clear();
            e->args.push_back(std::move(ex));
            e->decimals.push_back(decimals);
            i = close + 1;
        } else if (c == '}') {
            if (i + 1 < raw.size() && raw[i + 1] == '}') {
                part += '}';
                i += 2;
                continue;
            }
            fail(locAt(i, 1), "this } doesn't match a {", "to show a } in the text, write }} or \\}");
        } else {
            part += c;
            i++;
        }
    }
    e->parts.push_back(part);
    return e;
}

}  // namespace

FileAST parseSource(const SourceFile& file, bool prelude) {
    try {
        Parser parser(file, lexSource(file), prelude);
        return parser.parseFile();
    } catch (CompileAbort&) {
        FileAST ast;
        ast.ok = false;
        return ast;
    }
}
