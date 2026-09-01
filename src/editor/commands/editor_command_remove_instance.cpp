#include "editor_command_remove_instance.h"

#include <utility>

namespace hob::editor {
    EditorCommandRemoveInstance::EditorCommandRemoveInstance(std::string label,
                                                             sol::table instance,
                                                             EditorInstanceId instance_id)
        : EditorCommand(std::move(label))
        , m_instance(std::move(instance))
        , m_instance_id(instance_id) {}

    void EditorCommandRemoveInstance::undo(Editor& editor) {
        add_instance(editor, m_instance, m_index);
    }

    void EditorCommandRemoveInstance::redo(Editor& editor) {
        m_index = remove_instance(editor, m_instance_id);
    }
} // namespace hob::editor
