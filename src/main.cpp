#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "checker.hpp"
#include "codegen.hpp"
#include "common.hpp"
#include "project.hpp"
#include "query.hpp"
#include "format.hpp"
#include "templates.hpp"
#include "update.hpp"

#include <algorithm>
#include <set>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const char* kVersion = "0.9.7";

// Command lines are built in the system's own kind of text: wide (UTF-16) on Windows, so any file name works, and
// ordinary text on Linux. NS("...") is a piece of command line written in the code.
#ifdef _WIN32
using NStr = std::wstring;
#define NS(s) L##s
static NStr toNStr(long long n) { return std::to_wstring(n); }
static const char* kExe = ".exe";  // what a program's file name ends in
#else
using NStr = std::string;
#define NS(s) s
static NStr toNStr(long long n) { return std::to_string(n); }
static const char* kExe = "";
#endif

static fs::path exeDir() {
#ifdef _WIN32
    NStr buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), (DWORD)buf.size());
    return fs::path(buf.substr(0, n)).parent_path();
#else
    std::error_code ec;
    return fs::read_symlink("/proc/self/exe", ec).parent_path();
#endif
}

static NStr quote(const fs::path& p) { return NS("\"") + p.native() + NS("\""); }

#ifndef _WIN32
// Linux: runs a command line with the shell, its output going to `outFd` (or the console if -1). Returns its process.
static pid_t startShell(const std::string& cmdline, const fs::path& workDir, int outFd) {
    pid_t pid = fork();
    if (pid == 0) {
        std::signal(SIGINT, SIG_DFL);  // Ctrl+C stops the program, even while frg itself ignores it
        if (!workDir.empty() && chdir(workDir.c_str()) != 0) _exit(127);
        if (outFd >= 0) {
            dup2(outFd, 1);
            dup2(outFd, 2);
        }
        execl("/bin/sh", "sh", "-c", cmdline.c_str(), (char*)nullptr);
        _exit(127);
    }
    return pid;
}

// The exit code of a finished process: its own code, -1 if it couldn't be started (the shell's 127, "command not
// found"), or 128 + the signal's number if it crashed, like the shell says it.
static int exitCode(int status) {
    if (WIFEXITED(status)) return WEXITSTATUS(status) == 127 ? -1 : WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return -1;
}

static int waitFor(pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR) return -1;
    return exitCode(status);
}
#endif

// Runs a command line and waits for it. Returns the exit code, or -1 if it couldn't start.
static int runProcess(const NStr& cmdline, const fs::path& workDir = {}) {
#ifndef _WIN32
    pid_t pid = startShell(cmdline, workDir, -1);
    return pid < 0 ? -1 : waitFor(pid);
#else
    STARTUPINFOW si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    NStr cmd = cmdline;
    NStr dir = workDir.native();
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, 0, nullptr, dir.empty() ? nullptr : dir.c_str(), &si, &pi))
        return -1;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)code;
#endif
}

// Like runProcess, but collects what the command prints (stdout and stderr) instead of showing it.
static int runCaptured(const NStr& cmdline, std::string& output) {
#ifndef _WIN32
    FILE* p = popen((cmdline + " 2>&1").c_str(), "r");
    if (!p) return -1;
    char buf[4096];
    size_t got;
    while ((got = std::fread(buf, 1, sizeof buf, p)) > 0) output.append(buf, got);
    return exitCode(pclose(p));
#else
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
    NStr cmd = cmdline;
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
#endif
}

// <Vulkan> games: turns every GLSL shader in the game's folder (player.vert, sky.frag...) into the SPIR-V file
// Vulkan loads (player.vert.spv), using glslang. Only shaders that changed since last time are compiled.
static bool compileShaders(const fs::path& root, const fs::path& runtime) {
    static const char* kStages[] = {".vert", ".frag", ".comp", ".geom", ".tesc", ".tese"};
    fs::path glslang = runtime / "tools" / (std::string("glslang") + kExe);
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
        int code = runCaptured(quote(glslang) + NS(" -V ") + quote(file) + NS(" -o ") + quote(spv), out);
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
#ifdef _WIN32
    wchar_t gpp[MAX_PATH];
    if (SearchPathW(nullptr, NS("g++.exe"), nullptr, MAX_PATH, gpp, nullptr)) s += fs::path(gpp).string() + "@" + timeStamp(gpp);
#else
    if (const char* path = std::getenv("PATH")) {
        std::string dirs = path;
        for (size_t at = 0; at <= dirs.size();) {
            size_t end = dirs.find(':', at);
            if (end == std::string::npos) end = dirs.size();
            fs::path gpp = fs::path(dirs.substr(at, end - at)) / "g++";
            std::error_code ec;
            if (!dirs.substr(at, end - at).empty() && fs::exists(gpp, ec)) {
                s += gpp.string() + "@" + timeStamp(fs::canonical(gpp, ec));
                break;
            }
            at = end + 1;
        }
    }
#endif
    return s;
}

// Each cache folder has a stamp.txt naming the Forge and g++ it was made by. When a new folder is made, folders
// from an older Forge (or another g++) are deleted, so the cache doesn't keep growing after updates.
static void removeOldCache(const fs::path& cacheRoot, const std::string& tools) {
    std::error_code ec;
    std::string mine = hashName(tools);
    for (const auto& entry : fs::directory_iterator(cacheRoot, ec)) {
        if (!entry.is_directory()) continue;
        std::ifstream f(entry.path() / "stamp.txt");
        std::string theirs;
        std::getline(f, theirs);
        f.close();
        if (theirs != mine) fs::remove_all(entry.path(), ec);  // a folder in use by a running build just stays
    }
}

// The cache folders this build uses. They're never deleted by limitCache, even if they're the oldest.
static std::vector<fs::path> g_cacheInUse;

