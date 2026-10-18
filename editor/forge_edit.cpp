// Forge Edit: a small, light code editor for Forge games (Windows and Linux), drawn with raylib.
//
//   forge-edit [game folder]
//
// The game's files are on the left; the bottom panel shows Problems (frg check, after every save) and Output (frg run).
// F5 runs the game, Shift+F5 stops it, F8 checks it, F1 opens Forge's docs (the docs folder next to bin/).
// Testing: FORGE_EDIT_OPEN=file opens a file, FORGE_EDIT_DOCS=1 starts in the docs, FORGE_EDIT_SHOT=picture.png
// FORGE_EDIT_FRAMES=n takes a screenshot after n frames and quits, FORGE_EDIT_RUN=1 presses Run at the start.
#include "raylib.h"

#include "process.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---------------------------------------------------------------- colors (dark)

static const Color kBg = {30, 31, 34, 255};
static const Color kPanel = {43, 45, 48, 255};
static const Color kPanelDark = {36, 37, 40, 255};
static const Color kBorder = {57, 59, 64, 255};
static const Color kText = {212, 212, 212, 255};
static const Color kDim = {133, 133, 133, 255};
static const Color kKeyword = {197, 134, 192, 255};
static const Color kType = {78, 201, 176, 255};
static const Color kString = {206, 145, 120, 255};
static const Color kNumber = {181, 206, 168, 255};
static const Color kComment = {106, 153, 85, 255};
static const Color kFunc = {220, 220, 170, 255};
static const Color kDirective = {86, 156, 214, 255};
static const Color kSelection = {38, 79, 120, 255};
static const Color kCurLine = {42, 45, 46, 255};
static const Color kCursor = {174, 175, 173, 255};
static const Color kError = {241, 76, 76, 255};
static const Color kWarning = {204, 167, 0, 255};
static const Color kAccent = {255, 140, 40, 255};   // Forge's orange
static const Color kFound = {98, 81, 33, 255};

// ---------------------------------------------------------------- text helpers (UTF-8)

