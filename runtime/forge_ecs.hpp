// Forge runtime: <ECS> - entities made of components.
//
// An Entity is a named game object holding a list of Components. A Component is a class you write
// (class Health : Component { ... }) with start(), update(dt) and draw(). ECS.update(dt) and
// ECS.draw() run every component of every active entity in the world.
#pragma once

#include "forge.hpp"

namespace frg::ECS {

struct Entity_;

struct Component_ : std::enable_shared_from_this<Component_> {
    std::weak_ptr<Entity_> owner;
    bool started = false;
    Component_() = default;
    virtual ~Component_() = default;
    static std::shared_ptr<Component_> frg$new() { return std::make_shared<Component_>(); }
    virtual void start_() {}
    virtual void update_(float) {}
    virtual void draw_() {}
    virtual std::shared_ptr<Entity_> entity_() { return owner.lock(); }
    // Is this component (or a class it inherits from) called `n`? Forge classes add to this.
    virtual bool frg$is(const std::string& n) { return n == "Component"; }
};

struct Entity_ : std::enable_shared_from_this<Entity_> {
    std::string name_;
    bool active_ = true;
    List<std::shared_ptr<Component_>> components_;
    bool destroyed = false;

    Entity_() = default;
    virtual ~Entity_() = default;
    void frg$init(const std::string& name) { name_ = name; }
    static std::shared_ptr<Entity_> frg$new(const std::string& name) {
        auto e = std::make_shared<Entity_>();
        e->frg$init(name);
        return e;
    }
    virtual void add_(std::shared_ptr<Component_> c) {
        if (!c) return;
        c->owner = weak_from_this();
        components_.push_back(c);
    }
    virtual std::shared_ptr<Component_> get_(const std::string& type) {
        for (auto& c : components_)
            if (c->frg$is(type)) return c;
        return nullptr;
    }
    virtual bool has_(const std::string& type) { return get_(type) != nullptr; }
    virtual bool remove_(const std::string& type) {
        for (size_t i = 0; i < components_.size(); i++) {
            if (components_[i]->frg$is(type)) {
                components_.erase(components_.begin() + i);
                return true;
            }
        }
        return false;
    }
    virtual void destroy_() { destroyed = true; }
};

inline std::vector<std::shared_ptr<Entity_>>& world() {
    static std::vector<std::shared_ptr<Entity_>> w;
    return w;
}

inline void add_(std::shared_ptr<Entity_> e) {
    auto& w = world();
    if (e && std::find(w.begin(), w.end(), e) == w.end()) w.push_back(e);
}

inline std::shared_ptr<Entity_> create_(const std::string& name) {
    auto e = Entity_::frg$new(name);
    add_(e);
    return e;
}

inline void update_(float dt) {
    auto entities = world();  // a copy: components may create or destroy entities while updating
    for (auto& e : entities) {
        if (!e->active_ || e->destroyed) continue;
        auto comps = e->components_;
        for (auto& c : comps) {
            if (!c->started) {
                c->started = true;
                c->start_();
            }
            c->update_(dt);
        }
    }
    auto& w = world();
    w.erase(std::remove_if(w.begin(), w.end(), [](auto& e) { return e->destroyed; }), w.end());
}

inline void draw_() {
    auto entities = world();
    for (auto& e : entities) {
        if (!e->active_ || e->destroyed) continue;
        auto comps = e->components_;
        for (auto& c : comps) c->draw_();
    }
}

inline std::shared_ptr<Entity_> find_(const std::string& name) {
    for (auto& e : world())
        if (!e->destroyed && e->name_ == name) return e;
    return nullptr;
}

inline List<std::shared_ptr<Entity_>> all_() {
    List<std::shared_ptr<Entity_>> out;
    for (auto& e : world())
        if (!e->destroyed) out.push_back(e);
    return out;
}

inline List<std::shared_ptr<Entity_>> withComponent_(const std::string& type) {
    List<std::shared_ptr<Entity_>> out;
    for (auto& e : world())
        if (!e->destroyed && e->has_(type)) out.push_back(e);
    return out;
}

inline void clear_() { world().clear(); }
inline int32_t count_() { return (int32_t)all_().size(); }

inline std::string frg$str(const std::shared_ptr<Entity_>& e) {
    return e ? "Entity(" + e->name_ + ", " + str((int64_t)e->components_.size()) + " components)" : "null";
}
inline std::string frg$str(const std::shared_ptr<Component_>& c) { return c ? "Component" : "null"; }

}  // namespace frg::ECS
