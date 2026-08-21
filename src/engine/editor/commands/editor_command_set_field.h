#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor_command.h"
#include "engine/editor/editor_field_target.h"

namespace hob {
    class Engine;
} // namespace hob

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

        void undo(Engine& engine) override;
        void redo(Engine& engine) override;

        static void apply(Engine& engine, const EditorFieldTarget& target, const sol::object& value);
    };
} // namespace hob::editor
