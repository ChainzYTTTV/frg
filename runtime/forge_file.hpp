// Forge runtime: <file> - reading and writing text files (for save games, settings, levels...).
// Paths are relative to the game's folder.
#pragma once

#include "forge.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace frg::file {

namespace fs = std::filesystem;

inline std::string read_(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) panic("file.read()", "can't open '" + path + "' - use file.exists(path) to check first");
    std::stringstream ss;
    ss << in.rdbuf();
    std::string text = ss.str();
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
}

inline bool writeText(const std::string& path, const std::string& text, bool append) {
    std::error_code ec;
    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path(), ec);
    std::ofstream out(path, append ? std::ios::binary | std::ios::app : std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << text;
    return (bool)out;
}

inline bool write_(const std::string& path, const std::string& text) { return writeText(path, text, false); }
inline bool append_(const std::string& path, const std::string& text) { return writeText(path, text, true); }

inline bool exists_(const std::string& path) {
    std::error_code ec;
    return fs::is_regular_file(path, ec);
}

inline bool remove_(const std::string& path) {
    std::error_code ec;
    return fs::is_regular_file(path, ec) && fs::remove(path, ec);
}

inline List<std::string> readLines_(const std::string& path) {
    List<std::string> lines = str_split(read_(path), "\n", "");
    if (!lines.empty() && lines.back().empty()) lines.pop_back();  // the newline at the very end
    return lines;
}

inline bool writeLines_(const std::string& path, const List<std::string>& lines) {
    std::string text;
    for (auto& l : lines) text += l + "\n";
    return write_(path, text);
}

inline List<std::string> files_(const std::string& folder) {
    List<std::string> names;
    std::error_code ec;
    if (!fs::is_directory(folder, ec)) return names;
    for (auto& entry : fs::directory_iterator(folder, ec))
        if (entry.is_regular_file()) names.push_back(entry.path().filename().string());
    std::sort(names.begin(), names.end());
    return names;
}

}  // namespace frg::file
