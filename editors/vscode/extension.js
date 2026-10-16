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
        ['color', 'color color()', 'A random solid color.'],
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
        ['gamepadConnected', 'bool gamepadConnected(int gamepad = 0)', 'True if a controller is plugged in. gamepad is 0 for the first one, 1 for the second...'],
        ['gamepadName', 'string gamepadName(int gamepad = 0)', 'The controller\'s name, or "" if there isn\'t one.'],
        ['gamepadDown', 'bool gamepadDown(GamepadButton button, int gamepad = 0)', 'True while a controller button is held, like GamepadButton.A.'],
        ['gamepadPressed', 'bool gamepadPressed(GamepadButton button, int gamepad = 0)', 'True only in the frame the button was pressed.'],
        ['gamepadReleased', 'bool gamepadReleased(GamepadButton button, int gamepad = 0)', 'True only in the frame the button was let go.'],
        ['leftStick', 'vec2 leftStick(int gamepad = 0)', 'The left stick, from (-1, -1) (up-left) to (1, 1) (down-right). Small wobbles count as (0, 0).'],
        ['rightStick', 'vec2 rightStick(int gamepad = 0)', 'The right stick, like leftStick.'],
        ['leftTrigger', 'float leftTrigger(int gamepad = 0)', 'How far the left trigger is pressed: 0 to 1.'],
        ['rightTrigger', 'float rightTrigger(int gamepad = 0)', 'How far the right trigger is pressed: 0 to 1.'],
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
        ['setCamera', 'void setCamera(Camera2D camera)', 'Drawing after this is moved, zoomed and turned by the camera. Camera2D(target, offset, zoom = 1, rotation = 0): target is the world point to look at, offset is where it shows on the screen.'],
        ['resetCamera', 'void resetCamera()', 'Stops using the camera, so the next drawing is on the screen again (for scores and menus).'],
        ['camera', 'Camera2D camera()', 'The camera set with setCamera.'],
        ['screenToWorld', 'vec2 screenToWorld(vec2 point)', 'Where a point on the screen (like the mouse) is in the world, with the camera.'],
        ['worldToScreen', 'vec2 worldToScreen(vec2 point)', 'Where a point in the world shows on the screen, with the camera.'],
    ],
    assets: [
        ['loadTexture', 'Texture loadTexture(string path)', 'Loads an image (.png .jpg .bmp .tga .gif .qoi). Then: tex.draw(x, y), drawScaled, drawRotated, drawPart, width(), height(). For a sprite sheet: Animation(tex, frameWidth, frameHeight, frames, fps), then anim.update(dt) and anim.draw(x, y).'],
        ['loadFont', 'Font loadFont(string path, int quality = 64)', 'Loads a font (.ttf .otf). Then: font.drawText(text, x, y, size, color), font.measure(text, size).'],
        ['exists', 'bool exists(string path)', 'True if the file is in the game folder.'],
    ],
    audio: [
        ['loadSound', 'Sound loadSound(string path)', 'Loads a short sound (.wav .ogg .mp3 .flac). Then: sound.play(), stop(), isPlaying(), setVolume(v), setPitch(p).'],
        ['loadMusic', 'Music loadMusic(string path)', 'Loads music to stream (.ogg .mp3 .wav .flac .xm .mod). Loops by default. Then: play(), stop(), pause(), resume(), setVolume(v), setLooping(b).'],
        ['setVolume', 'void setVolume(float volume)', 'Volume of everything, from 0 to 1.'],
        ['update', 'void update()', 'Keeps music playing in programs without a window (the window does this by itself).'],
    ],
    graphics3d: [
        ['setCamera', 'void setCamera(Camera camera)', 'The camera the next 3D drawing uses. Camera(position, target), and cam.orbit(degrees).'],
        ['camera', 'Camera camera()', 'The camera being used.'],
        ['drawCube', 'void drawCube(vec3 position, vec3 size, color c)', 'A solid box, centered on position.'],
        ['drawCubeOutline', 'void drawCubeOutline(vec3 position, vec3 size, color c)', 'The edges of a box.'],
        ['drawSphere', 'void drawSphere(vec3 center, float radius, color c)', 'A ball.'],
        ['drawSphereOutline', 'void drawSphereOutline(vec3 center, float radius, color c)', 'A wireframe ball.'],
        ['drawCylinder', 'void drawCylinder(vec3 position, float radius, float height, color c)', 'A cylinder standing on position.'],
        ['drawPlane', 'void drawPlane(vec3 center, vec2 size, color c)', 'A flat floor.'],
        ['drawGrid', 'void drawGrid(int slices, float spacing)', 'Grid lines on the ground, to see where things are.'],
        ['drawLine', 'void drawLine(vec3 from, vec3 to, color c)', 'A line in 3D.'],
        ['toScreen', 'vec2 toScreen(vec3 point)', 'Where a 3D point shows up on screen - for 2D labels over 3D things.'],
        ['loadModel', 'Model loadModel(string path)', 'Loads a 3D model (.obj .gltf .glb .iqm .vox .m3d). Then: model.draw(position, scale, tint), drawRotated(position, axis, degrees, scale, tint).'],
    ],
    time: [
        ['now', 'double now()', 'Seconds since the game started.'],
        ['sleep', 'void sleep(float seconds)', 'Stops everything for a while (in games, use a task and wait() instead).'],
        ['clock', 'string clock()', 'The time of day, like "14:05:09".'],
        ['date', 'string date()', "Today's date, like \"2026-10-14\"."],
        ['year', 'int year()', 'The current year.'],
        ['month', 'int month()', 'The current month (1 to 12).'],
        ['day', 'int day()', 'The day of the month.'],
        ['hour', 'int hour()', 'The hour (0 to 23).'],
        ['minute', 'int minute()', 'The minute.'],
        ['second', 'int second()', 'The second.'],
    ],
    tilemap: [
        ['load', 'TileMap load(string path, float tileSize)', 'A level from a text file, one character per tile. Or TileMap("""...""", 32). Then: level.addBodies("#"), level.find("C"), level.get(col, row), level.draw("#", color), level.drawTiles(tileset, 16, "#=").'],
    ],
    tween: [
        ['ease', 'float ease(Ease ease, float t)', 'The eased amount for t from 0 to 1: tween.ease(Ease.OUT, 0.5) is 0.875. For a smooth change over time, use a Tween: Tween t = Tween(from, to, seconds, Ease.OUT); then t.update(dt) and t.value().'],
    ],
    save: [
        ['setInt', 'void setInt(string key, int value)', 'Saves a whole number under a name. The save file is written straight away.'],
        ['getInt', 'int getInt(string key, int backup = 0)', 'A saved whole number, or backup if it isn\'t saved.'],
        ['setFloat', 'void setFloat(string key, float value)', 'Saves a decimal number under a name.'],
        ['getFloat', 'float getFloat(string key, float backup = 0)', 'A saved decimal number, or backup if it isn\'t saved.'],
        ['setString', 'void setString(string key, string value)', 'Saves text under a name.'],
        ['getString', 'string getString(string key, string backup = "")', 'Saved text, or backup if it isn\'t saved.'],
        ['setBool', 'void setBool(string key, bool value)', 'Saves true or false under a name.'],
        ['getBool', 'bool getBool(string key, bool backup = false)', 'A saved true/false, or backup if it isn\'t saved.'],
        ['has', 'bool has(string key)', 'True if something is saved under that name.'],
        ['remove', 'void remove(string key)', 'Forgets one saved value.'],
        ['clear', 'void clear()', 'Forgets everything that was saved (for a "reset save" button).'],
    ],
    file: [
        ['read', 'string read(string path)', 'All the text in a file.'],
        ['write', 'bool write(string path, string text)', 'Saves text to a file (replacing what was there). True if it worked.'],
        ['append', 'bool append(string path, string text)', 'Adds text to the end of a file.'],
        ['exists', 'bool exists(string path)', 'True if the file is there.'],
        ['remove', 'bool remove(string path)', 'Deletes a file.'],
        ['readLines', 'list<string> readLines(string path)', 'Each line of a file.'],
        ['writeLines', 'bool writeLines(string path, list<string> lines)', 'Saves a list of lines to a file.'],
        ['files', 'list<string> files(string folder)', 'The names of the files in a folder.'],
    ],
    scenes: [
        ['change', 'void change(Scene next)', 'Switches to another scene at the start of the next update.'],
        ['update', 'void update(float dt)', "Runs the current scene's update(dt). Call it every frame."],
        ['draw', 'void draw()', "Runs the current scene's draw(). Call it every frame."],
        ['current', 'Scene? current()', 'The scene that is running.'],
    ],
    ui: [
        ['button', 'bool button(rect area, string label)', 'Draws a button. True on the frame it gets clicked.'],
        ['checkbox', 'bool checkbox(rect area, string label, bool checked)', 'Draws a checkbox and gives back its new value: on = ui.checkbox(..., on);'],
        ['slider', 'float slider(rect area, float value, float min, float max)', 'Draws a slider and gives back its new value: v = ui.slider(..., v, 0, 1);'],
        ['textBox', 'string textBox(rect area, string text, int maxLength = 0)', 'A box to type in. Gives back the new text: name = ui.textBox(..., name, 12); Click it to type; Enter or a click elsewhere finishes. maxLength 0 means no limit.'],
        ['isTyping', 'bool isTyping()', 'True while the player is typing in a text box, so the game can ignore its own key shortcuts.'],
        ['label', 'void label(string text, float x, float y, int size = 20, color c = color.WHITE)', 'Text.'],
        ['panel', 'void panel(rect area)', 'A rounded box to put other UI on.'],
        ['setTheme', 'void setTheme(color background, color accent, color text)', 'The colors of all UI.'],
    ],
    ECS: [
        ['create', 'Entity create(string name)', 'Makes an entity and adds it to the world. Then: entity.add(SomeComponent()).'],
        ['add', 'void add(Entity entity)', 'Adds an entity made with Entity("name") to the world.'],
        ['update', 'void update(float dt)', 'Runs update(dt) on every component (and start() the first time).'],
        ['draw', 'void draw()', 'Runs draw() on every component.'],
        ['find', 'Entity? find(string name)', 'The first entity with that name, or null.'],
        ['all', 'list<Entity> all()', 'Every entity.'],
        ['withComponent', 'list<Entity> withComponent(string type)', 'Every entity that has a component of that class.'],
        ['clear', 'void clear()', 'Removes every entity.'],
        ['count', 'int count()', 'How many entities there are.'],
    ],
    tasks: [
        ['update', 'void update(float dt)', 'Moves tasks forward (only needed without a window).'],
        ['runAll', 'void runAll()', 'Waits (for real) until every task has finished.'],
        ['count', 'int count()', 'How many tasks are waiting.'],
        ['stopAll', 'void stopAll()', 'Stops every task.'],
    ],
    physics: [
        ['add', 'void add(Body body)', 'Adds a body to the world so step() moves it. A Body has position, size, velocity, isStatic, solid, oneWay (a platform you can jump up through), gravityScale, and onGround / onCeiling / onWall.'],
        ['remove', 'void remove(Body body)', 'Takes a body out of the world.'],
        ['step', 'void step(float dt)', 'Moves every body: gravity, velocity, and stopping at static bodies. Call it once per frame.'],
        ['setGravity', 'void setGravity(vec2 gravity)', 'Gravity, in pixels per second per second. Starts at vec2(0, 980).'],
        ['gravity', 'vec2 gravity()', 'The current gravity.'],
        ['clear', 'void clear()', 'Removes every body.'],
        ['count', 'int count()', 'How many bodies are in the world.'],
        ['drawDebug', 'void drawDebug()', 'Draws the outline of every body: static blue, moving green, not solid purple, and a yellow bar under bodies that are onGround. Needs #include <graphics2d> too.'],
    ],
};

