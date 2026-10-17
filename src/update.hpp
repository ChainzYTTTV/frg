// Checking for a newer Forge (see update.cpp). Updating is always optional.
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace update {

// Whether version `candidate` ("0.9.5" or "v0.9.5") is newer than `current`.
bool isNewer(const std::string& candidate, const std::string& current);

// The daily check: started when a command begins (if a day has passed, and it isn't turned off), finished when
// it ends, printing a short note if a newer Forge is out.
void startDailyCheck(const fs::path& exeDir);
void finishDailyCheck(const fs::path& exeDir, const char* current);

// frg update [--off | --on]
int command(const std::vector<std::string>& args, const fs::path& exeDir, const char* current);

}  // namespace update
