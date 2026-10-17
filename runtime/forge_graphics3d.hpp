// Forge runtime: <graphics3d> - cameras, shapes and models in 3D.
//
// 3D drawing and 2D drawing can be mixed in a frame: every 3D call switches into 3D mode (using the
// camera set with graphics3d.setCamera), and every 2D call switches back, drawing on top.
#pragma once

#include "forge_assets.hpp"
#include "forge_window.hpp"

namespace frg::graphics3d {

inline ::Vector3 rl3(core::vec3_ v) { return ::Vector3{v.x_, v.y_, v.z_}; }

struct Camera_ {
    core::vec3_ position_{0, 5, 10};
    core::vec3_ target_{0, 0, 0};
    core::vec3_ up_{0, 1, 0};
    float fov_ = 45;
    static Camera_ frg$make() { return {}; }
    static Camera_ frg$make(core::vec3_ position, core::vec3_ target) {
        Camera_ c;
        c.position_ = position;
        c.target_ = target;
        return c;
    }
    // Moves the camera around its target, keeping the same distance and height.
    void orbit_(float degrees) {
        float r = degrees * 3.14159265f / 180.0f;
        float dx = position_.x_ - target_.x_, dz = position_.z_ - target_.z_;
        float c = std::cos(r), s = std::sin(r);
        position_.x_ = target_.x_ + dx * c - dz * s;
        position_.z_ = target_.z_ + dx * s + dz * c;
    }
    bool operator==(const Camera_&) const = default;
};

inline std::string frg$str(const Camera_& c) {
    return "Camera(position: " + core::frg$str(c.position_) + ", target: " + core::frg$str(c.target_) + ")";
}

struct State {
    Camera_ camera;
    bool in3D = false;
    int transforms = 0;  // pushTransform() calls not popped yet
    bool culling = true;     // skip things entirely outside what the camera sees
    long cullFrame = -1;     // the frame culledNow counts
    int32_t culledNow = 0;   // things skipped this frame
    int32_t culledLast = 0;  // ... and in the frame before
};

// raylib's matrix stack (rlgl, compiled into raylib).
extern "C" void rlPushMatrix(void);
extern "C" void rlPopMatrix(void);
extern "C" void rlTranslatef(float x, float y, float z);
extern "C" void rlRotatef(float angle, float x, float y, float z);
extern "C" void rlScalef(float x, float y, float z);
extern "C" ::Matrix rlGetMatrixModelview(void);   // the camera's view (BeginMode3D)
extern "C" ::Matrix rlGetMatrixProjection(void);
extern "C" ::Matrix rlGetMatrixTransform(void);   // pushTransform's matrix
inline State& st() {
    static State s;
    return s;
}

inline ::Camera3D rlCamera() {
    const Camera_& c = st().camera;
    ::Camera3D cam{};
    cam.position = rl3(c.position_);
    cam.target = rl3(c.target_);
    cam.up = rl3(c.up_);
    cam.fovy = c.fov_;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

void beginLight();
void endLight();

inline void leave3D() {
    if (!st().in3D) return;
    endLight();
    if (st().transforms > 0) {  // the 3D part ends with transforms still pushed: undo them, then say so
        int n = st().transforms;
        while (st().transforms > 0) {
            rlPopMatrix();
            st().transforms--;
        }
        EndMode3D();
        st().in3D = false;
        window::st().leave3D = nullptr;
        panic("graphics3d.pushTransform()",
              std::to_string(n) + " pushTransform() call" + (n == 1 ? " wasn't" : "s weren't") +
                  " undone with popTransform() before the frame ended (or before 2D drawing, or setCamera) - every pushTransform() needs a popTransform() after the drawing it's for");
    }
    EndMode3D();
    st().in3D = false;
    window::st().leave3D = nullptr;
}

inline void enter3D(const char* what) {
    if (!window::st().open) panic(what, "the window isn't open yet - call window.open(width, height, \"title\") first");
    if (!window::st().inFrame) window::beginFrame();
    if (st().in3D) return;
    window::leaveCamera2D();  // 3D uses its own camera; the 2D one comes back at the next 2D drawing
    BeginMode3D(rlCamera());
    st().in3D = true;
    beginLight();
    window::st().leave3D = &leave3D;
    addHook(window::beforeEndFrameHooks(), &leave3D);
}

// ---------------------------------------------------------------- light
// Off until setLight(): then every shape, mesh and model is lit by one light from a direction (like the sun) plus
// a soft light from everywhere (ambient). Surfaces facing the light are bright, the ones facing away get the
// ambient only. Lines and outlines aren't lit.
//
// Three versions of one shader: for raylib's batched shapes (their normals are already in world space), for
// Forge's meshes (cut-out: see-through pixels are dropped) and for models, spheres and cylinders (drawn as meshes,
// so they can be see-through, like smoke).
inline const char* litVertex() {
    return R"GLSL(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = USE_MODEL ? mat3(matModel) * vertexNormal : vertexNormal;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";
}
inline const char* litFragment() {
    return R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambient;
out vec4 finalColor;
void main() {
    vec4 c = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    if (CUTOUT && c.a < 0.5) discard;
    float len = length(fragNormal);
    if (len < 0.01) {          // no direction to light it by: as it is
        finalColor = c;
        return;
    }
    float d = max(dot(fragNormal / len, -lightDir), 0.0);
    finalColor = vec4(c.rgb * min(ambient + lightColor * d, vec3(1.0)), c.a);
}
)GLSL";
}

struct Light {
    bool on = false;
    ::Vector3 dir{-0.37f, -0.84f, -0.4f};     // the way the light travels (normalized)
    ::Vector3 color{1.0f, 0.98f, 0.94f};
    ::Vector3 ambient{0.4f, 0.42f, 0.48f};
    ::Shader batch{}, cutout{}, smooth{};
    bool loaded = false;
    ::Mesh sphere{}, cylinder{};               // unit shapes with normals, for lit spheres and cylinders
    ::Material material{};
    bool active = false;                       // the batch shader is in use (between beginLight and endLight)
};
inline Light& light() {
    static Light l;
    return l;
}

inline std::string litSource(const char* src, bool model, bool cutout) {
    std::string s = src;
    auto put = [&](const std::string& name, bool v) {
        for (size_t at = s.find(name); at != std::string::npos; at = s.find(name, at)) s.replace(at, name.size(), v ? "true" : "false");
    };
    put("USE_MODEL", model);
    put("CUTOUT", cutout);
    return s;
}

inline void unloadLight() {
    Light& l = light();
    if (!l.loaded) return;
    UnloadShader(l.batch);
    UnloadShader(l.cutout);
    UnloadShader(l.smooth);
    UnloadMesh(l.sphere);
    UnloadMesh(l.cylinder);
    if (l.material.maps) RL_FREE(l.material.maps);
    l.material.maps = nullptr;
    l.loaded = false;
}

inline void loadLight() {
    Light& l = light();
    if (l.loaded) return;
    l.batch = LoadShaderFromMemory(litSource(litVertex(), false, false).c_str(), litSource(litFragment(), false, false).c_str());
    l.cutout = LoadShaderFromMemory(litSource(litVertex(), true, false).c_str(), litSource(litFragment(), true, true).c_str());
    l.smooth = LoadShaderFromMemory(litSource(litVertex(), true, false).c_str(), litSource(litFragment(), true, false).c_str());
    l.sphere = GenMeshSphere(1, 16, 24);
    l.cylinder = GenMeshCylinder(1, 1, 24);
    l.material = LoadMaterialDefault();
    l.loaded = true;
    addHook(window::beforeCloseHooks(), &unloadLight);
}

// Gives a lit shader the light's numbers.
inline void lightUniforms(::Shader& s) {
    Light& l = light();
    SetShaderValue(s, GetShaderLocation(s, "lightDir"), &l.dir, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "lightColor"), &l.color, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "ambient"), &l.ambient, SHADER_UNIFORM_VEC3);
}

