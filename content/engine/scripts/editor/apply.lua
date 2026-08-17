-- Editor apply: the write side of the Editor.* contract.
--
-- Loaded ONLY in editor mode, alongside query.lua (see the notes there). The folder loads
-- alphabetically, so this file runs first -- it may only reach into `Editor` from inside a function.
--
-- These edit LIVE entities. Nothing is written back to the prefab, so the edits are lost on Stop.
--
-- Neither setter returns the old value: the undo snapshot is the value C++ already received from the
-- previous frame's query, which works precisely because the Inspector caches nothing.
--
--   Editor.set_live_field(entity_id, component_key, field, value)
--   Editor.set_live_lua_field(entity_id, component_index, field, value)

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
---@param component_key string Schema key, e.g. "sprite"
---@param field string
---@param value any
function Editor.set_live_field(entity_id, component_key, field, value)
    local entity = resolve_entity(entity_id)
    if entity == nil then
        return
    end

    local schema = _G.__component_schemas[component_key]
    if schema == nil or schema.map_setter then
        Log.error("Editor.set_live_field: '" .. tostring(component_key) .. "' has no per-field setters")
        return
    end

    local setter = schema.setters[field]
    if setter == nil then
        Log.error("Editor.set_live_field: unknown field '" .. tostring(field) .. "' on '" .. component_key .. "'")
        return
    end

    -- The getter, never `add`: editing a field must not bring a component into existence.
    local component = entity[schema.get](entity)
    if component == nil then
        return
    end

    _G.__call_component_setter(component, setter, unwrap_def(value))
end

--- Write a field of a Lua component. A plain assignment, which is also what correctly creates the
--- instance shadow when the field was still resolving to a class-table default.
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
