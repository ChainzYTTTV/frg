// Forge runtime: physics3d.drawDebug(), the outlines of every 3D physics body.
//
// <physics3d> itself never draws, so this is a separate file. Forge adds it to games that use both <physics3d> and
// <graphics3d>, and only lets physics3d.drawDebug() be called from files that include <graphics3d>.
#pragma once

#include "forge_physics3d.hpp"
#include "forge_graphics3d.hpp"

namespace frg::physics3d {

// Static bodies (floors, walls) are blue, moving bodies green, and bodies with solid = false (triggers) purple.
// A moving body that's standing on something gets a yellow square under it. It draws with the current 3D camera.
inline void drawDebug_() {
    graphics3d::enter3D("physics3d.drawDebug()");
    const ::Color kStatic{90, 160, 255, 255}, kMoving{80, 230, 120, 255}, kTrigger{200, 120, 255, 255}, kGround{255, 220, 60, 255};
    for (const auto& b : world().bodies) {
        ::Color c = !b->solid_ ? kTrigger : b->isStatic_ ? kStatic : kMoving;
        ::Vector3 at{b->position_.x_, b->position_.y_, b->position_.z_};
        if (b->sphere_) {
            DrawSphereWires(at, b->radius(), 8, 12, c);
        } else {
            DrawCubeWiresV(at, ::Vector3{b->size_.x_, b->size_.y_, b->size_.z_}, c);
        }
        if (!b->isStatic_ && b->onGround_) {
            float bottom = b->sphere_ ? b->position_.y_ - b->radius() : b->lo().y_;
            float w = (b->sphere_ ? b->radius() : std::min(b->size_.x_, b->size_.z_) / 2) * 0.8f;
            DrawCubeWiresV(::Vector3{at.x, bottom + 0.01f, at.z}, ::Vector3{w * 2, 0.0f, w * 2}, kGround);
        }
    }
}

}  // namespace frg::physics3d
