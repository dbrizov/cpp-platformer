#pragma once

#include <string>
#include <utility>
#include <vector>

#include <sol/sol.hpp>

#include "editor_enum_entry.h"
#include "engine/core/logging.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    // Keys on the result tables Editor.get_components() and Editor.get_enum_entries() return.
    // query.lua writes these literally -- renaming one here does not rename it there.
    namespace query_key {
        constexpr const char* NAME = "name";
        constexpr const char* VALUE = "value";
        constexpr const char* TYPE = "type";
        constexpr const char* IS_LUA = "is_lua";
        constexpr const char* INDEX = "index";
        constexpr const char* FIELDS = "fields";
    } // namespace query_key

    sol::protected_function get_editor_func(Engine& engine, const char* name);

    template<typename... Args>
    sol::object editor_call(Engine& engine, const char* name, Args&&... args) {
        const sol::protected_function func = get_editor_func(engine, name);
        if (!func.valid()) {
            return sol::object{};
        }

        sol::protected_function_result result = func(std::forward<Args>(args)...);
        if (!result.valid()) {
            const sol::error error = result;
            log::engine.error("Editor.{}: {}", name, error.what());

            return sol::object{};
        }

        return result;
    }

    std::vector<EditorEnumEntry> get_enum_entries(Engine& engine, const std::string& name);

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value);
} // namespace hob::editor
