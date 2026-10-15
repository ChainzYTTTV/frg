// Forge runtime: <OpenGL> - a low-level toolkit for drawing with OpenGL 3.3 yourself.
//
// This is NOT part of Forge's engine: a game using it opens its own window (OpenGL.openWindow) and does all
// of its drawing with shaders and buffers. The functions are thin wrappers around the real OpenGL calls,
// using Forge types: whole numbers for object handles, list<float> for data, strings for shaders.
// The code is in forge_opengl_impl.cpp; the matrix helpers are below.
#pragma once

#include "forge_native.hpp"

namespace frg::OpenGL {

using Key_ = native::Key_;
using MouseButton_ = native::MouseButton_;
inline std::string frg$str(Key_ k) { return native::keyName(k); }
inline std::string frg$str(MouseButton_ b) { return native::buttonName(b); }

// ---- the window
void openWindow_(int32_t width, int32_t height, const std::string& title);
void closeWindow_();
inline bool shouldClose_() { return native::shouldClose(); }
inline int32_t width_() { return native::width(); }
inline int32_t height_() { return native::height(); }
inline float deltaTime_() { return native::deltaTime(); }
inline double time_() { return native::time(); }
inline void setTitle_(const std::string& t) { native::setTitle(t); }
void setVSync_(bool on);
inline bool keyDown_(Key_ k) { return native::keyDown(k); }
inline bool keyPressed_(Key_ k) { return native::keyPressed(k); }
inline bool keyReleased_(Key_ k) { return native::keyReleased(k); }
inline bool mouseDown_(MouseButton_ b) { return native::mouseDown(b); }
inline bool mousePressed_(MouseButton_ b) { return native::mousePressed(b); }
inline bool mouseReleased_(MouseButton_ b) { return native::mouseReleased(b); }
inline core::vec2_ mousePosition_() { return native::mousePosition(); }
inline float mouseWheel_() { return native::mouseWheel(); }
std::string version_();

// ---- state
void clearColor_(float r, float g, float b, float a);
void clear_(int32_t mask);
void viewport_(int32_t x, int32_t y, int32_t w, int32_t h);
void enable_(int32_t cap);
void disable_(int32_t cap);
void blendFunc_(int32_t src, int32_t dst);
void depthFunc_(int32_t f);
void cullFace_(int32_t face);
void lineWidth_(float w);
int32_t getError_();

// ---- shaders
int32_t createShader_(int32_t type);
void shaderSource_(int32_t shader, const std::string& source);
void compileShader_(int32_t shader);
bool shaderCompiled_(int32_t shader);
std::string shaderLog_(int32_t shader);
void deleteShader_(int32_t shader);
int32_t createProgram_();
void attachShader_(int32_t program, int32_t shader);
void linkProgram_(int32_t program);
bool programLinked_(int32_t program);
std::string programLog_(int32_t program);
void useProgram_(int32_t program);
void deleteProgram_(int32_t program);
int32_t buildProgram_(const std::string& vertexSource, const std::string& fragmentSource);
int32_t getUniformLocation_(int32_t program, const std::string& name);
int32_t getAttribLocation_(int32_t program, const std::string& name);
void uniform1f_(int32_t loc, float x);
void uniform2f_(int32_t loc, float x, float y);
void uniform3f_(int32_t loc, float x, float y, float z);
void uniform4f_(int32_t loc, float x, float y, float z, float w);
void uniform1i_(int32_t loc, int32_t x);
void uniformMatrix4_(int32_t loc, const List<float>& m);

// ---- buffers and drawing
int32_t genVertexArray_();
void bindVertexArray_(int32_t vao);
void deleteVertexArray_(int32_t vao);
int32_t genBuffer_();
void bindBuffer_(int32_t target, int32_t buffer);
void deleteBuffer_(int32_t buffer);
void bufferData_(int32_t target, const List<float>& data, int32_t usage);
void bufferDataInts_(int32_t target, const List<int32_t>& data, int32_t usage);
void vertexAttribPointer_(int32_t index, int32_t size, int32_t strideFloats, int32_t offsetFloats);
void enableVertexAttribArray_(int32_t index);
void drawArrays_(int32_t mode, int32_t first, int32_t count);
void drawElements_(int32_t mode, int32_t count);

// ---- textures
int32_t genTexture_();
void bindTexture_(int32_t target, int32_t texture);
void deleteTexture_(int32_t texture);
void activeTexture_(int32_t unit);
void texParameteri_(int32_t target, int32_t name, int32_t value);
void generateMipmap_(int32_t target);
int32_t loadTexture_(const std::string& path);

// ---- matrices: 16 floats, column by column (the order OpenGL's uniformMatrix4 expects)

inline List<float> identity_() { return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; }

inline List<float> multiply_(const List<float>& a, const List<float>& b) {
    if (a.size() != 16 || b.size() != 16) panic("OpenGL.multiply()", "matrices are lists of exactly 16 numbers");
    List<float> r(16, 0.0f);
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++) {
            float s = 0;
            for (int k = 0; k < 4; k++) s += a[k * 4 + row] * b[c * 4 + k];
            r[c * 4 + row] = s;
        }
    return r;
}

inline List<float> perspective_(float fovDegrees, float aspect, float nearPlane, float farPlane) {
    float f = 1.0f / std::tan(fovDegrees * 3.14159265f / 360.0f);
    List<float> m(16, 0.0f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    m[11] = -1;
    m[14] = 2 * farPlane * nearPlane / (nearPlane - farPlane);
    return m;
}

inline List<float> orthographic_(float l, float r, float b, float t, float n, float f) {
    List<float> m = identity_();
    m[0] = 2 / (r - l);
    m[5] = 2 / (t - b);
    m[10] = -2 / (f - n);
    m[12] = -(r + l) / (r - l);
    m[13] = -(t + b) / (t - b);
    m[14] = -(f + n) / (f - n);
    return m;
}

inline List<float> lookAt_(core::vec3_ eye, core::vec3_ target, core::vec3_ up) {
    core::vec3_ f = (target - eye).normalized_();
    core::vec3_ s = f.cross_(up).normalized_();
    core::vec3_ u = s.cross_(f);
    return {s.x_, u.x_, -f.x_, 0, s.y_, u.y_, -f.y_, 0, s.z_, u.z_, -f.z_, 0, -s.dot_(eye), -u.dot_(eye), f.dot_(eye), 1};
}

inline List<float> translate_(core::vec3_ v) {
    List<float> m = identity_();
    m[12] = v.x_;
    m[13] = v.y_;
    m[14] = v.z_;
    return m;
}

inline List<float> scale_(core::vec3_ v) {
    List<float> m = identity_();
    m[0] = v.x_;
    m[5] = v.y_;
    m[10] = v.z_;
    return m;
}

inline List<float> rotate_(float degrees, core::vec3_ axis) {
    core::vec3_ a = axis.normalized_();
    float r = degrees * 3.14159265f / 180.0f, c = std::cos(r), s = std::sin(r), t = 1 - c;
    return {t * a.x_ * a.x_ + c,        t * a.x_ * a.y_ + s * a.z_, t * a.x_ * a.z_ - s * a.y_, 0,
            t * a.x_ * a.y_ - s * a.z_, t * a.y_ * a.y_ + c,        t * a.y_ * a.z_ + s * a.x_, 0,
            t * a.x_ * a.z_ + s * a.y_, t * a.y_ * a.z_ - s * a.x_, t * a.z_ * a.z_ + c,        0,
            0,                          0,                          0,                          1};
}

}  // namespace frg::OpenGL
