#pragma once

#include <vector>

#include "engine/entity/entity.h"

namespace hob {
    class TransformComponent;
} // namespace hob

namespace hob::editor {
    class Editor;

    class EditorHierarchy {
        bool m_scroll_to_primary = false;
        bool m_hovered = false;
        bool m_focused = false;

    public:
        static constexpr const char* PANEL_NAME = " Hierarchy ###Hierarchy";

        void draw(Editor& editor);

        bool is_hovered() const;
        bool is_focused() const;

        void scroll_to_primary();

    private:
        void draw_entity(const Editor& editor,
                         const TransformComponent* transform,
                         std::vector<EntityId>& visible_order,
                         EntityId& out_clicked_entity_id);
    };
} // namespace hob::editor
