// frg fmt: tidies the layout of .frg files without changing what they do.
//
// It only changes whitespace and one brace: each line's indentation (4 spaces for each { } level, with a switch's
// cases one level in and their code one more), spaces at the ends of lines, runs of blank lines (at most one in a
// row), and a function's or method's opening brace (`int main(){`, Forge's style; `if (x) {` keeps its space).
// Lines inside ``` comments and """raw text""" are left exactly as they are. Lines that continue a statement
// (inside ( ) or [ ], or after a line that doesn't end the statement) keep their own alignment, moved along with
// the line they belong to. Afterwards the result is read again, and if a single token differs, nothing is changed.
#include "format.hpp"

#include "common.hpp"
#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <regex>

namespace {

// One line of the file, with its strings and comments blanked out (so braces in them don't count).
struct Line {
    std::string text;      // as written
    std::string code;      // the same length, with the inside of strings and comments replaced by spaces
    bool verbatim = false;  // starts inside a ``` comment or """raw text""": leave it exactly as it is
    bool endsInRaw = false;  // ends inside raw text, so spaces at its end are part of the text
};

// Splits the text into lines, working out which characters are code. Mirrors how the lexer reads strings:
// inside "...{value}..." a { } pair can hold code with its own "strings".
std::vector<Line> scan(const std::string& src) {
    std::vector<Line> lines;
    enum State { Code, BlockComment, RawText } state = Code;
    Line cur;
    size_t i = 0;
    auto flush = [&]() {
        cur.endsInRaw = state == RawText;
        lines.push_back(cur);
        cur = Line{};
        cur.verbatim = state != Code;
    };
    while (i < src.size()) {
        char c = src[i];
        if (c == '\n') {
            flush();
            i++;
            continue;
        }
        auto put = [&](char t, bool isCode) {
            cur.text += t;
            cur.code += isCode ? t : ' ';
        };
        if (state == BlockComment) {
            if (src.compare(i, 3, "```") == 0) {
                for (int k = 0; k < 3; k++) put('`', false);
                i += 3;
                state = Code;
            } else {
                put(c, false);
                i++;
            }
            continue;
        }
        if (state == RawText) {
            if (src.compare(i, 3, "\"\"\"") == 0) {
                for (int k = 0; k < 3; k++) put('"', false);
                i += 3;
                state = Code;
            } else {
                put(c, false);
                i++;
            }
            continue;
        }
        if (src.compare(i, 3, "```") == 0) {
            for (int k = 0; k < 3; k++) put('`', false);
            i += 3;
            state = BlockComment;
            continue;
        }
        if (src.compare(i, 3, "\"\"\"") == 0) {
            for (int k = 0; k < 3; k++) put('"', false);
            i += 3;
            state = RawText;
            continue;
        }
        if (c == '/' && i + 1 < src.size() && src[i + 1] == '/') {
            while (i < src.size() && src[i] != '\n') put(src[i++], false);
            continue;
        }
        if (c == '"') {  // a string, to the end of the line at most
            put(c, false);
            i++;
            int depth = 0;
            while (i < src.size() && src[i] != '\n') {
                char d = src[i];
                if (d == '\\' && i + 1 < src.size() && src[i + 1] != '\n') {
                    put(d, false);
                    put(src[i + 1], false);
                    i += 2;
                    continue;
                }
                if (d == '{') {
                    if (depth == 0 && i + 1 < src.size() && src[i + 1] == '{') {
                        put(d, false);
                        put(d, false);
                        i += 2;
                        continue;
                    }
                    depth++;
                } else if (d == '}' && depth > 0) {
                    depth--;
                } else if (d == '"' && depth > 0) {  // a string inside a {value}
                    put(d, false);
                    i++;
                    while (i < src.size() && src[i] != '"' && src[i] != '\n') {
                        if (src[i] == '\\' && i + 1 < src.size()) put(src[i++], false);
                        put(src[i++], false);
                    }
                    if (i < src.size() && src[i] == '"') put(src[i++], false);
                    continue;
                } else if (d == '"') {
                    put(d, false);
                    i++;
                    break;
                }
                put(d, false);
                i++;
            }
            continue;
        }
        put(c, true);
        i++;
    }
    cur.endsInRaw = state == RawText;
    lines.push_back(cur);
    return lines;
}

std::string trimRight(std::string s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.pop_back();
    return s;
}

// The width of a line's indentation (a tab counts as 4 spaces), and where its text starts.
int indentOf(const std::string& s, size_t& start) {
    int w = 0;
    start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) w += s[start++] == '\t' ? 4 : 1;
    return w;
}

bool startsWithWord(const std::string& s, const char* word) {
    size_t n = std::char_traits<char>::length(word);
    return s.compare(0, n, word) == 0 && (s.size() == n || !(std::isalnum((unsigned char)s[n]) || s[n] == '_'));
}

// A function's, method's or constructor's first line ending in ") {": the brace moves next to the ")".
// Not if, while, for, switch, catch or anything with an =, which keep "x) {".
bool isFunctionHead(const std::string& code) {
    static const std::regex head(R"(^\s*((private|static|abstract)\s+)*([A-Za-z_][\w.]*(<[^=;{}]*>)?\??\s+)?[A-Za-z_]\w*\s*\(([^()]*)\)\s*\{\s*$)");
    if (code.find('=') != std::string::npos) return false;
    size_t s;
    indentOf(code, s);
    std::string t = code.substr(s);
    for (const char* kw : {"if", "while", "for", "switch", "catch", "else", "return", "do"})
        if (startsWithWord(t, kw)) return false;
    return std::regex_match(code, head);
}

