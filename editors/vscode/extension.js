// Forge for VS Code: errors on save, Run/Build commands, and help for the built-in libraries.
const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

// ---------------------------------------------------------------- built-in library reference
// [name, signature, description]. Keep in sync with src/prelude.cpp and src/builtins.cpp.

const LIBRARIES = {
    stdio: [
        ['print', 'void print(value)', 'Shows a value, then starts a new line. print() alone just starts a new line.'],
        ['write', 'void write(value)', 'Shows a value without starting a new line.'],
        ['input', 'string input(string prompt)', 'Waits for the player to type a line, and gives it back.'],
    ],
    math: [
        ['sqrt', 'float sqrt(x)', 'Square root.'],
        ['pow', 'float pow(x, power)', 'x to the power of `power`.'],
        ['abs', 'abs(x)', 'Distance from zero: abs(-3) is 3.'],
        ['sign', 'sign(x)', '-1, 0 or 1 depending on whether x is negative, zero or positive.'],
        ['min', 'min(a, b)', 'The smaller of two numbers.'],
        ['max', 'max(a, b)', 'The bigger of two numbers.'],
        ['clamp', 'clamp(x, low, high)', 'Keeps x between low and high.'],
        ['lerp', 'float lerp(a, b, t)', 'Blends from a (t = 0) to b (t = 1).'],
        ['floor', 'float floor(x)', 'Rounds down.'],
        ['ceil', 'float ceil(x)', 'Rounds up.'],
        ['round', 'float round(x)', 'Rounds to the nearest whole number.'],
        ['sin', 'float sin(radians)', 'Sine.'],
        ['cos', 'float cos(radians)', 'Cosine.'],
        ['tan', 'float tan(radians)', 'Tangent.'],
        ['asin', 'float asin(x)', 'Inverse sine.'],
        ['acos', 'float acos(x)', 'Inverse cosine.'],
        ['atan', 'float atan(x)', 'Inverse tangent.'],
        ['atan2', 'float atan2(y, x)', 'The angle of the point (x, y), in radians.'],
        ['exp', 'float exp(x)', 'e to the power of x.'],
        ['log', 'float log(x)', 'Natural logarithm.'],
        ['PI', 'float PI', '3.14159...: half a turn, in radians.'],
        ['TAU', 'float TAU', '6.28318...: a full turn, in radians.'],
        ['E', 'float E', '2.71828...'],
    ],
    random: [
        ['range', 'int range(int min, int max)\nfloat range(float min, float max)', 'A random number from min to max (both included for whole numbers).'],
        ['value', 'float value()', 'A random number from 0 to 1.'],
        ['chance', 'bool chance(float probability)', 'True with the given probability: chance(0.25) is true 1 time in 4.'],
        ['seed', 'void seed(int value)', 'Makes the random numbers repeat the same way every run.'],
    ],
    window: [
        ['open', 'void open(int width, int height, string title)', 'Opens the game window.'],
        ['shouldClose', 'bool shouldClose()', 'Shows the frame you drew and starts the next one. True when the player closes the window.'],
        ['run', 'void run()', 'Runs the game: calls update(float dt) and draw() from main.frg every frame until the window closes.'],
        ['close', 'void close()', 'Closes the window.'],
        ['deltaTime', 'float deltaTime()', 'Seconds since the last frame. Multiply speeds by this.'],
        ['time', 'double time()', 'Seconds since the window opened.'],
        ['width', 'int width()', 'Width of the window.'],
        ['height', 'int height()', 'Height of the window.'],
        ['fps', 'int fps()', 'Frames per second right now.'],
        ['setFPS', 'void setFPS(int fps)', 'The most frames per second to draw (60 to start with).'],
        ['setTitle', 'void setTitle(string title)', 'Changes the window title.'],
        ['setBackground', 'void setBackground(color c)', 'The color the screen is cleared to every frame.'],
        ['toggleFullscreen', 'void toggleFullscreen()', 'Switches between fullscreen and a window.'],
        ['screenshot', 'void screenshot(string fileName)', 'Saves what is on screen as a .png file.'],
    ],
    input: [
        ['keyDown', 'bool keyDown(Key key)', 'True while the key is held down.'],
        ['keyPressed', 'bool keyPressed(Key key)', 'True only on the frame the key was pressed.'],
        ['keyReleased', 'bool keyReleased(Key key)', 'True only on the frame the key was let go.'],
        ['mouseDown', 'bool mouseDown(MouseButton button)', 'True while the mouse button is held down.'],
        ['mousePressed', 'bool mousePressed(MouseButton button)', 'True only on the frame the button was pressed.'],
        ['mouseReleased', 'bool mouseReleased(MouseButton button)', 'True only on the frame the button was let go.'],
        ['mousePosition', 'vec2 mousePosition()', 'Where the mouse is.'],
        ['mouseX', 'float mouseX()', 'Mouse x position.'],
        ['mouseY', 'float mouseY()', 'Mouse y position.'],
        ['mouseWheel', 'float mouseWheel()', 'How far the mouse wheel moved this frame.'],
    ],
    graphics2d: [
        ['drawRect', 'void drawRect(float x, float y, float width, float height, color c)\nvoid drawRect(rect r, color c)', 'A filled rectangle.'],
        ['drawRectOutline', 'void drawRectOutline(float x, float y, float width, float height, color c, float thickness = 1)\nvoid drawRectOutline(rect r, color c, float thickness = 1)', 'The outline of a rectangle.'],
        ['drawCircle', 'void drawCircle(float x, float y, float radius, color c)\nvoid drawCircle(vec2 center, float radius, color c)', 'A filled circle.'],
        ['drawCircleOutline', 'void drawCircleOutline(float x, float y, float radius, color c)', 'The outline of a circle.'],
        ['drawLine', 'void drawLine(float x1, float y1, float x2, float y2, color c, float thickness = 1)\nvoid drawLine(vec2 from, vec2 to, color c, float thickness = 1)', 'A line.'],
        ['drawTriangle', 'void drawTriangle(vec2 a, vec2 b, vec2 c, color fill)', 'A filled triangle.'],
        ['drawText', 'void drawText(string text, float x, float y, int size, color c)', 'Text on the screen.'],
        ['textWidth', 'int textWidth(string text, int size)', 'How wide some text would be, in pixels.'],
        ['drawFPS', 'void drawFPS(float x, float y)', 'Shows the frames per second.'],
        ['clear', 'void clear(color c)', 'Fills the whole screen with a color.'],
    ],
    physics: [
        ['add', 'void add(Body body)', 'Adds a body to the world so step() moves it.'],
        ['remove', 'void remove(Body body)', 'Takes a body out of the world.'],
        ['step', 'void step(float dt)', 'Moves every body: gravity, velocity, and stopping at static bodies. Call it once per frame.'],
        ['setGravity', 'void setGravity(vec2 gravity)', 'Gravity, in pixels per second per second. Starts at vec2(0, 980).'],
        ['gravity', 'vec2 gravity()', 'The current gravity.'],
        ['clear', 'void clear()', 'Removes every body.'],
        ['count', 'int count()', 'How many bodies are in the world.'],
    ],
};

