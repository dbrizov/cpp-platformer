#include <algorithm>
#include <utility>
#include <vector>

#include <imgui.h>

#include "editor.h"
#include "editor_gui_utils.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"

namespace hob::editor {
    void Editor::draw_hierarchy() {
        if (begin_panel(PANEL_HIERARCHY)) {
            std::vector<Entity*> entities;
            m_engine.get_entity_spawner().get_entities(entities);

            ImGui::Text("Entities: %zu", entities.size());
            ImGui::Separator();

            // Only entities actually drawn land here, so a range select spans what the user can see.
            std::vector<EntityId> visible_order;
            visible_order.reserve(entities.size());

            EntityId clicked_entity_id = INVALID_ENTITY_ID;

            // Draw parentless entities. Child entities are drawn recursively
            for (const Entity* entity : entities) {
                const TransformComponent* transform = entity->get_transform();
                if (transform->get_parent() != nullptr) {
                    continue;
                }

                draw_hierarchy_entity(transform, visible_order, clicked_entity_id);
            }

            if (clicked_entity_id != INVALID_ENTITY_ID) {
                apply_hierarchy_click(clicked_entity_id, visible_order);
            }

            m_scroll_hierarchy_to_primary = false;
        }
        end_panel();
    }

    void Editor::draw_hierarchy_entity(const TransformComponent* transform,
                                       std::vector<EntityId>& visible_order,
                                       EntityId& out_clicked_entity_id) {
        const Entity& entity = transform->get_entity();
        const EntityId entity_id = entity.get_id();
        const std::vector<TransformComponent*>& children = transform->get_children();

        visible_order.push_back(entity_id);

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (children.empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (m_selection.contains(entity_id)) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity_id)),
                                            flags,
                                            "%s  #%lld",
                                            entity.get_display_name().c_str(),
                                            static_cast<long long>(entity_id));

        if (m_scroll_hierarchy_to_primary && entity_id == m_selection.primary()) {
            ImGui::SetScrollHereY(0.5f);
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            out_clicked_entity_id = entity_id;
        }

        if (open) {
            for (const TransformComponent* child : children) {
                draw_hierarchy_entity(child, visible_order, out_clicked_entity_id);
            }
            ImGui::TreePop();
        }
    }

    void Editor::apply_hierarchy_click(EntityId entity_id, const std::vector<EntityId>& visible_order) {
        const ImGuiIO& io = ImGui::GetIO();

        if (io.KeyCtrl) {
            m_selection.toggle(entity_id);
            m_range_selection_anchor = entity_id;
            return;
        }

        const auto anchor_it = std::find(visible_order.begin(), visible_order.end(), m_range_selection_anchor);
        if (io.KeyShift && anchor_it != visible_order.end()) {
            const auto clicked_it = std::find(visible_order.begin(), visible_order.end(), entity_id);

            auto first = anchor_it;
            auto last = clicked_it;
            if (first > last) {
                std::swap(first, last);
            }

            m_selection.clear();
            for (auto it = first; it != last + 1; ++it) {
                m_selection.add(*it);
            }

            // Keep the clicked row primary regardless of which way the range runs.
            m_selection.add(entity_id);
            return;
        }

        m_selection.set(entity_id);
        m_range_selection_anchor = entity_id;
    }
} // namespace hob::editor
