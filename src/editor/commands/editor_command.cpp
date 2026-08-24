#include "editor_command.h"

#include <utility>

namespace hob::editor {
    EditorCommand::EditorCommand(std::string label)
        : m_label(std::move(label)) {}

    const std::string& EditorCommand::get_label() const {
        return m_label;
    }
} // namespace hob::editor
