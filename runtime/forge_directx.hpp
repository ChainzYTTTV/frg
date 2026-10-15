// Forge runtime: <DirectX> - a low-level toolkit for drawing with Direct3D 11 yourself.
//
// Like <OpenGL>, this is NOT part of Forge's engine: a game using it opens its own window and draws everything
// with HLSL shaders and buffers. Objects (shaders, buffers, textures...) are whole-number handles.
// The code is in forge_directx_impl.cpp; the matrix helpers are below.
#pragma once

#include "forge_native.hpp"

namespace frg::DirectX {

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
std::string adapterName_();

// ---- drawing state
void clear_(float r, float g, float b, float a);
void setDepthTest_(bool on);
void setBlending_(bool on);
void setCulling_(int32_t mode);
void setTopology_(int32_t topology);

// ---- shaders
int32_t compileVertexShader_(const std::string& hlsl, const std::string& entry);
int32_t compilePixelShader_(const std::string& hlsl, const std::string& entry);
void setShaders_(int32_t vertexShader, int32_t pixelShader);
int32_t createInputLayout_(int32_t vertexShader, const List<std::string>& names, const List<int32_t>& sizes);
void setInputLayout_(int32_t layout);

// ---- buffers
int32_t createVertexBuffer_(const List<float>& data);
int32_t createIndexBuffer_(const List<int32_t>& indices);
int32_t createConstantBuffer_(int32_t floatCount);
void updateConstantBuffer_(int32_t buffer, const List<float>& data);
void setVertexBuffer_(int32_t buffer, int32_t strideFloats);
void setIndexBuffer_(int32_t buffer);
void setConstantBuffer_(int32_t slot, int32_t buffer);
void draw_(int32_t vertexCount, int32_t startVertex);
void drawIndexed_(int32_t indexCount, int32_t startIndex);

// ---- textures
int32_t loadTexture_(const std::string& path);
void setTexture_(int32_t slot, int32_t texture);
void setTextureFilter_(bool smooth);

// ---- matrices: 16 floats, column by column. In HLSL, use them as `mul(matrix, vector)`.
// Depth goes from 0 (near) to 1 (far), as Direct3D expects.

inline List<float> identity_() { return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; }

inline List<float> multiply_(const List<float>& a, const List<float>& b) {
    if (a.size() != 16 || b.size() != 16) panic("DirectX.multiply()", "matrices are lists of exactly 16 numbers");
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
    m[10] = farPlane / (nearPlane - farPlane);
    m[11] = -1;
    m[14] = nearPlane * farPlane / (nearPlane - farPlane);
    return m;
}

inline List<float> orthographic_(float l, float r, float b, float t, float n, float f) {
    List<float> m = identity_();
    m[0] = 2 / (r - l);
    m[5] = 2 / (t - b);
    m[10] = 1 / (n - f);
    m[12] = -(r + l) / (r - l);
    m[13] = -(t + b) / (t - b);
    m[14] = n / (n - f);
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

}  // namespace frg::DirectX