static bool isCont(unsigned char c) { return (c & 0xC0) == 0x80; }
static int nextPos(const std::string& s, int i) {
    if (i >= (int)s.size()) return (int)s.size();
    i++;
    while (i < (int)s.size() && isCont((unsigned char)s[i])) i++;
    return i;
}
static int prevPos(const std::string& s, int i) {
    if (i <= 0) return 0;
    i--;
    while (i > 0 && isCont((unsigned char)s[i])) i--;
    return i;
}
static int visCol(const std::string& s, int i) {
    int n = 0;
    for (int k = 0; k < i && k < (int)s.size(); k++)
        if (!isCont((unsigned char)s[k])) n++;
    return n;
}
static int byteAtCol(const std::string& s, int col) {
    int k = 0, n = 0;
    while (k < (int)s.size() && n < col) k = nextPos(s, k), n++;
    return k;
}
static bool isWord(unsigned char c) { return std::isalnum(c) || c == '_' || c >= 0x80; }
static std::string lower(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
static std::string readFileText(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string cur;
    for (char c : text) {
        if (c == '\n') {
            lines.push_back(cur);
            cur.clear();
        } else if (c != '\r') {
            cur += c;
        }
    }
    lines.push_back(cur);
    return lines;
}

// ---------------------------------------------------------------- fonts

struct Fonts {
    Font mono{};
    Font sans{};
    float size = 18;     // the code's size; Ctrl+= and Ctrl+- change it
    float charW = 10;
    float lineH = 24;
    fs::path monoPath, sansPath;
};
static Fonts F;

static std::vector<int> fontCodepoints() {
    std::vector<int> cps;
    for (int c = 32; c < 127; c++) cps.push_back(c);
    for (int c = 160; c < 256; c++) cps.push_back(c);
    for (int c : {0x2014, 0x2013, 0x2192, 0x2190, 0x2191, 0x2193, 0x2022, 0x2026, 0x2713, 0x2717, 0x00D7, 0x2264, 0x2265, 0x2260,
                  0x201C, 0x201D, 0x2018, 0x2019, 0x25B6, 0x25A0})
        cps.push_back(c);
    return cps;
}

static void loadMono() {
    if (F.mono.texture.id) UnloadFont(F.mono);
    std::vector<int> cps = fontCodepoints();
    F.mono = F.monoPath.empty() ? GetFontDefault() : LoadFontEx(F.monoPath.string().c_str(), (int)F.size, cps.data(), (int)cps.size());
    if (!F.monoPath.empty()) SetTextureFilter(F.mono.texture, TEXTURE_FILTER_POINT);
    F.charW = MeasureTextEx(F.mono, "M", F.size, 0).x;
    F.lineH = std::round(F.size * 1.4f);
}

static void loadFonts(const fs::path& exeDir) {
    for (const fs::path& dir : {exeDir / "fonts", exeDir.parent_path() / "editor" / "fonts"}) {
        if (F.monoPath.empty() && fs::exists(dir / "DejaVuSansMono.ttf")) F.monoPath = dir / "DejaVuSansMono.ttf";
        if (F.sansPath.empty() && fs::exists(dir / "DejaVuSans.ttf")) F.sansPath = dir / "DejaVuSans.ttf";
    }
    loadMono();
    std::vector<int> cps = fontCodepoints();
    if (!F.sansPath.empty()) {
        F.sans = LoadFontEx(F.sansPath.string().c_str(), 40, cps.data(), (int)cps.size());
        SetTextureFilter(F.sans.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        F.sans = F.mono;
    }
}

static void text(const Font& f, const std::string& s, float x, float y, float size, Color c) {
    DrawTextEx(f, s.c_str(), {std::floor(x), std::floor(y)}, size, 0, c);
}
static float textW(const Font& f, const std::string& s, float size) { return MeasureTextEx(f, s.c_str(), size, 0).x; }
static void ui(const std::string& s, float x, float y, Color c = kText, float size = 15) { text(F.sans, s, x, y, size, c); }
static float uiW(const std::string& s, float size = 15) { return textW(F.sans, s, size); }

// ---------------------------------------------------------------- Forge's colors

enum class Tok { Plain, Keyword, Type, String, Number, Comment, Func, Directive };
struct Span {
    int from, to;
    Tok tok;
};

static bool inList(const std::string& w, std::initializer_list<const char*> words) {
    for (const char* k : words)
        if (w == k) return true;
    return false;
}
static bool isKeyword(const std::string& w) {
    return inList(w, {"if", "else", "while", "for", "in", "do", "return", "break", "continue", "switch", "case", "default",
                      "class", "struct", "enum", "interface", "abstract", "static", "private", "const", "true", "false",
                      "null", "this", "is", "as", "and", "or", "not", "try", "catch", "throw", "task", "wait", "new",
                      "plus", "minus", "times", "div", "mod", "equals", "greater", "less", "negative", "bitnot", "super"});
}
static bool isTypeWord(const std::string& w) {
    if (inList(w, {"int", "float", "double", "bool", "string", "void", "list", "map", "vec2", "vec3", "color", "rect",
                   "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "Function", "Weak"}))
        return true;
    return !w.empty() && std::isupper((unsigned char)w[0]);
}

// state: 0 normal, 1 inside a ``` comment, 2 inside a """ string
static void highlight(const std::string& s, int& state, std::vector<Span>& out) {
    out.clear();
    int n = (int)s.size(), i = 0;
    auto closeBlock = [&](const char* mark, Tok t, int from) {
        size_t end = s.find(mark, (size_t)from);
        if (end == std::string::npos) {
            out.push_back({i, n, t});
            i = n;
        } else {
            out.push_back({i, (int)end + 3, t});
            i = (int)end + 3;
            state = 0;
        }
    };
    if (state == 1) closeBlock("```", Tok::Comment, 0);
    else if (state == 2) closeBlock("\"\"\"", Tok::String, 0);
    int firstNonSpace = 0;
    while (firstNonSpace < n && s[firstNonSpace] == ' ') firstNonSpace++;
    while (i < n) {
        char c = s[i];
        if (s.compare(i, 3, "```") == 0) {
            state = 1;
            int at = i;
            closeBlock("```", Tok::Comment, at + 3);
        } else if (s.compare(i, 3, "\"\"\"") == 0) {
            state = 2;
            int at = i;
            closeBlock("\"\"\"", Tok::String, at + 3);
        } else if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            out.push_back({i, n, Tok::Comment});
            i = n;
        } else if (c == '#' && i == firstNonSpace) {
            out.push_back({i, n, Tok::Directive});
            i = n;
        } else if (c == '"') {
            int j = i + 1;
            while (j < n && s[j] != '"') j += s[j] == '\\' ? 2 : 1;
            j = std::min(j + 1, n);
            out.push_back({i, j, Tok::String});
            i = j;
        } else if (std::isdigit((unsigned char)c)) {
            int j = i;
            while (j < n && (std::isalnum((unsigned char)s[j]) || s[j] == '.' || s[j] == '_')) {
                if (s[j] == '.' && j + 1 < n && s[j + 1] == '.') break;  // 0..10
                j++;
            }
            out.push_back({i, j, Tok::Number});
            i = j;
        } else if (isWord((unsigned char)c)) {
            int j = i;
            while (j < n && isWord((unsigned char)s[j])) j++;
            std::string w = s.substr(i, j - i);
            int k = j;
            while (k < n && s[k] == ' ') k++;
            Tok t = isKeyword(w) ? Tok::Keyword : isTypeWord(w) ? Tok::Type : (k < n && s[k] == '(') ? Tok::Func : Tok::Plain;
            out.push_back({i, j, t});
            i = j;
        } else {
            int j = nextPos(s, i);
            out.push_back({i, j, Tok::Plain});
            i = j;
        }
    }
}

static Color tokColor(Tok t) {
    switch (t) {
        case Tok::Keyword: return kKeyword;
        case Tok::Type: return kType;
        case Tok::String: return kString;
        case Tok::Number: return kNumber;
        case Tok::Comment: return kComment;
        case Tok::Func: return kFunc;
        case Tok::Directive: return kDirective;
        default: return kText;
    }
}

// ---------------------------------------------------------------- documents

struct Pos {
    int line = 0, col = 0;  // col is a byte in the line
    bool operator<(const Pos& o) const { return line != o.line ? line < o.line : col < o.col; }
    bool operator==(const Pos& o) const { return line == o.line && col == o.col; }
};
struct Snapshot {
    std::vector<std::string> lines;
    Pos cur;
};
struct Doc {
    fs::path path;
    std::vector<std::string> lines{""};
    Pos cur, anchor;
    bool sel = false;
    bool dirty = false;
    float scrollY = 0, scrollX = 0;
    std::vector<Snapshot> undo, redo;
    int lastKind = 0;
    double lastEdit = 0;
    int wantCol = -1;   // the column up/down try to keep
};

struct Problem {
    bool warning;
    fs::path file;
    int line, col, len;
    std::string message, hint;
};

// ---------------------------------------------------------------- the editor's state

enum class Panel { Problems, Output };
enum class Prompt { None, Find, NewFile, GoToLine, OpenFolder };

struct DocPage {
    fs::path path;
    std::string rel, title, textLower;
};
struct DocItem {   // one drawn piece of a laid-out docs page
    float x, y;
    std::string s;
    bool mono;
    float size;
    Color color;
    float boxH = 0;   // > 0: a code block's background, this tall
};

struct Editor {
    fs::path root, exeDir, frg, docsDir;
    std::vector<Doc> docs;
    int active = -1;
    std::vector<fs::path> files;   // the game's files, for the side bar
    double filesAt = -100;
    std::vector<Problem> problems;
    std::string output;
    Panel panel = Panel::Problems;
    float panelH = 190;
    float outputScroll = 0;
    bool followOutput = true;
    RunningProcess* run = nullptr;
    RunningProcess* check = nullptr;
    std::string checkOut;
    bool checkAgain = false;
    Prompt prompt = Prompt::None;
    std::string promptText, findText;
    std::string status;
    double statusAt = -100;
    bool dragging = false;
    double lastClick = 0;
    bool quitWarned = false;
    float sideScroll = 0;
    // docs
    bool docsMode = false;
    std::vector<DocPage> pages;
    int page = -1;
    std::string docSearch;
    std::vector<DocItem> layout;
    float layoutW = 0, layoutH = 0, docScroll = 0;
};
static Editor E;

static void say(const std::string& s) {
    E.status = s;
    E.statusAt = GetTime();
}

static Doc* cur() { return E.active >= 0 && E.active < (int)E.docs.size() ? &E.docs[E.active] : nullptr; }

static std::string relName(const fs::path& p) {
    std::error_code ec;
    fs::path r = fs::relative(p, E.root, ec);
    return ec || r.empty() ? p.filename().string() : r.generic_string();
}

static bool samePath(const fs::path& a, const fs::path& b) {
    std::error_code ec;
    if (fs::exists(a, ec) && fs::exists(b, ec)) return fs::equivalent(a, b, ec);
    return a.lexically_normal() == b.lexically_normal();
}

static bool isTextFile(const fs::path& p) {
    std::string e = lower(p.extension().string());
    return inList(e, {".frg", ".txt", ".md", ".json", ".csv", ".vert", ".frag", ".comp", ".geom", ".glsl", ".hlsl", ".cfg", ".ini", ""});
}

static void refreshFiles() {
    E.files.clear();
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(E.root, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        std::string name = it->path().filename().string();
        if (it->is_directory(ec)) {
            if (name == "build" || name == "dist" || name.starts_with(".")) it.disable_recursion_pending();
            continue;
        }
        if (!name.starts_with(".") && isTextFile(it->path())) E.files.push_back(it->path());
        if (E.files.size() > 2000) break;
    }
    std::sort(E.files.begin(), E.files.end(), [](const fs::path& a, const fs::path& b) {
        std::string ra = relName(a), rb = relName(b);
        bool ma = ra == "main.frg", mb = rb == "main.frg";
        if (ma != mb) return ma;
        int da = (int)std::count(ra.begin(), ra.end(), '/'), db = (int)std::count(rb.begin(), rb.end(), '/');
        if (da != db) return da < db;
        return lower(ra) < lower(rb);
    });
    E.filesAt = GetTime();
}

static void openFile(const fs::path& p, int line = -1, int col = 0) {
    for (int i = 0; i < (int)E.docs.size(); i++)
        if (samePath(E.docs[i].path, p)) {
            E.active = i;
            E.docsMode = false;
            if (line >= 0) {
                Doc& d = E.docs[i];
                d.cur = {std::clamp(line, 0, (int)d.lines.size() - 1), 0};
                d.cur.col = std::min(col, (int)d.lines[d.cur.line].size());
                d.sel = false;
                d.scrollY = std::max(0.0f, (d.cur.line - 5) * F.lineH);
            }
            return;
        }
    Doc d;
    d.path = p;
    std::string t = readFileText(p);
    // tabs become 4 spaces (the editor only writes spaces)
    std::string expanded;
    for (char c : t) expanded += c == '\t' ? std::string("    ") : std::string(1, c);
    d.lines = splitLines(expanded);
    if (line >= 0) {
        d.cur = {std::clamp(line, 0, (int)d.lines.size() - 1), 0};
        d.cur.col = std::min(col, (int)d.lines[d.cur.line].size());
        d.scrollY = std::max(0.0f, (d.cur.line - 5) * F.lineH);
    }
    E.docs.push_back(std::move(d));
    E.active = (int)E.docs.size() - 1;
    E.docsMode = false;
}

static void startCheck();

static bool saveDoc(Doc& d) {
    std::string out;
    for (size_t i = 0; i < d.lines.size(); i++) out += d.lines[i] + (i + 1 < d.lines.size() ? "\n" : "");
    std::ofstream f(d.path, std::ios::binary);
    if (!f) {
        say("couldn't save " + relName(d.path));
        return false;
    }
    f << out;
    d.dirty = false;
    return true;
}

static void saveAll() {
    for (Doc& d : E.docs)
        if (d.dirty) saveDoc(d);
}

// ---------------------------------------------------------------- frg: checking and running

static void startCheck() {
    if (E.check) {
        E.checkAgain = true;
        return;
    }
    if (!fs::exists(E.root / "main.frg")) return;
    E.checkOut.clear();
    E.check = startProcess({E.frg.string(), "check", E.root.string(), "--machine"}, E.root.string());
    if (!E.check) say("couldn't start frg - is Forge installed?");
}

static void finishCheck() {
    E.problems.clear();
    std::stringstream ss(E.checkOut);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::vector<std::string> f;
        size_t at = 0;
        for (;;) {
            size_t tab = line.find('\t', at);
            f.push_back(line.substr(at, tab == std::string::npos ? std::string::npos : tab - at));
            if (tab == std::string::npos) break;
            at = tab + 1;
        }
        if (f.size() < 6 || (f[0] != "E" && f[0] != "W")) continue;
        auto unescape = [](const std::string& s) {
            std::string o;
            for (size_t i = 0; i < s.size(); i++) {
                if (s[i] == '\\' && i + 1 < s.size()) {
                    char n = s[++i];
                    o += n == 't' ? '\t' : n == 'n' ? '\n' : n;
                } else {
                    o += s[i];
                }
            }
            return o;
        };
        Problem p{f[0] == "W", fs::path(unescape(f[1])), std::atoi(f[2].c_str()), std::atoi(f[3].c_str()),
                  std::atoi(f[4].c_str()), unescape(f[5]), f.size() > 6 ? unescape(f[6]) : ""};
        E.problems.push_back(p);
    }
    std::stable_sort(E.problems.begin(), E.problems.end(), [](const Problem& a, const Problem& b) { return !a.warning && b.warning; });
    int errors = (int)std::count_if(E.problems.begin(), E.problems.end(), [](const Problem& p) { return !p.warning; });
    if (errors && !E.run) E.panel = Panel::Problems;
}

static void startRun() {
    if (E.run) {
        freeProcess(E.run);
        E.run = nullptr;
    }
    saveAll();
    if (!fs::exists(E.root / "main.frg")) {
        E.output = "There's no main.frg in " + E.root.string() + ", so there's no game to run here.\n";
        E.panel = Panel::Output;
        return;
    }
    E.output.clear();
    E.followOutput = true;
    E.panel = Panel::Output;
    E.run = startProcess({E.frg.string(), "run", E.root.string()}, E.root.string());
    if (!E.run) E.output = "Couldn't start frg. Is Forge installed, with frg next to forge-edit or on the PATH?\n";
    startCheck();
}

static void pollProcesses() {
    if (E.run) {
        std::string got = takeOutput(E.run);
        for (char c : got)
            if (c != '\r') E.output += c;
        int code;
        if (processEnded(E.run, &code)) {
            E.output += takeOutput(E.run);
            E.output += "\n[the game ended" + (code ? " with code " + std::to_string(code) : std::string()) + "]\n";
            freeProcess(E.run);
            E.run = nullptr;
        }
        if (E.output.size() > 400000) E.output.erase(0, E.output.size() - 300000);
    }
    if (E.check) {
        E.checkOut += takeOutput(E.check);
        int code;
        if (processEnded(E.check, &code)) {
            E.checkOut += takeOutput(E.check);
            freeProcess(E.check);
            E.check = nullptr;
            finishCheck();
            if (E.checkAgain) {
                E.checkAgain = false;
                startCheck();
            }
        }
    }
}

// ---------------------------------------------------------------- the open folder

// "~/games/jumper" is in the home folder.
static fs::path expandHome(const std::string& s) {
    const char* home = std::getenv("HOME");
#ifdef _WIN32
    if (!home) home = std::getenv("USERPROFILE");
#endif
    if (home && (s == "~" || s.starts_with("~/"))) return fs::path(home) / s.substr(s.size() > 1 ? 2 : 1);
    return fs::path(s);
}

// The folder open last time is kept next to forge-edit, so opening it from a menu goes straight back to the game.
static fs::path lastFolderFile() { return E.exeDir / "forge-edit-folder.txt"; }
static void rememberFolder() {
    std::ofstream f(lastFolderFile(), std::ios::binary);
    if (f) f << E.root.string() << "\n";
}

static void openFolder(const fs::path& dir) {
    saveAll();
    E.docs.clear();
    E.active = -1;
    E.problems.clear();
    E.root = fs::absolute(dir).lexically_normal();
    if (!E.root.has_filename()) E.root = E.root.parent_path();
    rememberFolder();
    SetWindowTitle(("Forge Edit - " + E.root.filename().string()).c_str());
    refreshFiles();
    if (fs::exists(E.root / "main.frg")) openFile(E.root / "main.frg");
    startCheck();
    E.sideScroll = 0;
}

// Tab in the folder prompt: finishes the folder name being typed, if only one folder starts like that.
static void completeFolder() {
    fs::path typed = expandHome(E.promptText);
    std::string text = E.promptText;
    bool endsInSlash = !text.empty() && (text.back() == '/' || text.back() == '\\');
    fs::path dir = endsInSlash ? typed : typed.parent_path();
    std::string start = endsInSlash ? "" : typed.filename().string();
    std::vector<std::string> found;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(dir.empty() ? fs::path(".") : dir, ec))
        if (e.is_directory(ec) && lower(e.path().filename().string()).starts_with(lower(start)) && !e.path().filename().string().starts_with("."))
            found.push_back(e.path().filename().string());
    if (found.size() != 1) {
        if (found.size() > 1) {
            std::string list;
            for (size_t i = 0; i < found.size() && i < 8; i++) list += (i ? ", " : "") + found[i];
            say(list + (found.size() > 8 ? ", ..." : ""));
        }
        return;
    }
    E.promptText = text.substr(0, text.size() - start.size()) + found[0] + "/";
}

// ---------------------------------------------------------------- editing

static Pos selStart(const Doc& d) { return d.anchor < d.cur ? d.anchor : d.cur; }
static Pos selEnd(const Doc& d) { return d.anchor < d.cur ? d.cur : d.anchor; }
static bool hasSel(const Doc& d) { return d.sel && !(d.anchor == d.cur); }

// kind: 1 typing, 2 deleting, 0 anything else (always its own undo step)
static void remember(Doc& d, int kind) {
    double now = GetTime();
    if (kind == 0 || kind != d.lastKind || now - d.lastEdit > 1.0) {
        d.undo.push_back({d.lines, d.cur});
        if (d.undo.size() > 300) d.undo.erase(d.undo.begin());
        d.redo.clear();
    }
    d.lastKind = kind;
    d.lastEdit = now;
    d.dirty = true;
}

static std::string selectedText(const Doc& d) {
    if (!hasSel(d)) return "";
    Pos a = selStart(d), b = selEnd(d);
    if (a.line == b.line) return d.lines[a.line].substr(a.col, b.col - a.col);
    std::string s = d.lines[a.line].substr(a.col) + "\n";
    for (int l = a.line + 1; l < b.line; l++) s += d.lines[l] + "\n";
    return s + d.lines[b.line].substr(0, b.col);
}

static void deleteSel(Doc& d) {
    if (!hasSel(d)) {
        d.sel = false;
        return;
    }
    Pos a = selStart(d), b = selEnd(d);
    d.lines[a.line] = d.lines[a.line].substr(0, a.col) + d.lines[b.line].substr(b.col);
    d.lines.erase(d.lines.begin() + a.line + 1, d.lines.begin() + b.line + 1);
    d.cur = a;
    d.sel = false;
}

static void insertText(Doc& d, const std::string& t) {
    deleteSel(d);
    std::string clean;
    for (char c : t)
        if (c == '\t') clean += "    ";
        else if (c != '\r') clean += c;
    std::string& line = d.lines[d.cur.line];
    std::string after = line.substr(d.cur.col);
    line.erase(d.cur.col);
    size_t at = 0;
    for (;;) {
        size_t nl = clean.find('\n', at);
        std::string piece = clean.substr(at, nl == std::string::npos ? std::string::npos : nl - at);
        d.lines[d.cur.line] += piece;
        d.cur.col = (int)d.lines[d.cur.line].size();
        if (nl == std::string::npos) break;
        d.lines.insert(d.lines.begin() + d.cur.line + 1, "");
        d.cur.line++;
        d.cur.col = 0;
        at = nl + 1;
    }
    d.lines[d.cur.line] += after;
    d.wantCol = -1;
}

static int indentOf(const std::string& s) {
    int n = 0;
    while (n < (int)s.size() && s[n] == ' ') n++;
    return n;
}

static void newline(Doc& d) {
    remember(d, 0);
    deleteSel(d);
    std::string& line = d.lines[d.cur.line];
    int ind = indentOf(line);
    ind = std::min(ind, d.cur.col);
    std::string before = line.substr(0, d.cur.col);
    std::string after = line.substr(d.cur.col);
    size_t lastChar = before.find_last_not_of(' ');
    bool opens = lastChar != std::string::npos && (before[lastChar] == '{' || before[lastChar] == '(' || before[lastChar] == '[');
    size_t firstAfter = after.find_first_not_of(' ');
    bool closes = firstAfter != std::string::npos && (after[firstAfter] == '}' || after[firstAfter] == ')' || after[firstAfter] == ']');
    if (firstAfter != std::string::npos) after = after.substr(firstAfter);
    else after.clear();
    line = before;
    std::string pad(ind + (opens ? 4 : 0), ' ');
    d.lines.insert(d.lines.begin() + d.cur.line + 1, pad + (opens && closes ? "" : after));
    if (opens && closes) d.lines.insert(d.lines.begin() + d.cur.line + 2, std::string(ind, ' ') + after);
    d.cur = {d.cur.line + 1, (int)pad.size()};
    d.wantCol = -1;
}

static void backspace(Doc& d, bool word) {
    remember(d, 2);
    if (hasSel(d)) return deleteSel(d);
    d.sel = false;
    if (d.cur.col == 0) {
        if (d.cur.line == 0) return;
        int l = d.cur.line;
        d.cur = {l - 1, (int)d.lines[l - 1].size()};
        d.lines[l - 1] += d.lines[l];
        d.lines.erase(d.lines.begin() + l);
        return;
    }
    std::string& line = d.lines[d.cur.line];
    int from;
    if (word) {
        from = d.cur.col;
        while (from > 0 && line[from - 1] == ' ') from--;
        while (from > 0 && isWord((unsigned char)line[from - 1])) from--;
        if (from == d.cur.col) from = prevPos(line, d.cur.col);
    } else if (d.cur.col <= indentOf(line) && d.cur.col > 0) {
        from = ((d.cur.col - 1) / 4) * 4;   // back to the previous indent step
    } else {
        from = prevPos(line, d.cur.col);
    }
    line.erase(from, d.cur.col - from);
    d.cur.col = from;
    d.wantCol = -1;
}

static void deleteForward(Doc& d) {
    remember(d, 2);
    if (hasSel(d)) return deleteSel(d);
    d.sel = false;
    std::string& line = d.lines[d.cur.line];
    if (d.cur.col >= (int)line.size()) {
        if (d.cur.line + 1 >= (int)d.lines.size()) return;
        line += d.lines[d.cur.line + 1];
        d.lines.erase(d.lines.begin() + d.cur.line + 1);
        return;
    }
    line.erase(d.cur.col, nextPos(line, d.cur.col) - d.cur.col);
}

static void indentLines(Doc& d, bool out) {
    remember(d, 0);
    int a = hasSel(d) ? selStart(d).line : d.cur.line, b = hasSel(d) ? selEnd(d).line : d.cur.line;
    if (hasSel(d) && selEnd(d).col == 0 && b > a) b--;
    for (int l = a; l <= b; l++) {
        if (out) {
            int n = std::min(4, indentOf(d.lines[l]));
            d.lines[l].erase(0, n);
            if (l == d.cur.line) d.cur.col = std::max(0, d.cur.col - n);
            if (d.sel && l == d.anchor.line) d.anchor.col = std::max(0, d.anchor.col - n);
        } else {
            d.lines[l].insert(0, "    ");
            if (l == d.cur.line) d.cur.col += 4;
            if (d.sel && l == d.anchor.line) d.anchor.col += 4;
        }
    }
}

static void toggleComment(Doc& d) {
    remember(d, 0);
    int a = hasSel(d) ? selStart(d).line : d.cur.line, b = hasSel(d) ? selEnd(d).line : d.cur.line;
    if (hasSel(d) && selEnd(d).col == 0 && b > a) b--;
    bool all = true;
    int ind = 1 << 30;
    for (int l = a; l <= b; l++) {
        const std::string& s = d.lines[l];
        if (s.find_first_not_of(' ') == std::string::npos) continue;
        ind = std::min(ind, indentOf(s));
        if (s.compare(indentOf(s), 2, "//") != 0) all = false;
    }
    if (ind == 1 << 30) return;
    for (int l = a; l <= b; l++) {
        std::string& s = d.lines[l];
        if (s.find_first_not_of(' ') == std::string::npos) continue;
        if (all) {
            int at = indentOf(s);
            s.erase(at, s.compare(at, 3, "// ") == 0 ? 3 : 2);
        } else {
            s.insert(ind, "// ");
        }
    }
    d.cur.col = std::min(d.cur.col, (int)d.lines[d.cur.line].size());
    d.anchor.col = std::min(d.anchor.col, (int)d.lines[d.anchor.line].size());
}

static void undoRedo(Doc& d, bool redo) {
    auto& from = redo ? d.redo : d.undo;
    auto& to = redo ? d.undo : d.redo;
    if (from.empty()) return;
    to.push_back({d.lines, d.cur});
    d.lines = from.back().lines;
    d.cur = from.back().cur;
    from.pop_back();
    d.cur.line = std::clamp(d.cur.line, 0, (int)d.lines.size() - 1);
    d.cur.col = std::min(d.cur.col, (int)d.lines[d.cur.line].size());
    d.sel = false;
    d.dirty = true;
    d.lastKind = 0;
}

static bool findNext(Doc& d, const std::string& what, bool back) {
    if (what.empty()) return false;
    std::string w = lower(what);
    int n = (int)d.lines.size();
    Pos start = back ? selStart(d) : (hasSel(d) ? selEnd(d) : d.cur);
    for (int k = 0; k <= n; k++) {
        int l = back ? ((start.line - k) % n + n) % n : (start.line + k) % n;
        std::string s = lower(d.lines[l]);
        size_t at;
        if (!back) at = s.find(w, k == 0 ? (size_t)start.col : 0);
        else at = k == 0 ? (start.col > 0 ? s.rfind(w, (size_t)start.col - 1) : std::string::npos) : s.rfind(w);
        if (at != std::string::npos) {
            d.anchor = {l, (int)at};
            d.cur = {l, (int)(at + what.size())};
            d.sel = true;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- layout

struct Layout {
    float top = 36, side = 230, status = 24;
    float W, H, panelY, editX, editY, editW, editH, gutter;
};
static Layout L;

static void computeLayout() {
    L.W = (float)GetScreenWidth();
    L.H = (float)GetScreenHeight();
    E.panelH = std::clamp(E.panelH, 60.0f, L.H - 200);
    L.panelY = L.H - L.status - E.panelH;
    L.editX = L.side;
    L.editY = L.top + 28;   // tabs
    L.editW = L.W - L.side;
    L.editH = L.panelY - L.editY;
    Doc* d = cur();
    int digits = d ? std::max(3, (int)std::to_string(d->lines.size()).size()) : 3;
    L.gutter = (digits + 3) * F.charW;
}

static bool mouseIn(float x, float y, float w, float h) {
    Vector2 m = GetMousePosition();
    return m.x >= x && m.x < x + w && m.y >= y && m.y < y + h;
}

static void keepCursorVisible(Doc& d) {
    float y = d.cur.line * F.lineH;
    if (y < d.scrollY) d.scrollY = y;
    if (y + F.lineH > d.scrollY + L.editH - 4) d.scrollY = y + F.lineH - L.editH + 4;
    float x = visCol(d.lines[d.cur.line], d.cur.col) * F.charW;
    float textW = L.editW - L.gutter - 10;
    if (x < d.scrollX) d.scrollX = std::max(0.0f, x - 4 * F.charW);
    if (x > d.scrollX + textW - F.charW) d.scrollX = x - textW + 4 * F.charW;
}

static Pos posAtMouse(const Doc& d) {
    Vector2 m = GetMousePosition();
    int line = (int)std::floor((m.y - L.editY + d.scrollY) / F.lineH);
    line = std::clamp(line, 0, (int)d.lines.size() - 1);
    float x = m.x - L.editX - L.gutter + d.scrollX;
    int col = std::max(0, (int)std::lround(x / F.charW));
    return {line, byteAtCol(d.lines[line], col)};
}

// ---------------------------------------------------------------- docs

static std::string plainInline(const std::string& s);

static void loadDocs() {
    E.pages.clear();
    std::error_code ec;
    if (!fs::is_directory(E.docsDir, ec)) return;
    for (auto it = fs::recursive_directory_iterator(E.docsDir, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        if (it->path().extension() != ".md" || it->path().filename() == "FORGE_ALL_DOCS.md") continue;
        DocPage p;
        p.path = it->path();
        p.rel = fs::relative(p.path, E.docsDir, ec).generic_string();
        std::string t = readFileText(p.path);
        p.textLower = lower(t);
        std::stringstream ss(t);
        std::string line;
        while (std::getline(ss, line))
            if (line.starts_with("# ")) {
                p.title = line.substr(2);
                if (!p.title.empty() && p.title.back() == '\r') p.title.pop_back();
                p.title = plainInline(p.title);
                break;
            }
        if (p.title.empty()) p.title = p.rel;
        E.pages.push_back(p);
    }
    auto rank = [](const std::string& r) {
        if (r == "README.md") return 0;
        if (r == "getting-started.md") return 1;
        if (r.starts_with("language/")) return 2;
        if (r.starts_with("libraries/")) return 3;
        if (r.starts_with("guides/")) return 4;
        if (r.starts_with("reference/")) return 5;
        return 6;
    };
    std::sort(E.pages.begin(), E.pages.end(), [&](const DocPage& a, const DocPage& b) {
        int ra = rank(a.rel), rb = rank(b.rel);
        if (ra != rb) return ra < rb;
        if (a.rel.ends_with("README.md") != b.rel.ends_with("README.md")) return a.rel.ends_with("README.md");
        return a.rel < b.rel;
    });
}

// Markdown, simply: **bold** and `code` marks and links become plain text.
static std::string plainInline(const std::string& s) {
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s.compare(i, 2, "**") == 0) { i++; continue; }
        if (s[i] == '`') continue;
        if (s[i] == '[') {
            size_t close = s.find("](", i);
            size_t end = close == std::string::npos ? std::string::npos : s.find(')', close);
            if (end != std::string::npos) {
                o += s.substr(i + 1, close - i - 1);
                i = end;
                continue;
            }
        }
        if (s[i] == '\\' && i + 1 < s.size() && std::ispunct((unsigned char)s[i + 1])) { o += s[++i]; continue; }
        o += s[i];
    }
    return o;
}

static void layoutDocs(float width) {
    E.layout.clear();
    E.layoutW = width;
    if (E.page < 0 || E.page >= (int)E.pages.size()) return;
    std::vector<std::string> lines = splitLines(readFileText(E.pages[E.page].path));
    float base = 17, y = 16, left = 24, right = width - 24;
    auto wrap = [&](const std::string& s, float x, float size, Color c) {
        std::string lineText;
        float spaceW = uiW(" ", size);
        float lx = x;
        std::stringstream ss(s);
        std::string word;
        while (ss >> word) {
            float w = uiW(word, size);
            if (lx + w > right && !lineText.empty()) {
                E.layout.push_back({x, y, lineText, false, size, c});
                y += size * 1.45f;
                lineText.clear();
                lx = x;
            }
            lineText += (lineText.empty() ? "" : " ") + word;
            lx += w + spaceW;
        }
        if (!lineText.empty()) E.layout.push_back({x, y, lineText, false, size, c});
        y += size * 1.45f;
    };
    float mono = F.size;   // the size the code font was loaded at, so it stays sharp
    bool inCode = false;
    float codeTop = 0;
    size_t codeItem = 0;
    for (std::string line : lines) {
        if (line.starts_with("```") || line.starts_with("~~~")) {
            if (!inCode) {
                inCode = true;
                y += 4;
                codeTop = y;
                codeItem = E.layout.size();
                E.layout.push_back({left - 8, y - 6, "", true, mono, kText, 1});
            } else {
                inCode = false;
                E.layout[codeItem].boxH = y - codeTop + 12;
                y += 16;
            }
            continue;
        }
        if (inCode) {
            E.layout.push_back({left, y, line, true, mono, kText});
            y += mono * 1.4f;
            continue;
        }
        if (line.starts_with("<!--") || line.starts_with("<img")) continue;
        if (line.empty()) { y += base * 0.5f; continue; }
        int h = 0;
        while (h < (int)line.size() && line[h] == '#') h++;
        if (h > 0 && h < (int)line.size() && line[h] == ' ') {
            float size = h == 1 ? 30 : h == 2 ? 24 : h == 3 ? 20 : 18;
            y += h == 1 ? 4 : 12;
            wrap(plainInline(line.substr(h + 1)), left, size, h <= 2 ? kAccent : kText);
            y += 4;
            continue;
        }
        if (line.starts_with("|")) {
            if (line.find("---") != std::string::npos && line.find_first_not_of("|-: ") == std::string::npos) continue;
            // a table row: its cells, one after another
            std::string cells;
            std::stringstream ss(line);
            std::string cell;
            bool first = true;
            while (std::getline(ss, cell, '|')) {
                size_t a = cell.find_first_not_of(' '), b = cell.find_last_not_of(' ');
                if (a == std::string::npos) continue;
                cells += (first ? "" : "   |   ") + plainInline(cell.substr(a, b - a + 1));
                first = false;
            }
            wrap(cells, left, base - 1, kText);
            continue;
        }
        if (line.starts_with("> ")) { wrap(plainInline(line.substr(2)), left + 16, base, kDim); continue; }
        int ind = indentOf(line);
        std::string t = line.substr(ind);
        if (t.starts_with("- ") || t.starts_with("* ")) {
            float x = left + ind * 6;
            E.layout.push_back({x, y, "\xE2\x80\xA2", false, base, kAccent});
            wrap(plainInline(t.substr(2)), x + 16, base, kText);
            continue;
        }
        size_t dot = t.find(". ");
        if (dot != std::string::npos && dot > 0 && dot < 4 && std::all_of(t.begin(), t.begin() + dot, ::isdigit)) {
            float x = left + ind * 6;
            E.layout.push_back({x, y, t.substr(0, dot + 1), false, base, kAccent});
            wrap(plainInline(t.substr(dot + 2)), x + 22, base, kText);
            continue;
        }
        wrap(plainInline(t), left, base, kText);
    }
    E.layoutH = y + 40;
}

static void openPage(int i) {
    E.page = i;
    E.docScroll = 0;
    E.layoutW = 0;   // lay it out again
}

// ---------------------------------------------------------------- drawing

static bool button(const std::string& label, float x, float y, float w, float h, bool on = true) {
    bool hover = on && mouseIn(x, y, w, h);
    DrawRectangleRec({x, y, w, h}, hover ? Color{70, 73, 78, 255} : Color{55, 58, 62, 255});
    ui(label, x + (w - uiW(label)) / 2, y + (h - 15) / 2 - 1, on ? kText : kDim);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void drawTopBar() {
    DrawRectangle(0, 0, (int)L.W, (int)L.top, kPanel);
    ui("Forge Edit", 12, 10, kAccent, 16);
    float x = L.side;
    if (button(E.run ? "\xE2\x96\xA0 Stop (Shift+F5)" : "\xE2\x96\xB6 Run (F5)", x, 5, 140, 26)) {
        if (E.run) stopProcess(E.run);
        else startRun();
    }
    x += 148;
    if (button("Check (F8)", x, 5, 100, 26)) {
        saveAll();
        startCheck();
    }
    x += 108;
    if (button(E.docsMode ? "Code (Esc)" : "Docs (F1)", x, 5, 100, 26)) {
        E.docsMode = !E.docsMode;
        if (E.docsMode && E.pages.empty()) loadDocs();
        if (E.docsMode && E.page < 0 && !E.pages.empty()) openPage(0);
    }
    std::string folder = E.root.filename().string();
    ui(folder, L.W - uiW(folder) - 12, 10, kDim);
    DrawLine(0, (int)L.top, (int)L.W, (int)L.top, kBorder);
}

static void drawTabs() {
    DrawRectangle((int)L.side, (int)L.top, (int)(L.W - L.side), 28, kPanelDark);
    float x = L.side;
    for (int i = 0; i < (int)E.docs.size(); i++) {
        std::string name = E.docs[i].path.filename().string() + (E.docs[i].dirty ? " \xE2\x80\xA2" : "");
        float w = uiW(name, 14) + 40;
        bool act = i == E.active && !E.docsMode;
        DrawRectangleRec({x, L.top, w, 28}, act ? kBg : kPanelDark);
        if (act) DrawRectangle((int)x, (int)L.top, (int)w, 2, kAccent);
        ui(name, x + 12, L.top + 6, act ? kText : kDim, 14);
        bool overX = mouseIn(x + w - 22, L.top + 4, 18, 20);
        ui("\xC3\x97", x + w - 18, L.top + 5, overX ? kText : kDim, 15);
        if (mouseIn(x, L.top, w, 28)) {
            if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && overX) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                if (E.docs[i].dirty) saveDoc(E.docs[i]);
                E.docs.erase(E.docs.begin() + i);
                if (E.active >= (int)E.docs.size()) E.active = (int)E.docs.size() - 1;
                return;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                E.active = i;
                E.docsMode = false;
            }
        }
        x += w + 1;
    }
}

static void drawSideBar() {
    DrawRectangle(0, (int)L.top + 1, (int)L.side, (int)(L.H - L.top - L.status), kPanel);
    DrawLine((int)L.side - 1, (int)L.top, (int)L.side - 1, (int)(L.H - L.status), kBorder);
    float y0 = L.top + 8;
    float rowH = 22;
    if (E.docsMode) {
        // the docs: a search box, then the pages
        DrawRectangleRec({10, y0, L.side - 20, 26}, kBg);
        ui(E.docSearch.empty() ? "Search the docs..." : E.docSearch + "_", 18, y0 + 5, E.docSearch.empty() ? kDim : kText, 14);
        float y = y0 + 34 - E.sideScroll;
        BeginScissorMode(0, (int)(y0 + 32), (int)L.side - 1, (int)(L.H - L.status - y0 - 32));
        std::string q = lower(E.docSearch);
        for (int i = 0; i < (int)E.pages.size(); i++) {
            const DocPage& p = E.pages[i];
            if (!q.empty() && p.textLower.find(q) == std::string::npos && lower(p.title).find(q) == std::string::npos) continue;
            bool act = i == E.page;
            if (act) DrawRectangle(0, (int)y, (int)L.side - 1, (int)rowH, Color{55, 58, 62, 255});
            std::string section = p.rel.find('/') != std::string::npos ? p.rel.substr(0, p.rel.find('/')) : "";
            std::string label = (section.empty() ? "" : section + ": ") + p.title;
            while (uiW(label, 14) > L.side - 30 && label.size() > 4) label = label.substr(0, label.size() - 4) + "...";
            ui(label, 14, y + 3, act ? kText : Color{190, 190, 190, 255}, 14);
            if (mouseIn(0, y, L.side, rowH) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GetMousePosition().y > y0 + 32) openPage(i);
            y += rowH;
        }
        EndScissorMode();
        if (E.pages.empty()) ui("No docs folder found", 14, y0 + 40, kDim, 14);
        return;
    }
    ui("FILES", 14, y0, kDim, 13);
    float y = y0 + 24 - E.sideScroll;
    BeginScissorMode(0, (int)(y0 + 20), (int)L.side - 1, (int)(L.H - L.status - y0 - 20));
    std::string lastFolder;
    for (const fs::path& p : E.files) {
        std::string rel = relName(p);
        std::string folder = rel.find('/') != std::string::npos ? rel.substr(0, rel.rfind('/')) : "";
        if (folder != lastFolder && !folder.empty()) {
            ui(folder + "/", 14, y + 3, kDim, 14);
            y += rowH;
        }
        lastFolder = folder;
        bool act = cur() && !E.docsMode && samePath(cur()->path, p);
        if (act) DrawRectangle(0, (int)y, (int)L.side - 1, (int)rowH, Color{55, 58, 62, 255});
        bool err = false;
        for (const Problem& pr : E.problems)
            if (!pr.warning && samePath(pr.file, p)) err = true;
        float x = folder.empty() ? 14 : 28;
        ui(p.filename().string(), x, y + 3, err ? kError : act ? kText : Color{190, 190, 190, 255}, 14);
        if (mouseIn(0, y, L.side, rowH) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GetMousePosition().y > y0 + 20) openFile(p);
        y += rowH;
    }
    EndScissorMode();
    ui("Ctrl+N  new file", 14, L.H - L.status - 26, kDim, 13);
}

static void drawEditor() {
    Doc* dp = cur();
    DrawRectangle((int)L.editX, (int)L.editY, (int)L.editW, (int)L.editH, kBg);
    if (!dp) {
        ui("Open a file on the left, or press F1 for Forge's docs.", L.editX + 30, L.editY + 30, kDim, 16);
        ui("F5 runs the game, F8 checks it for errors, Ctrl+S saves.", L.editX + 30, L.editY + 56, kDim, 16);
        return;
    }
    Doc& d = *dp;
    BeginScissorMode((int)L.editX, (int)L.editY, (int)L.editW, (int)L.editH);
    int first = std::max(0, (int)(d.scrollY / F.lineH));
    int last = std::min((int)d.lines.size() - 1, first + (int)(L.editH / F.lineH) + 1);
    // the comment/string state at the first visible line
    int state = 0;
    std::vector<Span> spans;
    for (int l = 0; l < first; l++) highlight(d.lines[l], state, spans);
    float tx = L.editX + L.gutter - d.scrollX;
    Pos sa = selStart(d), sb = selEnd(d);
    bool sel = hasSel(d);
    std::string findLower = lower(E.findText);
    for (int l = first; l <= last; l++) {
        float y = L.editY + l * F.lineH - d.scrollY;
        const std::string& s = d.lines[l];
        if (l == d.cur.line && !sel) DrawRectangle((int)L.editX, (int)y, (int)L.editW, (int)F.lineH, kCurLine);
        // find matches
        if (E.prompt == Prompt::Find && !findLower.empty()) {
            std::string ls = lower(s);
            for (size_t at = ls.find(findLower); at != std::string::npos; at = ls.find(findLower, at + 1))
                DrawRectangle((int)(tx + visCol(s, (int)at) * F.charW), (int)y, (int)(visCol(E.findText, (int)E.findText.size()) * F.charW), (int)F.lineH, kFound);
        }
        // selection
        if (sel && l >= sa.line && l <= sb.line) {
            int a = l == sa.line ? visCol(s, sa.col) : 0;
            int b = l == sb.line ? visCol(s, sb.col) : visCol(s, (int)s.size()) + 1;
            DrawRectangle((int)(tx + a * F.charW), (int)y, (int)((b - a) * F.charW), (int)F.lineH, kSelection);
        }
        highlight(s, state, spans);
        float ty = y + (F.lineH - F.size) / 2;
        for (const Span& sp : spans) {
            float x = tx + visCol(s, sp.from) * F.charW;
            if (x > L.W) break;
            std::string piece = s.substr(sp.from, sp.to - sp.from);
            if (piece != " ") text(F.mono, piece, x, ty, F.size, tokColor(sp.tok));
        }
        // problems: a wavy line under them
        for (const Problem& p : E.problems) {
            if (p.line - 1 != l || !samePath(p.file, d.path)) continue;
            float x0 = tx + (p.col - 1) * F.charW, x1 = x0 + std::max(1, p.len) * F.charW;
            Color c = p.warning ? kWarning : kError;
            float wy = y + F.lineH - 3;
            for (float x = x0; x < x1; x += 4) DrawLineEx({x, wy + ((int)((x - x0) / 4) % 2 ? 1.5f : -1.0f)}, {x + 4, wy + ((int)((x - x0) / 4) % 2 ? -1.0f : 1.5f)}, 1.2f, c);
        }
        // cursor
        if (l == d.cur.line) DrawRectangle((int)(tx + visCol(s, d.cur.col) * F.charW), (int)y + 2, 2, (int)F.lineH - 4, kCursor);
    }
    // the line numbers, on top of anything scrolled under them
    DrawRectangle((int)L.editX, (int)L.editY, (int)L.gutter - 6, (int)L.editH, kBg);
    for (int l = first; l <= last; l++) {
        float y = L.editY + l * F.lineH - d.scrollY;
        std::string num = std::to_string(l + 1);
        bool err = false, warn = false;
        for (const Problem& p : E.problems)
            if (p.line - 1 == l && samePath(p.file, d.path)) (p.warning ? warn : err) = true;
        if (err || warn) DrawCircle((int)(L.editX + 8), (int)(y + F.lineH / 2), 3.5f, err ? kError : kWarning);
        text(F.mono, num, L.editX + L.gutter - 6 - F.charW * 1.5f - num.size() * F.charW, y + (F.lineH - F.size) / 2, F.size,
             l == d.cur.line ? kText : kDim);
    }
    EndScissorMode();
}

static void drawDocs() {
    float x0 = L.side, y0 = L.editY, w = L.W - L.side, h = L.H - L.status - y0;
    DrawRectangle((int)x0, (int)y0, (int)w, (int)h, kBg);
    if (E.page < 0) {
        ui(E.pages.empty() ? "Forge's docs weren't found next to forge-edit (" + E.docsDir.string() + ")." : "Pick a page on the left.",
           x0 + 30, y0 + 30, kDim, 16);
        return;
    }
    float width = std::min(w, 980.0f);
    if (E.layoutW != width) layoutDocs(width);
    E.docScroll = std::clamp(E.docScroll, 0.0f, std::max(0.0f, E.layoutH - h));
    BeginScissorMode((int)x0, (int)y0, (int)w, (int)h);
    for (const DocItem& it : E.layout) {
        float y = y0 + it.y - E.docScroll;
        if (it.boxH > 0) {
            if (y + it.boxH < y0 || y > y0 + h) continue;
            DrawRectangleRec({x0 + it.x, y, width - 2 * it.x + 16, it.boxH}, kPanelDark);
            continue;
        }
        if (y + 40 < y0 || y > y0 + h) continue;
        if (it.mono) {
            int state = 0;
            std::vector<Span> spans;
            highlight(it.s, state, spans);
            float cw = MeasureTextEx(F.mono, "M", it.size, 0).x;
            for (const Span& sp : spans)
                text(F.mono, it.s.substr(sp.from, sp.to - sp.from), x0 + it.x + visCol(it.s, sp.from) * cw, y, it.size, tokColor(sp.tok));
        } else {
            text(F.sans, it.s, x0 + it.x, y, it.size, it.color);
        }
    }
    EndScissorMode();
}

static void drawPanel() {
    float px = L.side, pw = L.W - L.side;   // under the editor, next to the file list
    DrawRectangle((int)px, (int)L.panelY, (int)pw, (int)E.panelH, kPanelDark);
    DrawLine((int)px, (int)L.panelY, (int)L.W, (int)L.panelY, kBorder);
    int errors = 0, warnings = 0;
    for (const Problem& p : E.problems) (p.warning ? warnings : errors)++;
    std::string probs = "Problems" + (errors + warnings ? " (" + std::to_string(errors + warnings) + ")" : std::string());
    float x = px + 10;
    for (int i = 0; i < 2; i++) {
        std::string label = i == 0 ? probs : std::string(E.run ? "Output (running)" : "Output");
        float w = uiW(label, 14) + 20;
        bool act = (int)E.panel == i;
        if (mouseIn(x, L.panelY + 2, w, 24) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) E.panel = (Panel)i;
        ui(label, x + 10, L.panelY + 7, act ? kText : kDim, 14);
        if (act) DrawRectangle((int)x + 8, (int)L.panelY + 25, (int)w - 16, 2, kAccent);
        x += w + 6;
    }
    if (E.check) ui("checking...", x + 10, L.panelY + 7, kDim, 14);
    float top = L.panelY + 32, h = E.panelH - 34;
    BeginScissorMode((int)px, (int)top, (int)pw, (int)h);
    if (E.panel == Panel::Problems) {
        float y = top;
        if (E.problems.empty()) ui(fs::exists(E.root / "main.frg") ? "No problems." : "No main.frg here, so there's nothing to check.", px + 16, y + 4, kDim, 14);
        for (const Problem& p : E.problems) {
            if (y > top + h) break;
            bool hover = mouseIn(px, y, pw, 22);
            if (hover) DrawRectangle((int)px, (int)y, (int)pw, 22, Color{50, 52, 56, 255});
            DrawCircle((int)px + 18, (int)y + 11, 4, p.warning ? kWarning : kError);
            std::string where = relName(p.file) + ":" + std::to_string(p.line);
            ui(where, px + 30, y + 3, kDim, 14);
            std::string msg = p.message + (p.hint.empty() ? "" : "   (hint: " + p.hint + ")");
            ui(msg, px + 40 + uiW(where, 14), y + 3, kText, 14);
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                openFile(p.file, p.line - 1, 0);
                Doc* d = cur();
                if (d) {
                    d->cur.col = byteAtCol(d->lines[d->cur.line], p.col - 1);
                    keepCursorVisible(*d);
                }
            }
            y += 22;
        }
    } else {
        std::vector<std::string> lines = splitLines(E.output);
        float lh = F.size * 1.3f, size = F.size;
        float total = lines.size() * lh;
        if (E.followOutput) E.outputScroll = std::max(0.0f, total - h);
        int first = std::max(0, (int)(E.outputScroll / lh));
        for (int i = first; i < (int)lines.size(); i++) {
            float y = top + i * lh - E.outputScroll;
            if (y > top + h) break;
            const std::string& s = lines[i];
            Color c = s.find("error") != std::string::npos ? kError : s.find("warning") != std::string::npos ? kWarning : kText;
            text(F.mono, s, px + 14, y, size, c);
        }
        if (lines.size() <= 1 && E.output.empty()) ui("Press F5 to run the game. What it prints shows here.", px + 16, top + 4, kDim, 14);
    }
    EndScissorMode();
}

static void drawStatus() {
    float y = L.H - L.status;
    DrawRectangle(0, (int)y, (int)L.W, (int)L.status, E.run ? Color{150, 80, 20, 255} : kPanel);
    if (E.prompt != Prompt::None) {
        std::string label = E.prompt == Prompt::Find ? "Find (Enter: next, Shift+Enter: back, Esc: done): "
                          : E.prompt == Prompt::NewFile ? "New file (like enemy.frg or levels/one.frg), Enter to make it: "
                          : E.prompt == Prompt::OpenFolder ? "Open the game folder (Tab finishes a name): "
                                                        : "Go to line: ";
        ui(label, 10, y + 4, kText, 14);
        text(F.mono, E.promptText + "_", 14 + uiW(label, 14), y + 4, 15, kAccent);
        return;
    }
    Doc* d = cur();
    std::string left;
    if (d && !E.docsMode) {
        left = "Ln " + std::to_string(d->cur.line + 1) + ", Col " + std::to_string(visCol(d->lines[d->cur.line], d->cur.col) + 1);
        for (const Problem& p : E.problems)
            if (p.line - 1 == d->cur.line && samePath(p.file, d->path)) {
                left += "    " + std::string(p.warning ? "warning: " : "error: ") + p.message;
                break;
            }
    }
    if (GetTime() - E.statusAt < 4) left = E.status;
    ui(left, 10, y + 4, kText, 14);
    std::string right = E.run ? "the game is running (Shift+F5 stops it)" : "F5 run   F8 check   F1 docs   Ctrl+F find   Ctrl+O open folder";
    ui(right, L.W - uiW(right, 14) - 10, y + 4, kText, 14);
}

// ---------------------------------------------------------------- input

static bool pressed(int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }

static void moveCursor(Doc& d, Pos p, bool shift) {
    if (shift && !d.sel) {
        d.anchor = d.cur;
        d.sel = true;
    }
    if (!shift) d.sel = false;
    d.cur = p;
    keepCursorVisible(d);
}

static void handlePrompt() {
    int c;
    while ((c = GetCharPressed()) > 0) {
        if (c < 32) continue;
        char buf[5] = {0};
        int n = 0;
        const char* u = CodepointToUTF8(c, &n);
        std::memcpy(buf, u, n);
        E.promptText += buf;
        if (E.prompt == Prompt::Find) E.findText = E.promptText;
    }
    if (pressed(KEY_BACKSPACE) && !E.promptText.empty()) {
        E.promptText.erase(prevPos(E.promptText, (int)E.promptText.size()));
        if (E.prompt == Prompt::Find) E.findText = E.promptText;
    }
    if (IsKeyPressed(KEY_TAB) && E.prompt == Prompt::OpenFolder) completeFolder();
    if (IsKeyPressed(KEY_ESCAPE)) {
        E.prompt = Prompt::None;
        return;
    }
    if (pressed(KEY_ENTER) || pressed(KEY_KP_ENTER)) {
        Doc* d = cur();
        if (E.prompt == Prompt::Find) {
            if (d && !findNext(*d, E.findText, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) say("\"" + E.findText + "\" isn't in this file");
            if (d) keepCursorVisible(*d);
            return;
        }
        if (E.prompt == Prompt::GoToLine && d) {
            int l = std::atoi(E.promptText.c_str());
            moveCursor(*d, {std::clamp(l - 1, 0, (int)d->lines.size() - 1), 0}, false);
            d->scrollY = std::max(0.0f, (d->cur.line - 5) * F.lineH);
        }
        if (E.prompt == Prompt::OpenFolder && !E.promptText.empty()) {
            fs::path p = expandHome(E.promptText);
            std::error_code ec;
            if (fs::is_directory(p, ec)) openFolder(p);
            else {
                say("there's no folder " + p.string());
                return;
            }
        }
        if (E.prompt == Prompt::NewFile && !E.promptText.empty()) {
            fs::path p = E.root / fs::path(E.promptText);
            std::error_code ec;
            if (fs::exists(p, ec)) {
                openFile(p);
            } else {
                fs::create_directories(p.parent_path(), ec);
                std::ofstream f(p, std::ios::binary);
                if (!f) say("couldn't make " + E.promptText);
                else {
                    f.close();
                    refreshFiles();
                    openFile(p);
                }
            }
        }
        E.prompt = Prompt::None;
    }
}

static void handleKeys() {
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    // everywhere
    if (IsKeyPressed(KEY_F5)) {
        if (shift) { if (E.run) stopProcess(E.run); }
        else startRun();
    }
    if (IsKeyPressed(KEY_F8)) {
        saveAll();
        startCheck();
    }
    if (IsKeyPressed(KEY_F1)) {
        E.docsMode = !E.docsMode;
        if (E.docsMode && E.pages.empty()) loadDocs();
        if (E.docsMode && E.page < 0 && !E.pages.empty()) openPage(0);
    }
    if (ctrl && (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_KP_SUBTRACT))) {
        F.size = std::clamp(F.size + ((IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) ? 2.0f : -2.0f), 10.0f, 40.0f);
        loadMono();
        E.layoutW = 0;
    }
    if (ctrl && IsKeyPressed(KEY_O)) {
        E.prompt = Prompt::OpenFolder;
        E.promptText = E.root.string();
        return;
    }
    if (ctrl && IsKeyPressed(KEY_N)) {
        E.prompt = Prompt::NewFile;
        E.promptText.clear();
        return;
    }
    if (E.prompt != Prompt::None) return handlePrompt();

    if (E.docsMode) {
        int c;
        while ((c = GetCharPressed()) > 0) {
            int n = 0;
            const char* u = CodepointToUTF8(c, &n);
            E.docSearch += std::string(u, n);
            E.sideScroll = 0;
        }
        if (pressed(KEY_BACKSPACE) && !E.docSearch.empty()) E.docSearch.erase(prevPos(E.docSearch, (int)E.docSearch.size()));
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (!E.docSearch.empty()) E.docSearch.clear();
            else E.docsMode = false;
        }
        float page = L.H - L.top - L.status - 40;
        if (pressed(KEY_PAGE_DOWN) || pressed(KEY_DOWN)) E.docScroll += IsKeyDown(KEY_DOWN) ? 40 : page;
        if (pressed(KEY_PAGE_UP) || pressed(KEY_UP)) E.docScroll -= IsKeyDown(KEY_UP) ? 40 : page;
        if (IsKeyPressed(KEY_HOME)) E.docScroll = 0;
        return;
    }

    Doc* dp = cur();
    if (!dp) return;
    Doc& d = *dp;
    if (ctrl) {
        if (IsKeyPressed(KEY_S)) {
            if (shift) saveAll();
            else saveDoc(d);
            startCheck();
            say("saved");
        }
        if (pressed(KEY_Z)) shift ? undoRedo(d, true) : undoRedo(d, false);
        if (pressed(KEY_Y)) undoRedo(d, true);
        if (IsKeyPressed(KEY_A)) {
            d.anchor = {0, 0};
            d.cur = {(int)d.lines.size() - 1, (int)d.lines.back().size()};
            d.sel = true;
        }
        if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_X)) {
            std::string t = hasSel(d) ? selectedText(d) : d.lines[d.cur.line] + "\n";   // nothing selected: the whole line
            SetClipboardText(t.c_str());
            if (IsKeyPressed(KEY_X)) {
                remember(d, 0);
                if (hasSel(d)) deleteSel(d);
                else if (d.lines.size() > 1) {
                    d.lines.erase(d.lines.begin() + d.cur.line);
                    d.cur.line = std::min(d.cur.line, (int)d.lines.size() - 1);
                    d.cur.col = std::min(d.cur.col, (int)d.lines[d.cur.line].size());
                } else {
                    d.lines[0].clear();
                    d.cur.col = 0;
                }
            }
        }
        if (IsKeyPressed(KEY_V)) {
            const char* t = GetClipboardText();
            if (t && *t) {
                remember(d, 0);
                insertText(d, t);
                keepCursorVisible(d);
            }
        }
        if (IsKeyPressed(KEY_F)) {
            E.prompt = Prompt::Find;
            E.promptText = hasSel(d) && selStart(d).line == selEnd(d).line ? selectedText(d) : E.findText;
            E.findText = E.promptText;
        }
        if (IsKeyPressed(KEY_G)) {
            E.prompt = Prompt::GoToLine;
            E.promptText.clear();
        }
        if (IsKeyPressed(KEY_SLASH)) toggleComment(d);
        if (IsKeyPressed(KEY_W)) {
            if (d.dirty) saveDoc(d);
            E.docs.erase(E.docs.begin() + E.active);
            if (E.active >= (int)E.docs.size()) E.active = (int)E.docs.size() - 1;
            return;
        }
        if (IsKeyPressed(KEY_TAB) && !E.docs.empty()) {
            E.active = (E.active + (shift ? (int)E.docs.size() - 1 : 1)) % (int)E.docs.size();
            return;
        }
    }
    // typing
    if (!ctrl) {
        int c;
        while ((c = GetCharPressed()) > 0) {
            if (c < 32) continue;
            int n = 0;
            const char* u = CodepointToUTF8(c, &n);
            std::string s(u, n);
            remember(d, 1);
            // typing } on an empty line puts it one step further left
            if (s == "}" && !hasSel(d)) {
                std::string& line = d.lines[d.cur.line];
                if (line.find_first_not_of(' ') == std::string::npos && line.size() >= 4 && d.cur.col == (int)line.size()) {
                    line.erase(0, 4);
                    d.cur.col -= 4;
                }
            }
            insertText(d, s);
            keepCursorVisible(d);
        }
    }
    if (pressed(KEY_ENTER) || pressed(KEY_KP_ENTER)) {
        newline(d);
        keepCursorVisible(d);
    }
    if (pressed(KEY_BACKSPACE)) {
        backspace(d, ctrl);
        keepCursorVisible(d);
    }
    if (pressed(KEY_DELETE)) deleteForward(d);
    if (pressed(KEY_TAB) && !ctrl) {
        if (shift || hasSel(d)) indentLines(d, shift);
        else {
            remember(d, 1);
            int col = visCol(d.lines[d.cur.line], d.cur.col);
            insertText(d, std::string(4 - col % 4, ' '));
        }
        keepCursorVisible(d);
    }
    if (IsKeyPressed(KEY_ESCAPE)) d.sel = false;

    // moving
    const std::string& line = d.lines[d.cur.line];
    if (pressed(KEY_LEFT)) {
        Pos p = d.cur;
        if (!shift && hasSel(d)) p = selStart(d);
        else if (p.col > 0) {
            if (ctrl) {
                int k = p.col;
                while (k > 0 && line[k - 1] == ' ') k--;
                while (k > 0 && isWord((unsigned char)line[k - 1])) k--;
                p.col = k == p.col ? prevPos(line, p.col) : k;
            } else {
                p.col = prevPos(line, p.col);
            }
        } else if (p.line > 0) {
            p = {p.line - 1, (int)d.lines[p.line - 1].size()};
        }
        d.wantCol = -1;
        moveCursor(d, p, shift);
    }
    if (pressed(KEY_RIGHT)) {
        Pos p = d.cur;
        if (!shift && hasSel(d)) p = selEnd(d);
        else if (p.col < (int)line.size()) {
            if (ctrl) {
                int k = p.col;
                while (k < (int)line.size() && isWord((unsigned char)line[k])) k++;
                while (k < (int)line.size() && line[k] == ' ') k++;
                p.col = k == p.col ? nextPos(line, p.col) : k;
            } else {
                p.col = nextPos(line, p.col);
            }
        } else if (p.line + 1 < (int)d.lines.size()) {
            p = {p.line + 1, 0};
        }
        d.wantCol = -1;
        moveCursor(d, p, shift);
    }
    int pageLines = std::max(1, (int)(L.editH / F.lineH) - 2);
    for (auto [key, delta] : {std::pair{KEY_UP, -1}, std::pair{KEY_DOWN, 1}, std::pair{KEY_PAGE_UP, -pageLines}, std::pair{KEY_PAGE_DOWN, pageLines}}) {
        if (!pressed(key)) continue;
        if (d.wantCol < 0) d.wantCol = visCol(line, d.cur.col);
        int l = std::clamp(d.cur.line + delta, 0, (int)d.lines.size() - 1);
        if (ctrl && (key == KEY_UP || key == KEY_DOWN)) {   // Ctrl+Up/Down scroll without moving
            d.scrollY = std::max(0.0f, d.scrollY + delta * F.lineH);
            continue;
        }
        int keep = d.wantCol;
        moveCursor(d, {l, byteAtCol(d.lines[l], d.wantCol)}, shift);
        d.wantCol = keep;
    }
    if (pressed(KEY_HOME)) {
        Pos p = ctrl ? Pos{0, 0} : d.cur;
        if (!ctrl) {
            int ind = indentOf(line);
            p.col = d.cur.col == ind ? 0 : ind;
        }
        d.wantCol = -1;
        moveCursor(d, p, shift);
    }
    if (pressed(KEY_END)) {
        Pos p = ctrl ? Pos{(int)d.lines.size() - 1, (int)d.lines.back().size()} : Pos{d.cur.line, (int)line.size()};
        d.wantCol = -1;
        moveCursor(d, p, shift);
    }
}

