-- Editor query: the read side of the Editor.* contract.

---@class Editor
_G.Editor = _G.Editor or {}

-- ---------------------------------------------------------------------------------------------
-- Editor field kinds
-- ---------------------------------------------------------------------------------------------

-- Factory registries whose built objects are usertypes worth recognizing by identity.
-- Path registries unwrap to plain path strings, so a field holding one of
-- their resources can only be identified from declared schema metadata.
local RESOURCE_REGISTRY_KINDS = {
    Materials = "material",
    AnimationClips = "animation_clip",
}

-- Metatable identity -> field kind
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

local function get_declared_kind(schema, field)
    local types = schema.types
    local declared = types ~= nil and types[field] or nil
    return declared ~= nil and declared.type or nil
end

local function get_schema_field_order(schema)
    if schema.__order ~= nil then
        return schema.__order
    end

    local names = {}
    for field in pairs(schema.getters) do
        names[#names + 1] = field
    end
    table.sort(names)

    return names
end

-- Only fields with both a getter and a setter are inspectable;
-- A read-only property has nothing for the Inspector to write back to.
local function append_schema_fields(fields, component, schema)
    for _, field in ipairs(get_schema_field_order(schema)) do
        local getter = schema.getters[field]
        if getter ~= nil and schema.setters[field] ~= nil then
            local ok, value = pcall(function()
                return component[getter](component)
            end)

            if ok then
                local kind = get_declared_kind(schema, field) or get_usertype_kind_from_value(value)
                fields[#fields + 1] = { name = field, value = value, kind = kind }
            end
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

function Editor.is_public_lua_field(key, value)
    return type(key) == "string"
        and key:sub(1, 1) ~= "_"
        and not HIDDEN_LUA_FIELDS[key]
        and type(value) ~= "function"
end

local is_lua_component_field = Editor.is_public_lua_field

-- A Lua table cannot report the order its keys were assigned in, so recover it from the source:
-- debug.getinfo gives init()'s file and line span, and the fields are whatever it assigns to self.
-- Weak-keyed, so the class tables a hot reload discards take their cached order with them.
local field_order_cache = setmetatable({}, { __mode = "k" })

local function scan_init_field_order(class)
    local init = rawget(class, "init")
    if type(init) ~= "function" then
        return {}
    end

    local info = debug.getinfo(init, "S")
    -- "@path" means a file chunk. Anything else (precompiled, packed, a string chunk) has no
    -- source to read, and the alphabetical fallback takes over.
    if info == nil or info.source:sub(1, 1) ~= "@" then
        return {}
    end

    local file = io.open(info.source:sub(2), "r")
    if file == nil then
        return {}
    end

    local order = {}
    local seen = {}
    local line_no = 0

    for line in file:lines() do
        line_no = line_no + 1
        if line_no > info.linedefined and line_no < info.lastlinedefined then
            local field = line:match("^%s*self%.([%w_]+)%s*=")
            if field ~= nil and not seen[field] then
                seen[field] = true
                order[#order + 1] = field
            end
        end
    end

    file:close()

    return order
end

local function get_declared_field_order(class)
    local cached = field_order_cache[class]
    if cached == nil then
        cached = scan_init_field_order(class)
        field_order_cache[class] = cached
    end

    return cached
end

local function get_lua_component_fields(comp_instance)
    local names = {}
    local present = {}

    local function gather(source)
        for key, value in pairs(source) do
            if not present[key] and is_lua_component_field(key, value) then
                present[key] = true
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

    -- Declared order first; then whatever the scan could not place -- class-table defaults,
    -- fields born outside init(), computed names -- alphabetically behind it.
    local ordered = {}
    local taken = {}

    if class ~= nil then
        for _, name in ipairs(get_declared_field_order(class)) do
            if present[name] and not taken[name] then
                taken[name] = true
                ordered[#ordered + 1] = name
            end
        end
    end

    for _, name in ipairs(names) do
        if not taken[name] then
            ordered[#ordered + 1] = name
        end
    end

    local fields = {}
    for _, name in ipairs(ordered) do
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
                local fields = {}
                append_schema_fields(fields, component, schema)
                out[#out + 1] = {
                    name = key,
                    is_lua = false,
                    fields = fields
                }
            end
        end
    end

    for index, instance in ipairs(entity:get_lua_components()) do
        out[#out + 1] = {
            name = instance.class_name or "?",
            is_lua = true,
            index = index,
            fields = get_lua_component_fields(instance),
        }
    end

    return out
end
