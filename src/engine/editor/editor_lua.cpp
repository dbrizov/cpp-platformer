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

    std::vector<EditorEnumEntry> get_enum_entries(Engine& engine, const std::string& name) {
        std::vector<EditorEnumEntry> entries;

        const sol::object result = editor_call(engine, "get_enum_entries", name);
        if (!result.is<sol::table>()) {
            return entries;
        }

        const sol::table rows = result.as<sol::table>();
        entries.reserve(rows.size());

        for (int i = 1; i <= static_cast<int>(rows.size()); ++i) {
            const sol::object row = rows[i];
            if (!row.is<sol::table>()) {
                continue;
            }

            const sol::table entry = row.as<sol::table>();
            entries.push_back({entry.get_or<std::string>("name", ""), entry.get_or<int64_t>("value", 0)});
        }

        return entries;
    }

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value) {
        if (!value.valid()) {
            return "none";
        }

        if (value.is<Texture>()) {
            const TextureRef texture = value.as<TextureRef>();
            return texture != nullptr ? texture->get_path() : "none";
        }

        if (value.is<AudioClip>()) {
            const AudioClipRef clip = value.as<AudioClipRef>();
            return clip != nullptr ? clip->get_path() : "none";
        }

        if (value.is<Material>()) {
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