inline bool lit() { return light().on; }

inline ::Vector3 colorVec(core::color_ c) { return ::Vector3{c.r_ / 255.0f, c.g_ / 255.0f, c.b_ / 255.0f}; }

inline void setLight_(core::vec3_ direction, core::color_ lightColor, core::color_ ambient) {
    float len = std::sqrt(direction.x_ * direction.x_ + direction.y_ * direction.y_ + direction.z_ * direction.z_);
    if (len < 1e-6f) panic("graphics3d.setLight()", "the light's direction can't be (0, 0, 0) - it's the way the light shines, like vec3(0, -1, 0) for straight down");
    leave3D();   // the 3D drawn next uses the new light
    Light& l = light();
    l.dir = ::Vector3{direction.x_ / len, direction.y_ / len, direction.z_ / len};
    l.color = colorVec(lightColor);
    l.ambient = colorVec(ambient);
    l.on = true;
}
inline void lightOff_() {
    leave3D();
    light().on = false;
}
inline bool isLit_() { return light().on; }

// A sphere or cylinder, lit: drawn from the unit shape, stretched and moved (and through pushTransform).
inline void drawLitShape(const ::Mesh& mesh, ::Vector3 scale, ::Vector3 at, core::color_ c) {
    Light& l = light();
    ::Matrix m{};
    m.m0 = scale.x;
    m.m5 = scale.y;
    m.m10 = scale.z;
    m.m15 = 1;
    m.m12 = at.x;
    m.m13 = at.y;
    m.m14 = at.z;
    l.material.shader = l.smooth;
    l.material.maps[MATERIAL_MAP_DIFFUSE].color = rl(c);
    lightUniforms(l.smooth);
    DrawMesh(mesh, l.material, m);
}

