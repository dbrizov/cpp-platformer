#include "editor_command_composite.h"

#include <utility>

namespace hob::editor {
    EditorCommandComposite::EditorCommandComposite(std::string label,
                                                   std::vector<std::unique_ptr<EditorCommand>> commands)
        : EditorCommand(std::move(label))
        , m_commands(std::move(commands)) {}

    void EditorCommandComposite::undo(Engine& engine) {
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
            (*it)->undo(engine);
        }
    }

    void EditorCommandComposite::redo(Engine& engine) {
        for (const std::unique_ptr<EditorCommand>& command : m_commands) {
            command->redo(engine);
        }
    }
} // namespace hob::editor
