#include "editor_command_composite.h"

#include <utility>

namespace hob::editor {
    EditorCommandComposite::EditorCommandComposite(std::string label,
                                                   std::vector<std::unique_ptr<EditorCommand>> commands)
        : EditorCommand(std::move(label))
        , m_commands(std::move(commands)) {}

    void EditorCommandComposite::undo(Editor& editor) {
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
            (*it)->undo(editor);
        }
    }

    void EditorCommandComposite::redo(Editor& editor) {
        for (const std::unique_ptr<EditorCommand>& command : m_commands) {
            command->redo(editor);
        }
    }
} // namespace hob::editor
