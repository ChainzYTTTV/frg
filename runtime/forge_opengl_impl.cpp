// Forge runtime: <OpenGL> implementation. Compiled into games that #include <OpenGL>.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#undef LoadImage  // windows.h renames it; we want raylib's

#include "forge_opengl.hpp"
#include "native_window.inl"

extern "C" {
FrgRlImage LoadImage(const char* fileName);
void UnloadImage(FrgRlImage image);
void ImageFormat(FrgRlImage* image, int newFormat);
}

namespace frg::OpenGL {

namespace {

// OpenGL 2.0+ functions aren't in opengl32.dll's export list; they're looked up once the context exists.
#define FRG_GL_FUNCS(X)                                                              \
    X(PFNGLCREATESHADERPROC, glCreateShader)                                         \
    X(PFNGLSHADERSOURCEPROC, glShaderSource)                                         \
    X(PFNGLCOMPILESHADERPROC, glCompileShader)                                       \
    X(PFNGLGETSHADERIVPROC, glGetShaderiv)                                           \
    X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                                 \
    X(PFNGLDELETESHADERPROC, glDeleteShader)                                         \
    X(PFNGLCREATEPROGRAMPROC, glCreateProgram)                                       \
    X(PFNGLATTACHSHADERPROC, glAttachShader)                                         \
    X(PFNGLLINKPROGRAMPROC, glLinkProgram)                                           \
    X(PFNGLGETPROGRAMIVPROC, glGetProgramiv)                                         \
    X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)                               \
    X(PFNGLUSEPROGRAMPROC, glUseProgram)                                             \
    X(PFNGLDELETEPROGRAMPROC, glDeleteProgram)                                       \
    X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)                             \
    X(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation)                               \
    X(PFNGLUNIFORM1FPROC, glUniform1f)                                               \
    X(PFNGLUNIFORM2FPROC, glUniform2f)                                               \
    X(PFNGLUNIFORM3FPROC, glUniform3f)                                               \
    X(PFNGLUNIFORM4FPROC, glUniform4f)                                               \
    X(PFNGLUNIFORM1IPROC, glUniform1i)                                               \
    X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv)                                 \
    X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)                                   \
    X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                                   \
    X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)                             \
    X(PFNGLGENBUFFERSPROC, glGenBuffers)                                             \
    X(PFNGLBINDBUFFERPROC, glBindBuffer)                                             \
    X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                                       \
    X(PFNGLBUFFERDATAPROC, glBufferData)                                             \
    X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)                           \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)                   \
    X(PFNGLACTIVETEXTUREPROC, glActiveTexture)                                       \
    X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap)

#define FRG_DECLARE(type, name) type p_##name = nullptr;
FRG_GL_FUNCS(FRG_DECLARE)
#undef FRG_DECLARE

HDC dc = nullptr;
HGLRC rc = nullptr;
PFNWGLSWAPINTERVALEXTPROC swapInterval = nullptr;

void* lookup(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    intptr_t v = (intptr_t)p;
    if (v == 0 || v == 1 || v == 2 || v == 3 || v == -1) {
        static HMODULE lib = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(lib, name);
    }
    return p;
}

void present() { SwapBuffers(dc); }

void screenshot(const std::string& path) {
    int w = native::width(), h = native::height();
    std::vector<unsigned char> px((size_t)w * h * 4), flipped((size_t)w * h * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    for (int y = 0; y < h; y++) std::memcpy(&flipped[(size_t)y * w * 4], &px[(size_t)(h - 1 - y) * w * 4], (size_t)w * 4);
    native::savePng(path, flipped.data(), w, h);
}

void need(const char* what) {
    if (!rc) native::fail(what, "the window isn't open yet - call OpenGL.openWindow(width, height, \"title\") first");
}

}  // namespace

