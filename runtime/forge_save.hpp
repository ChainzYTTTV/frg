// Forge runtime: <save> - remembering values between runs of the game (high scores, settings, progress).
//
// Everything is kept in save.txt in the game's folder, one "key=value" per line, and the file is written again
// after every change, so nothing is lost if the game is closed suddenly.
// The game's folder is the one with the .exe in it, or, for an .exe in build/ next to main.frg, the folder above.
#pragma once

#include "forge.hpp"

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

namespace frg::save {

namespace fs = std::filesystem;

struct State {
    bool loaded = false;
    fs::path path;
    std::map<std::string, std::string> values;
};
inline State& st() {
    static State s;
    return s;
}

inline fs::path savePath() {
    fs::path dir = exePath().parent_path();
    std::error_code ec;
    if (dir.filename() == "build" && fs::exists(dir.parent_path() / "main.frg", ec)) dir = dir.parent_path();
    return dir / "save.txt";
}

// In the file, \ = and new lines inside keys and values are written as \\ \= \n (and # at the start of a key as \#).
inline std::string escape(const std::string& s, bool key) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (key && c == '=') out += "\\=";
        else if (key && c == '#' && i == 0) out += "\\#";
        else out += c;
    }
    return out;
}

inline void load() {
    State& s = st();
    if (s.loaded) return;
    s.loaded = true;
    s.path = savePath();
    std::ifstream in(s.path, std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        std::string key, value;
        bool inKey = true, ok = false;
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            std::string& to = inKey ? key : value;
            if (c == '\\' && i + 1 < line.size()) {
                char n = line[++i];
                to += n == 'n' ? '\n' : n == 'r' ? '\r' : n;
            } else if (c == '=' && inKey) {
                inKey = false;
                ok = true;
            } else {
                to += c;
            }
        }
        if (ok) s.values[key] = value;  // a broken line (no '=') is skipped
    }
}

inline void write() {
    State& s = st();
    std::string text = "# Forge save file: one key=value per line\n";
    for (const auto& [k, v] : s.values) text += escape(k, true) + "=" + escape(v, false) + "\n";
    fs::path tmp = s.path;
    tmp += ".tmp";
    std::error_code ec;
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (out) out << text;
        if (!out) panic("save", "couldn't write the save file " + s.path.string() + " - is the game in a folder it can't write to?");
    }
    fs::rename(tmp, s.path, ec);  // replace the old file in one go, so it's never half-written
    if (ec) panic("save", "couldn't write the save file " + s.path.string() + " (" + ec.message() + ")");
}

inline const std::string* find(const std::string& key) {
    load();
    auto it = st().values.find(key);
    return it == st().values.end() ? nullptr : &it->second;
}

inline void set(const std::string& key, const std::string& value, const char* what) {
    if (key.empty()) panic(what, "the key can't be empty text - give it a name, like \"highScore\"");
    load();
    auto it = st().values.find(key);
    if (it != st().values.end() && it->second == value) return;  // nothing changed, so nothing to write
    st().values[key] = value;
    write();
}

inline void setInt_(const std::string& key, int32_t v) { set(key, str(v), "save.setInt()"); }
inline void setFloat_(const std::string& key, float v) { set(key, str(v), "save.setFloat()"); }
inline void setString_(const std::string& key, const std::string& v) { set(key, v, "save.setString()"); }
inline void setBool_(const std::string& key, bool v) { set(key, v ? "true" : "false", "save.setBool()"); }

// A value that's missing, or saved as a different kind (text where a number should be), gives back the backup.
inline int32_t getInt_(const std::string& key, int32_t backup) {
    int32_t v;
    const std::string* s = find(key);
    return s && parseInto(*s, v) == 0 ? v : backup;
}
inline float getFloat_(const std::string& key, float backup) {
    float v;
    const std::string* s = find(key);
    return s && parseInto(*s, v) == 0 ? v : backup;
}
inline std::string getString_(const std::string& key, const std::string& backup) {
    const std::string* s = find(key);
    return s ? *s : backup;
}
inline bool getBool_(const std::string& key, bool backup) {
    const std::string* s = find(key);
    return s && (*s == "true" || *s == "false") ? *s == "true" : backup;
}

inline bool has_(const std::string& key) { return find(key) != nullptr; }
inline void remove_(const std::string& key) {
    load();
    if (st().values.erase(key)) write();
}
inline void clear_() {
    load();
    st().values.clear();
    write();
}

}  // namespace frg::save