// The low-level toolkits (<OpenGL>, <DirectX>, <Vulkan>) all share these window, input and matrix functions.
function toolkitCommon(lib) {
    return [
        ['openWindow', 'void openWindow(int width, int height, string title)', `Opens the window and starts ${lib}. Call it once, first.`],
        ['shouldClose', 'bool shouldClose()', 'Call at the top of the game loop: shows the last frame, reads input, and is true when the window is closed (or Escape is pressed).'],
        ['closeWindow', 'void closeWindow()', 'Frees everything and closes the window.'],
        ['width', 'int width()', 'The window\'s width in pixels.'],
        ['height', 'int height()', 'The window\'s height in pixels.'],
        ['deltaTime', 'float deltaTime()', 'Seconds since the last frame.'],
        ['time', 'double time()', 'Seconds since the window opened.'],
        ['setTitle', 'void setTitle(string title)', 'Changes the window\'s title.'],
        ['setVSync', 'void setVSync(bool on)', 'On (the start): wait for the screen before each frame. Off: draw as fast as possible.'],
        ['keyDown', 'bool keyDown(Key key)', 'True while the key is held down.'],
        ['keyPressed', 'bool keyPressed(Key key)', 'True only on the frame the key was pressed.'],
        ['keyReleased', 'bool keyReleased(Key key)', 'True only on the frame the key was let go.'],
        ['mouseDown', 'bool mouseDown(MouseButton button)', 'True while the mouse button is held down.'],
        ['mousePressed', 'bool mousePressed(MouseButton button)', 'True only on the frame the button was pressed.'],
        ['mouseReleased', 'bool mouseReleased(MouseButton button)', 'True only on the frame the button was let go.'],
        ['mousePosition', 'vec2 mousePosition()', 'Where the mouse is, in pixels from the top-left corner.'],
        ['mouseWheel', 'float mouseWheel()', 'How far the wheel turned this frame.'],
        ['identity', 'list<float> identity()', 'A 4x4 matrix that does nothing (16 floats, column by column).'],
        ['multiply', 'list<float> multiply(list<float> a, list<float> b)', 'Combines two matrices: b happens first, then a.'],
        ['perspective', 'list<float> perspective(float fovDegrees, float aspect, float near, float far)', `A 3D projection matrix made for ${lib}. aspect is width / height.`],
        ['orthographic', 'list<float> orthographic(float left, float right, float bottom, float top, float near, float far)', `A flat (2D) projection matrix made for ${lib}.`],
        ['lookAt', 'list<float> lookAt(vec3 eye, vec3 target, vec3 up)', 'A camera (view) matrix: at eye, looking at target.'],
        ['translate', 'list<float> translate(vec3 offset)', 'A matrix that moves things.'],
        ['rotate', 'list<float> rotate(float degrees, vec3 axis)', 'A matrix that turns things around an axis.'],
        ['scale', 'list<float> scale(vec3 amount)', 'A matrix that stretches things.'],
    ];
}