inline void beginLight() {
    if (!lit()) return;
    loadLight();
    lightUniforms(light().batch);
    BeginShaderMode(light().batch);
    light().active = true;
}
inline void endLight() {
    if (!light().active) return;
    EndShaderMode();
    light().active = false;
}

// Lines and outlines aren't lit: drawn with raylib's own shader.
template <typename F>
inline void unlit(F draw) {
    if (!lit()) {
        draw();
        return;
    }
    if (!light().active) {
        draw();
        return;
    }
    EndShaderMode();
    draw();
    BeginShaderMode(light().batch);
}

// Everything drawn after pushTransform (until the matching popTransform) is turned by `degrees` around `axis`, made
// `scale` times bigger, then moved by `position`. Pairs can go inside each other: the inner one is relative to the outer.
inline void pushTransform_(core::vec3_ position, core::vec3_ axis, float degrees, float scale) {
    enter3D("graphics3d.pushTransform()");
    rlPushMatrix();
    st().transforms++;
    rlTranslatef(position.x_, position.y_, position.z_);
    float len = std::sqrt(axis.x_ * axis.x_ + axis.y_ * axis.y_ + axis.z_ * axis.z_);
    if (degrees != 0 && len > 0) rlRotatef(degrees, axis.x_ / len, axis.y_ / len, axis.z_ / len);
    if (scale != 1) rlScalef(scale, scale, scale);
}
inline void popTransform_() {
    if (st().transforms <= 0)
        panic("graphics3d.popTransform()", "there's no pushTransform() to undo - each popTransform() goes with one pushTransform() before it");
    rlPopMatrix();
    st().transforms--;
}

// OpenGL 1.1, in Windows' opengl32.dll (which raylib already uses).
extern "C" __declspec(dllimport) void __stdcall glClear(unsigned int mask);

// Forgets how far away everything drawn so far is, so the 3D drawn next shows in front of all of it: a first-person
// gun that never pokes into walls, or a 3D model on top of the world. The pictures already drawn stay.
inline void clearDepth_() {
    enter3D("graphics3d.clearDepth()");
    leave3D();         // draws everything waiting first
    glClear(0x0100);   // GL_DEPTH_BUFFER_BIT
}

