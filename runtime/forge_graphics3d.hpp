// Forge runtime: <graphics3d> - cameras, shapes and models in 3D.
//
// 3D drawing and 2D drawing can be mixed in a frame: every 3D call switches into 3D mode (using the
// camera set with graphics3d.setCamera), and every 2D call switches back, drawing on top.
#pragma once

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

}  // namespace frg::graphics3d
