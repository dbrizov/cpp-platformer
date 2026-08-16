#pragma once

#include <string>

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    class EditorCommand {
        std::string m_label;

    protected:
        explicit EditorCommand(std::string label);

    public:
        virtual ~EditorCommand() = default;

        const std::string& get_label() const;

        virtual void undo(Engine& engine) = 0;
        virtual void redo(Engine& engine) = 0;
    };
} // namespace hob::editor
