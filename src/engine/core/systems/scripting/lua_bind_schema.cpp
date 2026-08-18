#include "lua_meta.h"
#include "lua_schema_keys.h"
#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_type_names.h" // IWYU pragma: keep

namespace hob {
    void LuaScriptSystem::bind_schema() {
        sol::state& lua = m_impl->lua;
        LuaMetaRegistry& meta = m_impl->meta;

        bind_table(lua, meta, "Schema")
            .constant("COMPONENT_SCHEMAS", schema_key::COMPONENT_SCHEMAS)
            .constant("ADD", schema_key::ADD)
            .constant("GET", schema_key::GET)
            .constant("GETTERS", schema_key::GETTERS)
            .constant("SETTERS", schema_key::SETTERS)
            .constant("ORDER", schema_key::ORDER)
            .constant("MAP_SETTER", schema_key::MAP_SETTER)
            .constant("TYPES", schema_key::TYPES)
            .constant("REAPPLY_ON_HOT_RELOAD", schema_key::REAPPLY_ON_HOT_RELOAD)
            .constant("TYPE", schema_key::TYPE)
            .constant("ENUM", schema_key::ENUM)
            .constant("MIN", schema_key::MIN)
            .constant("MAX", schema_key::MAX)
            .constant("STEP", schema_key::STEP);

        bind_table(lua, meta, "FieldType")
            .constant("INT", field_type::INT)
            .constant("FLOAT", field_type::FLOAT)
            .constant("BOOL", field_type::BOOL)
            .constant("STRING", field_type::STRING)
            .constant("VECTOR2", field_type::VECTOR2)
            .constant("COLOR", field_type::COLOR)
            .constant("ANGLE", field_type::ANGLE)
            .constant("ENUM", field_type::ENUM)
            .constant("BITMASK", field_type::BITMASK)
            .constant("TEXTURE", field_type::TEXTURE)
            .constant("MATERIAL", field_type::MATERIAL)
            .constant("ANIMATION_CLIP", field_type::ANIMATION_CLIP)
            .constant("AUDIO_CLIP", field_type::AUDIO_CLIP)
            .constant("AABB", field_type::AABB)
            .constant("CAPSULE", field_type::CAPSULE)
            .constant("CIRCLE", field_type::CIRCLE)
            .constant("OTHER", field_type::OTHER);
    }
} // namespace hob