// ---------------------------------------------------------------- culling
// Every shape, mesh and model has a box around it. Before it's drawn, the box's 8 corners are moved the way the
// drawing will be (pushTransform, the camera, the lens): if all 8 are past the same edge of the view (left, right,
// top, bottom, too near or too far), none of it can show, so it isn't drawn at all. Anything partly in view is drawn.

// raylib's MatrixMultiply (raymath): `a` first, then `b`.
inline ::Matrix mul(const ::Matrix& a, const ::Matrix& b) {
    ::Matrix r;
    r.m0 = a.m0 * b.m0 + a.m1 * b.m4 + a.m2 * b.m8 + a.m3 * b.m12;
    r.m1 = a.m0 * b.m1 + a.m1 * b.m5 + a.m2 * b.m9 + a.m3 * b.m13;
    r.m2 = a.m0 * b.m2 + a.m1 * b.m6 + a.m2 * b.m10 + a.m3 * b.m14;
    r.m3 = a.m0 * b.m3 + a.m1 * b.m7 + a.m2 * b.m11 + a.m3 * b.m15;
    r.m4 = a.m4 * b.m0 + a.m5 * b.m4 + a.m6 * b.m8 + a.m7 * b.m12;
    r.m5 = a.m4 * b.m1 + a.m5 * b.m5 + a.m6 * b.m9 + a.m7 * b.m13;
    r.m6 = a.m4 * b.m2 + a.m5 * b.m6 + a.m6 * b.m10 + a.m7 * b.m14;
    r.m7 = a.m4 * b.m3 + a.m5 * b.m7 + a.m6 * b.m11 + a.m7 * b.m15;
    r.m8 = a.m8 * b.m0 + a.m9 * b.m4 + a.m10 * b.m8 + a.m11 * b.m12;
    r.m9 = a.m8 * b.m1 + a.m9 * b.m5 + a.m10 * b.m9 + a.m11 * b.m13;
    r.m10 = a.m8 * b.m2 + a.m9 * b.m6 + a.m10 * b.m10 + a.m11 * b.m14;
    r.m11 = a.m8 * b.m3 + a.m9 * b.m7 + a.m10 * b.m11 + a.m11 * b.m15;
    r.m12 = a.m12 * b.m0 + a.m13 * b.m4 + a.m14 * b.m8 + a.m15 * b.m12;
    r.m13 = a.m12 * b.m1 + a.m13 * b.m5 + a.m14 * b.m9 + a.m15 * b.m13;
    r.m14 = a.m12 * b.m2 + a.m13 * b.m6 + a.m14 * b.m10 + a.m15 * b.m14;
    r.m15 = a.m12 * b.m3 + a.m13 * b.m7 + a.m14 * b.m11 + a.m15 * b.m15;
    return r;
}

// Whether any of the box from `lo` to `hi` (before pushTransform) can be seen. Call it after enter3D(). A box that
// can't be seen is counted (culledLastFrame).
inline bool visible(::Vector3 lo, ::Vector3 hi) {
    long frame = window::st().frame;
    if (st().cullFrame != frame) {
        st().culledLast = st().cullFrame == frame - 1 ? st().culledNow : 0;
        st().culledNow = 0;
        st().cullFrame = frame;
    }
    if (!st().culling) return true;
    ::Matrix m = mul(mul(rlGetMatrixTransform(), rlGetMatrixModelview()), rlGetMatrixProjection());
    int out[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 8; i++) {
        float x = (i & 1) ? hi.x : lo.x, y = (i & 2) ? hi.y : lo.y, z = (i & 4) ? hi.z : lo.z;
        float cx = m.m0 * x + m.m4 * y + m.m8 * z + m.m12;
        float cy = m.m1 * x + m.m5 * y + m.m9 * z + m.m13;
        float cz = m.m2 * x + m.m6 * y + m.m10 * z + m.m14;
        float cw = m.m3 * x + m.m7 * y + m.m11 * z + m.m15;
        out[0] += cx < -cw;
        out[1] += cx > cw;
        out[2] += cy < -cw;
        out[3] += cy > cw;
        out[4] += cz < -cw;
        out[5] += cz > cw;
    }
    for (int k = 0; k < 6; k++)
        if (out[k] == 8) {
            st().culledNow++;
            return false;
        }
    return true;
}
inline bool visibleAround(core::vec3_ center, float hx, float hy, float hz) {
    return visible(::Vector3{center.x_ - hx, center.y_ - hy, center.z_ - hz}, ::Vector3{center.x_ + hx, center.y_ + hy, center.z_ + hz});
}

