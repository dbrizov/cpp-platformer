#pragma once

#include <string>
#include <string_view>

#include <imgui.h>

#include "engine/editor/actions/editor_action.h"

namespace hob::editor {
    class Editor;

    class EditorDock {
        std::string m_id;
        std::string m_label;
        std::string m_name; // The actual key name used for the ImGui widget
        EditorActionContext m_context;

    protected:
        bool m_hovered = false;
        bool m_focused = false;

        void set_label(std::string_view label);

        bool begin(ImGuiWindowFlags flags = 0);
        void end();

    public:
        EditorDock(std::string_view id, EditorActionContext context);
        virtual ~EditorDock() = default;

        EditorDock(const EditorDock&) = delete;
        EditorDock& operator=(const EditorDock&) = delete;

        EditorDock(EditorDock&&) = delete;
        EditorDock& operator=(EditorDock&&) = delete;

        virtual void draw(Editor& editor) = 0;

        const std::string& get_name() const;
        EditorActionContext get_context() const;

        bool is_hovered() const;
        bool is_focused() const;

    private:
        void update_name();
    };
} // namespace hob::editor