static void handleMouse() {
    Vector2 m = GetMousePosition();
    float wheel = GetMouseWheelMove();
    // dragging the line between the editor and the bottom panel
    static bool resizing = false;
    if (mouseIn(L.side, L.panelY - 3, L.W, 6)) SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
    else if (!resizing) SetMouseCursor(m.x > L.editX && m.y > L.editY && m.y < L.panelY && !E.docsMode ? MOUSE_CURSOR_IBEAM : MOUSE_CURSOR_DEFAULT);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouseIn(L.side, L.panelY - 3, L.W, 6)) resizing = true;
    if (resizing) {
        E.panelH = L.H - L.status - m.y;
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) resizing = false;
        return;
    }
    if (wheel != 0) {
        if (m.x < L.side) E.sideScroll = std::max(0.0f, E.sideScroll - wheel * 44);
        else if (m.y > L.panelY && !E.docsMode) {
            E.outputScroll = std::max(0.0f, E.outputScroll - wheel * 3 * F.size);
            E.followOutput = false;
        } else if (E.docsMode) {
            E.docScroll -= wheel * 60;
        } else if (Doc* d = cur()) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) d->scrollX = std::max(0.0f, d->scrollX - wheel * 6 * F.charW);
            else d->scrollY = std::clamp(d->scrollY - wheel * 3 * F.lineH, 0.0f, std::max(0.0f, (d->lines.size() - 3) * F.lineH));
        }
    }
    if (E.docsMode) return;
    Doc* dp = cur();
    if (!dp) return;
    Doc& d = *dp;
    bool inText = mouseIn(L.editX, L.editY, L.editW, L.editH);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inText) {
        Pos p = posAtMouse(d);
        double now = GetTime();
        if (now - E.lastClick < 0.35 && p.line == d.cur.line) {
            // double click: the word
            const std::string& s = d.lines[p.line];
            int a = p.col, b = p.col;
            while (a > 0 && isWord((unsigned char)s[a - 1])) a--;
            while (b < (int)s.size() && isWord((unsigned char)s[b])) b++;
            d.anchor = {p.line, a};
            d.cur = {p.line, b};
            d.sel = true;
            E.lastClick = 0;
            return;
        }
        E.lastClick = now;
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        if (shift) {
            if (!d.sel) d.anchor = d.cur;
            d.sel = true;
        } else {
            d.anchor = p;
            d.sel = true;
        }
        d.cur = p;
        d.wantCol = -1;
        E.dragging = true;
    }
    if (E.dragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            d.cur = posAtMouse(d);
            if (m.y < L.editY) d.scrollY = std::max(0.0f, d.scrollY - F.lineH);
            if (m.y > L.editY + L.editH) d.scrollY += F.lineH;
        } else {
            E.dragging = false;
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m.y > L.panelY) E.followOutput = false;
}

