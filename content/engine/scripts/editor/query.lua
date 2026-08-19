-- Editor query: the read side of the Editor.* contract.

---@class Editor
_G.Editor = _G.Editor or {}

-- ---------------------------------------------------------------------------------------------
-- Editor field types
-- ---------------------------------------------------------------------------------------------

-- Factory registries whose built objects are usertypes worth recognizing by identity.
-- Path registries unwrap to plain path strings, so a field holding one of
-- their resources can only be identified from declared schema metadata.
local RESOURCE_REGISTRY_TYPES = {
    Materials = FieldType.MATERIAL,
    AnimationClips = FieldType.ANIMATION_CLIP,
}

-- Metatable identity -> field type
local usertype_types = nil

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

local function ensure_usertype_types()
    if usertype_types ~= nil then
        return
    end

    usertype_types = {
        [getmetatable(Vector2())] = FieldType.VECTOR2,
        [getmetatable(Color())] = FieldType.COLOR,
    }

    for registry_name, field_type in pairs(RESOURCE_REGISTRY_TYPES) do
        local mt = get_registry_metatable(registry_name)
        if mt ~= nil then
            usertype_types[mt] = field_type
        end
    end
end

local function get_field_type_from_value(value)
    local t = type(value)
    if t == "number" then
        return math.type(value) == "integer" and FieldType.INT or FieldType.FLOAT
    elseif t == "boolean" then
        return FieldType.BOOL
    elseif t == "string" then
        return FieldType.STRING
    elseif t == "userdata" then
        ensure_usertype_types()
        local field_type = usertype_types[getmetatable(value)]
        if field_type ~= nil then
            return field_type
        end
    end

    return FieldType.OTHER
end

-- ---------------------------------------------------------------------------------------------
-- C++ components
-- ---------------------------------------------------------------------------------------------

local function get_field_meta(schema, field)
    local types = schema.types
    return types ~= nil and types[field] or nil
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

local function append_schema_fields(fields, component, schema)
    for _, field in ipairs(get_schema_field_order(schema)) do
        local getter = schema.getters[field]
        local setter = schema.setters[field]
        local field_meta = get_field_meta(schema, field)
        local is_hidden = field_meta ~= nil and field_meta.hidden == true
        if getter ~= nil and setter ~= nil and not is_hidden then
            local ok, value = pcall(function()
                return component[getter](component)
            end)

            if ok then
                local row = {}

                if field_meta ~= nil then
                    for key, meta_value in pairs(field_meta) do
                        row[key] = meta_value
                    end
                end

                row.name = field
                row.value = value
                row.type = (field_meta ~= nil and field_meta.type) or get_field_type_from_value(value)

                fields[#fields + 1] = row
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
        fields[#fields + 1] = { name = name, value = value, type = get_field_type_from_value(value) }
    end

    return fields
end

-- ---------------------------------------------------------------------------------------------
-- Enums
-- ---------------------------------------------------------------------------------------------

-- Serves both `enum` and `bitmask`.
---@param name string
---@return table|nil
function Editor.get_enum_entries(name)
    local source = _G[name]
    if type(source) ~= "table" then
        return nil
    end

    local entries = {}
    for key, value in pairs(source) do
        if type(key) == "string" and math.type(value) == "integer" then
            entries[#entries + 1] = { name = key, value = value }
        end
    end

    -- Sorting by value recovers declaration order for a C++ enum and bit order for a flag table.
    table.sort(entries, function(a, b)
        if a.value ~= b.value then
            return a.value < b.value
        end

        return a.name < b.name
    end)

    return entries
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
