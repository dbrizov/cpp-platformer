#include "engine/core/asset.h"
#include "lua_meta.h"
#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_type_names.h" // IWYU pragma: keep

namespace hob {
    void LuaScriptSystem::bind_asset() {
        sol::state& lua = m_impl->lua;
        LuaMetaRegistry& meta = m_impl->meta;

        bind_usertype<Asset>(lua, meta)
            .method("get_name", &Asset::get_name)
            .method("set_name", &Asset::set_name, {"name"});
    }
} // namespace hob
