#pragma once

#include <memory>
#include <string>
#include <vector>

#include "editor_command.h"

namespace hob::editor {
    class Editor;

    class EditorCommandStack {
        std::vector<std::unique_ptr<EditorCommand>> m_commands;
        uint32_t m_top = 0;

    public:
        // Drops the redo tail, then executes the command.
        void push(Editor& editor, std::unique_ptr<EditorCommand> command);

        void undo(Editor& editor);
        void redo(Editor& editor);

        bool can_undo() const;
        bool can_redo() const;

        std::string get_undo_label() const;
        std::string get_redo_label() const;

        void clear();
    };
} // namespace hob::editor
