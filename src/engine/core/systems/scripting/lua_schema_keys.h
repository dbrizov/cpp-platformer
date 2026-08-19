#pragma once

namespace hob {
    namespace schema_key {
        // Global the component schema file assigns itself to.
        constexpr const char* COMPONENT_SCHEMAS = "__component_schemas";

        // Component schema section.
        constexpr const char* ADD = "add";
        constexpr const char* GET = "get";
        constexpr const char* GETTERS = "getters";
        constexpr const char* SETTERS = "setters";
        constexpr const char* ORDER = "__order";
        constexpr const char* MAP_SETTER = "map_setter";
        constexpr const char* TYPES = "types";
        constexpr const char* REAPPLY_ON_HOT_RELOAD = "reapply_on_hot_reload";

        // Per-field metadata.
        constexpr const char* TYPE = "type";
        constexpr const char* ENUM = "enum";
        constexpr const char* MIN = "min";
        constexpr const char* MAX = "max";
        constexpr const char* HIDDEN = "hidden";
    } // namespace schema_key

    namespace field_type {
        constexpr const char* INT = "int";
        constexpr const char* FLOAT = "float";
        constexpr const char* BOOL = "bool";
        constexpr const char* STRING = "string";
        constexpr const char* VECTOR2 = "vector2";
        constexpr const char* COLOR = "color";
        constexpr const char* ANGLE = "angle"; // Stored in radians
        constexpr const char* ENUM = "enum";
        constexpr const char* BITMASK = "bitmask";
        constexpr const char* TEXTURE = "texture";
        constexpr const char* MATERIAL = "material";
        constexpr const char* ANIMATION_CLIP = "animation_clip";
        constexpr const char* AUDIO_CLIP = "audio_clip";
        constexpr const char* AABB = "aabb";
        constexpr const char* CAPSULE = "capsule";
        constexpr const char* CIRCLE = "circle";

        constexpr const char* OTHER = "other";
    } // namespace field_type
} // namespace hob
