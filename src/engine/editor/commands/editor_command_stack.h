#pragma once

#include <memory>
#include <string>
#include <vector>

#include "editor_command.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    class EditorCommandStack {
        std::vector<std::unique_ptr<EditorCommand>> m_commands;
        uint32_t m_top = 0;

    public:
        // Drops the redo tail, then executes the command.
        void push(Engine& engine, std::unique_ptr<EditorCommand> command);

        void undo(Engine& engine);
        void redo(Engine& engine);

        bool can_undo() const;
        bool can_redo() const;

        std::string get_undo_label() const;
        std::string get_redo_label() const;

        void clear();
    };
} // namespace hob::editor