const COLORS = ['WHITE', 'BLACK', 'BLANK', 'LIGHTGRAY', 'GRAY', 'DARKGRAY', 'RED', 'MAROON', 'ORANGE', 'YELLOW', 'GOLD',
    'GREEN', 'LIME', 'DARKGREEN', 'SKYBLUE', 'BLUE', 'DARKBLUE', 'PURPLE', 'VIOLET', 'PINK', 'MAGENTA', 'BROWN', 'BEIGE'];
const KEYS = [...'ABCDEFGHIJKLMNOPQRSTUVWXYZ'].concat([...'0123456789'].map(d => 'NUM' + d),
    ['SPACE', 'ENTER', 'ESCAPE', 'BACKSPACE', 'TAB', 'LEFT', 'RIGHT', 'UP', 'DOWN', 'LEFT_SHIFT', 'RIGHT_SHIFT',
        'LEFT_CONTROL', 'RIGHT_CONTROL', 'LEFT_ALT', 'RIGHT_ALT'], [...Array(12)].map((_, i) => 'F' + (i + 1)));

// Things reached with a dot on a type name: color.RED, vec2.ZERO, Key.SPACE.
const TYPE_STATICS = {
    color: COLORS,
    vec2: ['ZERO', 'ONE', 'UP', 'DOWN', 'LEFT', 'RIGHT'],
    vec3: ['ZERO', 'ONE', 'UP', 'FORWARD', 'RIGHT'],
    vec4: ['ZERO', 'ONE'],
    Key: KEYS,
    MouseButton: ['LEFT', 'RIGHT', 'MIDDLE'],
};

// ---------------------------------------------------------------- finding frg.exe and the game folder

let extensionDir = '';
let warnedMissing = false;

