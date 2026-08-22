#include "editor_gizmo.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include <imgui.h>
#include <sol/sol.hpp>

#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/core/systems/scripting/lua_schema_keys.h"
#include "engine/core/systems/scripting/lua_script_system.h"
#include "engine/editor/commands/editor_command_composite.h"
#include "engine/editor/commands/editor_command_set_field.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_field_target.h"
#include "engine/editor/editor_gui_utils.h"
#include "engine/editor/editor_style.h"
#include "engine/math/constants.h"
#include "engine/math/mathf.h"
#include "engine/math/matrix2x3.h"

namespace hob::editor {
    namespace {
        constexpr const char* LABEL_TRANSLATE = "Move";
        constexpr const char* LABEL_ROTATE = "Rotate";
        constexpr const char* LABEL_SCALE = "Scale";

        constexpr float MIN_ROTATE_RADIUS = 0.001f;

        constexpr float MIN_SCALE_REFERENCE = 0.001f;

        enum class GizmoOperation : uint8_t {
            Translate,
            Rotate,
            Scale,
        };

        GizmoOperation get_operation(EditorGizmoMode mode, EditorGizmoHandle handle) {
            if (handle == EditorGizmoHandle::Ring) {
                return GizmoOperation::Rotate;
            }

            if (mode == EditorGizmoMode::Scale) {
                return GizmoOperation::Scale;
            }

            return GizmoOperation::Translate;
        }

        const char* get_operation_label(GizmoOperation operation) {
            switch (operation) {
                case GizmoOperation::Rotate:
                    return LABEL_ROTATE;
                case GizmoOperation::Scale:
                    return LABEL_SCALE;
                default:
                    return LABEL_TRANSLATE;
            }
        }

        bool is_near_segment(
            const Vector2& origin, const Vector2& direction, float length, const Vector2& point, float tolerance) {
            const Vector2 delta = point - origin;
            const float along = Vector2::dot(delta, direction);
            if (along < 0.0f || along > length) {
                return false;
            }

            return (delta - direction * along).length() <= tolerance;
        }

        bool is_inside_box(const Vector2& center, float size, const Vector2& point) {
            const float half = size * 0.5f;
            return std::abs(point.x - center.x) <= half && std::abs(point.y - center.y) <= half;
        }

        Vector2 world_to_local_position(const TransformComponent& transform, const Vector2& world_position) {
            const TransformComponent* parent = transform.get_parent();
            if (parent == nullptr) {
                return world_position;
            }

            return parent->get_world_matrix().inverse().transform_point(world_position);
        }

        float get_scale_ratio(float current, float start) {
            if (std::abs(start) <= MIN_SCALE_REFERENCE) {
                return 1.0f;
            }

            return current / start;
        }

        EditorFieldTarget make_target(EntityId entity_id, const char* field) {
            return EditorFieldTarget{
                .entity_id = entity_id,
                .is_lua = false,
                .component_key = transform_key::SECTION,
                .component_index = 0,
                .field = field,
            };
        }

        template<typename T>
        void write_field(Engine& engine, sol::state& lua, EntityId entity_id, const char* field, const T& value) {
            EditorCommandSetField::apply(engine, make_target(entity_id, field), sol::make_object(lua, value));
        }

        bool is_changed(float a, float b) {
            return std::abs(a - b) > EPSILON;
        }

        bool is_changed(const Vector2& a, const Vector2& b) {
            return a != b;
        }

        template<typename T>
        void push_if_changed(std::vector<std::unique_ptr<EditorCommand>>& commands,
                             sol::state& lua,
                             const char* label,
                             EntityId entity_id,
                             const char* field,
                             const T& old_value,
                             const T& new_value) {
            if (!is_changed(old_value, new_value)) {
                return;
            }

            commands.push_back(std::make_unique<EditorCommandSetField>(label,
                                                                       make_target(entity_id, field),
                                                                       sol::make_object(lua, old_value),
                                                                       sol::make_object(lua, new_value)));
        }

