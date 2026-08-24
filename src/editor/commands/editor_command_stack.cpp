#include "editor_command_stack.h"

#include <utility>

namespace hob::editor {
    void EditorCommandStack::push(Editor& editor, std::unique_ptr<EditorCommand> command) {
        m_commands.resize(m_top);
        m_commands.push_back(std::move(command));
        m_commands.back()->redo(editor);
        m_top = static_cast<uint32_t>(m_commands.size());
    }

    void EditorCommandStack::undo(Editor& editor) {
        if (!can_undo()) {
            return;
        }

        m_top -= 1;
        m_commands[m_top]->undo(editor);
    }

    void EditorCommandStack::redo(Editor& editor) {
        if (!can_redo()) {
            return;
        }

        m_commands[m_top]->redo(editor);
        m_top += 1;
    }

    bool EditorCommandStack::can_undo() const {
        return m_top > 0;
    }

    bool EditorCommandStack::can_redo() const {
        return m_top < static_cast<uint32_t>(m_commands.size());
    }

    std::string EditorCommandStack::get_undo_label() const {
        return can_undo() ? m_commands[m_top - 1]->get_label() : std::string();
    }

    std::string EditorCommandStack::get_redo_label() const {
        return can_redo() ? m_commands[m_top]->get_label() : std::string();
    }

    void EditorCommandStack::clear() {
        m_commands.clear();
        m_top = 0;
    }
} // namespace hob::editor