// The folder for one cached thing: <frg.exe's folder>/cache/<hash>, or the game's build folder if that can't be written.
// `tools` is toolStamp(): what made it. Using a folder touches its stamp.txt, which is how limitCache knows which
// folders were used least recently.
static fs::path cacheFolder(const std::string& key, const std::string& tools, const fs::path& buildDir) {
    std::error_code ec;
    for (fs::path root : {exeDir() / "cache", buildDir / "cache"}) {
        fs::path dir = root / hashName(tools + "|" + key);
        if (fs::exists(dir / "stamp.txt", ec)) {
            fs::last_write_time(dir / "stamp.txt", fs::file_time_type::clock::now(), ec);
            g_cacheInUse.push_back(dir);
            return dir;
        }
        fs::create_directories(dir, ec);
        if (ec) continue;
        std::ofstream f(dir / "stamp.txt", std::ios::binary);
        if (!(f << hashName(tools) << "\n")) continue;
        f.close();
        removeOldCache(root, tools);
        g_cacheInUse.push_back(dir);
        return dir;
    }
    return buildDir;
}

static uintmax_t folderSize(const fs::path& dir) {
    std::error_code ec;
    uintmax_t total = 0;
    for (auto it = fs::recursive_directory_iterator(dir, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        if (it->is_regular_file(ec)) total += it->file_size(ec);
    }
    return total;
}

static std::string sizeText(uintmax_t bytes) {
    char buf[32];
    if (bytes < 1024 * 1024)
        std::snprintf(buf, sizeof buf, "%.0f KB", bytes / 1024.0);
    else if (bytes >= 1024ull * 1024 * 1024)
        std::snprintf(buf, sizeof buf, "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    else
        std::snprintf(buf, sizeof buf, "%.0f MB", bytes / (1024.0 * 1024.0));
    return buf;
}

// The cache next to frg.exe is kept under about 1 GB: after a build, the sets used least recently are deleted
// until it fits. The ones this build used always stay. Each set of prepared headers is 100-150 MB, and a game
// that's built normally, with --debug and with --release needs three (six, with programs that don't open a
// window). With 400 MB they pushed each other out, so switching between kinds of build prepared them every time.
static const uintmax_t kCacheLimit = 1024ull * 1024 * 1024;

static void limitCache() {
    fs::path root = exeDir() / "cache";
    std::error_code ec;
    struct Set {
        fs::path dir;
        fs::file_time_type used;
        uintmax_t size;
    };
    std::vector<Set> sets;
    uintmax_t total = 0;
    for (const auto& entry : fs::directory_iterator(root, ec)) {
        if (!entry.is_directory()) continue;
        Set s{entry.path(), fs::last_write_time(entry.path() / "stamp.txt", ec), folderSize(entry.path())};
        if (ec) s.used = fs::file_time_type::min();
        total += s.size;
        sets.push_back(s);
    }
    std::sort(sets.begin(), sets.end(), [](const Set& a, const Set& b) { return a.used < b.used; });
    for (const Set& s : sets) {
        if (total <= kCacheLimit) break;
        if (std::find(g_cacheInUse.begin(), g_cacheInUse.end(), s.dir) != g_cacheInUse.end()) continue;
        fs::remove_all(s.dir, ec);
        if (!ec) total -= s.size;
    }
}

// frg cache: shows where the cache is and how big. frg cache clear: deletes it.
static int cmdCache(bool clear) {
    fs::path root = exeDir() / "cache";
    std::error_code ec;
    if (clear && !fs::exists(root, ec)) {
        printStatus("Cleared", "nothing: the build cache is already empty");
        return 0;
    }
    int sets = 0;
    for (const auto& entry : fs::directory_iterator(root, ec))
        if (entry.is_directory()) sets++;
    uintmax_t size = folderSize(root);
    if (!clear) {
        printStatus("Cache", root.string());
        std::fprintf(stderr, "    %s in %d set%s (Forge keeps it under about %s)\n", sizeText(size).c_str(), sets, sets == 1 ? "" : "s",
                     sizeText(kCacheLimit).c_str());
        std::fprintf(stderr, "    It makes builds faster. To delete it: frg cache clear\n");
        return 0;
    }
    fs::remove_all(root, ec);
    if (ec) {
        reportPlainError("couldn't delete all of " + root.string() + " (" + ec.message() + ")",
                         "is a game being built right now? Wait for it to finish, then try again");
        return 1;
    }
    printStatus("Cleared", "the build cache (" + sizeText(size) + "). The next build prepares it again.");
    return 0;
}

// Runs one g++ command that makes `output`, writing to a temporary name first so a half-written file is never used.
static int compileTo(const NStr& args, const fs::path& output) {
    fs::path tmp = output;
#ifdef _WIN32
    tmp += NS(".") + toNStr(GetCurrentProcessId()) + NS(".tmp");
#else
    tmp += NS(".") + toNStr(getpid()) + NS(".tmp");
#endif
    int code = runProcess(NS("g++ ") + args + NS(" -o ") + quote(tmp));
    std::error_code ec;
    if (code == 0) fs::rename(tmp, output, ec);
    if (code != 0 || ec) fs::remove(tmp, ec);
    return code;
}

// ---------------------------------------------------------------- compiling several parts at once

// One g++ run. Its messages go to a log file instead of the console, because several run at the same time.
struct Job {
    NStr args;          // everything after "g++"
    fs::path log;
#ifdef _WIN32
    HANDLE process = nullptr;
    HANDLE logFile = nullptr;
#else
    pid_t process = -1;
#endif
    int code = 0;       // its exit code once it's done; -1 if it couldn't be started
};

#ifndef _WIN32
static bool startJob(Job& j) {
    int fd = open(j.log.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) return false;
    j.process = startShell("g++ " + j.args, {}, fd);
    close(fd);
    return j.process > 0;
}

static int jobLimit() {
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    long pages = sysconf(_SC_AVPHYS_PAGES), pageSize = sysconf(_SC_PAGESIZE);
    if (pages > 0 && pageSize > 0) cores = std::min<long>(cores, (long)((unsigned long long)pages * pageSize / (700ull * 1024 * 1024)));
    return (int)std::clamp<long>(cores, 1, 64);
}

static void runJobs(std::vector<Job>& jobs, int limit) {
    size_t next = 0;
    std::vector<Job*> running;
    while (next < jobs.size() || !running.empty()) {
        while (next < jobs.size() && (int)running.size() < limit) {
            Job& j = jobs[next++];
            if (startJob(j))
                running.push_back(&j);
            else
                j.code = -1;
        }
        if (running.empty()) break;
        int status = 0;
        pid_t done = waitpid(-1, &status, 0);
        if (done < 0) {
            if (errno == EINTR) continue;
            for (Job* j : running) j->code = waitFor(j->process);
            break;
        }
        for (size_t i = 0; i < running.size(); i++)
            if (running[i]->process == done) {
                running[i]->code = exitCode(status);
                running.erase(running.begin() + i);
                break;
            }
    }
}
#else
static bool startJob(Job& j) {
    SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
    j.logFile = CreateFileW(j.log.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (j.logFile == INVALID_HANDLE_VALUE) {
        j.logFile = nullptr;
        return false;
    }
    // Only this job's log file is passed on to g++, not the other jobs' (which would stay open until it ends).
    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    std::vector<char> buf(size);
    auto attrs = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buf.data());
    InitializeProcThreadAttributeList(attrs, 1, 0, &size);
    HANDLE inherit[1] = {j.logFile};
    UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherit, sizeof inherit, nullptr, nullptr);
    STARTUPINFOEXW si{};
    si.StartupInfo.cb = sizeof si;
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdOutput = si.StartupInfo.hStdError = j.logFile;
    si.lpAttributeList = attrs;
    PROCESS_INFORMATION pi{};
    NStr cmd = NS("g++ ") + j.args;
    BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr,
                             &si.StartupInfo, &pi);
    DeleteProcThreadAttributeList(attrs);
    if (!ok) {
        CloseHandle(j.logFile);
        j.logFile = nullptr;
        return false;
    }
    CloseHandle(pi.hThread);
    j.process = pi.hProcess;
    return true;
}

