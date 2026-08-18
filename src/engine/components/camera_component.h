#pragma once

#include "component.h"
#include "engine/math/matrix4x4.h"
#include "engine/math/vector2.h"

namespace hob {
    class CameraComponent : public Component {
        uint32_t m_pixels_per_meter = 64;
        float m_zoom = 1.0f;

    public:
        explicit CameraComponent(Entity& entity);

        void enter_play() override;
        void exit_play() override;

        std::string to_string() const override;

        uint32_t get_pixels_per_meter() const;
        void set_pixels_per_meter(uint32_t value);

        float get_effective_pixels_per_meter() const;

        float get_zoom() const;
        void set_zoom(float multiplier);

        Matrix4x4 build_view_projection() const;

        Vector2 world_to_screen(const Vector2& world_pos) const;
        Vector2 world_to_screen(const Vector2& world_pos, const Vector2& camera_pos) const;

        Vector2 screen_to_world(const Vector2& screen_pos) const;
        Vector2 screen_to_world(const Vector2& screen_pos, const Vector2& camera_pos) const;
    };
} // namespace hob