void openWindow_(int32_t width, int32_t height, const std::string& title) {
    native::open(width, height, title, &present, &screenshot);
    dc = GetDC((HWND)native::handle());

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof pfd;
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);

    // A temporary old-style context is needed to ask for a modern (3.3 core) one.
    HGLRC temp = wglCreateContext(dc);
    wglMakeCurrent(dc, temp);
    auto createAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    rc = nullptr;
    if (createAttribs) {
        const int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                               WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
        rc = createAttribs(dc, nullptr, attribs);
    }
    if (!rc) native::fail("OpenGL.openWindow()", "this computer's graphics driver doesn't support OpenGL 3.3");
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(temp);
    wglMakeCurrent(dc, rc);

#define FRG_LOAD(type, name) p_##name = (type)lookup(#name);
    FRG_GL_FUNCS(FRG_LOAD)
#undef FRG_LOAD
    swapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (swapInterval) swapInterval(1);
    glViewport(0, 0, width, height);
    addHook(shutdownHooks(), &closeWindow_);
}

void closeWindow_() {
    if (!rc) return;
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(rc);
    ReleaseDC((HWND)native::handle(), dc);
    rc = nullptr;
    native::close();
}

void setVSync_(bool on) {
    if (swapInterval) swapInterval(on ? 1 : 0);
}

std::string version_() {
    need("OpenGL.version()");
    const GLubyte* v = glGetString(GL_VERSION);
    return v ? (const char*)v : "";
}

void clearColor_(float r, float g, float b, float a) { need("OpenGL.clearColor()"); glClearColor(r, g, b, a); }
void clear_(int32_t mask) { need("OpenGL.clear()"); glClear((GLbitfield)mask); }
void viewport_(int32_t x, int32_t y, int32_t w, int32_t h) { need("OpenGL.viewport()"); glViewport(x, y, w, h); }
void enable_(int32_t cap) { need("OpenGL.enable()"); glEnable((GLenum)cap); }
void disable_(int32_t cap) { need("OpenGL.disable()"); glDisable((GLenum)cap); }
void blendFunc_(int32_t s, int32_t d) { need("OpenGL.blendFunc()"); glBlendFunc((GLenum)s, (GLenum)d); }
void depthFunc_(int32_t f) { need("OpenGL.depthFunc()"); glDepthFunc((GLenum)f); }
void cullFace_(int32_t f) { need("OpenGL.cullFace()"); glCullFace((GLenum)f); }
void lineWidth_(float w) { need("OpenGL.lineWidth()"); glLineWidth(w); }
int32_t getError_() { need("OpenGL.getError()"); return (int32_t)glGetError(); }

int32_t createShader_(int32_t type) { need("OpenGL.createShader()"); return (int32_t)p_glCreateShader((GLenum)type); }
void shaderSource_(int32_t shader, const std::string& source) {
    need("OpenGL.shaderSource()");
    const char* text = source.c_str();
    GLint len = (GLint)source.size();
    p_glShaderSource((GLuint)shader, 1, &text, &len);
}
void compileShader_(int32_t shader) { need("OpenGL.compileShader()"); p_glCompileShader((GLuint)shader); }
bool shaderCompiled_(int32_t shader) {
    need("OpenGL.shaderCompiled()");
    GLint ok = 0;
    p_glGetShaderiv((GLuint)shader, GL_COMPILE_STATUS, &ok);
    return ok != 0;
}
std::string shaderLog_(int32_t shader) {
    need("OpenGL.shaderLog()");
    GLint len = 0;
    p_glGetShaderiv((GLuint)shader, GL_INFO_LOG_LENGTH, &len);
    std::string log(len > 0 ? len : 0, '\0');
    if (len > 0) p_glGetShaderInfoLog((GLuint)shader, len, nullptr, log.data());
    while (!log.empty() && (log.back() == '\0' || log.back() == '\n')) log.pop_back();
    return log;
}
void deleteShader_(int32_t shader) { need("OpenGL.deleteShader()"); p_glDeleteShader((GLuint)shader); }
int32_t createProgram_() { need("OpenGL.createProgram()"); return (int32_t)p_glCreateProgram(); }
void attachShader_(int32_t program, int32_t shader) { need("OpenGL.attachShader()"); p_glAttachShader((GLuint)program, (GLuint)shader); }
void linkProgram_(int32_t program) { need("OpenGL.linkProgram()"); p_glLinkProgram((GLuint)program); }
bool programLinked_(int32_t program) {
    need("OpenGL.programLinked()");
    GLint ok = 0;
    p_glGetProgramiv((GLuint)program, GL_LINK_STATUS, &ok);
    return ok != 0;
}
std::string programLog_(int32_t program) {
    need("OpenGL.programLog()");
    GLint len = 0;
    p_glGetProgramiv((GLuint)program, GL_INFO_LOG_LENGTH, &len);
    std::string log(len > 0 ? len : 0, '\0');
    if (len > 0) p_glGetProgramInfoLog((GLuint)program, len, nullptr, log.data());
    while (!log.empty() && (log.back() == '\0' || log.back() == '\n')) log.pop_back();
    return log;
}
void useProgram_(int32_t program) { need("OpenGL.useProgram()"); p_glUseProgram((GLuint)program); }
void deleteProgram_(int32_t program) { need("OpenGL.deleteProgram()"); p_glDeleteProgram((GLuint)program); }

