#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "checker.hpp"
#include "codegen.hpp"
#include "common.hpp"
#include "project.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const char* kVersion = "0.1.0";

static fs::path exeDir() {
    std::wstring buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), (DWORD)buf.size());
    return fs::path(buf.substr(0, n)).parent_path();
}

static std::wstring quote(const fs::path& p) { return L"\"" + p.wstring() + L"\""; }

// Runs a command line and waits for it. Returns the exit code, or -1 if it couldn't start.
static int runProcess(const std::wstring& cmdline, const fs::path& workDir = {}) {
    STARTUPINFOW si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    std::wstring cmd = cmdline;
    std::wstring dir = workDir.wstring();
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, 0, nullptr, dir.empty() ? nullptr : dir.c_str(), &si, &pi))
        return -1;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)code;
}

// While the game runs, Ctrl+C should stop the game, not Forge.
static BOOL WINAPI ignoreCtrlC(DWORD type) { return type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT; }

static bool build(const fs::path& dir, fs::path& exeOut, bool release) {
    auto start = std::chrono::steady_clock::now();
    if (!fs::is_directory(dir)) {
        reportPlainError("the folder '" + dir.string() + "' doesn't exist");
        return false;
    }
    if (!fs::is_regular_file(dir / "main.frg")) {
        reportPlainError("there's no main.frg in " + fs::absolute(dir).string(),
                         "every Forge game needs a main.frg in its top folder. Make a new game with: frg new mygame");
        return false;
    }

    Project proj;
    if (!loadProject(proj, dir) || !checkProject(proj)) {
        int n = errorCount();
        std::fprintf(stderr, "Build failed: %d error%s.\n", n, n == 1 ? "" : "s");
        return false;
    }
    printStatus("Compiling", proj.name + " (" + std::to_string(proj.allModules.size()) + " file" +
                                 (proj.allModules.size() == 1 ? "" : "s") + ")");

    fs::path buildDir = proj.root / "build";
    std::error_code ec;
    fs::create_directories(buildDir, ec);
    fs::path cppPath = buildDir / "forge_generated.cpp";
    GeneratedCpp gen = generateCpp(proj);
    {
        std::ofstream f(cppPath, std::ios::binary);
        f << gen.code;
    }

    fs::path exe = buildDir / (proj.name + ".exe");
    if (fs::exists(exe)) {
        fs::remove(exe, ec);
        if (ec) {
            reportPlainError("couldn't replace " + exe.string(), "is the game still running? Close it and try again");
            return false;
        }
    }

    fs::path runtime = exeDir() / "runtime";
    if (!fs::exists(runtime / "forge.hpp")) {
        reportPlainError("Forge's runtime files are missing (" + (runtime / "forge.hpp").string() + ")",
                         "keep the runtime folder next to frg.exe");
        return false;
    }

    std::wstring cmd = L"g++ -std=gnu++20 -O2 -fwrapv -w -static -I" + quote(runtime) + L" " + quote(cppPath) +
                       L" -o " + quote(exe);
    if (release) cmd += L" -mwindows -s";  // no console window, smaller .exe
    if (gen.needsRaylib) {
        fs::path raylib = runtime / "raylib";
        cmd += L" -I" + quote(raylib / "include") + L" -L" + quote(raylib / "lib") + L" -lraylib -lopengl32 -lgdi32 -lwinmm";
    }
    int code = runProcess(cmd);
    if (code == -1) {
        reportPlainError("couldn't find g++, which Forge uses to make .exe files",
                         "install w64devkit and make sure its bin folder is on your PATH");
        return false;
    }
    if (code != 0) {
        reportPlainError("the C++ compiler rejected Forge's output - this is a bug in Forge, not in your code",
                         "the generated file is " + cppPath.string());
        return false;
    }

    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    char timeBuf[32];
    std::snprintf(timeBuf, sizeof timeBuf, "%.2fs", secs);
    printStatus("Finished", "build/" + exe.filename().string() + " in " + timeBuf);
    exeOut = exe;
    return true;
}

