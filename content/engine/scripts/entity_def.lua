-- DefineEntity: Prefab declaration for entities.

_G.__entity_prefab_registry = _G.__entity_prefab_registry or {}
_G.__entity_prefab_by_id = _G.__entity_prefab_by_id or {} -- entity id -> the prefab

---@class DefineEntity
_G.DefineEntity = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            Log.error("DefineEntity." .. tostring(name) .. " must be assigned a table")
            return
        end

        _G.__entity_prefab_registry[name] = def
    end,
    __index = function(_, name)
        return _G.__entity_prefab_registry[name]
    end,
})

-- `Entities.Foo` evaluates to the prefab name string `"Foo"`.
---@class Entities
_G.Entities = setmetatable({}, {
    __index = function(_, name) return name end,
})

function _G.__call_component_setter(component, setter, value)
    if type(setter) == "string" then
        component[setter](component, value)
    else
        setter(component, value)
    end
end

local call_setter = _G.__call_component_setter

local function should_reapply_field(schema, field)
    local flags = schema.reapply_on_hot_reload
    return flags == nil or flags[field] ~= false
end

local function resolve_ticking(prefab)
    if prefab.ticking == nil then
        return false
    end
    return prefab.ticking
end

local function for_each_section(entity, prefab, accessor, fn)
    local schemas = _G.__component_schemas
    for _, key in ipairs(schemas.__order) do
        local section = prefab[key]
        if section ~= nil then
            local schema = schemas[key]
            local component = entity[schema[accessor]](entity)
            if component ~= nil then
                fn(key, schema, section, component)
            end
        end
    end
end

local function apply_setters(component, section, setters)
    for prop, value in pairs(section) do
        local setter = setters[prop]
        if setter == nil then
            Log.error("Unknown prefab property '" .. tostring(prop) .. "' for component")
        else
            call_setter(component, setter, unwrap_def(value))
        end
    end
end

local function apply_prefab(entity, prefab)
    entity:set_ticking(resolve_ticking(prefab))

    if prefab.name then
        entity:set_name(prefab.name)
    end

    for_each_section(entity, prefab, "add", function(_, schema, section, component)
        if schema.map_setter then
            call_setter(component, schema.map_setter, unwrap_def(section))
        else
            apply_setters(component, section, schema.setters)
        end
    end)

    if prefab.lua_components then
        for _, entry in ipairs(prefab.lua_components) do
            entity:add_lua_component(entry)
        end
    end
end

-- Sentinel so a captured default of `nil` survives in a table, where a raw nil value would just remove the key.
local NIL_DEFAULT = {}

local function resolve_field_value(section, field, defaults)
    local value = section[field]
    if value ~= nil then
        return unwrap_def(value)
    end

    value = defaults[field]
    if value == NIL_DEFAULT then
        return nil
    end
    return value
end

local function reapply_prefab(entity, prefab, get_defaults)
    entity:set_ticking(resolve_ticking(prefab))

    for_each_section(entity, prefab, "get", function(key, schema, section, component)
        if schema.map_setter then
            call_setter(component, schema.map_setter, unwrap_def(section))
        else
            local defaults = get_defaults(key)
            for field, setter in pairs(schema.setters) do
                if should_reapply_field(schema, field) then
                    call_setter(component, setter, resolve_field_value(section, field, defaults))
                end
            end
        end
    end)
end

function _G.__reapply_prefabs_to_spawned_entities()
    local schemas = _G.__component_schemas
    local defaults_cache = {}
    local probes = {}

    local function get_defaults(key)
        local cached = defaults_cache[key]
        if cached then
            return cached
        end

        local schema = schemas[key]
        local probe = EntitySpawner.spawn_entity_c()
        probes[#probes + 1] = probe

        local component = probe[schema.add](probe)
        local defaults = {}
        if component ~= nil then
            for field, getter in pairs(schema.getters) do
                local v = component[getter](component)
                if v == nil then
                    v = NIL_DEFAULT
                end
                defaults[field] = v
            end
        end

        defaults_cache[key] = defaults
        return defaults
    end

    local live = {}
    EntitySpawner.for_each(function(entity)
        local id = entity:get_id()
        local name = _G.__entity_prefab_by_id[id]
        if name then
            live[id] = name
            local prefab = _G.__entity_prefab_registry[name]
            if prefab then
                reapply_prefab(entity, prefab, get_defaults)
            end
        end
    end)
    _G.__entity_prefab_by_id = live

    -- Probes are pending (never entered play), so this drops them from the spawn queue synchronously.
    for _, probe in ipairs(probes) do
        EntitySpawner.destroy_entity_c(probe)
    end
end

local spawn_entity_c = EntitySpawner.spawn_entity_c

---@param prefab_name string
---@param position? Vector2
---@param rotation_deg? number
---@param scale? Vector2
---@return Entity|nil
EntitySpawner.spawn_entity = function(prefab_name, position, rotation_deg, scale)
    local prefab = _G.__entity_prefab_registry[prefab_name]
    if not prefab then
        Log.error("EntitySpawner.spawn_entity: prefab '" .. prefab_name .. "' is not registered")
        return nil
    end

    local entity = spawn_entity_c()
    entity:set_prefab_name(prefab_name)

    apply_prefab(entity, prefab)
    _G.__entity_prefab_by_id[entity:get_id()] = prefab_name

    local transform = entity:get_transform()
    if position ~= nil then
        transform:set_position(position)
    end
    if rotation_deg ~= nil then
        transform:set_rotation(rotation_deg * Math.DEG_TO_RAD)
    end
    if scale ~= nil then
        transform:set_scale(scale)
    end

    return entity
end

local destroy_entity_c = EntitySpawner.destroy_entity_c

---@param entity Entity
EntitySpawner.destroy_entity = function(entity)
    if entity then
        _G.__entity_prefab_by_id[entity:get_id()] = nil
    end
    destroy_entity_c(entity)
end
