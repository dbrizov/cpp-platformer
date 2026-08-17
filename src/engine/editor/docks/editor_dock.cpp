#include "editor_dock.h"

#include "engine/editor/editor_gui_utils.h"

namespace hob::editor {
    EditorDock::EditorDock(const char* name, EditorActionContext context)
        : m_name(name)
        , m_context(context) {}

    bool EditorDock::begin(ImGuiWindowFlags flags) {
        m_hovered = false;
        m_focused = false;

        const bool visible = begin_panel(m_name, flags);
        if (visible) {
            m_hovered = ImGui::IsWindowHovered();
            m_focused = ImGui::IsWindowFocused();
        }

        return visible;
    }

    void EditorDock::end() {
        end_panel();
    }

    const char* EditorDock::get_name() const {
        return m_name;
    }

    EditorActionContext EditorDock::get_context() const {
        return m_context;
    }

    bool EditorDock::is_hovered() const {
        return m_hovered;
    }

    bool EditorDock::is_focused() const {
        return m_focused;
    }
} // namespace hob::editor
