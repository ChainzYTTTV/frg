#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, Tok>& keywords() {
    static const std::unordered_map<std::string, Tok> table = {
        {"if", Tok::KwIf},         {"else", Tok::KwElse},         {"while", Tok::KwWhile},
        {"do", Tok::KwDo},         {"interface", Tok::KwInterface}, {"abstract", Tok::KwAbstract},
        {"try", Tok::KwTry},       {"catch", Tok::KwCatch},         {"throw", Tok::KwThrow},
        {"for", Tok::KwFor},       {"return", Tok::KwReturn},     {"break", Tok::KwBreak},
        {"continue", Tok::KwContinue}, {"true", Tok::KwTrue},     {"false", Tok::KwFalse},
        {"null", Tok::KwNull},     {"and", Tok::KwAnd},           {"or", Tok::KwOr},
        {"not", Tok::KwNot},       {"const", Tok::KwConst},       {"void", Tok::KwVoid},
        {"struct", Tok::KwStruct}, {"class", Tok::KwClass},       {"enum", Tok::KwEnum},
        {"switch", Tok::KwSwitch}, {"case", Tok::KwCase},         {"default", Tok::KwDefault},
        {"this", Tok::KwThis},     {"super", Tok::KwSuper},       {"in", Tok::KwIn},
        {"static", Tok::KwStatic}, {"new", Tok::KwReserved},      {"delete", Tok::KwReserved},
        {"public", Tok::KwPublic},   {"private", Tok::KwPrivate},  {"is", Tok::KwIs},  {"as", Tok::KwAs},
        {"negative", Tok::KwNegative}, {"bitnot", Tok::KwBitNot},
    };
    return table;
}

bool isKeyword(const std::string& word) { return keywords().count(word) != 0; }

namespace {

class Lexer {
public:
    Lexer(const SourceFile* f, const std::string& s, int line, int col) : f(f), s(s), line(line), col(col) {}
    std::vector<Token> run();

private:
    const SourceFile* f;
    const std::string& s;
    size_t i = 0;
    int line, col;
    std::vector<Token> out;