// ---------------------------------------------------------------- main

int main(int argc, char** argv) {
    E.exeDir = fs::path(exeFolder());
    // the folder: the one given, or else the one open last time (or else where it was started)
    fs::path start = argc > 1 ? fs::path(argv[1]) : fs::current_path();
    if (argc <= 1) {
        std::string last = readFileText(lastFolderFile());
        while (!last.empty() && (last.back() == '\n' || last.back() == '\r')) last.pop_back();
        std::error_code ec;
        if (!last.empty() && fs::is_directory(last, ec)) start = last;
    }
    E.root = fs::absolute(start).lexically_normal();
    if (!E.root.has_filename()) E.root = E.root.parent_path();
    if (fs::is_regular_file(E.root)) E.root = E.root.parent_path();
    rememberFolder();
#ifdef _WIN32
    E.frg = fs::exists(E.exeDir / "frg.exe") ? E.exeDir / "frg.exe" : fs::path("frg");
#else
    E.frg = fs::exists(E.exeDir / "frg") ? E.exeDir / "frg" : fs::path("frg");
#endif
    E.docsDir = E.exeDir.parent_path() / "docs";

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1200, 760, ("Forge Edit - " + E.root.filename().string()).c_str());
    SetWindowMinSize(700, 420);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    loadFonts(E.exeDir);
    refreshFiles();
    if (const char* open = std::getenv("FORGE_EDIT_OPEN")) openFile(E.root / open);
    else if (fs::exists(E.root / "main.frg")) openFile(E.root / "main.frg");
    if (std::getenv("FORGE_EDIT_DOCS")) {
        loadDocs();
        E.docsMode = true;
        if (!E.pages.empty()) openPage(0);
    }
    startCheck();
    if (std::getenv("FORGE_EDIT_RUN")) startRun();
    const char* shot = std::getenv("FORGE_EDIT_SHOT");
    int shotFrames = std::getenv("FORGE_EDIT_FRAMES") ? std::atoi(std::getenv("FORGE_EDIT_FRAMES")) : 30;
    bool waiting = false;

    for (int frame = 0;; frame++) {
        if (WindowShouldClose()) {
            bool unsaved = std::any_of(E.docs.begin(), E.docs.end(), [](const Doc& d) { return d.dirty; });
            if (!unsaved || E.quitWarned) break;
            E.quitWarned = true;
            say("There are unsaved changes: Ctrl+Shift+S saves them all. Close again to quit without saving.");
        }
        // when nothing is happening, wait for the keyboard or mouse instead of drawing 60 times a second
        bool busy = E.run || E.check || shot;
        if (busy && waiting) DisableEventWaiting(), waiting = false;
        if (!busy && !waiting && frame > 2) EnableEventWaiting(), waiting = true;

        pollProcesses();
        if (GetTime() - E.filesAt > 3) refreshFiles();
        computeLayout();
        handleKeys();
        handleMouse();
        computeLayout();

        BeginDrawing();
        ClearBackground(kBg);
        if (E.docsMode) drawDocs();
        else drawEditor();
        if (!E.docsMode) drawPanel();
        drawSideBar();
        drawTabs();
        drawTopBar();
        drawStatus();
        EndDrawing();

        if (shot && frame == shotFrames) {
            TakeScreenshot(shot);
            break;
        }
    }
    if (E.run) freeProcess(E.run);
    if (E.check) freeProcess(E.check);
    CloseWindow();
    return 0;
}
