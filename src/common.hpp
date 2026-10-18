#pragma once
#include <string>
#include <vector>

// One .frg file loaded into memory.
struct SourceFile {
    std::string displayPath;  // path shown in errors, relative to the project root ("enemies/goblin.frg")
    std::string fullPath;     // absolute path on disk ("" for built-in code)
    std::string text;
    std::vector<size_t> lineStarts;

    void computeLines();
    std::string getLine(int line) const;
};

// A position in a source file. Lines and columns start at 1.
struct Loc {
    const SourceFile* file = nullptr;
    int line = 0;
    int col = 0;
    int len = 1;
};

// Thrown after an error that stops the current file from being parsed any further.
struct CompileAbort {};

void initConsole();
// Machine mode (for editors): each error or warning is one tab-separated line on stdout:
//   E (or W for a warning) <tab> full path <tab> line <tab> column <tab> length <tab> message <tab> hint
// with \, tabs and newlines inside fields escaped as \\, \t and \n. Columns count bytes.
void setMachineOutput(bool on);
// Quiet mode (for editor queries like "where is this defined?"): errors are counted but never printed.
void setQuietOutput(bool on);
void reportError(const Loc& loc, const std::string& msg, const std::string& hint = "");
// Warnings never stop a build. They're collected, and printed (in yellow) by flushWarnings.
void reportWarning(const Loc& loc, const std::string& msg, const std::string& hint = "");
// Prints the collected warnings and returns how many there were. While there are errors, only editors see them.
int flushWarnings();
// A warning about the game's files rather than a line of code (like a broken icon.png). Printed straight away.
void reportPlainWarning(const std::string& msg, const std::string& hint = "");
void reportPlainError(const std::string& msg, const std::string& hint = "");
void printStatus(const std::string& label, const std::string& msg);
int errorCount();

// Returns the candidate closest to `wrong` (for "did you mean" hints), or "" if none is close.
std::string suggestName(const std::string& wrong, const std::vector<std::string>& candidates);

// How to change a name everywhere at once (Replace in Files, matching case and whole words). `to` is the new name,
// or "" when the programmer picks one.
std::string replaceEverywhereHint(const std::string& from, const std::string& to);

// What a name Forge renamed is called now, or "" if it wasn't renamed. `owner` is "" for a type, a library's name
// ("graphics3d") for its functions, or a type's name ("Font") for its methods. `version` gets the Forge version
// that renamed it.
std::string renamedTo(const std::string& owner, const std::string& name, std::string* version = nullptr);