// How many g++ runs to have going at once: one per processor, but each needs several hundred MB of memory.
static int jobLimit() {
    int cores = (int)GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof mem;
    if (GlobalMemoryStatusEx(&mem)) cores = std::min<int>(cores, (int)(mem.ullAvailPhys / (700ull * 1024 * 1024)));
    return std::clamp(cores, 1, (int)MAXIMUM_WAIT_OBJECTS);
}

// Runs every job, up to `limit` at a time, and waits until they've all finished.
static void runJobs(std::vector<Job>& jobs, int limit) {
    size_t next = 0;
    std::vector<Job*> running;
    while (next < jobs.size() || !running.empty()) {
        while (next < jobs.size() && (int)running.size() < limit) {
            Job& j = jobs[next++];
            if (startJob(j))
                running.push_back(&j);
            else
                j.code = -1;
        }
        if (running.empty()) break;
        std::vector<HANDLE> handles;
        for (Job* j : running) handles.push_back(j->process);
        DWORD w = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), FALSE, INFINITE);
        size_t done = w - WAIT_OBJECT_0;
        if (done >= running.size()) done = 0;  // shouldn't happen; wait for the first one properly next time round
        Job* j = running[done];
        if (WaitForSingleObject(j->process, INFINITE) == WAIT_OBJECT_0) {
            DWORD code = 0;
            GetExitCodeProcess(j->process, &code);
            j->code = (int)code;
        }
        CloseHandle(j->process);
        CloseHandle(j->logFile);
        running.erase(running.begin() + done);
    }
}
#endif

