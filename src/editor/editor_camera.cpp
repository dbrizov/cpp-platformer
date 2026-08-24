#include "editor_camera.h"

#include <algorithm>
#include <cmath>

#include "engine/core/systems/renderer/renderer.h"
#include "engine/math/constants.h"

namespace hob::editor {
    namespace {
        constexpr float ZOOM_STEP = 1.1f;
        constexpr int64_t MIN_PIXELS_PER_METER = 2;
        constexpr int64_t MAX_PIXELS_PER_METER = 2048;
        constexpr float FOCUS_FIT_FACTOR = 0.5f;

        uint32_t clamp_pixels_per_meter(int64_t value) {
            return static_cast<uint32_t>(std::clamp(value, MIN_PIXELS_PER_METER, MAX_PIXELS_PER_METER));
        }
    } // namespace

    float EditorCamera::get_pixels_per_meter_f() const {
        return static_cast<float>(pixels_per_meter);
    }

    Matrix4x4 EditorCamera::build_view_projection(const Vector2& target_size) const {
        const float w = target_size.x;
        const float h = target_size.y;
        const float ppm = get_pixels_per_meter_f();

        Matrix4x4 world_to_pixels = Matrix4x4::identity();
        world_to_pixels.m[0] = ppm;
        world_to_pixels.m[5] = -ppm;
        world_to_pixels.m[12] = w * 0.5f - ppm * position.x;
        world_to_pixels.m[13] = h * 0.5f + ppm * position.y;

        return Renderer::ortho_top_left(w, h) * world_to_pixels;
    }

    Vector2 EditorCamera::screen_to_world(const Vector2& screen_pos, const EditorSceneRect& scene_rect) const {
        Vector2 delta_px = screen_pos - scene_rect.top_left - scene_rect.size * 0.5f;
        delta_px.y = -delta_px.y;

        return position + delta_px / get_pixels_per_meter_f();
    }

    Vector2 EditorCamera::world_to_screen(const Vector2& world_pos, const EditorSceneRect& scene_rect) const {
        Vector2 delta_px = (world_pos - position) * get_pixels_per_meter_f();
        delta_px.y = -delta_px.y;

        return scene_rect.top_left + scene_rect.size * 0.5f + delta_px;
    }

    void EditorCamera::pan_by_pixel_delta(const Vector2& pixel_delta) {
        const float ppm = get_pixels_per_meter_f();
        position.x -= pixel_delta.x / ppm;
        position.y += pixel_delta.y / ppm;
    }

    void EditorCamera::zoom_at(const Vector2& screen_pos, const EditorSceneRect& scene_rect, float wheel) {
        const Vector2 world_pos_before = screen_to_world(screen_pos, scene_rect);

        // Rounding alone would stall: at 5 ppm, 5 / 1.1 rounds back to 5 and zooming out
        // could never reach the minimum. Force at least one unit in the wheel's direction.
        const float factor = (wheel > 0.0f) ? ZOOM_STEP : (1.0f / ZOOM_STEP);
        const int64_t current = static_cast<int64_t>(pixels_per_meter);
        const int64_t scaled = std::llround(get_pixels_per_meter_f() * factor);
        const int64_t next = (wheel > 0.0f) ? std::max(scaled, current + 1) : std::min(scaled, current - 1);
        pixels_per_meter = clamp_pixels_per_meter(next);

        // Keep the world point under the cursor.
        const Vector2 world_pos_after = screen_to_world(screen_pos, scene_rect);
        position = position + (world_pos_before - world_pos_after);
    }

    void EditorCamera::focus_on(const AABB& world_bounds, const EditorSceneRect& scene_rect) {
        position = world_bounds.center;

        const Vector2 world_size = world_bounds.size();
        if (world_size.x <= EPSILON || world_size.y <= EPSILON) {
            return; // Degenerate bounds: recenter only, keep the current zoom.
        }

        const float fit_ppm = std::min(scene_rect.size.x / world_size.x, scene_rect.size.y / world_size.y);
        pixels_per_meter = clamp_pixels_per_meter(std::llround(fit_ppm * FOCUS_FIT_FACTOR));
    }
} // namespace hob::editor