function settings() {
    return vscode.workspace.getConfiguration('forge');
}

function frgPath() {
    const configured = settings().get('path');
    if (configured) return configured;
    // The packaging script records where frg.exe was when the extension was built.
    const recorded = path.join(extensionDir, 'frg-path.txt');
    if (fs.existsSync(recorded)) {
        const p = fs.readFileSync(recorded, 'utf8').trim();
        if (p && fs.existsSync(p)) return p;
    }
    return 'frg';
}

// The game's top folder: the highest folder above the file that has a main.frg (inside the workspace).
function projectRoot(fileName) {
    const uri = vscode.Uri.file(fileName);
    const ws = vscode.workspace.getWorkspaceFolder(uri);
    const stop = ws ? path.resolve(ws.uri.fsPath) : path.parse(fileName).root;
    let dir = path.dirname(path.resolve(fileName));
    let found = null;
    while (true) {
        if (fs.existsSync(path.join(dir, 'main.frg'))) found = dir;
        const parent = path.dirname(dir);
        if (dir.toLowerCase() === stop.toLowerCase() || parent === dir) break;
        dir = parent;
    }
    return found;
}

function showMissingFrg() {
    if (warnedMissing) return;
    warnedMissing = true;
    vscode.window.showErrorMessage(
        "Forge couldn't find frg.exe. Add Forge's bin folder to your PATH, or set 'forge.path' in the settings.",
        'Open Settings').then(choice => {
        if (choice) vscode.commands.executeCommand('workbench.action.openSettings', 'forge.path');
    });
}

// ---------------------------------------------------------------- errors as you save

let diagnostics;
const reportedFiles = new Map();  // game folder -> set of files that currently have errors
const running = new Map();        // game folder -> true while a check is running (or 'again' if one is queued)

function unescapeField(s) {
    return s.replace(/\\(.)/g, (_, c) => (c === 'n' ? '\n' : c === 't' ? '\t' : c));
}

// frg counts columns in bytes (UTF-8); VS Code counts UTF-16 characters.
function byteColToChar(lineText, byteCol) {
    let bytes = 0;
    let i = 0;
    while (i < lineText.length && bytes < byteCol) {
        const code = lineText.codePointAt(i);
        bytes += code < 0x80 ? 1 : code < 0x800 ? 2 : code < 0x10000 ? 3 : 4;
        i += code > 0xffff ? 2 : 1;
    }
    return i;
}

function fileLines(file, cache) {
    if (!cache.has(file)) {
        const open = vscode.workspace.textDocuments.find(d => d.fileName.toLowerCase() === file.toLowerCase());
        let text = '';
        if (open) text = open.getText();
        else if (fs.existsSync(file)) text = fs.readFileSync(file, 'utf8');
        cache.set(file, text.replace(/^﻿/, '').split(/\r?\n/));
    }
    return cache.get(file);
}

function check(doc, announce = false) {
    const root = projectRoot(doc.fileName);
    if (!root) return;
    if (running.get(root)) {
        running.set(root, 'again');
        return;
    }
    running.set(root, true);

    cp.execFile(frgPath(), ['check', root, '--machine'], { cwd: root, windowsHide: true }, (err, stdout) => {
        if (err && err.code === 'ENOENT') {
            running.delete(root);
            showMissingFrg();
            return;
        }
        const byFile = new Map();
        const cache = new Map();
        for (const line of stdout.split(/\r?\n/)) {
            if (!line.startsWith('E\t')) continue;
            const f = line.split('\t');
            let file = unescapeField(f[1] || '');
            const lineNo = Math.max(0, parseInt(f[2], 10) - 1);
            const col = Math.max(0, parseInt(f[3], 10) - 1);
            const len = Math.max(1, parseInt(f[4], 10));
            const msg = unescapeField(f[5] || '');
            const hint = unescapeField(f[6] || '');
            if (!file) file = path.join(root, 'main.frg');
            const text = fileLines(file, cache)[lineNo] || '';
            const start = byteColToChar(text, col);
            const end = byteColToChar(text, col + len);
            const d = new vscode.Diagnostic(new vscode.Range(lineNo, start, lineNo, Math.max(end, start + 1)),
                hint ? `${msg}\nhint: ${hint}` : msg, vscode.DiagnosticSeverity.Error);
            d.source = 'forge';
            const key = path.resolve(file);
            if (!byFile.has(key)) byFile.set(key, []);
            byFile.get(key).push(d);
        }

        const previous = reportedFiles.get(root) || new Set();
        for (const file of previous) if (!byFile.has(file)) diagnostics.delete(vscode.Uri.file(file));
        for (const [file, list] of byFile) diagnostics.set(vscode.Uri.file(file), list);
        reportedFiles.set(root, new Set(byFile.keys()));

        if (announce) {
            const count = [...byFile.values()].reduce((n, l) => n + l.length, 0);
            vscode.window.showInformationMessage(count ? `Forge found ${count} error${count === 1 ? '' : 's'}.` : 'Forge found no errors.');
        }
        const again = running.get(root) === 'again';
        running.delete(root);
        if (again) check(doc);
    });
}

