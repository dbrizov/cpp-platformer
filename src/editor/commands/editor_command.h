#pragma once

#include <string>

namespace hob::editor {
    class Editor;

    class EditorCommand {
        std::string m_label;

    protected:
        explicit EditorCommand(std::string label);

    public:
        virtual ~EditorCommand() = default;

        const std::string& get_label() const;

        virtual void undo(Editor& editor) = 0;
        virtual void redo(Editor& editor) = 0;
    };
} // namespace hob::editor
