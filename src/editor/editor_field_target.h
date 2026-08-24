#pragma once

#include <string>

#include "engine/entity/entity.h"

namespace hob::editor {
    struct EditorFieldTarget {
        EntityId entity_id = INVALID_ENTITY_ID;
        bool is_lua = false;
        std::string component_key; // Schema key, e.g. "sprite", or the Lua class name when is_lua
        std::string field;

        bool operator==(const EditorFieldTarget& other) const = default;
    };
} // namespace hob::editor
