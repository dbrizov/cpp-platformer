#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_ref.h"
#include "lua_meta.h"
#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_type_names.h" // IWYU pragma: keep

namespace hob {
    void LuaScriptSystem::bind_entity_spawner() {
        sol::state& lua = m_impl->lua;
        LuaMetaRegistry& meta = m_impl->meta;
        EntitySpawner& spawner = m_engine.get_entity_spawner();

        bind_table(lua, meta, "EntitySpawner")
            .func("spawn_entity_c",
                  [&spawner]() {
                      return EntityRef(spawner.spawn_entity().get_id(), spawner);
                  })
            .func("destroy_entity_c",
                  [&spawner](const EntityRef& r) {
                      spawner.destroy_entity(r.get_id());
                  },
                  {"entity"})
            .func("clear_c",
                  [&spawner]() {
                      spawner.clear();
                  })
            .func("get_entity",
                  [&spawner](EntityId id) {
                      return EntityRef(id, spawner);
                  },
                  {"id"})
            .func_sig(
                "for_each_entity",
                [&spawner](const sol::protected_function& func,
                           const sol::optional<sol::protected_function>& until_predicate) {
                    const auto visit = [&func, &spawner](Entity* entity) {
                        const sol::protected_function_result result = func(EntityRef(entity->get_id(), spawner));
                        if (!result.valid()) {
                            const sol::error err = result;
                            log::sol2.error("Lua error in EntitySpawner.for_each_entity: {}", err.what());
                        }
                    };

                    if (!until_predicate) {
                        spawner.for_each_entity(visit);
                        return;
                    }

                    spawner.for_each_entity(visit, [&until_predicate, &spawner](Entity* entity) {
                        const sol::protected_function_result result =
                            (*until_predicate)(EntityRef(entity->get_id(), spawner));
                        if (!result.valid()) {
                            const sol::error err = result;
                            log::sol2.error("Lua error in EntitySpawner.for_each_entity until_predicate: {}",
                                            err.what());
                            return true;
                        }

                        const sol::optional<bool> stop = result;
                        return stop.value_or(false);
                    });
                },
                "(func: fun(entity: Entity), until_predicate: (fun(entity: Entity): boolean)?)");
    }
} // namespace hob
