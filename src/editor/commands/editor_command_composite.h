#pragma once

#include <memory>
#include <string>
#include <vector>

#include "editor_command.h"

namespace hob::editor {
    class EditorCommandComposite : public EditorCommand {
        std::vector<std::unique_ptr<EditorCommand>> m_commands;

    public:
        EditorCommandComposite(std::string label, std::vector<std::unique_ptr<EditorCommand>> commands);

        void undo(Editor& editor) override;
        void redo(Editor& editor) override;
    };
} // namespace hob::editor
