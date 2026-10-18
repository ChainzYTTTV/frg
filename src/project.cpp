#include "project.hpp"

#include "builtins.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "prelude.hpp"
#include "types.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

static bool isIdentifier(const std::string& s) {
    if (s.empty() || std::isdigit((unsigned char)s[0])) return false;
    return std::all_of(s.begin(), s.end(), [](char c) { return std::isalnum((unsigned char)c) || c == '_'; });
}

// Files whose text comes from somewhere else than the disk: an editor's unsaved changes (frg query --stdin).
static std::map<std::string, std::string>& sourceOverrides() {
    static std::map<std::string, std::string> m;
    return m;
}

// Two spellings of the same file compare equal: Windows paths don't care about case or / versus \.
std::string normalPath(const fs::path& p) {
    std::error_code ec;
    std::string s = fs::weakly_canonical(fs::absolute(p), ec).string();
    for (char& c : s) c = c == '/' ? '\\' : (char)std::tolower((unsigned char)c);
    return s;
}

void setSourceOverride(const fs::path& path, const std::string& text) { sourceOverrides()[normalPath(path)] = text; }

static std::unique_ptr<SourceFile> readSource(const fs::path& path, const std::string& displayPath) {
    std::string text;
    auto over = sourceOverrides().find(normalPath(path));
    if (over != sourceOverrides().end()) {
        text = over->second;
    } else {
        std::ifstream in(path, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        text = ss.str();
    }
    if (text.compare(0, 3, "\xEF\xBB\xBF") == 0) text.erase(0, 3);  // UTF-8 byte order mark
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

    auto src = std::make_unique<SourceFile>();
    src->displayPath = displayPath;
    src->fullPath = fs::absolute(path).string();
    src->text = std::move(text);
    src->computeLines();
    return src;
}

// Why `name` can't be used as a file or folder name, or "" if it's fine.
static std::string badModuleName(const std::string& name) {
    if (!isIdentifier(name)) return "names can only use letters, numbers and _ (and can't start with a number)";
    if (isKeyword(name)) return "'" + name + "' is a Forge keyword";
    if (isBuiltinTypeName(name)) return "'" + name + "' is a built-in type name";
    return "";
}

static void loadFolder(Project& proj, Folder& folder) {
    std::vector<fs::path> files;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(folder.dir, ec))
        if (entry.is_regular_file() && entry.path().extension() == ".frg") files.push_back(entry.path());
    std::sort(files.begin(), files.end(), [](const fs::path& a, const fs::path& b) {
        bool aMain = a.stem() == "main", bMain = b.stem() == "main";
        if (aMain != bMain) return aMain;
        return a.filename() < b.filename();
    });

    std::vector<Module*> loaded;
    for (const fs::path& path : files) {
        std::string stem = path.stem().string();
        std::string display = fs::relative(path, proj.root).generic_string();
        if (std::string why = badModuleName(stem); !why.empty()) {
            reportPlainError("the file name '" + display + "' can't be used", why + " - rename the file");
            continue;
        }
        auto m = std::make_unique<Module>();
        m->name = stem;
        m->qualName = folder.qualName.empty() ? stem : folder.qualName + "." + stem;
        m->cppNs = "m_" + m->qualName;
        for (size_t pos; (pos = m->cppNs.find('.')) != std::string::npos;) m->cppNs.replace(pos, 1, "__");
        m->src = readSource(path, display);
        m->folder = &folder;
        m->ast = parseSource(*m->src);
        Module* raw = m.get();
        folder.modules[stem] = std::move(m);
        proj.allModules.push_back(raw);
        loaded.push_back(raw);
    }

    for (Module* m : loaded) {
        for (const Include& inc : m->ast.includes) {
            if (!inc.isFolder) {
                if (isStdLib(inc.name)) {
                    m->libs.insert(inc.name);
                } else if (isPlannedLib(inc.name)) {
                    reportError(inc.loc, "the <" + inc.name + "> library isn't built yet",
                                "it's planned for a later version of Forge");
                } else {
                    std::string guess = suggestName(inc.name, stdLibNames());
                    reportError(inc.loc, "there's no library called <" + inc.name + ">",
                                guess.empty() ? "" : "did you mean <" + guess + ">?");
                }
                continue;
            }

            if (inc.name.find_first_of("/\\") != std::string::npos) {
                reportError(inc.loc, "include one folder at a time",
                            "files inside a folder include that folder's own subfolders");
                continue;
            }
            if (std::string why = badModuleName(inc.name); !why.empty()) {
                reportError(inc.loc, "the folder name '" + inc.name + "' can't be used", why);
                continue;
            }
            fs::path dir = folder.dir / inc.name;
            if (!fs::is_directory(dir)) {
                reportError(inc.loc, "there's no folder called '" + inc.name + "' next to this file");
                continue;
            }
            if (folder.modules.count(inc.name)) {
                reportError(inc.loc, "there's a file " + inc.name + ".frg and a folder " + inc.name + " here",
                            "rename one of them so " + inc.name + ".something is never ambiguous");
                continue;
            }
            if (folder.folders.count(inc.name)) continue;  // already included by another file

            auto sub = std::make_unique<Folder>();
            sub->name = inc.name;
            sub->qualName = folder.qualName.empty() ? inc.name : folder.qualName + "." + inc.name;
            sub->dir = dir;
            sub->parent = &folder;
            Folder* raw = sub.get();
            folder.folders[inc.name] = std::move(sub);
            loadFolder(proj, *raw);
            if (raw->modules.empty()) reportError(inc.loc, "the folder '" + inc.name + "' has no .frg files in it");
        }
    }
}

