#include "editor_camera.h"

#include <algorithm>

#include "engine/core/systems/renderer/renderer.h"

namespace hob::editor {
    namespace {
        constexpr float ZOOM_STEP = 1.1f;
        constexpr float MIN_PIXELS_PER_METER = 2.0f;
        constexpr float MAX_PIXELS_PER_METER = 2048.0f;
    } // namespace

    Matrix4x4 EditorCamera::build_view_projection(const Vector2& target_size) const {
        const float w = target_size.x;
        const float h = target_size.y;
        const float ppm = pixels_per_meter;

        Matrix4x4 world_to_pixels = Matrix4x4::identity();
        world_to_pixels.m[0] = ppm;
        world_to_pixels.m[5] = -ppm;
        world_to_pixels.m[12] = w * 0.5f - ppm * position.x;
        world_to_pixels.m[13] = h * 0.5f + ppm * position.y;

        return Renderer::ortho_top_left(w, h) * world_to_pixels;
    }

    Vector2 EditorCamera::panel_to_world(const Vector2& panel_pos, const Vector2& panel_size) const {
        Vector2 delta_pixels = panel_pos - panel_size * 0.5f;
        delta_pixels.y = -delta_pixels.y;

        return position + delta_pixels / pixels_per_meter;
    }

    Vector2 EditorCamera::world_to_panel(const Vector2& world_pos, const Vector2& panel_size) const {
        Vector2 delta_pixels = (world_pos - position) * pixels_per_meter;
        delta_pixels.y = -delta_pixels.y;

        return panel_size * 0.5f + delta_pixels;
    }

    void EditorCamera::pan_by_panel_delta(const Vector2& panel_delta) {
        position.x -= panel_delta.x / pixels_per_meter;
        position.y += panel_delta.y / pixels_per_meter;
    }

    void EditorCamera::zoom_at(const Vector2& panel_pos, const Vector2& panel_size, float wheel) {
        const Vector2 world_before = panel_to_world(panel_pos, panel_size);

        const float factor = (wheel > 0.0f) ? ZOOM_STEP : (1.0f / ZOOM_STEP);
        pixels_per_meter = std::clamp(pixels_per_meter * factor, MIN_PIXELS_PER_METER, MAX_PIXELS_PER_METER);

        // Keep the world point under the cursor.
        const Vector2 world_after = panel_to_world(panel_pos, panel_size);
        position = position + (world_before - world_after);
    }
} // namespace hob::editor