function constants(names, doc) {
    return names.map(n => [n, `const int ${n}`, doc]);
}

LIBRARIES.OpenGL = toolkitCommon('OpenGL').concat([
    ['version', 'string version()', 'The OpenGL version and graphics card.'],
    ['clearColor', 'void clearColor(float r, float g, float b, float a)', 'The color clear() fills the screen with (glClearColor).'],
    ['clear', 'void clear(int mask)', 'Clears the screen: OpenGL.COLOR_BUFFER_BIT bitor OpenGL.DEPTH_BUFFER_BIT (glClear).'],
    ['viewport', 'void viewport(int x, int y, int width, int height)', 'Which part of the window to draw into (glViewport).'],
    ['enable', 'void enable(int capability)', 'Turns on DEPTH_TEST, BLEND or CULL_FACE (glEnable).'],
    ['disable', 'void disable(int capability)', 'Turns off DEPTH_TEST, BLEND or CULL_FACE (glDisable).'],
    ['blendFunc', 'void blendFunc(int source, int destination)', 'How see-through colors mix, usually SRC_ALPHA, ONE_MINUS_SRC_ALPHA (glBlendFunc).'],
    ['depthFunc', 'void depthFunc(int function)', 'When a pixel passes the depth test: LESS, LEQUAL, ALWAYS (glDepthFunc).'],
    ['cullFace', 'void cullFace(int face)', 'Which side CULL_FACE skips: BACK, FRONT, FRONT_AND_BACK (glCullFace).'],
    ['lineWidth', 'void lineWidth(float width)', 'How thick lines are (glLineWidth).'],
    ['getError', 'int getError()', 'The oldest error OpenGL noticed, or NO_ERROR (glGetError).'],
    ['createShader', 'int createShader(int type)', 'A new VERTEX_SHADER or FRAGMENT_SHADER (glCreateShader).'],
    ['shaderSource', 'void shaderSource(int shader, string source)', 'Gives a shader its GLSL code (glShaderSource).'],
    ['compileShader', 'void compileShader(int shader)', 'Compiles a shader (glCompileShader).'],
    ['shaderCompiled', 'bool shaderCompiled(int shader)', 'True if the shader compiled.'],
    ['shaderLog', 'string shaderLog(int shader)', 'The shader compiler\'s messages.'],
    ['deleteShader', 'void deleteShader(int shader)', 'Frees a shader (glDeleteShader).'],
    ['createProgram', 'int createProgram()', 'A new, empty program (glCreateProgram).'],
    ['attachShader', 'void attachShader(int program, int shader)', 'Adds a compiled shader to a program (glAttachShader).'],
    ['linkProgram', 'void linkProgram(int program)', 'Links a program\'s shaders (glLinkProgram).'],
    ['programLinked', 'bool programLinked(int program)', 'True if the program linked.'],
    ['programLog', 'string programLog(int program)', 'The linker\'s messages.'],
    ['useProgram', 'void useProgram(int program)', 'Draws with this program from now on (glUseProgram).'],
    ['deleteProgram', 'void deleteProgram(int program)', 'Frees a program (glDeleteProgram).'],
    ['buildProgram', 'int buildProgram(string vertexSource, string fragmentSource)', 'Compiles and links both shaders in one go. Stops the game with the error if one has a mistake.'],
    ['getUniformLocation', 'int getUniformLocation(int program, string name)', 'Where a uniform is, or -1 (glGetUniformLocation).'],
    ['getAttribLocation', 'int getAttribLocation(int program, string name)', 'The location of a vertex input (glGetAttribLocation).'],
    ['uniform1f', 'void uniform1f(int location, float x)', 'Sets a float uniform.'],
    ['uniform2f', 'void uniform2f(int location, float x, float y)', 'Sets a vec2 uniform.'],
    ['uniform3f', 'void uniform3f(int location, float x, float y, float z)', 'Sets a vec3 uniform.'],
    ['uniform4f', 'void uniform4f(int location, float x, float y, float z, float w)', 'Sets a vec4 uniform.'],
    ['uniform1i', 'void uniform1i(int location, int x)', 'Sets an int, bool or sampler2D uniform.'],
    ['uniformMatrix4', 'void uniformMatrix4(int location, list<float> matrix)', 'Sets a mat4 uniform from 16 floats.'],
    ['genVertexArray', 'int genVertexArray()', 'A new vertex array (glGenVertexArrays).'],
    ['bindVertexArray', 'void bindVertexArray(int vertexArray)', 'Uses this vertex array (glBindVertexArray).'],
    ['deleteVertexArray', 'void deleteVertexArray(int vertexArray)', 'Frees a vertex array.'],
    ['genBuffer', 'int genBuffer()', 'A new buffer (glGenBuffers).'],
    ['bindBuffer', 'void bindBuffer(int target, int buffer)', 'Uses this buffer as ARRAY_BUFFER or ELEMENT_ARRAY_BUFFER (glBindBuffer).'],
    ['deleteBuffer', 'void deleteBuffer(int buffer)', 'Frees a buffer.'],
    ['bufferData', 'void bufferData(int target, list<float> data, int usage)', 'Copies floats into the bound buffer (glBufferData).'],
    ['bufferDataInts', 'void bufferDataInts(int target, list<int> data, int usage)', 'Copies whole numbers into the bound buffer, for ELEMENT_ARRAY_BUFFER.'],
    ['vertexAttribPointer', 'void vertexAttribPointer(int index, int size, int strideFloats, int offsetFloats)', 'Where vertex input `index` is in the buffer. Counted in floats, not bytes.'],
    ['enableVertexAttribArray', 'void enableVertexAttribArray(int index)', 'Turns a vertex input on.'],
    ['drawArrays', 'void drawArrays(int mode, int first, int count)', 'Draws corners from the bound vertex array (glDrawArrays).'],
    ['drawElements', 'void drawElements(int mode, int count)', 'Draws using the corner numbers in the ELEMENT_ARRAY_BUFFER (glDrawElements).'],
    ['genTexture', 'int genTexture()', 'A new, empty texture.'],
    ['bindTexture', 'void bindTexture(int target, int texture)', 'Uses this texture (target TEXTURE_2D).'],
    ['deleteTexture', 'void deleteTexture(int texture)', 'Frees a texture.'],
    ['activeTexture', 'void activeTexture(int unit)', 'Which texture unit bindTexture sets: TEXTURE0 to TEXTURE3.'],
    ['texParameteri', 'void texParameteri(int target, int name, int value)', 'A texture setting, like TEXTURE_MAG_FILTER, NEAREST.'],
    ['generateMipmap', 'void generateMipmap(int target)', 'Makes the smaller copies used when the texture is drawn small.'],
    ['loadTexture', 'int loadTexture(string path)', 'Loads an image from the game folder into a new texture. (0, 0) is the bottom-left.'],
]).concat(constants(['COLOR_BUFFER_BIT', 'DEPTH_BUFFER_BIT', 'STENCIL_BUFFER_BIT', 'POINTS', 'LINES', 'LINE_LOOP', 'LINE_STRIP',
    'TRIANGLES', 'TRIANGLE_STRIP', 'TRIANGLE_FAN', 'ARRAY_BUFFER', 'ELEMENT_ARRAY_BUFFER', 'STATIC_DRAW', 'DYNAMIC_DRAW', 'STREAM_DRAW',
    'VERTEX_SHADER', 'FRAGMENT_SHADER', 'DEPTH_TEST', 'BLEND', 'CULL_FACE', 'ZERO', 'ONE', 'SRC_ALPHA', 'ONE_MINUS_SRC_ALPHA', 'LESS',
    'LEQUAL', 'ALWAYS', 'FRONT', 'BACK', 'FRONT_AND_BACK', 'TEXTURE_2D', 'TEXTURE0', 'TEXTURE1', 'TEXTURE2', 'TEXTURE3',
    'TEXTURE_MIN_FILTER', 'TEXTURE_MAG_FILTER', 'TEXTURE_WRAP_S', 'TEXTURE_WRAP_T', 'NEAREST', 'LINEAR', 'NEAREST_MIPMAP_NEAREST',
    'LINEAR_MIPMAP_LINEAR', 'REPEAT', 'CLAMP_TO_EDGE', 'NO_ERROR', 'INVALID_ENUM', 'INVALID_VALUE', 'INVALID_OPERATION', 'OUT_OF_MEMORY'],
    'An OpenGL constant, with the same number as GL_ in real OpenGL.'));