static int cmdRun(const fs::path& dir, bool release) {
    fs::path exe;
    if (!build(dir, exe, release)) return 1;
    printStatus("Running", exe.filename().string());
    std::fflush(stderr);
    SetConsoleCtrlHandler(ignoreCtrlC, TRUE);
    int code = runProcess(quote(exe), fs::path(exe).parent_path().parent_path());
    if (code == -1) {
        reportPlainError("couldn't start " + exe.string());
        return 1;
    }
    if (code != 0) {
        char buf[64];
        if ((unsigned)code > 255)
            std::snprintf(buf, sizeof buf, "crashed (code 0x%08X)", (unsigned)code);
        else
            std::snprintf(buf, sizeof buf, "exited with code %d", code);
        std::fprintf(stderr, "\n[%s %s]\n", exe.filename().string().c_str(), buf);
    }
    return code;
}

static int cmdNew(const std::string& name) {
    fs::path dir = name;
    if (fs::exists(dir)) {
        reportPlainError("'" + name + "' already exists", "pick a different name for the new game");
        return 1;
    }
    fs::create_directories(dir);
    std::ofstream f(dir / "main.frg", std::ios::binary);
    f << "#include <stdio>\n\nint main(){\n    print(\"Hello, World!\");\n    return 0;\n}\n";
    f.close();
    printStatus("Created", name + "/main.frg");
    std::fprintf(stderr, "\nTo run it:\n    cd %s\n    frg run\n", name.c_str());
    return 0;
}

static void printHelp() {
    std::printf(
        "Forge %s - a compiled language for making games\n\n"
        "Usage:\n"
        "  frg new <name>      make a new game folder with a main.frg\n"
        "  frg build [folder]  compile the game into build/<name>.exe\n"
        "  frg run [folder]    compile the game, then run it\n"
        "  frg check [folder]  look for errors without building\n"
        "  frg version         show the Forge version\n\n"
        "[folder] is the game's top folder (the one with main.frg). It defaults to the current folder.\n"
        "Add --release to build or run to make a finished game: no console window, smaller .exe.\n",
        kVersion);
}

int main(int argc, char** argv) {
    initConsole();
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty() || args[0] == "help" || args[0] == "--help" || args[0] == "-h") {
        printHelp();
        return 0;
    }
    const std::string& cmd = args[0];
    if (cmd == "version" || cmd == "--version") {
        std::printf("Forge %s\n", kVersion);
        return 0;
    }
    if (cmd == "build" || cmd == "run") {
        bool release = false;
        std::vector<std::string> rest;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--release")
                release = true;
            else
                rest.push_back(args[i]);
        }
        if (rest.size() > 1) {
            reportPlainError("too many arguments", "usage: frg " + cmd + " [folder] [--release]");
            return 1;
        }
        fs::path dir = rest.empty() ? fs::current_path() : fs::path(rest[0]);
        if (cmd == "run") return cmdRun(dir, release);
        fs::path exe;
        return build(dir, exe, release) ? 0 : 1;
    }
    if (cmd == "check") {
        // frg check [folder] [--machine]: find errors without building anything.
        fs::path dir = fs::current_path();
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--machine")
                setMachineOutput(true);
            else
                dir = args[i];
        }
        if (!fs::is_regular_file(dir / "main.frg")) {
            reportPlainError("there's no main.frg in " + fs::absolute(dir).string(),
                             "every Forge game needs a main.frg in its top folder");
            return 1;
        }
        Project proj;
        bool ok = loadProject(proj, dir) && checkProject(proj);
        if (!ok) {
            int n = errorCount();
            std::fprintf(stderr, "%d error%s found.\n", n, n == 1 ? "" : "s");
            return 1;
        }
        printStatus("Checked", proj.name + ": no errors");
        return 0;
    }
    if (cmd == "new") {
        if (args.size() != 2) {
            reportPlainError("frg new needs a name", "for example: frg new platformer");
            return 1;
        }
        return cmdNew(args[1]);
    }
    reportPlainError("unknown command '" + cmd + "'", "try frg build, frg run, or frg new <name>");
    return 1;
}
