#pragma once

#include "engine/math/aabb.h"
#include "engine/math/matrix4x4.h"
#include "engine/math/vector2.h"

namespace hob::editor {
    struct EditorSceneRect {
        Vector2 top_left; // In screen space
        Vector2 size;
    };

    struct EditorCamera {
        Vector2 position; // In world space
        uint32_t pixels_per_meter = 64;

        float get_pixels_per_meter_f() const;

        Matrix4x4 build_view_projection(const Vector2& target_size) const;

        Vector2 screen_to_world(const Vector2& screen_pos, const EditorSceneRect& scene_rect) const;
        Vector2 world_to_screen(const Vector2& world_pos, const EditorSceneRect& scene_rect) const;

        void pan_by_pixel_delta(const Vector2& pixel_delta);
        void zoom_at(const Vector2& screen_pos, const EditorSceneRect& scene_rect, float wheel);
        void focus_on(const AABB& world_bounds, const EditorSceneRect& scene_rect);
    };
} // namespace hob::editor
