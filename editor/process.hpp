// Forge Edit: starting frg in the background and collecting what it prints (process.cpp). Kept apart from the
// editor itself, because windows.h can't be mixed with raylib.h.
#pragma once

#include <string>
#include <vector>

struct RunningProcess;

// Starts a program (args[0], found on the PATH if it has no folder) in `dir`, with what it prints (both kinds of
// output) collected. Gives back nullptr if it couldn't be started.
RunningProcess* startProcess(const std::vector<std::string>& args, const std::string& dir);

// What it printed since the last call.
std::string takeOutput(RunningProcess* p);

// Whether it has ended, and if so its exit code. Never waits.
bool processEnded(RunningProcess* p, int* code);

// Stops it, and every program it started (frg run's game too).
void stopProcess(RunningProcess* p);

// Stops it if it's still running, and frees it.
void freeProcess(RunningProcess* p);

// The folder this program (forge-edit) is in.
std::string exeFolder();
