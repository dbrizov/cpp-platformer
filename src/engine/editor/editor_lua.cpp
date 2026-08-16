#include "editor_lua.h"

#include "engine/core/engine.h"
#include "engine/core/systems/audio/audio_clip.h"
#include "engine/core/systems/renderer/material.h"
#include "engine/core/systems/renderer/texture.h"
#include "engine/core/systems/scripting/lua_script_system.h"

namespace hob::editor {
    sol::protected_function get_editor_func(Engine& engine, const char* name) {
        const sol::object editor_table = engine.get_lua_script_system().get_lua()["Editor"];
        if (!editor_table.is<sol::table>()) {
            return sol::protected_function{};
        }

        return editor_table.as<sol::table>()[name];
    }

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value) {
        if (!value.valid()) {
            return "none";
        }

        if (value.is<TextureRef>()) {
            const TextureRef texture = value.as<TextureRef>();
            return texture != nullptr ? texture->get_path() : "none";
        }

        if (value.is<AudioClipRef>()) {
            const AudioClipRef clip = value.as<AudioClipRef>();
            return clip != nullptr ? clip->get_path() : "none";
        }

        if (value.is<MaterialRef>()) {
            const MaterialRef material = value.as<MaterialRef>();
            return material != nullptr ? material->get_name() : "none";
        }

        const sol::protected_function to_string = engine.get_lua_script_system().get_lua()["tostring"];
        if (to_string.valid()) {
            const sol::protected_function_result result = to_string(value);
            if (result.valid()) {
                return result.get<std::string>();
            }
        }

        return "?";
    }
} // namespace hob::editor
