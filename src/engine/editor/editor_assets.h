#pragma once

namespace hob::editor {
    class Editor;

    class EditorAssets {
        bool m_hovered = false;
        bool m_focused = false;

    public:
        static constexpr const char* PANEL_NAME = " Assets ###Assets";

        void draw(Editor& editor);

        bool is_hovered() const;
        bool is_focused() const;
    };
} // namespace hob::editor
