-- Editor query: the read side of the Editor.* contract.
--
-- Loaded ONLY in editor mode: `editor/` is excluded from bootstrap's engine scan, and the editor runs this folder itself.
--
-- The contract with C++: Lua never draws UI, C++ never touches the registries. Everything the
-- Inspector shows arrives as plain descriptor tables; `kind` is all C++ needs to pick a widget.
--
--   Editor.get_components(entity_id) ->
--     { { name = "sprite", is_lua = false, fields = { { name, value, kind }, ... } }, ... }

--- The editor's Lua support table. C++ reaches it as `_G.Editor[name]`, looked up fresh on
--- every call so a hot reload rebinds automatically.
---@class Editor
_G.Editor = _G.Editor or {}

-- ---------------------------------------------------------------------------------------------
-- Widget kinds
-- ---------------------------------------------------------------------------------------------

-- Factory registries whose built objects are usertypes worth recognizing by identity.
-- Path registries unwrap to plain path strings, so a field holding one of
-- their resources can only be identified from declared schema metadata.
local RESOURCE_REGISTRY_KINDS = {
    Materials = "material",
    AnimationClips = "animation_clip",
}

-- Metatable identity -> widget kind
local usertype_kinds = nil

local function get_registry_metatable(registry_name)
    local names = _G.__factory_alias_names[registry_name]
    if names == nil or names[1] == nil then
        return nil
    end

    local ok, object = pcall(unwrap_def, _G[registry_name][names[1]])
    if ok and type(object) == "userdata" then
        return getmetatable(object)
    end

    return nil
end

local function ensure_usertype_kinds()
    if usertype_kinds ~= nil then
        return
    end

    usertype_kinds = {
        [getmetatable(Vector2())] = "vector2",
        [getmetatable(Color())] = "color",
    }

    for registry_name, kind in pairs(RESOURCE_REGISTRY_KINDS) do
        local mt = get_registry_metatable(registry_name)
        if mt ~= nil then
            usertype_kinds[mt] = kind
        end
    end
end

local function get_usertype_kind_from_value(value)
    local t = type(value)
    if t == "number" then
        return math.type(value) == "integer" and "int" or "float"
    elseif t == "boolean" then
        return "bool"
    elseif t == "string" then
        return "string"
    elseif t == "userdata" then
        ensure_usertype_kinds()
        local kind = usertype_kinds[getmetatable(value)]
        if kind ~= nil then
            return kind
        end
    end

    return "other"
end

-- ---------------------------------------------------------------------------------------------
-- C++ components
-- ---------------------------------------------------------------------------------------------

-- A transform's position/rotation/scale are spawn arguments, not prefab data, so entity_def.lua
-- deliberately keeps them out of the component schema.
-- Synthesize them here instead: the Inspector then draws the transform exactly like every other component.
-- Rotation travels in degrees; the "angle" kind is what says so.
local function get_transform_fields(transform)
    return {
        { name = "position",     value = transform:get_local_position(),                   kind = "vector2" },
        { name = "rotation",     value = transform:get_local_rotation() * Math.RAD_TO_DEG, kind = "angle" },
        { name = "scale",        value = transform:get_local_scale(),                      kind = "vector2" },
    }
end

-- Only fields with both a getter and a setter are inspectable;
-- A read-only property has nothing for the Inspector to write back to.
local function append_schema_fields(fields, component, schema)
    local names = {}
    for field in pairs(schema.getters) do
        if schema.setters[field] then
            names[#names + 1] = field
        end
    end
    table.sort(names)

    for _, field in ipairs(names) do
        local getter = schema.getters[field]
        local ok, value = pcall(function()
            return component[getter](component)
        end)

        if ok then
            fields[#fields + 1] = { name = field, value = value, kind = get_usertype_kind_from_value(value) }
        end
    end
end

-- ---------------------------------------------------------------------------------------------
-- Lua components
-- ---------------------------------------------------------------------------------------------

local HIDDEN_LUA_FIELDS = {
    entity = true,
    class_name = true,
    priority = true,
    new = true,
}

local function is_lua_component_field(key, value)
    return type(key) == "string"
        and key:sub(1, 2) ~= "__"
        and not HIDDEN_LUA_FIELDS[key]
        and type(value) ~= "function"
end

local function get_lua_component_fields(comp_instance)
    local names = {}
    local seen = {}

    local function gather(source)
        for key, value in pairs(source) do
            if not seen[key] and is_lua_component_field(key, value) then
                seen[key] = true
                names[#names + 1] = key
            end
        end
    end

    gather(comp_instance)

    local class = getmetatable(comp_instance)
    if class ~= nil then
        gather(class)
    end

    table.sort(names)

    local fields = {}
    for _, name in ipairs(names) do
        local value = comp_instance[name]
        fields[#fields + 1] = { name = name, value = value, kind = get_usertype_kind_from_value(value) }
    end

    return fields
end

-- ---------------------------------------------------------------------------------------------
-- Public query
-- ---------------------------------------------------------------------------------------------

---@param entity_id integer
---@return table|nil
function Editor.get_components(entity_id)
    local entity = EntitySpawner.get_entity(entity_id)
    if not entity:is_valid() then
        return nil
    end

    local schemas = _G.__component_schemas
    local out = {}

    for _, key in ipairs(schemas.__order) do
        local schema = schemas[key]
        if not schema.map_setter then
            local component = entity[schema.get](entity)
            if component ~= nil then
                local fields = (key == "transform") and get_transform_fields(component) or {}
                append_schema_fields(fields, component, schema)
                out[#out + 1] = {
                    name = key,
                    is_lua = false,
                    fields = fields
                }
            end
        end
    end

    for _, instance in ipairs(entity:get_lua_components()) do
        out[#out + 1] = {
            name = instance.class_name or "?",
            is_lua = true,
            fields = get_lua_component_fields(instance),
        }
    end

    return out
end
