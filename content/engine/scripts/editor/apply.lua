-- Editor apply: the write side of the Editor.* contract.

---@class Editor
_G.Editor = _G.Editor or {}

local function resolve_entity(entity_id)
    local entity = EntitySpawner.get_entity(entity_id)
    if not entity:is_valid() then
        return nil
    end

    return entity
end

--- Write a field of a C++ component through its schema setter.
---@param entity_id integer
---@param component_key string Schema key, e.g. "sprite", "box_collider"
---@param field string
---@param value any
function Editor.set_live_field(entity_id, component_key, field, value)
    local entity = resolve_entity(entity_id)
    if entity == nil then
        return
    end

    local schema = _G[Schema.COMPONENT_SCHEMAS][component_key]
    if schema == nil or schema[Schema.MAP_SETTER] then
        Log.error("Editor.set_live_field: '" .. tostring(component_key) .. "' has no per-field setters")
        return
    end

    local setter = schema[Schema.SETTERS][field]
    if setter == nil then
        Log.error("Editor.set_live_field: unknown field '" .. tostring(field) .. "' on '" .. component_key .. "'")
        return
    end

    -- The getter, never `add`: editing a field must not bring a component into existence.
    local component = entity[schema[Schema.GET]](entity)
    if component == nil then
        return
    end

    _G.__call_component_setter(component, setter, unwrap_def(value))
end

--- Write a field of a Lua component.
---@param entity_id integer
---@param component_index integer Index into entity:get_lua_components()
---@param field string
---@param value any
function Editor.set_live_lua_field(entity_id, component_index, field, value)
    local entity = resolve_entity(entity_id)
    if entity == nil then
        return
    end

    local instance = entity:get_lua_components()[component_index]
    if instance == nil then
        Log.error("Editor.set_live_lua_field: no lua component at index " .. tostring(component_index))
        return
    end

    if not Editor.is_public_lua_field(field, instance[field]) then
        Log.error("Editor.set_live_lua_field: '" .. tostring(field) .. "' is not an editable field")
        return
    end

    instance[field] = unwrap_def(value)
end