int32_t buildProgram_(const std::string& vertexSource, const std::string& fragmentSource) {
    need("OpenGL.buildProgram()");
    int32_t vs = createShader_(GL_VERTEX_SHADER);
    shaderSource_(vs, vertexSource);
    compileShader_(vs);
    if (!shaderCompiled_(vs)) native::fail("OpenGL.buildProgram()", "the vertex shader has an error:\n" + shaderLog_(vs));
    int32_t fs = createShader_(GL_FRAGMENT_SHADER);
    shaderSource_(fs, fragmentSource);
    compileShader_(fs);
    if (!shaderCompiled_(fs)) native::fail("OpenGL.buildProgram()", "the fragment shader has an error:\n" + shaderLog_(fs));
    int32_t prog = createProgram_();
    attachShader_(prog, vs);
    attachShader_(prog, fs);
    linkProgram_(prog);
    if (!programLinked_(prog)) native::fail("OpenGL.buildProgram()", "the shaders couldn't be linked:\n" + programLog_(prog));
    deleteShader_(vs);
    deleteShader_(fs);
    return prog;
}

int32_t getUniformLocation_(int32_t program, const std::string& name) {
    need("OpenGL.getUniformLocation()");
    return p_glGetUniformLocation((GLuint)program, name.c_str());
}
int32_t getAttribLocation_(int32_t program, const std::string& name) {
    need("OpenGL.getAttribLocation()");
    return p_glGetAttribLocation((GLuint)program, name.c_str());
}
void uniform1f_(int32_t l, float x) { need("OpenGL.uniform1f()"); p_glUniform1f(l, x); }
void uniform2f_(int32_t l, float x, float y) { need("OpenGL.uniform2f()"); p_glUniform2f(l, x, y); }
void uniform3f_(int32_t l, float x, float y, float z) { need("OpenGL.uniform3f()"); p_glUniform3f(l, x, y, z); }
void uniform4f_(int32_t l, float x, float y, float z, float w) { need("OpenGL.uniform4f()"); p_glUniform4f(l, x, y, z, w); }
void uniform1i_(int32_t l, int32_t x) { need("OpenGL.uniform1i()"); p_glUniform1i(l, x); }
void uniformMatrix4_(int32_t l, const List<float>& m) {
    need("OpenGL.uniformMatrix4()");
    if (m.size() != 16) native::fail("OpenGL.uniformMatrix4()", "a 4x4 matrix is a list of exactly 16 numbers, but this has " + str((int64_t)m.size()));
    p_glUniformMatrix4fv(l, 1, GL_FALSE, m.data());
}

