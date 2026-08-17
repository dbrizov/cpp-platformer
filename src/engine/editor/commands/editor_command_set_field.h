#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor_command.h"
#include "engine/entity/entity.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    struct EditorFieldTarget {
        EntityId entity_id = INVALID_ENTITY_ID;
        bool is_lua = false;
        std::string component_key; // Schema key, e.g. "sprite" (C++ components)
        int component_index = 0; // Index into get_lua_components() (Lua components)
        std::string field;

        bool operator==(const EditorFieldTarget& other) const = default;
    };

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
