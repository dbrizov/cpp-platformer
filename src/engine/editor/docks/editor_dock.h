#pragma once

#include <imgui.h>

#include "engine/editor/actions/editor_action.h"

namespace hob::editor {
    class Editor;

    class EditorDock {
        const char* m_name;
        EditorActionContext m_context;

    protected:
        bool m_hovered = false;
        bool m_focused = false;

        bool begin(ImGuiWindowFlags flags = 0);
        void end();

    public:
        EditorDock(const char* name, EditorActionContext context);
        virtual ~EditorDock() = default;

        EditorDock(const EditorDock&) = delete;
        EditorDock& operator=(const EditorDock&) = delete;

        EditorDock(EditorDock&&) = delete;
        EditorDock& operator=(EditorDock&&) = delete;

        virtual void draw(Editor& editor) = 0;

        const char* get_name() const;
        EditorActionContext get_context() const;

        bool is_hovered() const;
        bool is_focused() const;
    };
} // namespace hob::editor
