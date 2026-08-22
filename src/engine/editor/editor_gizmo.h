#pragma once

#include <vector>

#include "engine/editor/editor_camera.h"
#include "engine/entity/entity.h"
#include "engine/math/vector2.h"

struct ImDrawList;

namespace hob::editor {
    class Editor;

    enum class EditorGizmoMode : uint8_t {
        Translate,
        Rotate,
        Scale,
    };

    enum class EditorGizmoHandle : uint8_t {
        None,
        AxisX,
        AxisY,
        Plane,
        Ring,
        Uniform,
    };

    class EditorGizmo {
        struct Frame {
            bool valid = false;
            Vector2 pivot_world;
            Vector2 pivot_screen;
            Vector2 axis_x_screen;
            Vector2 axis_y_screen;
            Vector2 axis_x_world;
            Vector2 axis_y_world;
            float axis_length_world = 0.0f;
        };

        struct DragEntity {
            EntityId entity_id = INVALID_ENTITY_ID;
            Vector2 start_local_position;
            float start_local_rotation = 0.0f;
            Vector2 start_local_scale;
            Vector2 start_world_position;
        };

        EditorGizmoMode m_mode = EditorGizmoMode::Translate;

        EditorGizmoHandle m_hovered_handle = EditorGizmoHandle::None;

        EditorGizmoHandle m_dragged_handle = EditorGizmoHandle::None;
        std::vector<DragEntity> m_drag_entities;
        Vector2 m_drag_pivot_world;
        Vector2 m_drag_axis_x_world;
        Vector2 m_drag_axis_y_world;
        Vector2 m_drag_grab_world;
        float m_drag_axis_length_world = 0.0f;
        float m_drag_previous_angle = 0.0f;
        float m_drag_total_rotation = 0.0f;

    public:
        EditorGizmoMode get_mode() const;
        void set_mode(EditorGizmoMode mode);

        bool is_dragging() const;

        bool update_input(Editor& editor,
                          const Vector2& mouse_screen_position,
                          const EditorCamera& camera,
                          const EditorSceneRect& scene_rect);

        void draw(const Editor& editor,
                  ImDrawList* draw_list,
                  const EditorCamera& camera,
                  const EditorSceneRect& scene_rect) const;

        void clear_hover();
        void reset();

    private:
        Frame build_frame(const Editor& editor, const EditorCamera& camera, const EditorSceneRect& scene_rect) const;

        EditorGizmoHandle pick_handle(const Frame& frame, const Vector2& mouse_screen_position) const;

        void begin_drag(const Editor& editor,
                        const Frame& frame,
                        EditorGizmoHandle handle,
                        const Vector2& mouse_world_position);
        void update_drag(Editor& editor, const Vector2& mouse_world_position);
        void end_drag(Editor& editor);
    };
} // namespace hob::editor
