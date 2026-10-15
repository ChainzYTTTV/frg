#pragma once
#include <string>
#include <utility>
#include <vector>

// The starter games `frg new <name> --template <template>` makes. Each is a list of (file name, contents),
// with every $NAME$ replaced by the game's name.
const std::vector<std::string>& templateNames();
std::vector<std::pair<std::string, std::string>> templateFiles(const std::string& templateName, const std::string& gameName);