    char at(size_t k = 0) const { return i + k < s.size() ? s[i + k] : '\0'; }
    void adv(size_t n = 1) {
        while (n-- && i < s.size()) {
            if (s[i] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            i++;
        }
    }
    Loc here(int len = 1) const { return Loc{f, line, col, len}; }
    [[noreturn]] void fail(const Loc& l, const std::string& msg, const std::string& hint = "") {
        reportError(l, msg, hint);
        throw CompileAbort{};
    }
    void lexNumber();
    void lexString();
    void lexRawString();
    bool lexOperator();
};

bool isIdentStart(char c) { return std::isalpha((unsigned char)c) || c == '_'; }
bool isIdentChar(char c) { return std::isalnum((unsigned char)c) || c == '_'; }

std::vector<Token> Lexer::run() {
    while (i < s.size()) {
        char c = at();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            adv();
            continue;
        }
        if (c == '/' && at(1) == '/') {
            while (i < s.size() && at() != '\n') adv();
            continue;
        }
        if (c == '/' && at(1) == '*')
            fail(here(2), "Forge uses ``` for multi-line comments, not /* */",
                 "start the comment with ``` and end it with ```");
        if (c == '`') {
            if (at(1) == '`' && at(2) == '`') {
                Loc start = here(3);
                adv(3);
                while (true) {
                    if (i >= s.size()) fail(start, "this ``` comment is never closed", "add ``` where the comment should end");
                    if (at() == '`' && at(1) == '`' && at(2) == '`') {
                        adv(3);
                        break;
                    }
                    adv();
                }
                continue;
            }
            fail(here(), "a single ` isn't valid here", "multi-line comments use three backticks: ``` like this ```");
        }

        Loc start = here();
        size_t startIdx = i;
        if (isIdentStart(c)) {
            while (isIdentChar(at())) adv();
            std::string word = s.substr(startIdx, i - startIdx);
            start.len = (int)word.size();
            auto it = keywords().find(word);
            out.push_back({it != keywords().end() ? it->second : Tok::Ident, word, start});
            continue;
        }
        if (std::isdigit((unsigned char)c)) {
            lexNumber();
            continue;
        }
        if (c == '"' && at(1) == '"' && at(2) == '"') {
            lexRawString();
            continue;
        }
        if (c == '"') {
            lexString();
            continue;
        }
        if (c == '\'') fail(start, "text in Forge uses double quotes", "write \"hello\" instead of 'hello'");
        if (lexOperator()) continue;
        fail(start, std::string("unexpected character '") + c + "'");
    }
    out.push_back({Tok::Eof, "", here()});
    return out;
}

void Lexer::lexNumber() {
    Loc start = here();
    size_t startIdx = i;
    bool isFloat = false;
    auto digits = [&](auto pred) {
        while (pred(at()) || at() == '_') adv();
    };
    auto isDec = [](char c) { return c >= '0' && c <= '9'; };

    if (at() == '0' && (at(1) == 'x' || at(1) == 'X')) {
        adv(2);
        digits([](char c) { return std::isxdigit((unsigned char)c) != 0; });
    } else if (at() == '0' && (at(1) == 'b' || at(1) == 'B')) {
        adv(2);
        digits([](char c) { return c == '0' || c == '1'; });
    } else {
        digits(isDec);
        if (at() == '.' && isDec(at(1))) {
            isFloat = true;
            adv();
            digits(isDec);
        }
        if ((at() == 'e' || at() == 'E') && (isDec(at(1)) || ((at(1) == '+' || at(1) == '-') && isDec(at(2))))) {
            isFloat = true;
            adv(2);
            digits(isDec);
        }
    }
    std::string raw = s.substr(startIdx, i - startIdx);
    start.len = (int)raw.size();
    if (isIdentChar(at()))
        fail(here(), "unexpected letter right after a number", "put a space or an operator between them");
    std::string text;
    for (char c : raw)
        if (c != '_') text += c;
    if ((text.size() == 2) && (text[1] == 'x' || text[1] == 'X' || text[1] == 'b' || text[1] == 'B'))
        fail(start, "this number has no digits after " + text);
    out.push_back({isFloat ? Tok::FloatLit : Tok::IntLit, text, start});
}

// """raw text""": can span many lines, and is taken exactly as written - no {values}, no \ escapes.
// A line break right after the opening """ isn't part of the text. Handy for shader code.
void Lexer::lexRawString() {
    Loc start = here(3);
    adv(3);
    if (at() == '\r') adv();
    if (at() == '\n') adv();
    std::string text;
    while (true) {
        if (i >= s.size()) fail(start, "this \"\"\" text is never closed", "end it with another \"\"\"");
        if (at() == '"' && at(1) == '"' && at(2) == '"') break;
        if (at() != '\r') text += at();
        adv();
    }
    adv(3);
    out.push_back({Tok::RawStrLit, text, start});
}

void Lexer::lexString() {
    Loc start = here();
    adv();  // opening quote
    std::string raw;
    int depth = 0;  // inside {interpolation}, quotes start a nested string: "{name.contains("x")}"
    while (true) {
        char c = at();
        if (i >= s.size() || c == '\n')
            fail(start, "this text is missing its closing \"", "text has to start and end on the same line");
        if (c == '{') {
            if (depth == 0 && at(1) == '{') {
                raw += "{{";
                adv(2);
                continue;
            }
            depth++;
        } else if (c == '}' && depth > 0) {
            depth--;
        } else if (c == '"' && depth > 0) {
            raw += c;
            adv();
            while (at() != '"') {
                if (i >= s.size() || at() == '\n')
                    fail(start, "this text is missing its closing \"", "text has to start and end on the same line");
                if (at() == '\\') {
                    raw += at();
                    adv();
                }
                raw += at();
                adv();
            }
            raw += c;
            adv();
            continue;
        }
        if (c == '"') break;
        if (c == '\\') {
            if (i + 1 >= s.size() || at(1) == '\n')
                fail(start, "this text is missing its closing \"", "text has to start and end on the same line");
            raw += c;
            raw += at(1);
            adv(2);
            continue;
        }
        raw += c;
        adv();
    }
    adv();  // closing quote
    start.len = (int)raw.size() + 2;
    out.push_back({Tok::StrLit, raw, start});
}

bool Lexer::lexOperator() {
    static const struct {
        const char* text;
        Tok kind;
    } ops[] = {
        {"++", Tok::PlusPlus}, {"--", Tok::MinusMinus}, {"+=", Tok::PlusEq}, {"-=", Tok::MinusEq},
        {"*=", Tok::StarEq},   {"/=", Tok::SlashEq},    {"%=", Tok::PercentEq}, {"==", Tok::EqEq},
        {"!=", Tok::NotEq},    {"<=", Tok::LtEq},       {">=", Tok::GtEq},   {"&&", Tok::AndAnd},
        {"||", Tok::OrOr},     {"<<", Tok::Shl},        {">>", Tok::Shr},    {"??", Tok::QuestionQuestion},
        {"..", Tok::DotDot},
        {"(", Tok::LParen},    {")", Tok::RParen},      {"{", Tok::LBrace},  {"}", Tok::RBrace},
        {"[", Tok::LBracket},  {"]", Tok::RBracket},    {",", Tok::Comma},   {";", Tok::Semi},
        {".", Tok::Dot},       {":", Tok::Colon},       {"?", Tok::Question}, {"+", Tok::Plus},
        {"-", Tok::Minus},     {"*", Tok::Star},        {"/", Tok::Slash},   {"%", Tok::Percent},
        {"=", Tok::Assign},    {"<", Tok::Lt},          {">", Tok::Gt},      {"!", Tok::Bang},
        {"&", Tok::Amp},       {"|", Tok::Pipe},        {"^", Tok::Caret},   {"~", Tok::Tilde},
        {"#", Tok::Hash},
    };
    for (const auto& op : ops) {
        size_t n = std::char_traits<char>::length(op.text);
        if (s.compare(i, n, op.text) == 0) {
            out.push_back({op.kind, op.text, here((int)n)});
            adv(n);
            return true;
        }
    }
    return false;
}

}  // namespace

std::vector<Token> lexSource(const SourceFile& file) {
    Lexer lx(&file, file.text, 1, 1);
    return lx.run();
}

std::vector<Token> lexFragment(const SourceFile& file, const std::string& text, int line, int col) {
    Lexer lx(&file, text, line, col);
    return lx.run();
}
