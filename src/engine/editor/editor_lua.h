#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sol/sol.hpp>

#include "editor_inspector_entries.h"
#include "engine/core/logging.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    namespace query_key {
        constexpr const char* NAME = "name";
        constexpr const char* VALUE = "value";
        constexpr const char* TYPE = "type";
        constexpr const char* IS_LUA = "is_lua";
        constexpr const char* FIELDS = "fields";
    } // namespace query_key

    namespace editor_func {
        constexpr const char* GET_SCENE_NAMES = "get_scene_names";
        constexpr const char* IS_SCENE_DIRTY = "is_scene_dirty";
        constexpr const char* OPEN_SCENE = "open_scene";
        constexpr const char* LOAD_SCENE = "load_scene";
        constexpr const char* CLEAR_WORLD = "clear_world";
        constexpr const char* GET_ENTITY_ID = "get_entity_id";
        constexpr const char* GET_INSTANCE_ID = "get_instance_id";
        constexpr const char* REBIND_INSTANCE_DEFS = "rebind_instance_defs";
        constexpr const char* SET_INSTANCE_FIELD = "set_instance_field";

        constexpr const char* GET_COMPONENTS = "get_components";
        constexpr const char* GET_ENUM_ENTRIES = "get_enum_entries";
        constexpr const char* GET_ASSET_ENTRIES = "get_asset_entries";
        constexpr const char* GET_ASSET_NAME = "get_asset_name";
        constexpr const char* GET_ASSET_REF = "get_asset_ref";

        constexpr const char* SET_COMPONENT_FIELD = "set_component_field";
        constexpr const char* SET_LUA_COMPONENT_FIELD = "set_lua_component_field";
    } // namespace editor_func

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

    bool is_asset_set(const sol::object& value);

    std::string get_asset_name(Engine& engine, const std::string& factory_name, const sol::object& object);
    const char* get_asset_factory_name_for_field_type(std::string_view type);

    void clear_asset_entry_cache();
    const std::vector<EditorInspectorEntryAsset>& get_asset_entries(Engine& engine, const std::string& factory_name);
    std::vector<EditorInspectorEntryEnum> get_enum_entries(Engine& engine, const std::string& name);

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value);
} // namespace hob::editor