std::string formatText(const std::string& input) {
    std::string src = input;
    if (src.compare(0, 3, "\xEF\xBB\xBF") == 0) src.erase(0, 3);
    src.erase(std::remove(src.begin(), src.end(), '\r'), src.end());
    std::vector<Line> lines = scan(src);

    std::vector<std::string> out;
    std::vector<char> blocks;   // the { } blocks we're inside: 's' for a switch's, 'b' for any other
    int parens = 0;             // ( and [ still open
    bool continuing = false;    // the previous code line didn't end its statement
    int shift = 0;              // how far the current statement's first line moved (its continuation lines move too)
    int statementIndent = 0;    // where the current statement's first line is
    bool lastBlank = true;      // no blank lines at the very start

    // After each line: the { } blocks, brackets, and whether its statement goes on to the next line.
    auto update = [&](const std::string& code, const std::string& body) {
        size_t f = code.find_first_not_of(' ');
        bool switchLine = f != std::string::npos && startsWithWord(code.substr(f), "switch");
        for (char c : code) {
            if (c == '{') blocks.push_back(switchLine ? 's' : 'b');
            else if (c == '}' && !blocks.empty()) blocks.pop_back();
            else if (c == '(' || c == '[') parens++;
            else if ((c == ')' || c == ']') && parens > 0) parens--;
        }
        std::string end = trimRight(code);
        size_t e = end.find_first_not_of(' ');
        if (e == std::string::npos) return;  // no code on this line (blank, or only a comment): nothing changes
        char last = end.back();
        bool label = startsWithWord(body, "case") || startsWithWord(body, "default");
        continuing = parens == 0 && last != ';' && last != '{' && last != '}' && !label && end[e] != '#';
    };

    for (const Line& ln : lines) {
        if (ln.verbatim) {  // inside a ``` comment or raw text: exactly as written
            out.push_back(ln.text);
            lastBlank = false;
            update(ln.code, "");
            continue;
        }
        std::string text = ln.endsInRaw ? ln.text : trimRight(ln.text);
        if (text.empty()) {
            if (!lastBlank) out.push_back("");
            lastBlank = true;
            continue;
        }
        size_t start;
        int oldIndent = indentOf(text, start);
        std::string body = text.substr(start);
        std::string code = trimRight(ln.code);
        std::string codeBody = start < code.size() ? code.substr(start) : "";

        int level = (int)blocks.size();
        for (char b : blocks)
            if (b == 's') level++;  // the code under a case is one more level in
        if (!codeBody.empty() && codeBody[0] == '}') {
            level--;
            if (!blocks.empty() && blocks.back() == 's') level--;
        }
        if (!blocks.empty() && blocks.back() == 's' && (startsWithWord(body, "case") || startsWithWord(body, "default"))) level--;
        level = std::max(level, 0);

        int newIndent;
        bool statementStart = parens == 0 && !continuing;
        if (statementStart) {
            newIndent = level * 4;
            shift = newIndent - oldIndent;
            statementIndent = newIndent;
        } else {
            // keeps its alignment, moved along with its statement, but always further in than the statement's first
            // line (unless it starts by closing a bracket, like "});")
            newIndent = std::max(0, oldIndent + shift);
            bool closes = !codeBody.empty() && (codeBody[0] == ')' || codeBody[0] == ']' || codeBody[0] == '}');
            if (!closes) newIndent = std::max(newIndent, statementIndent + 4);
        }
        if (statementStart && isFunctionHead(code)) {
            size_t close = body.rfind(')');
            body = body.substr(0, close + 1) + "{";
        }
        out.push_back(std::string(newIndent, ' ') + body);
        lastBlank = false;
        update(code, body);
    }
    while (!out.empty() && out.back().empty()) out.pop_back();
    std::string result;
    for (const std::string& l : out) result += l + "\n";
    return result;
}

// The tokens of a text, to check that formatting changed nothing but whitespace. False if it can't be read.
bool tokensOf(const std::string& text, std::vector<std::pair<int, std::string>>& out) {
    SourceFile f;
    f.displayPath = "<fmt>";
    f.text = text;
    f.computeLines();
    try {
        for (const Token& t : lexSource(f)) out.push_back({(int)t.kind, t.text});
    } catch (...) {
        return false;
    }
    return true;
}

}  // namespace

FormatResult formatSource(const std::string& text) {
    FormatResult r;
    std::string clean = text;
    clean.erase(std::remove(clean.begin(), clean.end(), '\r'), clean.end());
    std::vector<std::pair<int, std::string>> before, after;
    setQuietOutput(true);
    bool readable = tokensOf(clean, before);
    if (!readable) {
        setQuietOutput(false);
        r.error = "it has a mistake Forge can't read past (like a text without its closing \"): fix that first";
        return r;
    }
    r.text = formatText(clean);
    bool same = tokensOf(r.text, after) && after == before;
    setQuietOutput(false);
    if (!same) {
        r.error = "formatting would have changed more than spaces (this is a bug in Forge), so it was left as it is";
        r.text.clear();
        return r;
    }
    r.ok = true;
    r.changed = r.text != clean;
    return r;
}
