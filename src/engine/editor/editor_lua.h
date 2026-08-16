#pragma once

#include <string>
#include <utility>

#include <sol/sol.hpp>

#include "engine/core/logging.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
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

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value);
} // namespace hob::editor
