#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor_command.h"
#include "engine/editor/editor_field_target.h"

namespace hob::editor {
    class EditorCommandSetField : public EditorCommand {
        EditorFieldTarget m_target;
        sol::object m_old_value;
        sol::object m_new_value;

    public:
        EditorCommandSetField(std::string label,
                              EditorFieldTarget target,
                              sol::object old_value,
                              sol::object new_value);

        void undo(Editor& editor) override;
        void redo(Editor& editor) override;

        static void apply(Editor& editor, const EditorFieldTarget& target, const sol::object& value);
    };
} // namespace hob::editor