int32_t genVertexArray_() {
    need("OpenGL.genVertexArray()");
    GLuint id = 0;
    p_glGenVertexArrays(1, &id);
    return (int32_t)id;
}
void bindVertexArray_(int32_t vao) { need("OpenGL.bindVertexArray()"); p_glBindVertexArray((GLuint)vao); }
void deleteVertexArray_(int32_t vao) {
    need("OpenGL.deleteVertexArray()");
    GLuint id = (GLuint)vao;
    p_glDeleteVertexArrays(1, &id);
}
int32_t genBuffer_() {
    need("OpenGL.genBuffer()");
    GLuint id = 0;
    p_glGenBuffers(1, &id);
    return (int32_t)id;
}
void bindBuffer_(int32_t target, int32_t buffer) { need("OpenGL.bindBuffer()"); p_glBindBuffer((GLenum)target, (GLuint)buffer); }
void deleteBuffer_(int32_t buffer) {
    need("OpenGL.deleteBuffer()");
    GLuint id = (GLuint)buffer;
    p_glDeleteBuffers(1, &id);
}
void bufferData_(int32_t target, const List<float>& data, int32_t usage) {
    need("OpenGL.bufferData()");
    p_glBufferData((GLenum)target, (GLsizeiptr)(data.size() * sizeof(float)), data.data(), (GLenum)usage);
}
void bufferDataInts_(int32_t target, const List<int32_t>& data, int32_t usage) {
    need("OpenGL.bufferDataInts()");
    p_glBufferData((GLenum)target, (GLsizeiptr)(data.size() * sizeof(int32_t)), data.data(), (GLenum)usage);
}
void vertexAttribPointer_(int32_t index, int32_t size, int32_t strideFloats, int32_t offsetFloats) {
    need("OpenGL.vertexAttribPointer()");
    p_glVertexAttribPointer((GLuint)index, size, GL_FLOAT, GL_FALSE, strideFloats * (GLsizei)sizeof(float),
                            (const void*)(intptr_t)(offsetFloats * sizeof(float)));
}
void enableVertexAttribArray_(int32_t index) { need("OpenGL.enableVertexAttribArray()"); p_glEnableVertexAttribArray((GLuint)index); }
void drawArrays_(int32_t mode, int32_t first, int32_t count) { need("OpenGL.drawArrays()"); glDrawArrays((GLenum)mode, first, count); }
void drawElements_(int32_t mode, int32_t count) {
    need("OpenGL.drawElements()");
    glDrawElements((GLenum)mode, count, GL_UNSIGNED_INT, nullptr);
}

int32_t genTexture_() {
    need("OpenGL.genTexture()");
    GLuint id = 0;
    glGenTextures(1, &id);
    return (int32_t)id;
}
void bindTexture_(int32_t target, int32_t texture) { need("OpenGL.bindTexture()"); glBindTexture((GLenum)target, (GLuint)texture); }
void deleteTexture_(int32_t texture) {
    need("OpenGL.deleteTexture()");
    GLuint id = (GLuint)texture;
    glDeleteTextures(1, &id);
}
void activeTexture_(int32_t unit) { need("OpenGL.activeTexture()"); p_glActiveTexture((GLenum)unit); }
void texParameteri_(int32_t target, int32_t name, int32_t value) {
    need("OpenGL.texParameteri()");
    glTexParameteri((GLenum)target, (GLenum)name, value);
}
void generateMipmap_(int32_t target) { need("OpenGL.generateMipmap()"); p_glGenerateMipmap((GLenum)target); }

// Loads an image file into a new texture, flipped so that texture coordinate (0, 0) is the bottom-left
// (OpenGL's usual way round).
int32_t loadTexture_(const std::string& path) {
    need("OpenGL.loadTexture()");
    std::string file = native::findGameFile(path);
    if (file.empty()) native::fail("OpenGL.loadTexture()", "can't find the file '" + path + "' - put it in the game's folder (next to main.frg)");
    FrgRlImage img = LoadImage(file.c_str());
    if (!img.data) native::fail("OpenGL.loadTexture()", "couldn't read '" + path + "' as an image");
    ImageFormat(&img, 7);  // RGBA, 8 bits each
    int w = img.width, h = img.height;
    std::vector<unsigned char> flipped((size_t)w * h * 4);
    const unsigned char* src = (const unsigned char*)img.data;
    for (int y = 0; y < h; y++) std::memcpy(&flipped[(size_t)y * w * 4], &src[(size_t)(h - 1 - y) * w * 4], (size_t)w * 4);
    UnloadImage(img);

    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, flipped.data());
    p_glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return (int32_t)id;
}

}  // namespace frg::OpenGL