inline void setCulling_(bool on) { st().culling = on; }
inline bool isCulling_() { return st().culling; }
inline int32_t culledLastFrame_() {
    long frame = window::st().frame;
    if (st().cullFrame == frame) return st().culledLast;
    return st().cullFrame == frame - 1 ? st().culledNow : 0;   // nothing drawn yet this frame
}

inline void setCamera_(Camera_ c) {
    leave3D();  // the next 3D drawing starts again with the new camera
    st().camera = c;
}
inline Camera_ camera_() { return st().camera; }

inline void drawCube_(core::vec3_ p, core::vec3_ size, core::color_ c) {
    enter3D("graphics3d.drawCube()");
    if (visibleAround(p, size.x_ / 2, size.y_ / 2, size.z_ / 2)) DrawCubeV(rl3(p), rl3(size), rl(c));
}
inline void drawCubeOutline_(core::vec3_ p, core::vec3_ size, core::color_ c) {
    enter3D("graphics3d.drawCubeOutline()");
    if (visibleAround(p, size.x_ / 2, size.y_ / 2, size.z_ / 2)) unlit([&] { DrawCubeWiresV(rl3(p), rl3(size), rl(c)); });
}
inline void drawSphere_(core::vec3_ p, float radius, core::color_ c) {
    enter3D("graphics3d.drawSphere()");
    if (!visibleAround(p, radius, radius, radius)) return;
    if (lit())
        drawLitShape(light().sphere, ::Vector3{radius, radius, radius}, rl3(p), c);
    else
        DrawSphereEx(rl3(p), radius, 16, 24, rl(c));
}
inline void drawSphereOutline_(core::vec3_ p, float radius, core::color_ c) {
    enter3D("graphics3d.drawSphereOutline()");
    if (visibleAround(p, radius, radius, radius)) unlit([&] { DrawSphereWires(rl3(p), radius, 12, 16, rl(c)); });
}
inline void drawCylinder_(core::vec3_ p, float radius, float height, core::color_ c) {
    enter3D("graphics3d.drawCylinder()");
    if (visible(::Vector3{p.x_ - radius, p.y_ + std::min(0.0f, height), p.z_ - radius}, ::Vector3{p.x_ + radius, p.y_ + std::max(0.0f, height), p.z_ + radius}))
        if (lit())
            drawLitShape(light().cylinder, ::Vector3{radius, height, radius}, rl3(p), c);
        else
            DrawCylinder(rl3(p), radius, radius, height, 24, rl(c));
}
inline void drawPlane_(core::vec3_ center, core::vec2_ size, core::color_ c) {
    enter3D("graphics3d.drawPlane()");
    if (visibleAround(center, size.x_ / 2, 0, size.y_ / 2)) DrawPlane(rl3(center), ::Vector2{size.x_, size.y_}, rl(c));
}
inline void drawGrid_(int32_t slices, float spacing) {
    enter3D("graphics3d.drawGrid()");
    unlit([&] { DrawGrid(slices, spacing); });
}
inline void drawLine_(core::vec3_ a, core::vec3_ b, core::color_ c) {
    enter3D("graphics3d.drawLine()");
    if (visible(::Vector3{std::min(a.x_, b.x_), std::min(a.y_, b.y_), std::min(a.z_, b.z_)}, ::Vector3{std::max(a.x_, b.x_), std::max(a.y_, b.y_), std::max(a.z_, b.z_)}))
        unlit([&] { DrawLine3D(rl3(a), rl3(b), rl(c)); });
}