LIBRARIES.DirectX = toolkitCommon('DirectX').concat([
    ['adapterName', 'string adapterName()', 'The graphics card\'s name.'],
    ['clear', 'void clear(float r, float g, float b, float a)', 'Fills the screen with a color and resets the depth buffer.'],
    ['setDepthTest', 'void setDepthTest(bool on)', 'Whether near things hide far things (on to start with).'],
    ['setBlending', 'void setBlending(bool on)', 'Whether see-through colors mix with what\'s behind them (off to start with).'],
    ['setCulling', 'void setCulling(int mode)', 'CULL_NONE (the start), CULL_FRONT or CULL_BACK.'],
    ['setTopology', 'void setTopology(int topology)', 'What corners make: TRIANGLES (the start), TRIANGLE_STRIP, LINES, LINE_STRIP, POINTS.'],
    ['compileVertexShader', 'int compileVertexShader(string hlsl, string entry = "main")', 'Compiles an HLSL function as a vertex shader (vs_5_0).'],
    ['compilePixelShader', 'int compilePixelShader(string hlsl, string entry = "main")', 'Compiles an HLSL function as a pixel shader (ps_5_0).'],
    ['setShaders', 'void setShaders(int vertexShader, int pixelShader)', 'Draws with these shaders from now on.'],
    ['createInputLayout', 'int createInputLayout(int vertexShader, list<string> names, list<int> sizes)', 'Describes one corner: each input\'s semantic name (POSITION, COLOR...) and size in floats.'],
    ['setInputLayout', 'void setInputLayout(int layout)', 'Uses this input layout from now on.'],
    ['createVertexBuffer', 'int createVertexBuffer(list<float> data)', 'A buffer of corners.'],
    ['createIndexBuffer', 'int createIndexBuffer(list<int> indices)', 'A buffer of corner numbers, for drawIndexed.'],
    ['createConstantBuffer', 'int createConstantBuffer(int floatCount)', 'A buffer of values for the shaders (a cbuffer), which can change every frame.'],
    ['updateConstantBuffer', 'void updateConstantBuffer(int buffer, list<float> data)', 'Replaces a constant buffer\'s values.'],
    ['setVertexBuffer', 'void setVertexBuffer(int buffer, int strideFloats)', 'Draws corners from this buffer, each strideFloats floats long.'],
    ['setIndexBuffer', 'void setIndexBuffer(int buffer)', 'Uses these corner numbers for drawIndexed.'],
    ['setConstantBuffer', 'void setConstantBuffer(int slot, int buffer)', 'Gives the buffer to both shaders as register(b<slot>).'],
    ['draw', 'void draw(int vertexCount, int startVertex = 0)', 'Draws corners from the vertex buffer.'],
    ['drawIndexed', 'void drawIndexed(int indexCount, int startIndex = 0)', 'Draws using the index buffer.'],
    ['loadTexture', 'int loadTexture(string path)', 'Loads an image from the game folder. (0, 0) is the top-left.'],
    ['setTexture', 'void setTexture(int slot, int texture)', 'Gives the texture to the pixel shader as register(t<slot>).'],
    ['setTextureFilter', 'void setTextureFilter(bool smooth)', 'True (the start) blends stretched pixels; false keeps them sharp.'],
]).concat(constants(['POINTS', 'LINES', 'LINE_STRIP', 'TRIANGLES', 'TRIANGLE_STRIP'], 'A topology, for setTopology.'),
    constants(['CULL_NONE', 'CULL_FRONT', 'CULL_BACK'], 'A culling mode, for setCulling.'));

