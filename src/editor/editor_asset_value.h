#pragma once

#include <string>

#include <sol/sol.hpp>

namespace hob::editor {
    struct EditorAssetValue {
        std::string asset_name;
        sol::object inline_asset; // Set only when the registry cannot name the asset; keeps it alive.
    };
} // namespace hob::editor