static std::string readFile(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Writes a file only if its contents changed, so unchanged files keep their old dates.
static void writeIfChanged(const fs::path& p, const std::string& text) {
    std::error_code ec;
    if (fs::exists(p, ec) && readFile(p) == text) return;
    std::ofstream f(p, std::ios::binary);
    f << text;
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
#ifndef _WIN32
    return {};  // Linux programs don't carry an icon inside them; the window still gets icon.png
#endif
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
    int code = runProcess(NS("windres -O coff -i icon.rc -o icon_res.o"), buildDir);
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
#ifdef _WIN32
static BOOL WINAPI ignoreCtrlC(DWORD type) { return type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT; }
static void ignoreCtrlCWhileRunning() { SetConsoleCtrlHandler(ignoreCtrlC, TRUE); }
#else
static void ignoreCtrlCWhileRunning() { std::signal(SIGINT, SIG_IGN); }
#endif

// How a game is built. The console is on for normal builds (to see print() and errors while testing) and off
// for --release builds; --console and --no-console choose it either way.
struct BuildOptions {
    bool release = false;
    bool console = true;
    bool debug = false;  // --debug: not optimized, with debug information and each line of C++ marked with its .frg line
    bool test = false;   // frg test: with the tests/ folder, and a main() that runs the tests, in build/tests/
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
    if (!loadProject(proj, dir, opt.test) || !checkProject(proj)) {
        int n = errorCount();
        std::fprintf(stderr, "Build failed: %d error%s.\n", n, n == 1 ? "" : "s");
        return false;
    }
    flushWarnings();
    printStatus("Compiling", proj.name + " (" + std::to_string(proj.allModules.size()) + " file" +
                                 (proj.allModules.size() == 1 ? "" : "s") + ")");

    fs::path buildDir = opt.test ? proj.root / "build" / "tests" : proj.root / "build";
    std::error_code ec;
    fs::create_directories(buildDir, ec);
    // FRG_VERBOSE=1 shows how long each step of the build takes.
    bool verbose = std::getenv("FRG_VERBOSE") != nullptr;
    auto phaseStart = start;
    auto phase = [&](const char* name) {
        auto now = std::chrono::steady_clock::now();
        if (verbose) std::fprintf(stderr, "    [build] %s: %.2fs\n", name, std::chrono::duration<double>(now - phaseStart).count());
        phaseStart = now;
    };
    phase("reading and checking");
    GeneratedCpp gen = generateCpp(proj, opt.debug, opt.test);
    phase("generating C++");
    fs::path exe = buildDir / (proj.name + (opt.test ? "_tests" : "") + kExe);

    fs::path runtime = exeDir() / "runtime";
    if (!fs::exists(runtime / "forge.hpp")) {
        reportPlainError("Forge's runtime files are missing (" + (runtime / "forge.hpp").string() + ")",
                         "keep the runtime folder next to frg.exe");
        return false;
    }

#ifndef _WIN32
    // The low-level toolkits open their own windows with Windows' own code, so they're Windows-only for now.
    if (!gen.sources.empty()) {
        reportPlainError("the <OpenGL>, <DirectX> and <Vulkan> toolkits only work on Windows for now",
                         "everything else in Forge works on Linux: <window>, <graphics2d>, <graphics3d> and the rest");
        return false;
    }
#endif
    bool vulkan = std::find(gen.sources.begin(), gen.sources.end(), "forge_vulkan_impl.cpp") != gen.sources.end();
    if (vulkan && !compileShaders(proj.root, runtime)) {
        std::fprintf(stderr, "Build failed: a shader has errors.\n");
        return false;
    }

    // Release builds are optimized more (-O2). Other builds use -O1, which compiles faster and still runs well.
    // -fwrapv: whole numbers wrap around when they overflow, the same at every optimization level.
    NStr flags = NStr(NS("-std=gnu++20 ")) + (opt.debug ? NS("-O0 -g") : opt.release ? NS("-O2") : NS("-O1")) + NS(" -fwrapv -w -I") + quote(runtime);
#ifdef _WIN32
    if (!opt.console) flags += NS(" -DFRG_NO_CONSOLE");
#endif  // no console window: runtime errors pop up in a box instead
    if (vulkan) flags += NS(" -I") + quote(runtime / "vulkan" / "include");
    fs::path raylib = runtime / "raylib";
    if (gen.needsRaylib) flags += NS(" -I") + quote(raylib / "include");
    std::string tools = toolStamp(runtime), stamp = fs::path(flags).string();

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
    fs::path pchDir = cacheFolder("pch|" + stamp + "|" + pchText, tools, buildDir);
    fs::path pchHeader = pchDir / "forge_pch.hpp", pch = pchDir / "forge_pch.hpp.gch";
    if (!fs::exists(pchHeader)) {
        std::ofstream f(pchHeader, std::ios::binary);
        f << pchText;
    }
    if (!fs::exists(pch)) {
        printStatus("Preparing", "Forge's runtime for this kind of build (once each for normal, --debug and --release builds, and again after Forge is updated)");
        if (compileTo(flags + NS(" -x c++-header ") + quote(pchHeader), pch) == -1) {
            reportPlainError("couldn't find g++, which Forge uses to make .exe files",
                             "install w64devkit and make sure its bin folder is on your PATH");
            return false;
        }
    }

    // A toolkit's own runtime code, compiled once into an object file.
    std::vector<fs::path> objects;
    for (const std::string& src : gen.sources) {
        fs::path obj = cacheFolder("obj|" + stamp + "|" + src, tools, buildDir) / (fs::path(src).stem().string() + ".o");
        if (!fs::exists(obj)) {
            printStatus("Preparing", src + " (only the first time)");
            if (compileTo(flags + NS(" -c ") + quote(runtime / src), obj) != 0) {
                reportPlainError("the C++ compiler rejected Forge's runtime file " + src + " - this is a bug in Forge, not in your code");
                return false;
            }
        }
        objects.push_back(obj);
    }

    // ---- The game's C++, in parts (build/forge/). See "split builds" in docs/how-forge-works.md.
    // A small game is one part. A bigger one is split into more parts, each a share of the .frg files, so the
    // parts can be compiled at the same time and an edit only recompiles the part with that file in it. Each part
    // also has to load Forge's runtime (about half a second), so parts aren't made smaller than about 48 KB of C++.
    phase("preparing the runtime");
    fs::path forgeDir = buildDir / "forge";
    fs::create_directories(forgeDir, ec);
    fs::remove(buildDir / "forge_generated.cpp", ec);  // the one big file made by Forge 0.8 and older
    size_t total = 0;
    for (const GeneratedModule& m : gen.modules) total += m.code.size();
    const size_t kPartSize = 48 * 1024;
    size_t parts = 1;
    if (total > 64 * 1024)
        while (parts < 64 && parts * kPartSize < total) parts *= 2;  // only powers of two, so it rarely changes
    // Each file always goes in the same part (chosen from its name, not its size), so an edit can't move the others.
    std::vector<std::string> partCode(parts);
    for (const GeneratedModule& m : gen.modules) partCode[parts == 1 ? 0 : std::stoull(hashName(m.path), nullptr, 16) % parts] += m.code;

    // Every part includes the game's declarations (game.hpp). In a big game those are a lot to read, so then they're
    // precompiled too, together with the runtime (game_pch.hpp), and kept in build/forge/ (this one is the game's own).
    writeIfChanged(forgeDir / "game.hpp", gen.header);
    bool bigHeader = parts > 1 && gen.header.size() > 96 * 1024;
    std::string partStart, pchKey;
    if (bigHeader) {
        std::string gamePchText = pchText + "#include \"game.hpp\"\n";
        writeIfChanged(forgeDir / "game_pch.hpp", gamePchText);
        pchKey = hashName(tools + "|" + stamp + "|" + gamePchText + "|" + gen.header);
        fs::path gamePch = forgeDir / "game_pch.hpp.gch", keyFile = forgeDir / "game_pch.key";
        if (!fs::exists(gamePch, ec) || readFile(keyFile) != pchKey) {
            fs::remove(keyFile, ec);
            if (compileTo(flags + NS(" -x c++-header ") + quote(forgeDir / "game_pch.hpp"), gamePch) != 0) {
                reportPlainError("the C++ compiler rejected Forge's output (game.hpp) - this is a bug in Forge, not in your code",
                                 "the generated file is " + (forgeDir / "game.hpp").string());
                return false;
            }
            writeIfChanged(keyFile, pchKey);
        }
        partStart = "#include \"game_pch.hpp\"  // Forge's runtime and this game's declarations, precompiled\n";
    } else {
        fs::remove(forgeDir / "game_pch.hpp.gch", ec);
        pchKey = hashName(gen.header);
        partStart = "#include \"forge_pch.hpp\"  // Forge's runtime headers, precompiled to make builds faster\n#include \"game.hpp\"\n";
    }
    const std::string partHead = "// Generated by Forge from your .frg files. Edit those instead - this file is rebuilt every time.\n" + partStart + "\n";

    struct Part {
        std::string name;  // "part3" or "main"
        std::string code;
    };
    std::vector<Part> all;
    if (parts == 1) {
        // A small game is a single part, with main() in it too: each part costs a runtime load, so fewer is faster.
        all.push_back({"game", partHead + partCode[0] + gen.mainCode});
    } else {
        for (size_t i = 0; i < parts; i++)
            if (!partCode[i].empty()) all.push_back({"part" + std::to_string(i + 1), partHead + partCode[i]});
        all.push_back({"main", partHead + gen.mainCode});
    }

    // Parts left over from a build that was split differently.
    std::set<std::string> keep = {"game.hpp", "game_pch.hpp", "game_pch.hpp.gch", "game_pch.key", "link.key"};
    for (const Part& p : all)
        for (const char* ext : {".cpp", ".o", ".key", ".log"}) keep.insert(p.name + ext);
    for (const auto& entry : fs::directory_iterator(forgeDir, ec))
        if (entry.is_regular_file() && !keep.count(entry.path().filename().string())) fs::remove(entry.path(), ec);

    phase("writing the parts");
    // Compile the parts that changed. A part's key says everything it was made from; if it matches, its .o is reused.
    // A one-part game is compiled and linked by a single g++ run below instead, which is quicker than two.
    bool single = all.size() == 1;
    std::vector<Job> jobs;
    std::vector<const Part*> jobParts;
    std::string linkKey = tools + "|" + stamp + "|" + pchKey;
    for (const Part& p : all) {
        fs::path cpp = forgeDir / (p.name + ".cpp"), obj = forgeDir / (p.name + ".o"), keyFile = forgeDir / (p.name + ".key");
        writeIfChanged(cpp, p.code);
        std::string key = hashName(tools + "|" + stamp + "|" + pchKey + "|" + p.code);
        linkKey += "|" + key;
        if (single || (fs::exists(obj, ec) && readFile(keyFile) == key)) continue;
        fs::remove(keyFile, ec);
        Job j;
        j.args = flags + NS(" -I") + quote(pchDir) + NS(" -I") + quote(forgeDir) + NS(" -c ") + quote(cpp) + NS(" -o ") + quote(obj);
        j.log = forgeDir / (p.name + ".log");
        jobs.push_back(j);
        jobParts.push_back(&p);
    }
    if (!jobs.empty()) {
        // The biggest parts start first, so a slow one doesn't end up running alone at the end.
        std::vector<size_t> order(jobs.size());
        for (size_t i = 0; i < order.size(); i++) order[i] = i;
        std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) { return jobParts[a]->code.size() > jobParts[b]->code.size(); });
        std::vector<Job> sorted;
        std::vector<const Part*> sortedParts;
        for (size_t i : order) {
            sorted.push_back(jobs[i]);
            sortedParts.push_back(jobParts[i]);
        }
        jobs = std::move(sorted);
        jobParts = std::move(sortedParts);
        int limit = jobLimit();
        if (std::getenv("FRG_VERBOSE"))
            std::fprintf(stderr, "    [build] %zu parts, compiling %zu, %d at a time, declarations %s\n", all.size(), jobs.size(), limit,
                         bigHeader ? "precompiled" : "read by each part");
        auto t0 = std::chrono::steady_clock::now();
        runJobs(jobs, limit);
        if (std::getenv("FRG_VERBOSE"))
            std::fprintf(stderr, "    [build] compiling took %.2fs\n", std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        for (size_t i = 0; i < jobs.size(); i++) {
            const Part& p = *jobParts[i];
            if (jobs[i].code == -1) {
                reportPlainError("couldn't find g++, which Forge uses to make .exe files",
                                 "install w64devkit and make sure its bin folder is on your PATH");
                return false;
            }
            if (jobs[i].code != 0) {
                std::string log = readFile(jobs[i].log);
                if (log.size() > 2000) log = log.substr(0, 2000) + "\n...";
                reportPlainError("the C++ compiler rejected Forge's output - this is a bug in Forge, not in your code",
                                 "the generated file is " + (forgeDir / (p.name + ".cpp")).string() + ", and g++'s message is in " +
                                     jobs[i].log.string());
                std::fprintf(stderr, "%s\n", log.c_str());
                return false;
            }
            writeIfChanged(forgeDir / (p.name + ".key"), hashName(tools + "|" + stamp + "|" + pchKey + "|" + p.code));
        }
    }

    phase("compiling");
    // Link: the parts, a toolkit's runtime code and the icon, into the .exe. Skipped if none of it changed.
    NStr cmd = NS("g++");
    if (single)
        cmd += NS(" ") + flags + NS(" -I") + quote(pchDir) + NS(" -I") + quote(forgeDir) + NS(" ") + quote(forgeDir / (all[0].name + ".cpp"));
    else
        for (const Part& p : all) cmd += NS(" ") + quote(forgeDir / (p.name + ".o"));
    for (const fs::path& obj : objects) cmd += NS(" ") + quote(obj);
    fs::path icon = makeIcon(proj.root, buildDir);
    if (!icon.empty()) cmd += NS(" ") + quote(icon);
#ifdef _WIN32
    cmd += NS(" -o ") + quote(exe) + NS(" -static");
    if (!opt.console) cmd += NS(" -mwindows");
#else
    // Linux: C++'s own library goes into the game, so it runs on other Linux PCs too; the system's are shared.
    cmd += NS(" -o ") + quote(exe) + NS(" -static-libstdc++ -static-libgcc");
#endif
    if (opt.release) cmd += NS(" -s");  // a smaller .exe
#ifdef _WIN32
    if (gen.needsRaylib) cmd += NS(" -L") + quote(raylib / "lib") + NS(" -lraylib -lopengl32 -lgdi32 -lwinmm");
#else
    if (gen.needsRaylib) cmd += " -L" + quote(raylib / "lib-linux") + " -lraylib -lGL -lm -lpthread -ldl -lrt -lX11";
    else cmd += " -lpthread";
#endif
    for (const std::string& flag : gen.linkFlags) cmd += NS(" ") + fs::path(flag).native();
    linkKey = hashName(linkKey + "|" + fs::path(cmd).string() + "|" + (icon.empty() ? "" : timeStamp(icon)));
    bool linked = false;
    if (!fs::exists(exe, ec) || readFile(forgeDir / "link.key") != linkKey) {
        if (fs::exists(exe, ec)) {
            fs::remove(exe, ec);
            if (ec) {
                reportPlainError("couldn't replace " + exe.string(), "is the game still running? Close it and try again");
                return false;
            }
        }
        fs::remove(forgeDir / "link.key", ec);
        if (verbose) std::fprintf(stderr, "    [build] %s\n", fs::path(cmd).string().c_str());
        int code = runProcess(cmd);
        if (code == -1) {
            reportPlainError("couldn't find g++, which Forge uses to make .exe files",
                             "install w64devkit and make sure its bin folder is on your PATH");
            return false;
        }
        if (code != 0) {
            reportPlainError(std::string("the C++ ") + (single ? "compiler" : "linker") + " rejected Forge's output - this is a bug in Forge, not in your code",
                             "the generated files are in " + forgeDir.string());
            return false;
        }
        writeIfChanged(forgeDir / "link.key", linkKey);
        linked = true;
    }
    phase("linking");
    limitCache();

    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    char timeBuf[32];
    std::snprintf(timeBuf, sizeof timeBuf, "%.2fs", secs);
    std::string note = !linked ? " (nothing changed)" : all.size() > 2 ? " (" + std::to_string(jobs.size()) + " of " + std::to_string(all.size()) + " parts compiled)" : "";
#ifdef _WIN32
    bool noConsole = !opt.console;
#else
    bool noConsole = false;  // a Linux program shows its output wherever it's started from
#endif
    printStatus("Finished", fs::relative(exe, proj.root, ec).generic_string() + (opt.debug ? " (debug)" : "") + (noConsole ? " (no console)" : "") + note + " in " + timeBuf);
    if (noConsole && !gen.needsRaylib)
        std::fprintf(stderr, "    note: this game doesn't open a window, so without a console nothing will show when it runs\n");
    exeOut = exe;
    return true;
}

// frg test: builds the game with its tests/ folder (into build/tests/) and runs every test function in there.
static int cmdTest(const fs::path& dir, BuildOptions opt) {
    std::error_code ec;
    if (fs::is_directory(dir, ec) && !fs::is_directory(dir / "tests", ec)) {
        reportPlainError("there's no tests folder in " + fs::absolute(dir).string(),
                         "make a folder called tests next to main.frg, with files like tests/player_test.frg holding functions "
                         "like void testJump(){ ... } that use testing.check(...) (they need #include <testing>)");
        return 1;
    }
    opt.test = true;
    opt.release = false;
    fs::path exe;
    if (!build(dir, exe, opt)) return 1;
    printStatus("Testing", exe.filename().string());
    std::fflush(stderr);
    int code = runProcess(quote(exe), fs::absolute(dir));
    if (code == -1) {
        reportPlainError("couldn't start " + exe.string());
        return 1;
    }
    return code;
}

static int cmdRun(const fs::path& dir, const BuildOptions& opt) {
    fs::path exe;
    if (!build(dir, exe, opt)) return 1;
    printStatus("Running", exe.filename().string());
    std::fflush(stderr);
    ignoreCtrlCWhileRunning();
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

#ifdef _WIN32
    // Windows 10 and newer come with tar.exe, which can make .zip files. It's called by its full path, because
    // w64devkit has a different tar that can't.
    wchar_t sys[MAX_PATH];
    UINT n = GetSystemDirectoryW(sys, MAX_PATH);
    fs::path tar = fs::path(NStr(sys, n)) / "tar.exe";
    int code = runProcess(quote(tar) + NS(" -a -c -f ") + quote(zip.filename()) + NS(" ") + quote(fs::path(name)), dist);
    const char* missing = "Windows' tar.exe wasn't found (it comes with Windows 10 and newer). ";
#else
    int code = runProcess("command -v zip > /dev/null") == 0 ? runProcess("zip -q -r " + quote(zip.filename()) + " " + quote(fs::path(name)), dist) : -1;
    const char* missing = "";
    if (code == -1) {  // no zip program: a .tar.gz, which every Linux can make (and unpack)
        zip = dist / (name + ".tar.gz");
        code = runProcess("tar -czf " + quote(zip.filename()) + " " + quote(fs::path(name)), dist);
    }
#endif
    if (code != 0 || !fs::exists(zip)) {
        reportPlainError("couldn't make the .zip file", code == -1 ? std::string(missing) +
                                                                    "The game is ready in " + out.string() + " - you can zip that folder yourself"
                                                                  : "the game is ready in " + out.string() + " - you can zip that folder yourself");
        return 1;
    }
    printStatus("Packaged", "dist/" + zip.filename().string());
#ifdef _WIN32
    std::fprintf(stderr, "\nYour game is ready to share:\n    %s\nPlayers unzip it and double-click %s.\n", zip.string().c_str(),
                 exe.filename().string().c_str());
#else
    std::fprintf(stderr, "\nYour game is ready to share:\n    %s\nPlayers unpack it and run ./%s in its folder.\n", zip.string().c_str(),
                 exe.filename().string().c_str());
#endif
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
        "  frg test [folder]   run the tests in the game's tests folder\n"
        "  frg fmt [folders or files]  tidy the layout of .frg files (indentation, braces, blank lines)\n"
        "      --check                  only list the files that aren't tidy (exit code 1 if there are any)\n"
        "      --print                  print the tidied files instead of changing them\n"
        "  frg package [folder]  make a finished build and zip it, ready to share: dist/<name>.zip\n"
        "  frg cache           show how big the build cache is (frg cache clear deletes it)\n"
        "  frg update          check for a newer Forge, and install it if you want (--off: no daily check)\n"
        "  frg version         show the Forge version\n\n"
        "[folder] is the game's top folder (the one with main.frg). It defaults to the current folder.\n"
        "Options for build, run, test and package:\n"
        "  --release           make a finished game: no console window, and a smaller .exe\n"
        "  --debug             a build for the debugger: breakpoints and variables on your .frg lines\n"
        "  --no-console        no console window (print() output isn't shown; runtime errors pop up in a box)\n"
        "  --console           keep the console window, even with --release\n"
        "Without options the console is on, so print() and errors show while you test the game.\n\n"
        "Every command and option is explained in docs/reference/commands.md.\n",
        kVersion);
}

