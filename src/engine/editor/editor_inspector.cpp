#include <memory>

#include <imgui.h>

#include "commands/editor_command_set_transform.h"
#include "editor.h"
#include "editor_gui_utils.h"
#include "editor_style.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/entity/entity.h"
#include "engine/math/constants.h"
#include "engine/math/mathf.h"

namespace hob::editor {
    void Editor::draw_inspector() {
        if (begin_panel(PANEL_INSPECTOR)) {
            // Multi-selection inspects the primary; the rest still move together via the SceneView.
            Entity* entity = m_engine.get_entity_spawner().get_entity(m_selection.primary());
            if (entity == nullptr) {
                ImGui::TextDisabled("Select an entity");
            }
            else {
                ImGui::Text("%s", entity->get_display_name().c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("#%lld", static_cast<long long>(entity->get_id()));

                if (!entity->get_prefab_name().empty()) {
                    ImGui::TextDisabled("Prefab: %s", entity->get_prefab_name().c_str());
                }

                if (m_selection.ids.size() > 1) {
                    ImGui::TextDisabled("(%zu selected)", m_selection.ids.size());
                }

                draw_inspector_transform(*entity);
            }
        }
        end_panel();
    }

    void Editor::draw_inspector_transform(Entity& entity) {
        TransformComponent* transform = entity.get_transform();
        if (transform == nullptr) {
            return;
        }

        ImGui::SeparatorText("Transform");
        ImGui::PushID("transform");

        const TransformState state = TransformState::capture(*transform);

        const auto capture_on_activate = [&] {
            if (ImGui::IsItemActivated()) {
                m_drag_start_transform = state;
            }
        };

        const auto push_on_release = [&](const char* label) {
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_commands.push(
                    m_engine,
                    std::make_unique<EditorCommandSetTransform>(
                        label, entity.get_id(), m_drag_start_transform, TransformState::capture(*transform)));
            }
        };

        const auto apply_live = [&](const TransformState& next) {
            EditorCommandSetTransform::apply(m_engine, entity.get_id(), next);
        };

        Vector2 position = state.position;
        if (field_vector2("Position", position, INSPECTOR_DRAG_SPEED_POSITION)) {
            TransformState next = state;
            next.position = position;
            apply_live(next);
        }
        capture_on_activate();
        push_on_release("Move");

        float rotation_deg = math::normalize_angle(state.rotation * RAD_TO_DEG);
        if (field_float("Rotation", rotation_deg, INSPECTOR_DRAG_SPEED_ROTATION_DEG)) {
            TransformState next = state;
            next.rotation = rotation_deg * DEG_TO_RAD;
            apply_live(next);
        }
        capture_on_activate();
        push_on_release("Rotate");

        Vector2 scale = state.scale;
        if (field_vector2("Scale", scale, INSPECTOR_DRAG_SPEED_SCALE)) {
            TransformState next = state;
            next.scale = scale;
            apply_live(next);
        }
        capture_on_activate();
        push_on_release("Scale");

        ImGui::PopID();
    }
} // namespace hob::editor