        void draw_arrow(ImDrawList* draw_list, const Vector2& origin, const Vector2& direction, ImU32 color) {
            const Vector2 tip = origin + direction * GIZMO_AXIS_LENGTH_PX;
            const Vector2 head_base = tip - direction * GIZMO_ARROW_HEAD_LENGTH_PX;
            const Vector2 side(-direction.y, direction.x);
            const float half_width = GIZMO_ARROW_HEAD_WIDTH_PX * 0.5f;

            draw_list->AddLine(to_imvec(origin), to_imvec(head_base), color, GIZMO_AXIS_THICKNESS);
            draw_list->AddTriangleFilled(
                to_imvec(tip), to_imvec(head_base + side * half_width), to_imvec(head_base - side * half_width), color);
        }

        void draw_box(ImDrawList* draw_list, const Vector2& center, float size, ImU32 color) {
            const float half = size * 0.5f;
            draw_list->AddRectFilled(
                ImVec2(center.x - half, center.y - half), ImVec2(center.x + half, center.y + half), color);
        }

        void draw_axis_with_box(ImDrawList* draw_list, const Vector2& origin, const Vector2& direction, ImU32 color) {
            const Vector2 box_center = origin + direction * (GIZMO_AXIS_LENGTH_PX - GIZMO_SCALE_BOX_PX * 0.5f);

            draw_list->AddLine(to_imvec(origin), to_imvec(box_center), color, GIZMO_AXIS_THICKNESS);
            draw_box(draw_list, box_center, GIZMO_SCALE_BOX_PX, color);
        }
    } // namespace

    EditorGizmoMode EditorGizmo::get_mode() const {
        return m_mode;
    }

    void EditorGizmo::set_mode(EditorGizmoMode mode) {
        if (m_mode == mode) {
            return;
        }

        m_mode = mode;
        m_hovered_handle = EditorGizmoHandle::None;
    }

    EditorGizmoSpace EditorGizmo::get_space() const {
        return m_space;
    }

    void EditorGizmo::set_space(EditorGizmoSpace space) {
        if (m_space == space) {
            return;
        }

        m_space = space;
        m_hovered_handle = EditorGizmoHandle::None;
    }

    void EditorGizmo::toggle_space() {
        set_space(m_space == EditorGizmoSpace::World ? EditorGizmoSpace::Local : EditorGizmoSpace::World);
    }

    bool EditorGizmo::is_dragging() const {
        return m_dragged_handle != EditorGizmoHandle::None;
    }

    bool EditorGizmo::update_input(Editor& editor,
                                   const Vector2& mouse_screen_position,
                                   const EditorCamera& camera,
                                   const EditorSceneRect& scene_rect) {
        const Vector2 mouse_world_position = camera.screen_to_world(mouse_screen_position, scene_rect);

        if (is_dragging()) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                update_drag(editor, mouse_world_position);
            }
            else {
                end_drag(editor);
            }