static int dispatch(const std::vector<std::string>& args) {
    if (args.empty() || args[0] == "help" || args[0] == "--help" || args[0] == "-h") {
        printHelp();
        return 0;
    }
    const std::string& cmd = args[0];
    if (cmd == "version" || cmd == "--version") {
        std::printf("Forge %s\n", kVersion);
        return 0;
    }
    if (cmd == "build" || cmd == "run" || cmd == "package" || cmd == "test") {
        BuildOptions opt;
        int consoleChoice = -1;  // -1: not chosen, 0: --no-console, 1: --console
        std::vector<std::string> rest;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--release") {
                opt.release = true;
            } else if (args[i] == "--debug") {
                opt.debug = true;
            } else if (args[i] == "--console" || args[i] == "--no-console") {
                int choice = args[i] == "--console" ? 1 : 0;
                if (consoleChoice != -1 && consoleChoice != choice) {
                    reportPlainError("--console and --no-console can't both be used", "pick one of them");
                    return 1;
                }
                consoleChoice = choice;
            } else if (args[i].starts_with("--")) {
                reportPlainError("there's no option called " + args[i], "the options are --release, --debug, --console and --no-console");
                return 1;
            } else {
                rest.push_back(args[i]);
            }
        }
        if (rest.size() > 1) {
            reportPlainError("too many arguments", "usage: frg " + cmd + " [folder] [--release | --debug] [--console | --no-console]");
            return 1;
        }
        if (opt.release && opt.debug) {
            reportPlainError("--release and --debug can't both be used",
                             "--debug is for finding mistakes with a debugger, --release for the finished game");
            return 1;
        }
        opt.console = consoleChoice == -1 ? !opt.release : consoleChoice == 1;
        fs::path dir = rest.empty() ? fs::current_path() : fs::path(rest[0]);
        if (cmd == "run") return cmdRun(dir, opt);
        if (cmd == "test") return cmdTest(dir, opt);
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
    if (cmd == "fmt") {
        // frg fmt [folders or files] [--check] [--print] [--stdin]: tidies the layout of .frg files (see format.cpp).
        bool check = false, print = false, fromStdin = false;
        std::vector<fs::path> targets;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--check") check = true;
            else if (args[i] == "--print") print = true;
            else if (args[i] == "--stdin") fromStdin = true;
            else if (args[i].starts_with("--")) {
                reportPlainError("there's no option called " + args[i], "the options are --check, --print and --stdin");
                return 1;
            } else targets.push_back(args[i]);
        }
        if (fromStdin) {  // for editors: the text on standard input, tidied, on standard output
            std::string text((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
            FormatResult r = formatSource(text);
            if (!r.ok) {
                std::fprintf(stderr, "%s\n", r.error.c_str());
                return 1;
            }
#ifdef _WIN32
            _setmode(_fileno(stdout), _O_BINARY);  // the text exactly: Windows would turn each line ending into two characters
#endif
            std::fwrite(r.text.data(), 1, r.text.size(), stdout);
            return 0;
        }
        if (targets.empty()) targets.push_back(fs::current_path());
        std::vector<fs::path> files;
        std::error_code ec;
        for (const fs::path& t : targets) {
            if (fs::is_directory(t, ec)) {
                for (auto it = fs::recursive_directory_iterator(t, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
                    std::string name = it->path().filename().string();
                    if (it->is_directory() && (name == "build" || name == "dist" || name.starts_with("."))) {
                        it.disable_recursion_pending();
                        continue;
                    }
                    if (it->is_regular_file() && it->path().extension() == ".frg") files.push_back(it->path());
                }
            } else if (fs::is_regular_file(t, ec)) {
                files.push_back(t);
            } else {
                reportPlainError("there's no file or folder called '" + t.string() + "'");
                return 1;
            }
        }
        std::sort(files.begin(), files.end());
        int changed = 0, failed = 0;
        for (const fs::path& f : files) {
            std::ifstream in(f, std::ios::binary);
            std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();
            FormatResult r = formatSource(text);
            std::string shown = fs::relative(f, fs::current_path(), ec).string();
            if (ec || shown.empty()) shown = f.string();
            if (!r.ok) {
                reportPlainError("couldn't tidy " + shown + ": " + r.error);
                failed++;
                continue;
            }
            if (print) {
                std::fwrite(r.text.data(), 1, r.text.size(), stdout);
                continue;
            }
            if (!r.changed) continue;
            changed++;
            if (check) {
                printStatus("Untidy", shown);
                continue;
            }
            std::ofstream out(f, std::ios::binary);
            out << r.text;
            printStatus("Tidied", shown);
        }
        if (print) return failed ? 1 : 0;
        if (check) {
            if (changed) std::fprintf(stderr, "%d of %zu files would be tidied by frg fmt.\n", changed, files.size());
            else printStatus("Tidy", std::to_string(files.size()) + " file" + (files.size() == 1 ? " is" : "s are") + " already tidy");
            return changed || failed ? 1 : 0;
        }
        if (!changed && !failed) printStatus("Tidy", std::to_string(files.size()) + " file" + (files.size() == 1 ? " was" : "s were") + " already tidy");
        return failed ? 1 : 0;
    }
    if (cmd == "query") {
        // frg query <question> <folder> <file> <line> <column> [--stdin] [new name]: for the VS Code extension.
        std::vector<std::string> rest;
        bool fromStdin = false;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--stdin")
                fromStdin = true;
            else
                rest.push_back(args[i]);
        }
        if (rest.size() < 5) {
            std::printf("{\"error\":\"usage: frg query <question> <folder> <file> <line> <column> [--stdin] [new name]\"}\n");
            return 1;
        }
        int line = std::atoi(rest[3].c_str()), col = std::atoi(rest[4].c_str());
        return runQuery(rest[0], rest[1], rest[2], line, col, fromStdin, rest.size() > 5 ? rest[5] : "");
    }
    if (cmd == "check") {
        // frg check [folder] [--machine]: find errors without building anything.
        fs::path dir = fs::current_path();
        int folders = 0;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--machine") {
                setMachineOutput(true);
            } else if (args[i].starts_with("--")) {
                reportPlainError("there's no option called " + args[i], "frg check only takes a folder: frg check [folder]");
                return 1;
            } else if (folders++) {
                reportPlainError("too many arguments", "usage: frg check [folder]");
                return 1;
            } else {
                dir = args[i];
            }
        }
        if (!fs::is_regular_file(dir / "main.frg")) {
            reportPlainError("there's no main.frg in " + fs::absolute(dir).string(),
                             "every Forge game needs a main.frg in its top folder");
            return 1;
        }
        Project proj;
        bool ok = loadProject(proj, dir, true) && checkProject(proj);  // the tests folder too
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
    if (cmd == "cache") {
        if (args.size() == 1) return cmdCache(false);
        if (args.size() == 2 && args[1] == "clear") return cmdCache(true);
        reportPlainError("frg cache doesn't know '" + (args.size() > 1 ? args[1] : "") + "'",
                         "use frg cache to see how big the build cache is, or frg cache clear to delete it");
        return 1;
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
    if (cmd == "update") return update::command(args, exeDir(), kVersion);
    reportPlainError("unknown command '" + cmd + "'", "try frg build, frg run, or frg new <name>");
    return 1;
}