LIBRARIES.Vulkan = toolkitCommon('Vulkan').concat([
    ['deviceName', 'string deviceName()', 'The graphics card\'s name.'],
    ['setTopology', 'void setTopology(int topology)', 'For pipelines made after this: TRIANGLES (the start), TRIANGLE_STRIP, TRIANGLE_FAN, LINES, LINE_STRIP, POINTS.'],
    ['setDepthTest', 'void setDepthTest(bool on)', 'For pipelines made after this: whether near things hide far things (on to start with).'],
    ['setCulling', 'void setCulling(int mode)', 'For pipelines made after this: CULL_NONE (the start), CULL_FRONT or CULL_BACK.'],
    ['setBlending', 'void setBlending(bool on)', 'For pipelines made after this: whether see-through colors mix (off to start with).'],
    ['loadShader', 'int loadShader(string spvPath)', 'Loads a compiled shader. frg build makes shape.vert.spv from shape.vert.'],
    ['createPipeline', 'int createPipeline(int vertexShader, int fragmentShader, list<int> attributeSizes, int pushConstantFloats = 0)', 'A pipeline: the shaders, each vertex input\'s size in floats, and room for push constants.'],
    ['createVertexBuffer', 'int createVertexBuffer(list<float> data)', 'A buffer of corners.'],
    ['createIndexBuffer', 'int createIndexBuffer(list<int> indices)', 'A buffer of corner numbers, for drawIndexed.'],
    ['beginFrame', 'bool beginFrame(float r, float g, float b, float a)', 'Starts a frame cleared to this color. False means skip drawing this time (don\'t call endFrame).'],
    ['bindPipeline', 'void bindPipeline(int pipeline)', 'Draws with this pipeline.'],
    ['bindVertexBuffer', 'void bindVertexBuffer(int buffer)', 'Draws corners from this buffer.'],
    ['bindIndexBuffer', 'void bindIndexBuffer(int buffer)', 'Uses these corner numbers for drawIndexed.'],
    ['pushConstants', 'void pushConstants(int pipeline, list<float> data)', 'Sends floats to the shaders\' push_constant block, for the draws after it.'],
    ['draw', 'void draw(int vertexCount, int firstVertex = 0)', 'Draws corners from the vertex buffer.'],
    ['drawIndexed', 'void drawIndexed(int indexCount, int firstIndex = 0)', 'Draws using the index buffer.'],
    ['endFrame', 'void endFrame()', 'Finishes the frame. It appears at the next shouldClose().'],
]).concat(constants(['POINTS', 'LINES', 'LINE_STRIP', 'TRIANGLES', 'TRIANGLE_STRIP', 'TRIANGLE_FAN'], 'A topology, for setTopology.'),
    constants(['CULL_NONE', 'CULL_FRONT', 'CULL_BACK'], 'A culling mode, for setCulling.'));

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
    Ease: ['LINEAR', 'IN', 'OUT', 'IN_OUT', 'BOUNCE'],
    GamepadButton: ['A', 'B', 'X', 'Y', 'LEFT_BUMPER', 'RIGHT_BUMPER', 'LEFT_TRIGGER', 'RIGHT_TRIGGER', 'BACK', 'START', 'LEFT_STICK', 'RIGHT_STICK', 'UP', 'DOWN', 'LEFT', 'RIGHT'],
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
            // E = error (red), W = warning (yellow)
            const isWarning = line.startsWith('W\t');
            if (!line.startsWith('E\t') && !isWarning) continue;
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
                hint ? `${msg}\nhint: ${hint}` : msg,
                isWarning ? vscode.DiagnosticSeverity.Warning : vscode.DiagnosticSeverity.Error);
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
            const all = [...byFile.values()].flat();
            const count = all.filter(d => d.severity === vscode.DiagnosticSeverity.Error).length;
            const warnings = all.length - count;
            const warningText = warnings ? ` and ${warnings} warning${warnings === 1 ? '' : 's'}` : '';
            vscode.window.showInformationMessage(count ? `Forge found ${count} error${count === 1 ? '' : 's'}${warningText}.`
                : `Forge found no errors${warningText}.`);
        }
        const again = running.get(root) === 'again';
        running.delete(root);
        if (again) check(doc);
    });
}

