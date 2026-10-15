#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "common.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static int g_errors = 0;
static bool g_color = false;

static const char* red() { return g_color ? "\x1b[1;31m" : ""; }
static const char* yellow() { return g_color ? "\x1b[1;33m" : ""; }
static const char* green() { return g_color ? "\x1b[1;32m" : ""; }
static const char* blue() { return g_color ? "\x1b[1;34m" : ""; }
static const char* cyan() { return g_color ? "\x1b[1;36m" : ""; }
static const char* bold() { return g_color ? "\x1b[1m" : ""; }
static const char* reset() { return g_color ? "\x1b[0m" : ""; }

void initConsole() {
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode) && SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        g_color = true;
}

void SourceFile::computeLines() {
    lineStarts = {0};
    for (size_t i = 0; i < text.size(); i++)
        if (text[i] == '\n') lineStarts.push_back(i + 1);
}

std::string SourceFile::getLine(int line) const {
    if (line < 1 || line > (int)lineStarts.size()) return "";
    size_t start = lineStarts[line - 1];
    size_t end = text.find('\n', start);
    if (end == std::string::npos) end = text.size();
    return text.substr(start, end - start);
}

static bool g_machine = false;

void setMachineOutput(bool on) { g_machine = on; }

static std::string machineEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\t') out += "\\t";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

// Prints one error or warning: the message, where it is (with the line and ^^^ under the spot), and the hint.
static void printDiagnostic(bool warning, const Loc& loc, const std::string& msg, const std::string& hint) {
    if (g_machine) {
        std::string path = loc.file ? loc.file->fullPath : "";
        std::printf("%s\t%s\t%d\t%d\t%d\t%s\t%s\n", warning ? "W" : "E", machineEscape(path).c_str(), loc.line, loc.col, loc.len,
                    machineEscape(msg).c_str(), machineEscape(hint).c_str());
        return;
    }
    const char* color = warning ? yellow() : red();
    std::fprintf(stderr, "%s%s:%s %s%s%s\n", color, warning ? "warning" : "error", reset(), bold(), msg.c_str(), reset());
    if (loc.file) {
        std::fprintf(stderr, "  %s-->%s %s:%d:%d\n", blue(), reset(), loc.file->displayPath.c_str(), loc.line, loc.col);
        std::string src = loc.file->getLine(loc.line);
        // Tabs are shown as 4 spaces; the caret padding follows the same rule so it lines up.
        std::string shown, pad;
        for (size_t i = 0; i < src.size(); i++) {
            unsigned char c = (unsigned char)src[i];
            shown += c == '\t' ? std::string("    ") : std::string(1, (char)c);
            bool continuationByte = (c & 0xC0) == 0x80;
            if ((int)i < loc.col - 1 && !continuationByte) pad += c == '\t' ? "    " : " ";
        }
        std::string num = std::to_string(loc.line);
        std::string gutter(num.size(), ' ');
        std::string carets(std::max(1, loc.len), '^');
        std::fprintf(stderr, " %s %s|%s\n", gutter.c_str(), blue(), reset());
        std::fprintf(stderr, " %s%s |%s %s\n", blue(), num.c_str(), reset(), shown.c_str());
        std::fprintf(stderr, " %s %s|%s %s%s%s%s\n", gutter.c_str(), blue(), reset(), pad.c_str(), color, carets.c_str(), reset());
    }
    if (!hint.empty()) std::fprintf(stderr, "  %shint:%s %s\n", cyan(), reset(), hint.c_str());
    std::fprintf(stderr, "\n");
}

void reportError(const Loc& loc, const std::string& msg, const std::string& hint) {
    g_errors++;
    printDiagnostic(false, loc, msg, hint);
}

struct Warning {
    Loc loc;
    std::string msg, hint;
};
static std::vector<Warning> g_warnings;

void reportWarning(const Loc& loc, const std::string& msg, const std::string& hint) {
    for (const Warning& w : g_warnings)
        if (w.loc.file == loc.file && w.loc.line == loc.line && w.loc.col == loc.col && w.msg == msg) return;
    g_warnings.push_back({loc, msg, hint});
}

void reportPlainWarning(const std::string& msg, const std::string& hint) {
    if (!g_machine) printDiagnostic(true, Loc{}, msg, hint);
}

int flushWarnings() {
    // For people, warnings wait until the errors are fixed, so the errors stand out. Editors get both.
    if (g_errors > 0 && !g_machine) return 0;
    std::stable_sort(g_warnings.begin(), g_warnings.end(), [](const Warning& a, const Warning& b) {
        std::string fa = a.loc.file ? a.loc.file->displayPath : "", fb = b.loc.file ? b.loc.file->displayPath : "";
        if (fa != fb) return fa < fb;
        return a.loc.line != b.loc.line ? a.loc.line < b.loc.line : a.loc.col < b.loc.col;
    });
    for (const Warning& w : g_warnings) printDiagnostic(true, w.loc, w.msg, w.hint);
    int n = (int)g_warnings.size();
    g_warnings.clear();
    return n;
}

void reportPlainError(const std::string& msg, const std::string& hint) {
    reportError(Loc{}, msg, hint);
}

void printStatus(const std::string& label, const std::string& msg) {
    std::fprintf(stderr, "%s%12s%s %s\n", green(), label.c_str(), reset(), msg.c_str());
}

int errorCount() { return g_errors; }

static std::string lower(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

static size_t editDistance(const std::string& a, const std::string& b) {
    std::vector<size_t> prev(b.size() + 1), cur(b.size() + 1);
    for (size_t j = 0; j <= b.size(); j++) prev[j] = j;
    for (size_t i = 1; i <= a.size(); i++) {
        cur[0] = i;
        for (size_t j = 1; j <= b.size(); j++) {
            size_t sub = prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
            cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, sub});
        }
        std::swap(prev, cur);
    }
    return prev[b.size()];
}

std::string suggestName(const std::string& wrong, const std::vector<std::string>& candidates) {
    std::string best;
    size_t bestDist = std::max<size_t>(1, wrong.size() / 3) + 1;
    for (const std::string& c : candidates) {
        if (c == wrong) continue;
        size_t d = editDistance(lower(wrong), lower(c));
        if (d < bestDist) {
            bestDist = d;
            best = c;
        }
    }
    return best;
}
