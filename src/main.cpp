#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "checker.hpp"
#include "codegen.hpp"
#include "common.hpp"
#include "project.hpp"
#include "templates.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
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

// ---------------------------------------------------------------- build cache
//
// Most of a build's time goes into reading Forge's runtime headers (and raylib's). So they're compiled once into a
// precompiled header, and the toolkits' runtime .cpp files into object files, and kept in a cache folder next to
// frg.exe. Games that use the same libraries with the same settings share them. If a cached file is out of date or
// broken, g++ simply reads the real headers instead, so the cache can only make builds faster, never different.

// A short name for a piece of text (FNV-1a hash), used to name cache folders.
static std::string hashName(const std::string& s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    char buf[17];
    std::snprintf(buf, sizeof buf, "%016llx", (unsigned long long)h);
    return buf;
}

static std::string timeStamp(const fs::path& p) {
    std::error_code ec;
    auto t = fs::last_write_time(p, ec);
    return ec ? "?" : std::to_string(t.time_since_epoch().count());
}

// Everything that makes cached compiler output out of date: Forge's version, the runtime files, and which g++ is used.
static std::string toolStamp(const fs::path& runtime) {
    std::string s = std::string(kVersion) + "|";
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(runtime, ec))
        if (entry.is_regular_file()) s += entry.path().filename().string() + "@" + timeStamp(entry.path()) + ";";
    wchar_t gpp[MAX_PATH];
    if (SearchPathW(nullptr, L"g++.exe", nullptr, MAX_PATH, gpp, nullptr)) s += fs::path(gpp).string() + "@" + timeStamp(gpp);
    return s;
}

// The folder for one cached thing: <frg.exe's folder>/cache/<hash>, or the game's build folder if that can't be written.
static fs::path cacheFolder(const std::string& key, const fs::path& buildDir) {
    fs::path dir = exeDir() / "cache" / hashName(key);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (!ec) {
        fs::path probe = dir / "write-test";
        std::ofstream f(probe, std::ios::binary);
        if (f << "ok") {
            f.close();
            fs::remove(probe, ec);
            return dir;
        }
    }
    dir = buildDir / "cache" / hashName(key);
    fs::create_directories(dir, ec);
    return dir;
}

// Runs one g++ command that makes `output`, writing to a temporary name first so a half-written file is never used.
static int compileTo(const std::wstring& args, const fs::path& output) {
    fs::path tmp = output;
    tmp += L"." + std::to_wstring(GetCurrentProcessId()) + L".tmp";
    int code = runProcess(L"g++ " + args + L" -o " + quote(tmp));
    std::error_code ec;
    if (code == 0) fs::rename(tmp, output, ec);
    if (code != 0 || ec) fs::remove(tmp, ec);
    return code;
}