// Where a 3D point shows up on the screen (for putting 2D text over 3D things).
inline core::vec2_ toScreen_(core::vec3_ p) {
    ::Vector2 v = GetWorldToScreen(rl3(p), rlCamera());
    return {v.x, v.y};
}

// ---------------------------------------------------------------- models

struct Model_;
inline std::vector<Model_*>& liveModels() {
    static std::vector<Model_*> v;
    return v;
}
inline void unloadModels();

struct Model_ : std::enable_shared_from_this<Model_> {
    std::string path;
    ::Model model{};
    bool loaded = false;
    Model_() = default;
    Model_(const Model_&) = delete;
    virtual ~Model_() {
        auto& live = liveModels();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        if (loaded && window::st().open) UnloadModel(model);
    }
    static std::shared_ptr<Model_> frg$new() { return std::make_shared<Model_>(); }

    bool ready(const char* what) {
        if (path.empty()) return false;
        enter3D(what);
        if (!loaded) {
            SetTraceLogLevel(LOG_ERROR);  // raylib warns about harmless things while loading some .obj files
            model = LoadModel(path.c_str());
            SetTraceLogLevel(LOG_WARNING);
            if (model.meshCount == 0) panic(what, "couldn't read '" + path + "' as a 3D model (use .obj, .gltf, .glb, .iqm, .vox or .m3d)");
            bounds = GetModelBoundingBox(model);
            loaded = true;
            liveModels().push_back(this);
            addHook(window::beforeCloseHooks(), &unloadModels);
        }
        return true;
    }
    // While the light is on, the model's materials use the lit shader (and get their own back after).
    template <typename F>
    void withLight(F draw) {
        if (!lit()) {
            draw();
            return;
        }
        std::vector<::Shader> own;
        for (int i = 0; i < model.materialCount; i++) {
            own.push_back(model.materials[i].shader);
            model.materials[i].shader = light().smooth;
        }
        lightUniforms(light().smooth);
        draw();
        for (int i = 0; i < model.materialCount; i++) model.materials[i].shader = own[i];
    }
    // The box around the model, as loaded (for culling).
    ::BoundingBox bounds{};
    // How far the model reaches from its origin in any direction (a box that fits it however it's turned).
    float reach() {
        float r = 0;
        for (int i = 0; i < 8; i++) {
            float x = (i & 1) ? bounds.max.x : bounds.min.x, y = (i & 2) ? bounds.max.y : bounds.min.y, z = (i & 4) ? bounds.max.z : bounds.min.z;
            r = std::max(r, std::sqrt(x * x + y * y + z * z));
        }
        return r;
    }
    virtual void draw_(core::vec3_ position, float scale, core::color_ tint) {
        if (!ready("model.draw()")) return;
        float s = std::fabs(scale);
        ::Vector3 lo{position.x_ + bounds.min.x * s, position.y_ + bounds.min.y * s, position.z_ + bounds.min.z * s};
        ::Vector3 hi{position.x_ + bounds.max.x * s, position.y_ + bounds.max.y * s, position.z_ + bounds.max.z * s};
        if (scale < 0) {
            float r = reach() * s;
            lo = ::Vector3{position.x_ - r, position.y_ - r, position.z_ - r};
            hi = ::Vector3{position.x_ + r, position.y_ + r, position.z_ + r};
        }
        if (visible(lo, hi)) withLight([&] { DrawModel(model, rl3(position), scale, rl(tint)); });
    }
    virtual void drawRotated_(core::vec3_ position, core::vec3_ axis, float degrees, float scale, core::color_ tint) {
        if (!ready("model.drawRotated()")) return;
        float r = reach() * std::fabs(scale);
        if (visibleAround(position, r, r, r)) withLight([&] { DrawModelEx(model, rl3(position), rl3(axis), degrees, ::Vector3{scale, scale, scale}, rl(tint)); });
    }
};

inline void unloadModels() {
    for (Model_* m : liveModels()) {
        UnloadModel(m->model);
        m->loaded = false;
    }
    liveModels().clear();
}

