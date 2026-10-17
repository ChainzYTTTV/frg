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
};
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

inline void leave3D() {
    if (!st().in3D) return;
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
    window::st().leave3D = &leave3D;
    addHook(window::beforeEndFrameHooks(), &leave3D);
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

inline void setCamera_(Camera_ c) {
    leave3D();  // the next 3D drawing starts again with the new camera
    st().camera = c;
}
inline Camera_ camera_() { return st().camera; }

inline void drawCube_(core::vec3_ p, core::vec3_ size, core::color_ c) {
    enter3D("graphics3d.drawCube()");
    DrawCubeV(rl3(p), rl3(size), rl(c));
}
inline void drawCubeOutline_(core::vec3_ p, core::vec3_ size, core::color_ c) {
    enter3D("graphics3d.drawCubeOutline()");
    DrawCubeWiresV(rl3(p), rl3(size), rl(c));
}
inline void drawSphere_(core::vec3_ p, float radius, core::color_ c) {
    enter3D("graphics3d.drawSphere()");
    DrawSphereEx(rl3(p), radius, 16, 24, rl(c));
}
inline void drawSphereOutline_(core::vec3_ p, float radius, core::color_ c) {
    enter3D("graphics3d.drawSphereOutline()");
    DrawSphereWires(rl3(p), radius, 12, 16, rl(c));
}
inline void drawCylinder_(core::vec3_ p, float radius, float height, core::color_ c) {
    enter3D("graphics3d.drawCylinder()");
    DrawCylinder(rl3(p), radius, radius, height, 24, rl(c));
}
inline void drawPlane_(core::vec3_ center, core::vec2_ size, core::color_ c) {
    enter3D("graphics3d.drawPlane()");
    DrawPlane(rl3(center), ::Vector2{size.x_, size.y_}, rl(c));
}
inline void drawGrid_(int32_t slices, float spacing) {
    enter3D("graphics3d.drawGrid()");
    DrawGrid(slices, spacing);
}
inline void drawLine_(core::vec3_ a, core::vec3_ b, core::color_ c) {
    enter3D("graphics3d.drawLine()");
    DrawLine3D(rl3(a), rl3(b), rl(c));
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
            loaded = true;
            liveModels().push_back(this);
            addHook(window::beforeCloseHooks(), &unloadModels);
        }
        return true;
    }
    virtual void draw_(core::vec3_ position, float scale, core::color_ tint) {
        if (ready("model.draw()")) DrawModel(model, rl3(position), scale, rl(tint));
    }
    virtual void drawRotated_(core::vec3_ position, core::vec3_ axis, float degrees, float scale, core::color_ tint) {
        if (ready("model.drawRotated()"))
            DrawModelEx(model, rl3(position), rl3(axis), degrees, ::Vector3{scale, scale, scale}, rl(tint));
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
