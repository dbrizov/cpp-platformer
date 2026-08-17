#include "editor_assets.h"

#include <imgui.h>

#include "editor_gui_utils.h"

namespace hob::editor {
    void EditorAssets::draw(Editor&) {
        m_hovered = false;
        m_focused = false;

        if (begin_panel(PANEL_NAME)) {
            m_hovered = ImGui::IsWindowHovered();
            m_focused = ImGui::IsWindowFocused();

            ImGui::TextDisabled("(empty)");
        }
        end_panel();
    }

    bool EditorAssets::is_hovered() const {
        return m_hovered;
    }

    bool EditorAssets::is_focused() const {
        return m_focused;
    }
} // namespace hob::editor
