#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "checker.hpp"
#include "codegen.hpp"
#include "common.hpp"
#include "project.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const char* kVersion = "0.6.0";

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

// Like runProcess, but collects what the command prints (stdout and stderr) instead of showing it.
static int runCaptured(const std::wstring& cmdline, std::string& output) {
    SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
    HANDLE readEnd = nullptr, writeEnd = nullptr;
    if (!CreatePipe(&readEnd, &writeEnd, &sa, 0)) return -1;
    SetHandleInformation(readEnd, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si{};
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = si.hStdError = writeEnd;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};
    std::wstring cmd = cmdline;
    BOOL started = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
    CloseHandle(writeEnd);
    if (!started) {
        CloseHandle(readEnd);
        return -1;
    }
    char buf[4096];
    DWORD n = 0;
    while (ReadFile(readEnd, buf, sizeof buf, &n, nullptr) && n > 0) output.append(buf, n);
    CloseHandle(readEnd);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)code;
}

// <Vulkan> games: turns every GLSL shader in the game's folder (player.vert, sky.frag...) into the SPIR-V file
// Vulkan loads (player.vert.spv), using glslang. Only shaders that changed since last time are compiled.
static bool compileShaders(const fs::path& root, const fs::path& runtime) {
    static const char* kStages[] = {".vert", ".frag", ".comp", ".geom", ".tesc", ".tese"};
    fs::path glslang = runtime / "tools" / "glslang.exe";
    std::error_code ec;
    bool ok = true;
    for (auto it = fs::recursive_directory_iterator(root, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        const fs::path& file = it->path();
        if (it->is_directory() && (file.filename() == "build" || file.filename().string().starts_with("."))) {
            it.disable_recursion_pending();
            continue;
        }
        std::string ext = file.extension().string();
        if (std::find(std::begin(kStages), std::end(kStages), ext) == std::end(kStages)) continue;
        fs::path spv = file;
        spv += ".spv";
        if (fs::exists(spv) && fs::last_write_time(spv, ec) >= fs::last_write_time(file, ec)) continue;
        std::string shown = fs::relative(file, root, ec).generic_string();
        printStatus("Shader", shown);
        std::string out;
        int code = runCaptured(quote(glslang) + L" -V " + quote(file) + L" -o " + quote(spv), out);
        if (code == -1) {
            reportPlainError("Forge's shader compiler is missing (" + glslang.string() + ")", "keep the runtime folder next to frg.exe");
            return false;
        }
        if (code != 0) {
            ok = false;
            reportPlainError("the shader " + shown + " has errors");
            // glslang's lines look like "ERROR: C:\game\cube.vert:12: 'x' : undeclared identifier"
            size_t start = 0;
            while (start < out.size()) {
                size_t end = out.find('\n', start);
                if (end == std::string::npos) end = out.size();
                std::string line = out.substr(start, end - start);
                while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
                if (line.starts_with("ERROR: ") && line.find("compilation terminated") == std::string::npos &&
                    line.find("compilation errors") == std::string::npos) {
                    line = line.substr(7);
                    std::string full = file.string();
                    if (line.starts_with(full)) line = shown + line.substr(full.size());
                    std::fprintf(stderr, "    %s\n", line.c_str());
                }
                start = end + 1;
            }
        }
    }
    return ok;
}

// While the game runs, Ctrl+C should stop the game, not Forge.
static BOOL WINAPI ignoreCtrlC(DWORD type) { return type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT; }

// How a game is built. The console is on for normal builds (to see print() and errors while testing) and off
// for --release builds; --console and --no-console choose it either way.
struct BuildOptions {
    bool release = false;
    bool console = true;
};

static bool build(const fs::path& dir, fs::path& exeOut, const BuildOptions& opt) {
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

    bool vulkan = std::find(gen.sources.begin(), gen.sources.end(), "forge_vulkan_impl.cpp") != gen.sources.end();
    if (vulkan && !compileShaders(proj.root, runtime)) {
        std::fprintf(stderr, "Build failed: a shader has errors.\n");
        return false;
    }

    std::wstring cmd = L"g++ -std=gnu++20 -O2 -fwrapv -w -static -I" + quote(runtime) + L" " + quote(cppPath);
    if (vulkan) cmd += L" -I" + quote(runtime / "vulkan" / "include");
    for (const std::string& src : gen.sources) cmd += L" " + quote(runtime / src);  // a toolkit's own runtime code
    cmd += L" -o " + quote(exe);
    if (!opt.console) cmd += L" -mwindows -DFRG_NO_CONSOLE";  // no console window: runtime errors pop up in a box instead
    if (opt.release) cmd += L" -s";                           // a smaller .exe
    if (gen.needsRaylib) {
        fs::path raylib = runtime / "raylib";
        cmd += L" -I" + quote(raylib / "include") + L" -L" + quote(raylib / "lib") + L" -lraylib -lopengl32 -lgdi32 -lwinmm";
    }
    for (const std::string& flag : gen.linkFlags) cmd += L" " + fs::path(flag).wstring();
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
    printStatus("Finished", "build/" + exe.filename().string() + (opt.console ? "" : " (no console)") + " in " + timeBuf);
    if (!opt.console && !gen.needsRaylib)
        std::fprintf(stderr, "    note: this game doesn't open a window, so without a console nothing will show when it runs\n");
    exeOut = exe;
    return true;
}

static int cmdRun(const fs::path& dir, const BuildOptions& opt) {
    fs::path exe;
    if (!build(dir, exe, opt)) return 1;
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
        "Options for build and run:\n"
        "  --release           make a finished game: no console window, and a smaller .exe\n"
        "  --no-console        no console window (print() output isn't shown; runtime errors pop up in a box)\n"
        "  --console           keep the console window, even with --release\n"
        "Without options the console is on, so print() and errors show while you test the game.\n",
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
        BuildOptions opt;
        int consoleChoice = -1;  // -1: not chosen, 0: --no-console, 1: --console
        std::vector<std::string> rest;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--release") {
                opt.release = true;
            } else if (args[i] == "--console" || args[i] == "--no-console") {
                int choice = args[i] == "--console" ? 1 : 0;
                if (consoleChoice != -1 && consoleChoice != choice) {
                    reportPlainError("--console and --no-console can't both be used", "pick one of them");
                    return 1;
                }
                consoleChoice = choice;
            } else if (args[i].starts_with("--")) {
                reportPlainError("there's no option called " + args[i], "the options are --release, --console and --no-console");
                return 1;
            } else {
                rest.push_back(args[i]);
            }
        }
        if (rest.size() > 1) {
            reportPlainError("too many arguments", "usage: frg " + cmd + " [folder] [--release] [--console | --no-console]");
            return 1;
        }
        opt.console = consoleChoice == -1 ? !opt.release : consoleChoice == 1;
        fs::path dir = rest.empty() ? fs::current_path() : fs::path(rest[0]);
        if (cmd == "run") return cmdRun(dir, opt);
        fs::path exe;
        return build(dir, exe, opt) ? 0 : 1;
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
