// Forge Edit: starting programs in the background (see process.hpp). On Windows they get no console window of their
// own and are put in a job, so stopping one also stops what it started; on Linux they get their own process group.
#include "process.hpp"

#include <mutex>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {
std::wstring wide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}
std::string narrow(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}
// One argument for a Windows command line, quoted the way programs read them back.
std::wstring quoteArg(const std::wstring& a) {
    if (!a.empty() && a.find_first_of(L" \t\"") == std::wstring::npos) return a;
    std::wstring out = L"\"";
    int slashes = 0;
    for (wchar_t c : a) {
        if (c == L'\\') { slashes++; continue; }
        if (c == L'"') out.append(slashes * 2 + 1, L'\\');
        else out.append(slashes, L'\\');
        slashes = 0;
        out += c;
    }
    out.append(slashes * 2, L'\\');
    return out + L"\"";
}
}  // namespace

struct RunningProcess {
    HANDLE process = nullptr;
    HANDLE job = nullptr;
    HANDLE readEnd = nullptr;
    std::thread reader;
    std::mutex lock;
    std::string output;
};

RunningProcess* startProcess(const std::vector<std::string>& args, const std::string& dir) {
    std::wstring cmd;
    for (const std::string& a : args) cmd += (cmd.empty() ? L"" : L" ") + quoteArg(wide(a));
    SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
    HANDLE readEnd = nullptr, writeEnd = nullptr;
    if (!CreatePipe(&readEnd, &writeEnd, &sa, 0)) return nullptr;
    SetHandleInformation(readEnd, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si{};
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = si.hStdError = writeEnd;
    si.hStdInput = nullptr;
    PROCESS_INFORMATION pi{};
    std::wstring wdir = wide(dir);
    BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr,
                             wdir.empty() ? nullptr : wdir.c_str(), &si, &pi);
    CloseHandle(writeEnd);
    if (!ok) {
        CloseHandle(readEnd);
        return nullptr;
    }
    auto* p = new RunningProcess;
    p->process = pi.hProcess;
    p->readEnd = readEnd;
    // everything it starts is in the job too, and the job ends them all when it's closed
    p->job = CreateJobObjectW(nullptr, nullptr);
    if (p->job) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
        info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(p->job, JobObjectExtendedLimitInformation, &info, sizeof info);
        AssignProcessToJobObject(p->job, pi.hProcess);
    }
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    p->reader = std::thread([p] {
        char buf[4096];
        DWORD n = 0;
        while (ReadFile(p->readEnd, buf, sizeof buf, &n, nullptr) && n > 0) {
            std::lock_guard<std::mutex> g(p->lock);
            p->output.append(buf, n);
        }
    });
    return p;
}

bool processEnded(RunningProcess* p, int* code) {
    if (WaitForSingleObject(p->process, 0) != WAIT_OBJECT_0) return false;
    DWORD c = 0;
    GetExitCodeProcess(p->process, &c);
    if (code) *code = (int)c;
    return true;
}

void stopProcess(RunningProcess* p) {
    if (p->job) TerminateJobObject(p->job, 1);
    else TerminateProcess(p->process, 1);
}

void freeProcess(RunningProcess* p) {
    if (!p) return;
    int code;
    if (!processEnded(p, &code)) stopProcess(p);
    WaitForSingleObject(p->process, 5000);
    if (p->job) CloseHandle(p->job);  // ends anything it started that's still running
    if (p->reader.joinable()) p->reader.join();
    CloseHandle(p->readEnd);
    CloseHandle(p->process);
    delete p;
}

std::string exeFolder() {
    std::wstring buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), (DWORD)buf.size());
    buf.resize(n);
    size_t slash = buf.find_last_of(L"\\/");
    return narrow(slash == std::wstring::npos ? buf : buf.substr(0, slash));
}

#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

struct RunningProcess {
    pid_t pid = -1;
    int readEnd = -1;
    bool ended = false;
    int code = 0;
    std::thread reader;
    std::mutex lock;
    std::string output;
};

RunningProcess* startProcess(const std::vector<std::string>& args, const std::string& dir) {
    int fds[2];
    if (pipe(fds) != 0) return nullptr;
    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return nullptr;
    }
    if (pid == 0) {
        setpgid(0, 0);  // its own group, so stopping it stops what it starts too
        dup2(fds[1], 1);
        dup2(fds[1], 2);
        close(fds[0]);
        close(fds[1]);
        if (!dir.empty() && chdir(dir.c_str()) != 0) _exit(127);
        std::vector<char*> argv;
        for (const std::string& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }
    setpgid(pid, pid);  // here too, so it's in place even before the child gets to it
    close(fds[1]);
    auto* p = new RunningProcess;
    p->pid = pid;
    p->readEnd = fds[0];
    p->reader = std::thread([p] {
        char buf[4096];
        for (;;) {
            ssize_t n = read(p->readEnd, buf, sizeof buf);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) break;
            std::lock_guard<std::mutex> g(p->lock);
            p->output.append(buf, (size_t)n);
        }
    });
    return p;
}

bool processEnded(RunningProcess* p, int* code) {
    if (!p->ended) {
        int status = 0;
        if (waitpid(p->pid, &status, WNOHANG) != p->pid) return false;
        p->ended = true;
        p->code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    }
    if (code) *code = p->code;
    return true;
}

void stopProcess(RunningProcess* p) {
    if (!p->ended) kill(-p->pid, SIGTERM);
}

void freeProcess(RunningProcess* p) {
    if (!p) return;
    int code;
    if (!processEnded(p, &code)) {
        kill(-p->pid, SIGKILL);
        int status;
        waitpid(p->pid, &status, 0);
    }
    if (p->reader.joinable()) p->reader.join();
    close(p->readEnd);
    delete p;
}

std::string exeFolder() {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1);
    std::string path = n > 0 ? std::string(buf, (size_t)n) : std::string(".");
    size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
}
#endif

std::string takeOutput(RunningProcess* p) {
    std::lock_guard<std::mutex> g(p->lock);
    std::string out;
    out.swap(p->output);
    return out;
}
