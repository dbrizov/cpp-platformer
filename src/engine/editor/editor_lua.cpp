#include "editor_lua.h"

#include <unordered_map>

#include "editor_style.h"
#include "engine/animation/animation_clip.h"
#include "engine/core/engine.h"
#include "engine/core/systems/audio/audio_clip.h"
#include "engine/core/systems/renderer/material.h"
#include "engine/core/systems/renderer/texture.h"
#include "engine/core/systems/scripting/lua_schema_keys.h"
#include "engine/core/systems/scripting/lua_script_system.h"

namespace hob::editor {
    namespace {
        std::unordered_map<std::string, std::vector<EditorInspectorEntryAsset>> g_asset_entries;

    } // namespace

    sol::protected_function get_editor_func(Engine& engine, const char* name) {
        const sol::object editor_table = engine.get_lua_script_system().get_lua()["Editor"];
        if (!editor_table.is<sol::table>()) {
            return sol::protected_function{};
        }

        return editor_table.as<sol::table>()[name];
    }

    bool is_resource_set(const sol::object& value) {
        if (!value.valid()) {
            return false;
        }

        if (value.is<Texture>()) {
            return value.as<TextureRef>() != nullptr;
        }

        if (value.is<Material>()) {
            return value.as<MaterialRef>() != nullptr;
        }

        if (value.is<AnimationClip>()) {
            return value.as<AnimationClipRef>() != nullptr;
        }

        if (value.is<AudioClip>()) {
            return value.as<AudioClipRef>() != nullptr;
        }

        return true;
    }

    std::string get_asset_alias(Engine& engine, const std::string& registry, const sol::object& object) {
        if (!is_resource_set(object)) {
            return {};
        }

        const sol::object result = editor_call(engine, "get_asset_alias", registry, object);
        return result.is<std::string>() ? result.as<std::string>() : std::string();
    }

    const char* get_asset_registry_for_field_type(std::string_view type) {
        if (type == field_type::TEXTURE) {
            return "Textures";
        }

        if (type == field_type::MATERIAL) {
            return "Materials";
        }

        if (type == field_type::ANIMATION_CLIP) {
            return "AnimationClips";
        }

        if (type == field_type::AUDIO_CLIP) {
            return "AudioClips";
        }

        return nullptr;
    }

    void clear_asset_entry_cache() {
        g_asset_entries.clear();
    }

    const std::vector<EditorInspectorEntryAsset>& get_asset_entries(Engine& engine, const std::string& registry) {
        const auto cached = g_asset_entries.find(registry);
        if (cached != g_asset_entries.end()) {
            return cached->second;
        }

        const sol::object result = editor_call(engine, "get_asset_entries", registry);
        if (!result.is<sol::table>()) {
            static const std::vector<EditorInspectorEntryAsset> empty;
            return empty;
        }

        const sol::table rows = result.as<sol::table>();

        std::vector<EditorInspectorEntryAsset> entries;
        entries.reserve(rows.size());

        for (int32_t i = 1; i <= static_cast<int32_t>(rows.size()); ++i) {
            const sol::object row = rows[i];
            if (!row.is<sol::table>()) {
                continue;
            }

            const sol::table entry = row.as<sol::table>();
            entries.push_back({.display_name = entry.get_or<std::string>(query_key::DISPLAY_NAME, ""),
                               .registry_alias = entry.get_or<std::string>(query_key::REGISTRY_ALIAS, "")});
        }

        return g_asset_entries.emplace(registry, std::move(entries)).first->second;
    }

    std::vector<EditorInspectorEntryEnum> get_enum_entries(Engine& engine, const std::string& name) {
        std::vector<EditorInspectorEntryEnum> entries;

        const sol::object result = editor_call(engine, "get_enum_entries", name);
        if (!result.is<sol::table>()) {
            return entries;
        }

        const sol::table rows = result.as<sol::table>();
        entries.reserve(rows.size());

        for (int32_t i = 1; i <= static_cast<int32_t>(rows.size()); ++i) {
            const sol::object row = rows[i];
            if (!row.is<sol::table>()) {
                continue;
            }

            const sol::table entry = row.as<sol::table>();
            entries.emplace_back(entry.get_or<std::string>(query_key::NAME, ""),
                                 entry.get_or<int64_t>(query_key::VALUE, 0));
        }

        return entries;
    }

    std::string lua_object_to_display_string(Engine& engine, const sol::object& value) {
        if (!value.valid()) {
            return INSPECTOR_NONE_LABEL;
        }

        if (value.is<Texture>()) {
            const TextureRef& texture = value.as<TextureRef>();
            return texture != nullptr ? texture->get_path() : INSPECTOR_NONE_LABEL;
        }

        if (value.is<AudioClip>()) {
            const AudioClipRef& clip = value.as<AudioClipRef>();
            return clip != nullptr ? clip->get_path() : INSPECTOR_NONE_LABEL;
        }

        if (value.is<Material>()) {
            const MaterialRef& material = value.as<MaterialRef>();
            return material != nullptr ? material->get_name() : INSPECTOR_NONE_LABEL;
        }

        if (value.is<AnimationClip>()) {
            const AnimationClipRef& clip = value.as<AnimationClipRef>();
            if (clip == nullptr) {
                return INSPECTOR_NONE_LABEL;
            }

            return !clip->name.empty() ? clip->name : "(unnamed)";
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
