// Is there a newer Forge? Asks GitHub for the latest release, at most once a day (in the background while a
// command runs), and says so if there is one. Updating is never required: `frg update` checks straight away and
// can download and start the installer, `frg update --off` stops the daily check, and FRG_NO_UPDATE_CHECK=1 skips
// it (the tests set that, so they never use the network).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "update.hpp"
#include "common.hpp"

namespace update {

namespace {

const wchar_t* kHost = L"api.github.com";
const wchar_t* kLatest = L"/repos/ChainzYTTTV/frg/releases/latest";
const char* kReleases = "https://github.com/ChainzYTTTV/frg/releases";

struct Release {
    std::string version;    // like 0.9.5
    std::string installer;  // the setup .exe's download link, if the release has one
};

// The version numbers of "v0.9.10" -> {0, 9, 10}.
std::vector<int> parts(std::string v) {
    if (!v.empty() && (v[0] == 'v' || v[0] == 'V')) v.erase(0, 1);
    std::vector<int> out;
    std::stringstream ss(v);
    std::string piece;
    while (std::getline(ss, piece, '.')) out.push_back(std::atoi(piece.c_str()));
    return out;
}

// GETs an https link (following redirects) and gives back the body; nothing on any problem.
std::optional<std::string> get(const std::wstring& host, const std::wstring& path, int timeoutMs, const std::wstring& headers) {
    std::optional<std::string> result;
    HINTERNET session = WinHttpOpen(L"Forge update check", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return result;
    WinHttpSetTimeouts(session, timeoutMs, timeoutMs, timeoutMs, timeoutMs);
    HINTERNET conn = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET req = conn ? WinHttpOpenRequest(conn, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;
    if (req && WinHttpSendRequest(req, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(), (DWORD)-1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(req, nullptr)) {
        DWORD status = 0, size = sizeof status;
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
        if (status == 200) {
            std::string body;
            for (;;) {
                DWORD avail = 0;
                if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0) break;
                std::string chunk(avail, '\0');
                DWORD got = 0;
                if (!WinHttpReadData(req, chunk.data(), avail, &got) || got == 0) break;
                body.append(chunk.data(), got);
            }
            result = body;
        }
    }
    if (req) WinHttpCloseHandle(req);
    if (conn) WinHttpCloseHandle(conn);
    WinHttpCloseHandle(session);
    return result;
}

// The string value of the first "key": "..." in a JSON text at or after `from`.
std::string jsonString(const std::string& json, const std::string& key, size_t from = 0, size_t* end = nullptr) {
    size_t at = json.find("\"" + key + "\"", from);
    if (at == std::string::npos) return "";
    at = json.find(':', at);
    if (at == std::string::npos) return "";
    at = json.find('"', at);
    if (at == std::string::npos) return "";
    size_t close = json.find('"', at + 1);
    if (close == std::string::npos) return "";
    if (end) *end = close;
    return json.substr(at + 1, close - at - 1);
}

std::optional<Release> latest(int timeoutMs) {
    auto body = get(kHost, kLatest, timeoutMs, L"Accept: application/vnd.github+json\r\n");
    if (!body) return std::nullopt;
    Release r;
    r.version = jsonString(*body, "tag_name");
    if (r.version.empty()) return std::nullopt;
    if (r.version[0] == 'v') r.version.erase(0, 1);
    for (size_t at = 0;;) {
        size_t end = 0;
        std::string link = jsonString(*body, "browser_download_url", at, &end);
        if (link.empty()) break;
        if (link.size() > 10 && link.ends_with("-setup.exe")) r.installer = link;
        at = end + 1;
    }
    return r;
}

// The daily check's notes: update.txt next to frg.exe.
fs::path stateFile(const fs::path& exeDir) { return exeDir / "update.txt"; }

struct State {
    long long checked = 0;  // seconds since 1970 of the last check
    std::string latest;     // what it found
    bool off = false;       // frg update --off
};

State readState(const fs::path& exeDir) {
    State s;
    std::ifstream f(stateFile(exeDir));
    std::string key;
    while (f >> key) {
        if (key == "checked") f >> s.checked;
        else if (key == "latest") f >> s.latest;
        else if (key == "off") s.off = true;
    }
    return s;
}

void writeState(const fs::path& exeDir, const State& s) {
    std::ofstream f(stateFile(exeDir));
    f << "checked " << s.checked << "\n";
    if (!s.latest.empty()) f << "latest " << s.latest << "\n";
    if (s.off) f << "off\n";
}

long long now() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

std::future<std::optional<Release>> g_check;
bool g_started = false;

void notice(const std::string& version, const char* current) {
    std::fprintf(stderr, "\n    Update  Forge %s is out (you have %s). Updating is optional: run frg update to get it,\n", version.c_str(), current);
    std::fprintf(stderr, "            or frg update --off to stop these notes.\n");
}

}  // namespace

bool isNewer(const std::string& candidate, const std::string& current) {
    std::vector<int> a = parts(candidate), b = parts(current);
    for (size_t i = 0; i < std::max(a.size(), b.size()); i++) {
        int x = i < a.size() ? a[i] : 0, y = i < b.size() ? b[i] : 0;
        if (x != y) return x > y;
    }
    return false;
}

void startDailyCheck(const fs::path& exeDir) {
    const char* skip = std::getenv("FRG_NO_UPDATE_CHECK");
    if (skip && *skip && std::string(skip) != "0") return;
    State s = readState(exeDir);
    if (s.off || now() - s.checked < 24 * 60 * 60) return;
    s.checked = now();
    writeState(exeDir, s);   // once a day, even when there's no network
    g_check = std::async(std::launch::async, [] { return latest(4000); });
    g_started = true;
}

void finishDailyCheck(const fs::path& exeDir, const char* current) {
    if (!g_started) return;
    g_started = false;
    // the command is done: give the check a moment more, then let it go (it never holds anything up for long)
    if (g_check.wait_for(std::chrono::milliseconds(1500)) != std::future_status::ready) return;
    std::optional<Release> r = g_check.get();
    if (!r) return;
    State s = readState(exeDir);
    s.latest = r->version;
    writeState(exeDir, s);
    if (isNewer(r->version, current)) notice(r->version, current);
}

int command(const std::vector<std::string>& args, const fs::path& exeDir, const char* current) {
    State s = readState(exeDir);
    if (args.size() == 2 && (args[1] == "--off" || args[1] == "--on")) {
        s.off = args[1] == "--off";
        writeState(exeDir, s);
        printStatus("Updates", s.off ? "Forge won't check for new versions any more (frg update still checks when you ask)"
                                     : "Forge checks for new versions once a day again");
        return 0;
    }
    if (args.size() > 1) {
        reportPlainError("frg update doesn't know '" + args[1] + "'", "use frg update, frg update --off or frg update --on");
        return 1;
    }
    std::fprintf(stderr, "    Checking for a new version of Forge...\n");
    std::optional<Release> r = latest(10000);
    if (!r) {
        reportPlainError("couldn't reach GitHub to check for updates", std::string("check your internet connection, or look at ") + kReleases);
        return 1;
    }
    s.checked = now();
    s.latest = r->version;
    writeState(exeDir, s);
    if (!isNewer(r->version, current)) {
        printStatus("Up to date", std::string("you have the latest Forge (") + current + ")");
        return 0;
    }
    std::fprintf(stderr, "    Forge %s is out (you have %s). What's new: %s/tag/v%s\n", r->version.c_str(), current, kReleases, r->version.c_str());
    if (r->installer.empty()) {
        std::fprintf(stderr, "    That release has no installer to download; get it from the page above.\n");
        return 0;
    }
    std::fprintf(stderr, "    Download and start its installer now? It keeps your games, settings and build cache. [y/N] ");
    std::string answer;
    if (!std::getline(std::cin, answer) || (answer != "y" && answer != "Y" && answer != "yes")) {
        std::fprintf(stderr, "    Not now. You can update any time with frg update.\n");
        return 0;
    }
    // download it into the temp folder, then start it (it shows its own window)
    URL_COMPONENTSW uc{sizeof uc};
    std::wstring link(r->installer.begin(), r->installer.end());
    wchar_t host[256], path[2048];
    uc.lpszHostName = host;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(link.c_str(), 0, 0, &uc)) {
        reportPlainError("couldn't read the installer's link", "download it from " + std::string(kReleases));
        return 1;
    }
    std::fprintf(stderr, "    Downloading...\n");
    std::optional<std::string> data = get(host, path, 60000, L"");
    if (!data || data->size() < 100000) {
        reportPlainError("couldn't download the installer", "download it from " + std::string(kReleases));
        return 1;
    }
    wchar_t temp[MAX_PATH];
    GetTempPathW(MAX_PATH, temp);
    fs::path setup = fs::path(temp) / ("Forge-" + r->version + "-setup.exe");
    {
        std::ofstream f(setup, std::ios::binary);
        f.write(data->data(), (std::streamsize)data->size());
    }
    printStatus("Starting", setup.string());
    if ((INT_PTR)ShellExecuteW(nullptr, L"open", setup.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL) <= 32) {
        reportPlainError("couldn't start the installer", "run it yourself: " + setup.string());
        return 1;
    }
    return 0;
}

}  // namespace update
