-- DefineScene: a level as data.

_G.__scene_registry = _G.__scene_registry or {}
_G.__scene_overrides_by_entity_id = _G.__scene_overrides_by_entity_id or {}

---@class DefineScene
_G.DefineScene = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" or type(def.entities) ~= "table" then
            Log.error("DefineScene." .. tostring(name) .. " must be assigned a table with an 'entities' list")
            return
        end

        _G.__scene_registry[name] = def
    end,
    __index = function(_, name)
        return _G.__scene_registry[name]
    end,
})

-- `Scenes.Foo` evaluates to the scene name string `"Foo"`.
---@class Scenes
_G.Scenes = setmetatable({}, {
    __index = function(_, name) return name end,
})

_G.Scene = _G.Scene or {}

---@param entity Entity
---@param overrides table
function Scene.apply_overrides(entity, overrides)
    local schemas = _G.__component_schemas
    local call_setter = _G.__call_component_setter

    for _, key in ipairs(schemas.__order) do
        local section = overrides[key]
        if section ~= nil then
            local schema = schemas[key]
            local component = entity[schema.get](entity)
            if component == nil then
                Log.error("Scene override: entity has no '" .. key .. "' component")
            elseif schema.map_setter then
                call_setter(component, schema.map_setter, unwrap_def(section))
            else
                for prop, value in pairs(section) do
                    local setter = schema.setters[prop]
                    if setter == nil then
                        Log.error("Scene override: unknown property '" .. tostring(prop) .. "' for '" .. key .. "'")
                    else
                        call_setter(component, setter, unwrap_def(value))
                    end
                end
            end
        end
    end
end

---@param name string
---@return { index: integer, entity: Entity }[]|nil
function Scene.load(name)
    local def = _G.__scene_registry[name]
    if not def then
        Log.error("Scene.load: scene '" .. tostring(name) .. "' is not registered")
        return nil
    end

    local spawned = {}
    for index, inst in ipairs(def.entities) do
        local entity = EntitySpawner.spawn_entity(inst.prefab, inst.position, inst.rotation_deg, inst.scale)

        if entity then
            if inst.overrides then
                Scene.apply_overrides(entity, inst.overrides)
                _G.__scene_overrides_by_entity_id[entity:get_id()] = inst.overrides
            end

            spawned[#spawned + 1] = { index = index, entity = entity }
        end
    end

    return spawned
end

function _G.__reapply_scene_overrides_to_spawned_entities()
    local live = {}

    EntitySpawner.for_each_entity(function(entity)
        local id = entity:get_id()
        local overrides = _G.__scene_overrides_by_entity_id[id]
        if overrides then
            live[id] = overrides
            Scene.apply_overrides(entity, overrides)
        end
    end)

    _G.__scene_overrides_by_entity_id = live
end