inline std::shared_ptr<Model_> loadModel_(const std::string& path) {
    auto m = std::make_shared<Model_>();
    m->path = requireGameFile(path, "graphics3d.loadModel()");
    return m;
}

inline std::string frg$str(const std::shared_ptr<Model_>& m) { return m ? "Model(" + m->path + ")" : "null"; }

// ---------------------------------------------------------------- meshes the game builds itself
//
// graphics3d.makeMesh(positions, uvs, shades, texture) uploads triangles to the graphics card once, so drawing them
// is a single call however many there are (a voxel world builds one per chunk). Every pixel of the texture whose
// alpha is under 0.5 is cut out (not drawn at all), so leaves and glass can have holes and still sort correctly.

struct Mesh_;
inline std::vector<Mesh_*>& liveMeshes() {
    static std::vector<Mesh_*> v;
    return v;
}

// The cut-out shader: raylib's normal one, plus "discard" for see-through pixels.
inline ::Shader& cutoutShader() {
    static ::Shader shader{};
    static bool loaded = false;
    if (!loaded) {
        static const char* vs = R"GLSL(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
uniform mat4 mvp;
out vec2 fragTexCoord;
out vec4 fragColor;
void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";
        static const char* fs = R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
void main() {
    vec4 c = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    if (c.a < 0.5) discard;
    finalColor = c;
}
)GLSL";
        shader = LoadShaderFromMemory(vs, fs);
        loaded = true;
    }
    return shader;
}

struct Mesh_ : std::enable_shared_from_this<Mesh_> {
    ::Mesh mesh{};
    ::Material material{};
    std::shared_ptr<assets::Texture_> texture;
    int32_t vertices = 0;
    bool uploaded = false;
    ::Vector3 lo{0, 0, 0}, hi{0, 0, 0};   // the box around its corners (for culling)

    Mesh_() = default;
    Mesh_(const Mesh_&) = delete;
    virtual ~Mesh_() {
        auto& live = liveMeshes();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        release();
    }
    static std::shared_ptr<Mesh_> frg$new() { return std::make_shared<Mesh_>(); }

    void release() {
        if (uploaded && window::st().open) UnloadMesh(mesh);  // the texture belongs to the Texture, the shader is shared
        if (material.maps) RL_FREE(material.maps);
        material.maps = nullptr;
        uploaded = false;
    }
    virtual void draw_(core::vec3_ position, core::color_ tint) {
        enter3D("mesh.draw()");
        if (!uploaded) return;
        material.maps[MATERIAL_MAP_DIFFUSE].color = rl(tint);
        ::Matrix at{};  // moved to `position`, nothing else
        at.m0 = at.m5 = at.m10 = at.m15 = 1;
        at.m12 = position.x_;
        at.m13 = position.y_;
        at.m14 = position.z_;
        if (!visible(::Vector3{lo.x + position.x_, lo.y + position.y_, lo.z + position.z_}, ::Vector3{hi.x + position.x_, hi.y + position.y_, hi.z + position.z_}))
            return;
        if (lit()) {
            material.shader = light().cutout;
            lightUniforms(light().cutout);
        } else {
            material.shader = cutoutShader();
        }
        DrawMesh(mesh, material, at);
    }
    virtual int32_t vertexCount_() { return vertices; }
};

inline void unloadMeshes() {
    for (Mesh_* m : liveMeshes()) m->release();
    liveMeshes().clear();
    if (IsShaderValid(cutoutShader())) UnloadShader(cutoutShader());
}