            return true;
        }

        const Frame frame = build_frame(editor, camera, scene_rect);
        m_hovered_handle = frame.valid ? pick_handle(frame, mouse_screen_position) : EditorGizmoHandle::None;

        if (m_hovered_handle == EditorGizmoHandle::None || !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            return false;
        }

        begin_drag(editor, frame, m_hovered_handle, mouse_world_position);

        return is_dragging();
    }

    void EditorGizmo::draw(const Editor& editor,
                           ImDrawList* draw_list,
                           const EditorCamera& camera,
                           const EditorSceneRect& scene_rect) const {
        Frame frame = build_frame(editor, camera, scene_rect);
        if (!frame.valid) {
            return;
        }

        if (is_dragging()) {
            frame.axis_x_world = m_drag_axis_x_world;
            frame.axis_y_world = m_drag_axis_y_world;
            frame.axis_x_screen =
                (camera.world_to_screen(frame.pivot_world + frame.axis_x_world, scene_rect) - frame.pivot_screen)
                    .normalized();
            frame.axis_y_screen =
                (camera.world_to_screen(frame.pivot_world + frame.axis_y_world, scene_rect) - frame.pivot_screen)
                    .normalized();
        }

        const EditorGizmoHandle active_handle = is_dragging() ? m_dragged_handle : m_hovered_handle;

        switch (m_mode) {
            case EditorGizmoMode::TranslateRotate: {
                draw_rotate_ring(draw_list, frame, active_handle);
                draw_translate_handles(draw_list, frame, active_handle);
                break;
            }
            case EditorGizmoMode::Translate: {
                draw_translate_handles(draw_list, frame, active_handle);
                break;
            }
            case EditorGizmoMode::Rotate: {
                draw_rotate_ring(draw_list, frame, active_handle);
                break;
            }
            case EditorGizmoMode::Scale: {
                draw_scale_handles(draw_list, frame, active_handle);
                break;
            }
        }

        if (m_mode != EditorGizmoMode::Scale) {
            draw_list->AddCircleFilled(
                to_imvec(frame.pivot_screen), GIZMO_ORIGIN_RADIUS_PX, ImGui::GetColorU32(COLOR_GIZMO_ORIGIN));
        }
    }

    void EditorGizmo::clear_hover() {
        m_hovered_handle = EditorGizmoHandle::None;
    }

    void EditorGizmo::reset() {
        m_hovered_handle = EditorGizmoHandle::None;
        m_dragged_handle = EditorGizmoHandle::None;
        m_drag_entities.clear();
        m_drag_total_rotation = 0.0f;
    }

    EditorGizmo::Frame EditorGizmo::build_frame(const Editor& editor,
                                                const EditorCamera& camera,
                                                const EditorSceneRect& scene_rect) const {
        Frame frame;

        const Entity* primary = editor.get_engine().get_entity_spawner().get_entity(editor.get_selection().primary());
        if (primary == nullptr) {
            return frame;
        }

        const Matrix2x3& world_matrix = primary->get_transform()->get_world_matrix();
        frame.pivot_world = world_matrix.origin;

        if (m_space == EditorGizmoSpace::Local || m_mode == EditorGizmoMode::Scale) {
            const float rotation = world_matrix.get_rotation();
            frame.axis_x_world = Vector2(std::cos(rotation), std::sin(rotation));
            frame.axis_y_world = Vector2(-std::sin(rotation), std::cos(rotation));
        }
        else {
            frame.axis_x_world = Vector2::right();
            frame.axis_y_world = Vector2::up();
        }

        frame.pivot_screen = camera.world_to_screen(frame.pivot_world, scene_rect);
        frame.axis_x_screen =
            (camera.world_to_screen(frame.pivot_world + frame.axis_x_world, scene_rect) - frame.pivot_screen)
                .normalized();
        frame.axis_y_screen =
            (camera.world_to_screen(frame.pivot_world + frame.axis_y_world, scene_rect) - frame.pivot_screen)
                .normalized();

        frame.valid = frame.axis_x_screen != Vector2::zero() && frame.axis_y_screen != Vector2::zero();

        return frame;
    }

    Vector2 EditorGizmo::get_composite_center(const Frame& frame) {
        return frame.pivot_screen + (frame.axis_x_screen + frame.axis_y_screen) * GIZMO_COMPOSITE_OFFSET_PX;
    }

    void EditorGizmo::draw_translate_handles(ImDrawList* draw_list,
                                             const Frame& frame,
                                             EditorGizmoHandle active_handle) const {
        const ImU32 highlight = ImGui::GetColorU32(COLOR_GIZMO_HIGHLIGHT);

        draw_box(draw_list,
                 get_composite_center(frame),
                 GIZMO_COMPOSITE_SIZE_PX,
                 (active_handle == EditorGizmoHandle::Plane) ? highlight : ImGui::GetColorU32(COLOR_GIZMO_COMPOSITE));

        draw_arrow(draw_list,
                   frame.pivot_screen,
                   frame.axis_x_screen,
                   (active_handle == EditorGizmoHandle::AxisX) ? highlight : ImGui::GetColorU32(COLOR_GIZMO_AXIS_X));
        draw_arrow(draw_list,
                   frame.pivot_screen,
                   frame.axis_y_screen,
                   (active_handle == EditorGizmoHandle::AxisY) ? highlight : ImGui::GetColorU32(COLOR_GIZMO_AXIS_Y));
    }

    void EditorGizmo::draw_rotate_ring(ImDrawList* draw_list,
                                       const Frame& frame,
                                       EditorGizmoHandle active_handle) const {
        const ImU32 highlight = ImGui::GetColorU32(COLOR_GIZMO_HIGHLIGHT);
        const bool ring_active = active_handle == EditorGizmoHandle::Ring;

        draw_list->AddCircle(to_imvec(frame.pivot_screen),
                             GIZMO_RING_RADIUS_PX,
                             ring_active ? highlight : ImGui::GetColorU32(COLOR_GIZMO_RING),
                             GIZMO_RING_SEGMENTS,
                             GIZMO_RING_THICKNESS);

        if (!is_dragging() || m_dragged_handle != EditorGizmoHandle::Ring) {
            return;
        }

        const float start_angle = m_drag_previous_angle - m_drag_total_rotation;
        const Vector2 start_spoke =
            frame.axis_x_screen * std::cos(start_angle) + frame.axis_y_screen * std::sin(start_angle);
        const Vector2 current_spoke = frame.axis_x_screen * std::cos(m_drag_previous_angle) +
                                      frame.axis_y_screen * std::sin(m_drag_previous_angle);

        draw_list->AddLine(to_imvec(frame.pivot_screen),
                           to_imvec(frame.pivot_screen + start_spoke * GIZMO_RING_RADIUS_PX),
                           ImGui::GetColorU32(COLOR_GIZMO_RING),
                           GIZMO_AXIS_THICKNESS);
        draw_list->AddLine(to_imvec(frame.pivot_screen),
                           to_imvec(frame.pivot_screen + current_spoke * GIZMO_RING_RADIUS_PX),
                           highlight,
                           GIZMO_AXIS_THICKNESS);
    }

    void EditorGizmo::draw_scale_handles(ImDrawList* draw_list,
                                         const Frame& frame,
                                         EditorGizmoHandle active_handle) const {
        const ImU32 highlight = ImGui::GetColorU32(COLOR_GIZMO_HIGHLIGHT);

        draw_axis_with_box(draw_list,
                           frame.pivot_screen,
                           frame.axis_x_screen,
                           (active_handle == EditorGizmoHandle::AxisX) ? highlight
                                                                       : ImGui::GetColorU32(COLOR_GIZMO_AXIS_X));
        draw_axis_with_box(draw_list,
                           frame.pivot_screen,
                           frame.axis_y_screen,
                           (active_handle == EditorGizmoHandle::AxisY) ? highlight
                                                                       : ImGui::GetColorU32(COLOR_GIZMO_AXIS_Y));
        draw_box(draw_list,
                 get_composite_center(frame),
                 GIZMO_COMPOSITE_SIZE_PX,
                 (active_handle == EditorGizmoHandle::Uniform) ? highlight : ImGui::GetColorU32(COLOR_GIZMO_COMPOSITE));
    }

    EditorGizmoHandle EditorGizmo::pick_handle(const Frame& frame, const Vector2& mouse_screen_position) const {
        switch (m_mode) {
            case EditorGizmoMode::TranslateRotate: {
                if (is_inside_box(get_composite_center(frame), GIZMO_COMPOSITE_SIZE_PX, mouse_screen_position)) {
                    return EditorGizmoHandle::Plane;
                }

                const EditorGizmoHandle axis = pick_axis_handle(frame, mouse_screen_position);
                if (axis != EditorGizmoHandle::None) {
                    return axis;
                }

                return is_on_ring(frame, mouse_screen_position) ? EditorGizmoHandle::Ring : EditorGizmoHandle::None;
            }
            case EditorGizmoMode::Translate: {
                if (is_inside_box(get_composite_center(frame), GIZMO_COMPOSITE_SIZE_PX, mouse_screen_position)) {
                    return EditorGizmoHandle::Plane;
                }
                break;
            }
            case EditorGizmoMode::Rotate: {
                return is_on_ring(frame, mouse_screen_position) ? EditorGizmoHandle::Ring : EditorGizmoHandle::None;
            }
            case EditorGizmoMode::Scale: {
                if (is_inside_box(get_composite_center(frame), GIZMO_COMPOSITE_SIZE_PX, mouse_screen_position)) {
                    return EditorGizmoHandle::Uniform;
                }
                break;
            }
        }

        return pick_axis_handle(frame, mouse_screen_position);
    }

    EditorGizmoHandle EditorGizmo::pick_axis_handle(const Frame& frame, const Vector2& mouse_screen_position) {
        if (is_near_segment(frame.pivot_screen,
                            frame.axis_x_screen,
                            GIZMO_AXIS_LENGTH_PX,
                            mouse_screen_position,
                            GIZMO_PICK_TOLERANCE_PX)) {
            return EditorGizmoHandle::AxisX;
        }

        if (is_near_segment(frame.pivot_screen,
                            frame.axis_y_screen,
                            GIZMO_AXIS_LENGTH_PX,
                            mouse_screen_position,
                            GIZMO_PICK_TOLERANCE_PX)) {
            return EditorGizmoHandle::AxisY;
        }

        return EditorGizmoHandle::None;
    }

    bool EditorGizmo::is_on_ring(const Frame& frame, const Vector2& mouse_screen_position) {
        const float distance = (mouse_screen_position - frame.pivot_screen).length();
        return std::abs(distance - GIZMO_RING_RADIUS_PX) <= GIZMO_PICK_TOLERANCE_PX;
    }

    void EditorGizmo::begin_drag(const Editor& editor,
                                 const Frame& frame,
                                 EditorGizmoHandle handle,
                                 const Vector2& mouse_world_position) {
        m_drag_entities.clear();

        const EntitySpawner& spawner = editor.get_engine().get_entity_spawner();
        for (EntityId entity_id : editor.get_selection().ids) {
            const Entity* entity = spawner.get_entity(entity_id);
            if (entity == nullptr) {
                continue;
            }

            const TransformComponent* transform = entity->get_transform();

            DragEntity& drag = m_drag_entities.emplace_back();
            drag.entity_id = entity_id;
            drag.start_local_position = transform->get_local_position();
            drag.start_local_rotation = transform->get_local_rotation();
            drag.start_local_scale = transform->get_local_scale();
            drag.start_world_position = transform->get_position();
        }

        if (m_drag_entities.empty()) {
            return;
        }

        const Vector2 grab_offset = mouse_world_position - frame.pivot_world;

        m_dragged_handle = handle;
        m_drag_pivot_world = frame.pivot_world;
        m_drag_axis_x_world = frame.axis_x_world;
        m_drag_axis_y_world = frame.axis_y_world;
        m_drag_grab_world = mouse_world_position;
        m_drag_previous_angle = std::atan2(grab_offset.y, grab_offset.x);
        m_drag_total_rotation = 0.0f;
    }

    void EditorGizmo::update_drag(Editor& editor, const Vector2& mouse_world_position) {
        Engine& engine = editor.get_engine();
        const EntitySpawner& spawner = engine.get_entity_spawner();
        sol::state& lua = engine.get_lua_script_system().get_lua();

        const GizmoOperation operation = get_operation(m_mode, m_dragged_handle);

        Vector2 translation = Vector2::zero();
        Vector2 scale_factor = Vector2::one();

        switch (operation) {
            case GizmoOperation::Translate: {
                const Vector2 delta = mouse_world_position - m_drag_grab_world;

                if (m_dragged_handle == EditorGizmoHandle::AxisX) {
                    translation = m_drag_axis_x_world * Vector2::dot(delta, m_drag_axis_x_world);
                }
                else if (m_dragged_handle == EditorGizmoHandle::AxisY) {
                    translation = m_drag_axis_y_world * Vector2::dot(delta, m_drag_axis_y_world);
                }
                else {
                    translation = delta;
                }
                break;
            }
            case GizmoOperation::Rotate: {
                const Vector2 offset = mouse_world_position - m_drag_pivot_world;
                if (offset.length() <= MIN_ROTATE_RADIUS) {
                    return;
                }

                const float angle = std::atan2(offset.y, offset.x);
                m_drag_total_rotation += math::wrap_angle_rad(angle - m_drag_previous_angle);
                m_drag_previous_angle = angle;
                break;
            }
            case GizmoOperation::Scale: {
                const Vector2 start_offset = m_drag_grab_world - m_drag_pivot_world;
                const Vector2 current_offset = mouse_world_position - m_drag_pivot_world;

                if (m_dragged_handle == EditorGizmoHandle::Uniform) {
                    const Vector2 diagonal = (m_drag_axis_x_world + m_drag_axis_y_world).normalized();
                    const float uniform =
                        get_scale_ratio(Vector2::dot(current_offset, diagonal), Vector2::dot(start_offset, diagonal));
                    scale_factor = Vector2(uniform, uniform);
                }
                else if (m_dragged_handle == EditorGizmoHandle::AxisX) {
                    scale_factor.x = get_scale_ratio(Vector2::dot(current_offset, m_drag_axis_x_world),
                                                     Vector2::dot(start_offset, m_drag_axis_x_world));
                }
                else if (m_dragged_handle == EditorGizmoHandle::AxisY) {
                    scale_factor.y = get_scale_ratio(Vector2::dot(current_offset, m_drag_axis_y_world),
                                                     Vector2::dot(start_offset, m_drag_axis_y_world));
                }
                break;
            }
        }

        for (const DragEntity& drag : m_drag_entities) {
            const Entity* entity = spawner.get_entity(drag.entity_id);
            if (entity == nullptr) {
                continue;
            }

            const TransformComponent* transform = entity->get_transform();
            const Vector2 pivot_offset = drag.start_world_position - m_drag_pivot_world;

            const bool is_on_pivot = pivot_offset.length_sqr() <= EPSILON * EPSILON;

            switch (operation) {
                case GizmoOperation::Translate: {
                    const Vector2 world_position = drag.start_world_position + translation;
                    write_field(engine,
                                lua,
                                drag.entity_id,
                                transform_key::POSITION,
                                world_to_local_position(*transform, world_position));
                    break;
                }
                case GizmoOperation::Rotate: {
                    write_field(engine,
                                lua,
                                drag.entity_id,
                                transform_key::ROTATION,
                                drag.start_local_rotation + m_drag_total_rotation);

                    if (!is_on_pivot) {
                        const Vector2 world_position = Vector2::rotate_around(
                            drag.start_world_position, m_drag_pivot_world, m_drag_total_rotation);
                        write_field(engine,
                                    lua,
                                    drag.entity_id,
                                    transform_key::POSITION,
                                    world_to_local_position(*transform, world_position));
                    }
                    break;
                }
                case GizmoOperation::Scale: {
                    write_field(
                        engine,
                        lua,
                        drag.entity_id,
                        transform_key::SCALE,
                        Vector2(drag.start_local_scale.x * scale_factor.x, drag.start_local_scale.y * scale_factor.y));

                    if (!is_on_pivot) {
                        const float along_x = Vector2::dot(pivot_offset, m_drag_axis_x_world) * scale_factor.x;
                        const float along_y = Vector2::dot(pivot_offset, m_drag_axis_y_world) * scale_factor.y;
                        const Vector2 world_position =
                            m_drag_pivot_world + m_drag_axis_x_world * along_x + m_drag_axis_y_world * along_y;
                        write_field(engine,
                                    lua,
                                    drag.entity_id,
                                    transform_key::POSITION,
                                    world_to_local_position(*transform, world_position));
                    }
                    break;
                }
            }
        }
    }

    void EditorGizmo::end_drag(Editor& editor) {
        Engine& engine = editor.get_engine();
        const EntitySpawner& spawner = engine.get_entity_spawner();
        sol::state& lua = engine.get_lua_script_system().get_lua();
        const char* label = get_operation_label(get_operation(m_mode, m_dragged_handle));

        std::vector<std::unique_ptr<EditorCommand>> commands;
        for (const DragEntity& drag : m_drag_entities) {
            const Entity* entity = spawner.get_entity(drag.entity_id);
            if (entity == nullptr) {
                continue;
            }

            const TransformComponent* transform = entity->get_transform();

            push_if_changed(commands,
                            lua,
                            label,
                            drag.entity_id,
                            transform_key::POSITION,
                            drag.start_local_position,
                            transform->get_local_position());
            push_if_changed(commands,
                            lua,
                            label,
                            drag.entity_id,
                            transform_key::ROTATION,
                            drag.start_local_rotation,
                            transform->get_local_rotation());
            push_if_changed(commands,
                            lua,
                            label,
                            drag.entity_id,
                            transform_key::SCALE,
                            drag.start_local_scale,
                            transform->get_local_scale());
        }

        m_dragged_handle = EditorGizmoHandle::None;
        m_drag_entities.clear();
        m_drag_total_rotation = 0.0f;

        if (commands.empty()) {
            return;
        }

        std::unique_ptr<EditorCommand> command =
            (commands.size() == 1) ? std::move(commands.front())
                                   : std::make_unique<EditorCommandComposite>(label, std::move(commands));

        editor.get_commands().push(engine, std::move(command));
    }
} // namespace hob::editor
