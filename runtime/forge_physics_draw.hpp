// Forge runtime: physics.drawDebug(), the outlines of every physics Body.
//
// <physics> itself never draws (it doesn't use raylib), so this is a separate file. Forge adds it to games that use
// both <physics> and <graphics2d>, and only lets physics.drawDebug() be called from files that include <graphics2d>.
#pragma once

#include "forge_physics.hpp"
#include "forge_window.hpp"

namespace frg::physics {

// Static bodies (floors, walls) are blue, one-way platforms light blue with a thick top, moving bodies green, and
// bodies with solid = false (coins, triggers) purple.
// A moving body that's standing on something gets a yellow bar along its bottom edge.
// It draws through the 2D camera like anything else, so call it between setCamera and resetCamera.
inline void drawDebug_() {
    window::ensureFrame("physics.drawDebug()");
    const ::Color kStatic{90, 160, 255, 255}, kOneWay{120, 230, 255, 255}, kMoving{80, 230, 120, 255}, kTrigger{200, 120, 255, 255},
        kGround{255, 220, 60, 255};
    for (const auto& b : world().bodies) {
        ::Rectangle r{b->position_.x_, b->position_.y_, b->size_.x_, b->size_.y_};
        ::Color c = !b->solid_ ? kTrigger : b->isStatic_ ? (b->oneWay_ ? kOneWay : kStatic) : kMoving;
        if (b->circle_) {
            core::vec2_ m = b->middle();
            DrawCircleLinesV(::Vector2{m.x_, m.y_}, b->radius(), c);
            DrawCircleLinesV(::Vector2{m.x_, m.y_}, b->radius() - 1, c);
            if (!b->isStatic_ && b->onGround_) DrawCircleV(::Vector2{m.x_, m.y_ + b->radius() - 3}, 3, kGround);
            continue;
        }
        if (b->slope_ != 0) {
            float x0 = r.x, y0 = r.y, x1 = r.x + r.width, y1 = r.y + r.height;
            ::Vector2 a = b->slope_ > 0 ? ::Vector2{x0, y1} : ::Vector2{x0, y0};
            ::Vector2 t = b->slope_ > 0 ? ::Vector2{x1, y0} : ::Vector2{x1, y1};
            ::Vector2 k = b->slope_ > 0 ? ::Vector2{x1, y1} : ::Vector2{x0, y1};
            DrawLineEx(a, t, 4, c);   // the slanted top, thick: the side you walk on
            DrawLineEx(t, k, 2, c);
            DrawLineEx(k, a, 2, c);
            continue;
        }
        DrawRectangleLinesEx(r, 2, c);
        if (b->isStatic_ && b->oneWay_) DrawRectangleRec(::Rectangle{r.x, r.y, r.width, 4}, c);  // the side you can land on
        if (!b->isStatic_ && b->onGround_) DrawRectangleRec(::Rectangle{r.x, r.y + r.height - 4, r.width, 4}, kGround);
    }
}

}  // namespace frg::physics