// ---------------------------------------------------------------- run and build

let gameTerminal = null;

async function runCommand(command) {
    const editor = vscode.window.activeTextEditor;
    const file = editor ? editor.document.fileName : null;
    const root = file ? projectRoot(file) : null;
    if (!root) {
        vscode.window.showErrorMessage('Open a .frg file from a Forge game first (its top folder needs a main.frg).');
        return;
    }
    await vscode.workspace.saveAll(false);
    if (gameTerminal) gameTerminal.dispose();
    gameTerminal = vscode.window.createTerminal({
        name: `Forge: ${path.basename(root)}`,
        cwd: root,
        shellPath: frgPath(),
        shellArgs: [command, root],
    });
    gameTerminal.show(true);
}

// ---------------------------------------------------------------- autocomplete and hover help

function libraryCompletions(document, position) {
    const before = document.lineAt(position).text.slice(0, position.character);
    const m = before.match(/\b([A-Za-z_][A-Za-z0-9_]*)\.\s*[A-Za-z0-9_]*$/);
    if (!m) return undefined;
    const owner = m[1];
    if (LIBRARIES[owner]) {
        return LIBRARIES[owner].map(([name, sig, doc]) => {
            const isConst = /^[A-Z]+$/.test(name);
            const item = new vscode.CompletionItem(name, isConst ? vscode.CompletionItemKind.Constant : vscode.CompletionItemKind.Function);
            item.detail = sig.split('\n')[0];
            item.documentation = new vscode.MarkdownString().appendCodeblock(sig, 'forge').appendMarkdown(doc);
            return item;
        });
    }
    if (TYPE_STATICS[owner]) {
        return TYPE_STATICS[owner].map(name => {
            const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.EnumMember);
            item.detail = `${owner}.${name}`;
            return item;
        });
    }
    return undefined;
}

function libraryHover(document, position) {
    const range = document.getWordRangeAtPosition(position);
    if (!range) return undefined;
    const word = document.getText(range);
    const before = document.lineAt(position).text.slice(0, range.start.character);
    const m = before.match(/\b([A-Za-z_][A-Za-z0-9_]*)\.\s*$/);
    if (!m || !LIBRARIES[m[1]]) return undefined;
    const entry = LIBRARIES[m[1]].find(([name]) => name === word);
    if (!entry) return undefined;
    const md = new vscode.MarkdownString().appendCodeblock(entry[1], 'forge').appendMarkdown(entry[2]);
    return new vscode.Hover(md, range);
}

// ---------------------------------------------------------------- activation

function activate(context) {
    extensionDir = context.extensionPath;
    diagnostics = vscode.languages.createDiagnosticCollection('forge');
    context.subscriptions.push(
        diagnostics,
        vscode.commands.registerCommand('forge.run', () => runCommand('run')),
        vscode.commands.registerCommand('forge.build', () => runCommand('build')),
        vscode.commands.registerCommand('forge.check', () => {
            const editor = vscode.window.activeTextEditor;
            if (editor && editor.document.languageId === 'forge') check(editor.document, true);
        }),
        vscode.workspace.onDidSaveTextDocument(doc => {
            if (doc.languageId === 'forge' && settings().get('checkOnSave')) check(doc);
        }),
        vscode.workspace.onDidOpenTextDocument(doc => {
            if (doc.languageId === 'forge') check(doc);
        }),
        vscode.workspace.onDidChangeConfiguration(e => {
            if (e.affectsConfiguration('forge.path')) warnedMissing = false;
        }),
        vscode.languages.registerCompletionItemProvider('forge', { provideCompletionItems: libraryCompletions }, '.'),
        vscode.languages.registerHoverProvider('forge', { provideHover: libraryHover }),
    );
    for (const doc of vscode.workspace.textDocuments) if (doc.languageId === 'forge') check(doc);
}

function deactivate() {}

module.exports = { activate, deactivate };
