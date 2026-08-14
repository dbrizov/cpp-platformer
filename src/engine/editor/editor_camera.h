#pragma once

#include "engine/math/matrix4x4.h"
#include "engine/math/vector2.h"

namespace hob {
    struct EditorCamera {
        Vector2 position;
        float pixels_per_meter = 64.0f;

        Matrix4x4 build_view_projection(const Vector2& target_size) const;

        Vector2 panel_to_world(const Vector2& panel_pos, const Vector2& panel_size) const;
        Vector2 world_to_panel(const Vector2& world_pos, const Vector2& panel_size) const;

        void pan_by_panel_delta(const Vector2& panel_delta);
        void zoom_at(const Vector2& panel_pos, const Vector2& panel_size, float wheel);
    };
} // namespace hob
