#pragma once
#include <filesystem>
#include <string>

// frg query <question> <game folder> <file> <line> <column> [--stdin] [new name]
// Answers an editor's question about the code at a position (lines and columns start at 1, columns count bytes),
// as one line of JSON on standard output. The questions:
//   definition   {"location": {file, line, col, len}}, where the name at the position is declared
//   references   {"locations": [...]}, everywhere that name is written
//   hover        {"text": markdown, "builtin": bool}, what the name is
//   complete     {"items": [{label, kind, detail}]}, what could be typed at the position (after a '.', the members)
//   rename       {"edits": [...]} with every place to change, or {"error": why it can't be renamed}
//   symbols      {"symbols": [{name, kind, detail, line, col, children}]}, the file's outline (line/col ignored)
// With --stdin, the file's text comes from standard input (an editor's unsaved changes) instead of the disk.
int runQuery(const std::string& what, const std::filesystem::path& folder, const std::filesystem::path& file, int line, int col,
             bool fromStdin, const std::string& newName);