// ---------------------------------------------------------------- run and build

let gameTerminal = null;

// command is 'run', 'build' or 'package'. Unless extraArgs says otherwise, the forge.console setting decides the console.
async function runCommand(command, extraArgs) {
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
        shellArgs: [command, root].concat(extraArgs || (settings().get('console', true) ? [] : ['--no-console'])),
    });
    gameTerminal.show(true);
}

// ---------------------------------------------------------------- debugging
//
// Forge: Debug Game makes a debug build (frg build --debug: every line of the C++ is marked with its .frg line),
// then starts gdb (from w64devkit) through Microsoft's C/C++ extension, so breakpoints go on .frg lines.

let output = null;

function outputChannel() {
    if (!output) output = vscode.window.createOutputChannel('Forge');
    return output;
}

// A program's full path, found on the PATH like a terminal would.
function findOnPath(exe) {
    for (const dir of (process.env.PATH || '').split(path.delimiter)) {
        if (!dir) continue;
        const full = path.join(dir.replace(/^"|"$/g, ''), exe);
        if (fs.existsSync(full)) return full;
    }
    return null;
}

async function debugGame() {
    const editor = vscode.window.activeTextEditor;
    const file = editor ? editor.document.fileName : null;
    const root = file ? projectRoot(file) : null;
    if (!root) {
        vscode.window.showErrorMessage('Open a .frg file from a Forge game first (its top folder needs a main.frg).');
        return;
    }
    if (!vscode.extensions.getExtension('ms-vscode.cpptools')) {
        const choice = await vscode.window.showErrorMessage(
            "Debugging Forge games uses Microsoft's C/C++ extension (it drives the gdb debugger). Install it, then try again.",
            'Show the C/C++ extension');
        if (choice) vscode.commands.executeCommand('workbench.extensions.search', 'ms-vscode.cpptools');
        return;
    }
    await vscode.workspace.saveAll(false);
    const out = outputChannel();
    out.clear();
    out.show(true);
    out.appendLine('Making a debug build...');
    const built = await new Promise(resolve => {
        const child = cp.spawn(frgPath(), ['build', root, '--debug'], { cwd: root, windowsHide: true });
        const show = data => out.append(data.toString().replace(/\x1b\[[0-9;]*m/g, ''));  // without the terminal's colors
        child.stdout.on('data', show);
        child.stderr.on('data', show);
        child.on('error', err => {
            if (err.code === 'ENOENT') showMissingFrg();
            resolve(false);
        });
        child.on('close', code => resolve(code === 0));
    });
    if (!built) {
        vscode.window.showErrorMessage('Forge: the debug build failed. The Forge output panel says why.');
        return;
    }
    const exe = path.join(root, 'build', path.basename(root) + '.exe');
    await vscode.debug.startDebugging(vscode.workspace.getWorkspaceFolder(vscode.Uri.file(file)), {
        name: `Forge: ${path.basename(root)}`,
        type: 'cppdbg',
        request: 'launch',
        program: exe,
        args: [],
        cwd: root,
        MIMode: 'gdb',
        miDebuggerPath: findOnPath('gdb.exe') || 'gdb',
        externalConsole: true,  // print() output appears in the game's own console window, as with Run
        stopAtEntry: false,
    });
}

// ---------------------------------------------------------------- autocomplete and hover help

function libraryCompletions(document, position) {
    const before = document.lineAt(position).text.slice(0, position.character);
    const m = before.match(/\b([A-Za-z_][A-Za-z0-9_]*)\.\s*[A-Za-z0-9_]*$/);
    if (!m) return undefined;
    const owner = m[1];
    if (LIBRARIES[owner]) {
        return LIBRARIES[owner].map(([name, sig, doc]) => {
            const isConst = /^[A-Z][A-Z0-9_]*$/.test(name);
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

// ---------------------------------------------------------------- your own code: definitions, hover, autocomplete, rename
//
// Each question runs `frg query`, which reads the whole game (with this file's unsaved text, sent on its standard input)
// exactly as a build would, and answers in JSON. Positions go to frg as lines and byte columns starting at 1.

const COMPLETION_KINDS = {
    variable: vscode.CompletionItemKind.Variable, constant: vscode.CompletionItemKind.Constant,
    function: vscode.CompletionItemKind.Function, method: vscode.CompletionItemKind.Method,
    field: vscode.CompletionItemKind.Field, property: vscode.CompletionItemKind.Property,
    class: vscode.CompletionItemKind.Class, struct: vscode.CompletionItemKind.Struct,
    enum: vscode.CompletionItemKind.Enum, enumMember: vscode.CompletionItemKind.EnumMember,
    interface: vscode.CompletionItemKind.Interface, module: vscode.CompletionItemKind.Module,
    folder: vscode.CompletionItemKind.Folder, keyword: vscode.CompletionItemKind.Keyword,
};

const SYMBOL_KINDS = {
    struct: vscode.SymbolKind.Struct, class: vscode.SymbolKind.Class, 'abstract class': vscode.SymbolKind.Class,
    enum: vscode.SymbolKind.Enum, interface: vscode.SymbolKind.Interface, enumMember: vscode.SymbolKind.EnumMember,
    field: vscode.SymbolKind.Field, method: vscode.SymbolKind.Method, constructor: vscode.SymbolKind.Constructor,
    function: vscode.SymbolKind.Function, variable: vscode.SymbolKind.Variable, constant: vscode.SymbolKind.Constant,
};

function query(question, document, position, extra, token) {
    return new Promise(resolve => {
        const root = projectRoot(document.fileName);
        if (!root) return resolve(null);
        const lineText = document.lineAt(position.line).text;
        const col = Buffer.byteLength(lineText.slice(0, position.character), 'utf8') + 1;
        const args = ['query', question, root, document.fileName, String(position.line + 1), String(col), '--stdin'];
        if (extra) args.push(extra);
        const child = cp.execFile(frgPath(), args, { cwd: root, windowsHide: true, maxBuffer: 32 * 1024 * 1024 }, (err, stdout) => {
            if (err && err.code === 'ENOENT') {
                showMissingFrg();
                return resolve(null);
            }
            try {
                const lines = stdout.trim().split(/\r?\n/);
                resolve(JSON.parse(lines[lines.length - 1] || '{}'));
            } catch (e) {
                resolve(null);
            }
        });
        if (token) token.onCancellationRequested(() => { try { child.kill(); } catch (e) { /* already finished */ } });
        child.stdin.on('error', () => {});
        child.stdin.end(document.getText());
    });
}

// A place frg names ({file, line, col, len}, byte columns) as a VS Code location.
function toLocation(loc, cache) {
    const text = fileLines(loc.file, cache)[loc.line - 1] || '';
    const start = byteColToChar(text, loc.col - 1);
    const end = byteColToChar(text, loc.col - 1 + loc.len);
    return new vscode.Location(vscode.Uri.file(loc.file), new vscode.Range(loc.line - 1, start, loc.line - 1, Math.max(end, start)));
}

const definitionProvider = {
    async provideDefinition(document, position, token) {
        const r = await query('definition', document, position, null, token);
        return r && r.location ? toLocation(r.location, new Map()) : undefined;
    },
};

const referenceProvider = {
    async provideReferences(document, position, context, token) {
        const r = await query('references', document, position, null, token);
        if (!r || !r.locations) return [];
        const cache = new Map();
        return r.locations.map(l => toLocation(l, cache));
    },
};

const hoverProvider = {
    async provideHover(document, position, token) {
        const library = libraryHover(document, position);  // the built-in libraries have longer descriptions
        if (library) return library;
        const r = await query('hover', document, position, null, token);
        if (!r || !r.text) return undefined;
        return new vscode.Hover(new vscode.MarkdownString(r.text), document.getWordRangeAtPosition(position));
    },
};

const completionProvider = {
    async provideCompletionItems(document, position, token) {
        const library = libraryCompletions(document, position);
        if (library) return library;
        const r = await query('complete', document, position, null, token);
        if (!r || !r.items) return undefined;
        return r.items.map(i => {
            const item = new vscode.CompletionItem(i.label, COMPLETION_KINDS[i.kind] ?? vscode.CompletionItemKind.Text);
            if (i.detail) item.detail = i.detail;
            return item;
        });
    },
};

const renameProvider = {
    async prepareRename(document, position, token) {
        const range = document.getWordRangeAtPosition(position, /[A-Za-z_][A-Za-z0-9_]*/);
        if (!range) throw new Error('Put the cursor on a name to rename it.');
        const r = await query('rename', document, position, document.getText(range), token);
        if (!r || (!r.edits && !r.error)) throw new Error("Forge doesn't know what this name is, so it can't rename it.");
        if (r.error) throw new Error(r.error);
        return range;
    },
    async provideRenameEdits(document, position, newName, token) {
        const r = await query('rename', document, position, newName, token);
        if (!r) throw new Error("Forge couldn't work out what to rename.");
        if (r.error) throw new Error(r.error);
        const edit = new vscode.WorkspaceEdit();
        const cache = new Map();
        for (const l of r.edits || []) {
            const loc = toLocation(l, cache);
            edit.replace(loc.uri, loc.range, newName);
        }
        return edit;
    },
};

const symbolProvider = {
    async provideDocumentSymbols(document, token) {
        const r = await query('symbols', document, new vscode.Position(0, 0), null, token);
        if (!r || !r.symbols) return [];
        const lastLine = document.lineCount - 1;
        // Each symbol reaches down to the line before the next one (the outline only needs a rough end).
        const make = (list, endLine) => {
            const sorted = [...list].sort((a, b) => a.line - b.line || a.col - b.col);
            return sorted.map((s, i) => {
                const start = Math.min(Math.max(s.line - 1, 0), lastLine);
                const next = i + 1 < sorted.length ? sorted[i + 1].line - 2 : endLine;
                const end = Math.min(Math.max(start, next), lastLine);
                const text = document.lineAt(start).text;
                const startCh = byteColToChar(text, s.col - 1);
                const range = new vscode.Range(start, 0, end, document.lineAt(end).text.length);
                const selection = new vscode.Range(start, startCh, start, Math.min(startCh + s.name.length, text.length));
                const sym = new vscode.DocumentSymbol(s.name, s.detail || '', SYMBOL_KINDS[s.kind] ?? vscode.SymbolKind.Variable, range, selection);
                sym.children = make(s.children || [], end);
                return sym;
            });
        };
        return make(r.symbols, lastLine);
    },
};

// ---------------------------------------------------------------- activation

function activate(context) {
    extensionDir = context.extensionPath;
    diagnostics = vscode.languages.createDiagnosticCollection('forge');
    context.subscriptions.push(
        diagnostics,
        vscode.commands.registerCommand('forge.run', () => runCommand('run')),
        vscode.commands.registerCommand('forge.build', () => runCommand('build')),
        vscode.commands.registerCommand('forge.debug', debugGame),
        vscode.commands.registerCommand('forge.buildRelease', () => runCommand('build', ['--release'])),
        vscode.commands.registerCommand('forge.package', () => runCommand('package', [])),
        vscode.commands.registerCommand('forge.toggleConsole', async () => {
            const on = !settings().get('console', true);
            await settings().update('console', on, vscode.ConfigurationTarget.Global);
            vscode.window.showInformationMessage(on
                ? 'Forge: the console is on. Running a game shows print() output and errors next to it.'
                : 'Forge: the console is off. Games run the way players will see them; runtime errors pop up in a box.');
        }),
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
        vscode.languages.registerCompletionItemProvider('forge', completionProvider, '.'),
        vscode.languages.registerHoverProvider('forge', hoverProvider),
        vscode.languages.registerDefinitionProvider('forge', definitionProvider),
        vscode.languages.registerReferenceProvider('forge', referenceProvider),
        vscode.languages.registerRenameProvider('forge', renameProvider),
        vscode.languages.registerDocumentSymbolProvider('forge', symbolProvider),
    );
    for (const doc of vscode.workspace.textDocuments) if (doc.languageId === 'forge') check(doc);
}

function deactivate() {}

module.exports = { activate, deactivate };