static void loadPrelude(Project& proj) {
    for (const PreludeLib& lib : preludeLibs()) {
        auto m = std::make_unique<Module>();
        m->name = lib.name;
        m->qualName = lib.name;
        m->cppNs = "frg::" + lib.name;
        m->isPrelude = true;
        for (const std::string& used : lib.uses) m->libs.insert(used);
        m->src = std::make_unique<SourceFile>();
        m->src->displayPath = "<" + lib.name + ">";
        m->src->text = lib.source;
        m->src->computeLines();
        m->ast = parseSource(*m->src, true);
        proj.libModules[lib.name] = m.get();
        if (lib.name == "core") proj.core = m.get();
        proj.preludeStore.push_back(std::move(m));
    }
}

// A game uses either Forge's built-in engine or one low-level toolkit (OpenGL, DirectX, Vulkan): each toolkit
// opens its own window and does its own drawing, so they can't be mixed with the engine or with each other.
static void checkEngineAndToolkits(Project& proj) {
    const Include* engineUse = nullptr;
    const Include* toolkitUse = nullptr;
    for (Module* m : proj.allModules) {
        for (const Include& inc : m->ast.includes) {
            if (inc.isFolder) continue;
            const PreludeLib* lib = findPreludeLib(inc.name);
            if (!lib) continue;
            if (lib->kind == LibKind::Engine && !engineUse) engineUse = &inc;
            if (lib->kind == LibKind::Toolkit) {
                if (toolkitUse && toolkitUse->name != inc.name) {
                    reportError(inc.loc, "a game can only use one of <OpenGL>, <DirectX> and <Vulkan>",
                                "this game already uses <" + toolkitUse->name + ">");
                    return;
                }
                toolkitUse = &inc;
            }
        }
    }
    if (engineUse && toolkitUse)
        reportError(toolkitUse->loc,
                    "<" + toolkitUse->name + "> is a separate toolkit, so it can't be used together with Forge's engine (<" +
                        engineUse->name + ">)",
                    "a game either uses the engine (<window>, <input>, <graphics2d>, <graphics3d>, <assets>, <ui>) or one "
                    "toolkit, which opens its own window and does its own drawing - " +
                        toolkitUse->name + " has its own window, keyboard and mouse functions");
}

bool loadProject(Project& proj, const fs::path& root, bool withTests) {
    loadPrelude(proj);
    proj.root = fs::weakly_canonical(fs::absolute(root));
    proj.name = proj.root.filename().string();
    if (proj.name.empty()) proj.name = "game";
    proj.rootFolder.dir = proj.root;
    loadFolder(proj, proj.rootFolder);
    fs::path tests = proj.root / "tests";
    std::error_code ec;
    if (withTests && fs::is_directory(tests, ec) && !proj.rootFolder.folders.count("tests") && !proj.rootFolder.modules.count("tests")) {
        auto f = std::make_unique<Folder>();
        f->name = "tests";
        f->qualName = "tests";
        f->dir = tests;
        f->parent = &proj.rootFolder;
        Folder* raw = f.get();
        proj.rootFolder.folders["tests"] = std::move(f);
        loadFolder(proj, *raw);
    }
    checkEngineAndToolkits(proj);

    auto it = proj.rootFolder.modules.find("main");
    if (it != proj.rootFolder.modules.end()) proj.mainModule = it->second.get();
    return errorCount() == 0 && proj.mainModule != nullptr;
}