// ---------------------------------------------------------------- the game's icon
//
// If the game's folder has icon.png, it becomes the .exe's icon (and, through raylib's window library GLFW, which
// looks for an icon called GLFW_ICON, the window's icon too). An .ico file can simply hold a PNG image, so the PNG's
// bytes are wrapped in a 22-byte .ico header, and windres (part of w64devkit) turns that into an object file to link.
// Returns the object file, or an empty path if there's no icon (or it couldn't be made, after saying why).
static fs::path makeIcon(const fs::path& root, const fs::path& buildDir) {
    fs::path png = root / "icon.png";
    std::error_code ec;
    if (!fs::is_regular_file(png, ec)) return {};
    fs::path obj = buildDir / "icon_res.o";
    if (fs::exists(obj, ec) && fs::last_write_time(obj, ec) >= fs::last_write_time(png, ec)) return obj;

    std::ifstream in(png, std::ios::binary);
    std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    static const char kPngStart[] = "\x89PNG\r\n\x1a\n";
    if (data.size() < 24 || data.compare(0, 8, kPngStart, 8) != 0 || data.compare(12, 4, "IHDR") != 0) {
        reportPlainWarning("icon.png isn't a PNG image, so the game won't have an icon",
                           "save the picture as a .png file (256 x 256 pixels works best)");
        return {};
    }
    auto be32 = [&](size_t at) {
        return (uint32_t)(unsigned char)data[at] << 24 | (uint32_t)(unsigned char)data[at + 1] << 16 |
               (uint32_t)(unsigned char)data[at + 2] << 8 | (uint32_t)(unsigned char)data[at + 3];
    };
    uint32_t w = be32(16), h = be32(20);
    std::string ico;
    auto put16 = [&](uint32_t v) { ico += (char)(v & 0xFF), ico += (char)(v >> 8 & 0xFF); };
    auto put32 = [&](uint32_t v) { put16(v & 0xFFFF), put16(v >> 16); };
    put16(0), put16(1), put16(1);                                    // an icon file with 1 image
    ico += (char)(w >= 256 ? 0 : w), ico += (char)(h >= 256 ? 0 : h);  // its size (0 means 256 or more)
    ico += '\0', ico += '\0';                                        // no palette
    put16(1), put16(32), put32((uint32_t)data.size()), put32(22);     // 32-bit color, the PNG's size, where it starts
    ico += data;
    {
        std::ofstream f(buildDir / "icon.ico", std::ios::binary);
        f << ico;
        std::ofstream rc(buildDir / "icon.rc", std::ios::binary);
        rc << "GLFW_ICON ICON \"icon.ico\"\n";
    }
    int code = runProcess(L"windres -O coff -i icon.rc -o icon_res.o", buildDir);
    if (code != 0) {
        reportPlainWarning("couldn't put icon.png into the .exe, so it won't have an icon",
                           code == -1 ? "windres, which comes with w64devkit, wasn't found - is w64devkit's bin folder on your PATH?"
                                      : "windres didn't accept it - try saving icon.png again with a paint program");
        fs::remove(obj, ec);
        return {};
    }
    return obj;
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
    flushWarnings();
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

    // Release builds are optimized more (-O2). Other builds use -O1, which compiles faster and still runs well.
    // -fwrapv: whole numbers wrap around when they overflow, the same at every optimization level.
    std::wstring flags = std::wstring(L"-std=gnu++20 ") + (opt.release ? L"-O2" : L"-O1") + L" -fwrapv -w -I" + quote(runtime);
    if (!opt.console) flags += L" -DFRG_NO_CONSOLE";  // no console window: runtime errors pop up in a box instead
    if (vulkan) flags += L" -I" + quote(runtime / "vulkan" / "include");
    fs::path raylib = runtime / "raylib";
    if (gen.needsRaylib) flags += L" -I" + quote(raylib / "include");
    std::string stamp = toolStamp(runtime) + "|" + fs::path(flags).string();

    // Forge's runtime headers, precompiled once (see "build cache" above). There are two sets, so that all games share
    // them: the core and the libraries that don't use raylib, and for engine games, those plus every raylib library.
    // (Including a header the game doesn't use changes nothing: they only hold declarations and inline functions.)
    // Any other header the game uses (a toolkit, <time>) is read after it as usual.
    std::vector<std::string> pchHeaders = {"forge.hpp", "forge_physics.hpp", "forge_scenes.hpp", "forge_ecs.hpp", "forge_file.hpp"};
    const std::vector<std::string> engineHeaders = {"forge_window.hpp", "forge_assets.hpp", "forge_audio.hpp", "forge_ui.hpp", "forge_graphics3d.hpp"};
    for (const std::string& h : gen.headers)
        if (std::find(engineHeaders.begin(), engineHeaders.end(), h) != engineHeaders.end()) {
            pchHeaders.insert(pchHeaders.end(), engineHeaders.begin(), engineHeaders.end());
            break;
        }
    std::string pchText;
    for (const std::string& h : pchHeaders) pchText += "#include \"" + h + "\"\n";
    fs::path pchDir = cacheFolder("pch|" + stamp + "|" + pchText, buildDir);
    fs::path pchHeader = pchDir / "forge_pch.hpp", pch = pchDir / "forge_pch.hpp.gch";
    if (!fs::exists(pchHeader)) {
        std::ofstream f(pchHeader, std::ios::binary);
        f << pchText;
    }
    if (!fs::exists(pch)) {
        printStatus("Preparing", "Forge's runtime for these libraries (only the first time, to make later builds faster)");
        if (compileTo(flags + L" -x c++-header " + quote(pchHeader), pch) == -1) {
            reportPlainError("couldn't find g++, which Forge uses to make .exe files",
                             "install w64devkit and make sure its bin folder is on your PATH");
            return false;
        }
    }

    // A toolkit's own runtime code, compiled once into an object file.
    std::vector<fs::path> objects;
    for (const std::string& src : gen.sources) {
        fs::path obj = cacheFolder("obj|" + stamp + "|" + src, buildDir) / (fs::path(src).stem().string() + ".o");
        if (!fs::exists(obj)) {
            printStatus("Preparing", src + " (only the first time)");
            if (compileTo(flags + L" -c " + quote(runtime / src), obj) != 0) {
                reportPlainError("the C++ compiler rejected Forge's runtime file " + src + " - this is a bug in Forge, not in your code");
                return false;
            }
        }
        objects.push_back(obj);
    }

    std::wstring cmd = L"g++ " + flags + L" -I" + quote(pchDir) + L" " + quote(cppPath);
    for (const fs::path& obj : objects) cmd += L" " + quote(obj);
    fs::path icon = makeIcon(proj.root, buildDir);
    if (!icon.empty()) cmd += L" " + quote(icon);
    cmd += L" -o " + quote(exe) + L" -static";
    if (!opt.console) cmd += L" -mwindows";
    if (opt.release) cmd += L" -s";  // a smaller .exe
    if (gen.needsRaylib) cmd += L" -L" + quote(raylib / "lib") + L" -lraylib -lopengl32 -lgdi32 -lwinmm";
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

// frg package: a --release build, then dist/<name>/ with the .exe and every file the game needs, zipped.
static int cmdPackage(const fs::path& dir, const BuildOptions& opt) {
    fs::path exe;
    if (!build(dir, exe, opt)) return 1;
    fs::path root = fs::absolute(dir).lexically_normal();
    if (root.filename().empty()) root = root.parent_path();  // "C:\game\" -> "C:\game"
    std::string name = exe.stem().string();
    fs::path dist = root / "dist", out = dist / name, zip = dist / (name + ".zip");
    std::error_code ec;
    fs::remove_all(out, ec);
    fs::remove(zip, ec);
    fs::create_directories(out, ec);
    if (ec) {
        reportPlainError("couldn't make the folder " + out.string(), "is a file in it still open in another program?");
        return 1;
    }

    // Everything the game might load comes along, except Forge's own source files and things only used while
    // making the game: .frg files, build/ and dist/, hidden files and folders (.git), shader sources (their
    // compiled .spv files do come along), and save.txt, so players don't start with your saved game.
    static const char* kSkip[] = {".frg", ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese"};
    int copied = 0;
    for (auto it = fs::recursive_directory_iterator(root, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        const fs::path& p = it->path();
        std::string fname = p.filename().string();
        fs::path rel = fs::relative(p, root, ec);
        bool top = rel.has_parent_path() == false;
        if (it->is_directory()) {
            if (fname.starts_with(".") || (top && (fname == "build" || fname == "dist"))) it.disable_recursion_pending();
            continue;
        }
        std::string ext = p.extension().string();
        for (char& c : ext) c = (char)std::tolower((unsigned char)c);
        if (fname.starts_with(".") || (top && fname == "save.txt") ||
            std::find_if(std::begin(kSkip), std::end(kSkip), [&](const char* s) { return ext == s; }) != std::end(kSkip))
            continue;
        fs::create_directories((out / rel).parent_path(), ec);
        fs::copy_file(p, out / rel, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            reportPlainError("couldn't copy " + rel.generic_string() + " into " + out.string());
            return 1;
        }
        copied++;
    }
    fs::copy_file(exe, out / exe.filename(), fs::copy_options::overwrite_existing, ec);
    if (ec) {
        reportPlainError("couldn't copy " + exe.filename().string() + " into " + out.string());
        return 1;
    }
    printStatus("Copied", exe.filename().string() + (copied ? " and " + std::to_string(copied) + " file" + (copied == 1 ? "" : "s") : "") +
                              " into dist/" + name + "/");

    // Windows 10 and newer come with tar.exe, which can make .zip files. It's called by its full path, because
    // w64devkit has a different tar that can't.
    wchar_t sys[MAX_PATH];
    UINT n = GetSystemDirectoryW(sys, MAX_PATH);
    fs::path tar = fs::path(std::wstring(sys, n)) / "tar.exe";
    int code = runProcess(quote(tar) + L" -a -c -f " + quote(zip.filename()) + L" " + quote(fs::path(name)), dist);
    if (code != 0 || !fs::exists(zip)) {
        reportPlainError("couldn't make the .zip file", code == -1 ? "Windows' tar.exe wasn't found (it comes with Windows 10 and newer). "
                                                                    "The game is ready in " + out.string() + " - you can zip that folder yourself"
                                                                  : "the game is ready in " + out.string() + " - you can zip that folder yourself");
        return 1;
    }
    printStatus("Packaged", "dist/" + name + ".zip");
    std::fprintf(stderr, "\nYour game is ready to share:\n    %s\nPlayers unzip it and double-click %s.\n", zip.string().c_str(),
                 exe.filename().string().c_str());
    return 0;
}

static int cmdNew(const std::string& name, const std::string& templateName) {
    const std::vector<std::string>& names = templateNames();
    if (std::find(names.begin(), names.end(), templateName) == names.end()) {
        std::string guess = suggestName(templateName, names);
        reportPlainError("there's no template called '" + templateName + "'",
                         (guess.empty() ? std::string() : "did you mean " + guess + "? ") + "The templates are console, window, platformer and 3d");
        return 1;
    }
    fs::path dir = name;
    std::string game = dir.filename().string();
    bool validName = !game.empty() && !std::isdigit((unsigned char)game[0]);
    for (char c : game) validName = validName && (std::isalnum((unsigned char)c) || c == '_');
    if (!validName) {
        reportPlainError("'" + game + "' can't be the name of a game",
                         "use only letters, numbers and _, and don't start with a number - for example: frg new my_game");
        return 1;
    }
    if (fs::exists(dir)) {
        reportPlainError("'" + name + "' already exists", "pick a different name for the new game");
        return 1;
    }
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        reportPlainError("couldn't make the folder '" + name + "'");
        return 1;
    }
    for (const auto& [file, text] : templateFiles(templateName, game)) {
        std::ofstream f(dir / file, std::ios::binary);
        f << text;
        printStatus("Created", name + "/" + file);
    }
    std::fprintf(stderr, "\nTo run it:\n    cd %s\n    frg run\n", name.c_str());
    return 0;
}

static void printHelp() {
    std::printf(
        "Forge %s - a compiled language for making games\n\n"
        "Usage:\n"
        "  frg new <name>      make a new game folder with a main.frg\n"
        "      --template window | platformer | 3d   start from a window game, a platformer or a 3D scene\n"
        "                                            instead of the console hello world\n"
        "  frg build [folder]  compile the game into build/<name>.exe\n"
        "  frg run [folder]    compile the game, then run it\n"
        "  frg check [folder]  look for errors without building\n"
        "  frg package [folder]  make a finished build and zip it, ready to share: dist/<name>.zip\n"
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
    if (cmd == "build" || cmd == "run" || cmd == "package") {
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
        if (cmd == "package") {
            if (!opt.release) {
                opt.release = true;  // a package is always a finished build
                if (consoleChoice == -1) opt.console = false;
            }
            return cmdPackage(dir, opt);
        }
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
        int warnings = flushWarnings();
        if (!ok) {
            int n = errorCount();
            std::fprintf(stderr, "%d error%s found.\n", n, n == 1 ? "" : "s");
            return 1;
        }
        printStatus("Checked", proj.name + ": no errors" +
                                   (warnings ? ", " + std::to_string(warnings) + " warning" + (warnings == 1 ? "" : "s") : ""));
        return 0;
    }
    if (cmd == "new") {
        // frg new <name> [--template console|window|platformer|3d]
        std::string name, templateName = "console";
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--template") {
                if (i + 1 >= args.size()) {
                    reportPlainError("--template needs the name of a template", "for example: frg new mygame --template window");
                    return 1;
                }
                templateName = args[++i];
            } else if (args[i].starts_with("--")) {
                reportPlainError("there's no option called " + args[i], "the only option for new is --template");
                return 1;
            } else if (name.empty()) {
                name = args[i];
            } else {
                reportPlainError("too many names", "usage: frg new <name> [--template console|window|platformer|3d]");
                return 1;
            }
        }
        if (name.empty()) {
            reportPlainError("frg new needs a name", "for example: frg new mygame, or frg new mygame --template platformer");
            return 1;
        }
        return cmdNew(name, templateName);
    }
    reportPlainError("unknown command '" + cmd + "'", "try frg build, frg run, or frg new <name>");
    return 1;
}
