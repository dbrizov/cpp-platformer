#pragma once

#include <sol/sol.hpp>

#include "lua_meta.h"
#include "lua_schema_asset_factory.h"
#include "lua_schema_component.h"

namespace hob {
    struct LuaScriptSystemImpl {
        sol::state lua;
        LuaMetaRegistry meta;
        LuaComponentSchemaRegistry component_schemas;
        LuaAssetFactorySchemaRegistry asset_factory_schemas;
    };
} // namespace hob
