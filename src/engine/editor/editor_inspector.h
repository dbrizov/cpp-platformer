#pragma once

#include <memory>

namespace hob::editor {
    class Editor;

    // Holds a sol::object, so it is defined in the .cpp to keep <sol/sol.hpp> out of this header.
    struct EditorInspectorPendingEdit;

    class EditorInspector {
        std::unique_ptr<EditorInspectorPendingEdit> m_pending;
        bool m_hovered = false;
        bool m_focused = false;

    public:
        static constexpr const char* PANEL_NAME = " Inspector ###Inspector";

        EditorInspector();
        ~EditorInspector();

        void draw(Editor& editor);

        bool is_hovered() const;
        bool is_focused() const;

        void reset_edit_state();
    };
} // namespace hob::editor
