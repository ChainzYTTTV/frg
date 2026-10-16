#pragma once
#include <string>

// The result of tidying one .frg file's layout (see format.cpp for exactly what changes).
struct FormatResult {
    bool ok = false;
    bool changed = false;
    std::string text;   // the tidied text, when ok
    std::string error;  // why it wasn't tidied, when not ok
};

FormatResult formatSource(const std::string& text);