// An installed Forge has its own g++ next to it (w64devkit, from the installer): that one is used, so a build never
// depends on what else is on the PATH. (Without it, the g++ on the PATH is used.)
static void useBundledCompiler() {
#ifndef _WIN32
    return;  // on Linux, the system's g++ is used
#endif
    fs::path kit = exeDir().parent_path() / "w64devkit" / "bin";
    std::error_code ec;
    if (!fs::exists(kit / "g++.exe", ec)) return;
#ifdef _WIN32
    static wchar_t old[32767];
    DWORD n = GetEnvironmentVariableW(NS("PATH"), old, 32767);
    NStr path = kit.native() + NS(";") + (n ? NStr(old, n) : NS(""));
#endif
#ifdef _WIN32
    SetEnvironmentVariableW(NS("PATH"), path.c_str());
#endif
}

int main(int argc, char** argv) {
    initConsole();
    useBundledCompiler();
    std::vector<std::string> args(argv + 1, argv + argc);
    // once a day, building, running, testing, packaging or making a game also looks for a newer Forge (in the
    // background; a short note at the end if there is one)
    std::string cmd = args.empty() ? "" : args[0];
    bool daily = cmd == "build" || cmd == "run" || cmd == "test" || cmd == "package" || cmd == "new";
    if (daily) update::startDailyCheck(exeDir());
    int code = dispatch(args);
    if (daily) update::finishDailyCheck(exeDir(), kVersion);
    return code;
}