// positions: x, y, z for each corner, three corners per triangle (front side counter-clockwise, as seen from outside).
// uvs: u, v for each corner, the place in the texture from 0 to 1 (v = 0 is the top of the image).
// shades: how bright each corner is, from 0 (black) to 1 (the texture's own colors), or empty for 1 everywhere.
inline std::shared_ptr<Mesh_> makeMesh_(const List<float>& positions, const List<float>& uvs, const List<float>& shades,
                                        const std::shared_ptr<assets::Texture_>& texture) {
    const char* what = "graphics3d.makeMesh()";
    if (!window::st().open) panic(what, "the window isn't open yet - meshes are made on the graphics card, so open it first");
    if (!texture) panic(what, "the texture is null");
    if (positions.size() % 9 != 0)
        panic(what, "positions needs 3 numbers (x, y, z) for each corner and 3 corners for each triangle, so a multiple of 9 numbers, but it has " +
                        str((int64_t)positions.size()));
    size_t n = positions.size() / 3;
    if (uvs.size() != n * 2) panic(what, "uvs needs 2 numbers (u, v) for each of the " + str((int64_t)n) + " corners, but it has " + str((int64_t)uvs.size()));
    if (!shades.empty() && shades.size() != n)
        panic(what, "shades needs 1 number for each of the " + str((int64_t)n) + " corners (or none), but it has " + str((int64_t)shades.size()));
    auto m = std::make_shared<Mesh_>();
    m->texture = texture;
    m->vertices = (int32_t)n;
    if (n == 0) return m;  // nothing to draw
    texture->ready(what);
    ::Mesh& mesh = m->mesh;
    mesh.vertexCount = (int)n;
    mesh.triangleCount = (int)(n / 3);
    mesh.vertices = (float*)RL_MALLOC(n * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(n * 2 * sizeof(float));
    mesh.colors = (unsigned char*)RL_MALLOC(n * 4);
    std::memcpy(mesh.vertices, positions.data(), n * 3 * sizeof(float));
    m->lo = m->hi = ::Vector3{positions[0], positions[1], positions[2]};
    for (size_t i = 0; i < n; i++) {
        m->lo = ::Vector3{std::min(m->lo.x, positions[i * 3]), std::min(m->lo.y, positions[i * 3 + 1]), std::min(m->lo.z, positions[i * 3 + 2])};
        m->hi = ::Vector3{std::max(m->hi.x, positions[i * 3]), std::max(m->hi.y, positions[i * 3 + 1]), std::max(m->hi.z, positions[i * 3 + 2])};
    }
    mesh.normals = (float*)RL_MALLOC(n * 3 * sizeof(float));
    for (size_t t = 0; t + 2 < n; t += 3) {
        const float* a = &positions[t * 3];
        float ux = a[3] - a[0], uy = a[4] - a[1], uz = a[5] - a[2];
        float vx = a[6] - a[0], vy = a[7] - a[1], vz = a[8] - a[2];
        float nx = uy * vz - uz * vy, ny = uz * vx - ux * vz, nz = ux * vy - uy * vx;
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0) {
            nx /= len;
            ny /= len;
            nz /= len;
        }
        for (int k = 0; k < 3; k++) {
            mesh.normals[(t + k) * 3] = nx;
            mesh.normals[(t + k) * 3 + 1] = ny;
            mesh.normals[(t + k) * 3 + 2] = nz;
        }
    }
    std::memcpy(mesh.texcoords, uvs.data(), n * 2 * sizeof(float));
    for (size_t i = 0; i < n; i++) {
        float s = shades.empty() ? 1.0f : std::clamp(shades[i], 0.0f, 1.0f);
        unsigned char b = (unsigned char)(s * 255.0f + 0.5f);
        mesh.colors[i * 4] = mesh.colors[i * 4 + 1] = mesh.colors[i * 4 + 2] = b;
        mesh.colors[i * 4 + 3] = 255;
    }
    UploadMesh(&mesh, false);
    m->material = LoadMaterialDefault();
    m->material.shader = cutoutShader();
    m->material.maps[MATERIAL_MAP_DIFFUSE].texture = texture->tex;
    m->uploaded = true;
    liveMeshes().push_back(m.get());
    addHook(window::beforeCloseHooks(), &unloadMeshes);
    return m;
}

inline std::string frg$str(const std::shared_ptr<Mesh_>& m) {
    return m ? "Mesh(" + std::to_string(m->vertices / 3) + " triangles)" : "null";
}

}  // namespace frg::graphics3d
