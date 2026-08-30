-- Editor query: the read side of the Editor.* contract.

---@class Editor
_G.Editor = _G.Editor or {}

-- ---------------------------------------------------------------------------------------------
-- Editor field types
-- ---------------------------------------------------------------------------------------------

local ASSET_FACTORY_TYPES = {
    Textures = FieldType.TEXTURE,
    Materials = FieldType.MATERIAL,
    AnimationClips = FieldType.ANIMATION_CLIP,
    AudioClips = FieldType.AUDIO_CLIP,
}

-- Metatable identity -> field type
local usertype_types = nil

local function get_asset_factory_metatable(factory_name)
    local asset_names = _G.__asset_names[factory_name]
    if asset_names == nil or asset_names[1] == nil then
        return nil
    end

    local ok, object = pcall(unwrap_def, _G[factory_name][asset_names[1]])
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

    for factory_name, field_type in pairs(ASSET_FACTORY_TYPES) do
        local mt = get_asset_factory_metatable(factory_name)
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
            local ok, value = pcall(component[getter], component)

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
-- Assets
-- ---------------------------------------------------------------------------------------------

---@param factory_name string
---@return table|nil
function Editor.get_asset_entries(factory_name)
    local asset_names = _G.__asset_names[factory_name]
    local asset_defs = _G.__asset_defs[factory_name]
    if asset_names == nil or asset_defs == nil then
        return nil
    end

    local entries = {}
    for _, asset_name in ipairs(asset_names) do
        if asset_defs[asset_name] ~= nil then
            entries[#entries + 1] = { name = asset_name }
        end
    end

    return entries
end

---@param factory_name string
---@param object any
---@return string|nil
function Editor.get_asset_name(factory_name, object)
    if type(object) ~= "userdata" or object.get_name == nil then
        return nil
    end

    local asset_defs = _G.__asset_defs[factory_name]
    if asset_defs == nil then
        return nil
    end

    local asset_name = object:get_name()
    if asset_name == nil or asset_defs[asset_name] == nil then
        return nil
    end

    return asset_name
end

---@param factory_name string
---@param asset_name string
---@return any
function Editor.get_asset_ref(factory_name, asset_name)
    local asset_factory_table = _G[factory_name]
    if asset_factory_table == nil then
        return nil
    end

    return asset_factory_table[asset_name]
end

-- ---------------------------------------------------------------------------------------------
-- Definition catalogue
-- ---------------------------------------------------------------------------------------------

local CATALOGUE_REGISTRIES = {
    DefRegistry.SCENES,
    DefRegistry.ENTITIES,
    DefRegistry.TEXTURES,
    DefRegistry.MATERIALS,
    DefRegistry.SHADERS,
    DefRegistry.ANIMATION_CLIPS,
    DefRegistry.AUDIO_CLIPS,
}

local DEFINITION_REGISTRY_TABLE = {
    [DefRegistry.SCENES] = function() return _G.__scene_registry end,
    [DefRegistry.ENTITIES] = function() return _G.__entity_prefab_registry end,
}

local function sorted_string_keys(source)
    local keys = {}
    for key in pairs(source) do
        if type(key) == "string" then
            keys[#keys + 1] = key
        end
    end
    table.sort(keys)

    return keys
end

local function get_definition_names(registry)
    local get_table = DEFINITION_REGISTRY_TABLE[registry]
    local source = get_table ~= nil and get_table() or _G.__asset_defs[registry]

    return source ~= nil and sorted_string_keys(source) or {}
end

local function count_defs_per_file()
    local counts = {}
    for _, by_name in pairs(_G.__def_sources) do
        for _, path in pairs(by_name) do
            counts[path] = (counts[path] or 0) + 1
        end
    end

    return counts
end

---@return table
function Editor.get_definitions()
    local def_count_by_file = count_defs_per_file()
    local rows = {}

    for _, registry in ipairs(CATALOGUE_REGISTRIES) do
        for _, name in ipairs(get_definition_names(registry)) do
            local file = __get_def_source(registry, name)
            rows[#rows + 1] = {
                registry = registry,
                name = name,
                file = file,
                read_only = file == nil or def_count_by_file[file] ~= 1,
            }
        end
    end

    return rows
end

---@param factory_name string
---@param asset_name string
---@return any
function Editor.build_asset(factory_name, asset_name)
    local asset_factory_table = _G[factory_name]
    if asset_factory_table == nil then
        return nil
    end

    return unwrap_def(asset_factory_table[asset_name])
end


-- ---------------------------------------------------------------------------------------------
-- Definition documents
-- ---------------------------------------------------------------------------------------------

local function get_definition_table(registry, name)
    local get_table = DEFINITION_REGISTRY_TABLE[registry]
    if get_table ~= nil then
        return get_table()[name]
    end

    local asset_defs = _G.__asset_defs[registry]
    return asset_defs ~= nil and asset_defs[name] or nil
end

-- A deferred reference (Textures.X, Shaders.X) is a table whose metatable names its registry.
local function is_asset_ref(value)
    local mt = getmetatable(value)
    return mt ~= nil and mt.__registry ~= nil
end

local function describe_item(value)
    if type(value) ~= "table" or is_asset_ref(value) then
        return tostring(value)
    end

    return nil
end

local function describe_table(value)
    local parts = {}
    for _, item in ipairs(value) do
        local text = describe_item(item)
        if text == nil then
            return #value .. " items"
        end

        parts[#parts + 1] = text
    end

    if #parts > 0 then
        return table.concat(parts, ", ")
    end

    return "{" .. table.concat(sorted_string_keys(value), ", ") .. "}"
end

-- A prefab section is a component section, so its declared field metadata is the same one the
-- Inspector uses for a live component -- without it a collision mask reads as a bare integer.
local function get_section_schema(registry, section_key)
    if registry ~= DefRegistry.ENTITIES then
        return nil
    end

    local schemas = _G.__component_schemas
    return schemas ~= nil and schemas[section_key] or nil
end

local function to_definition_field(key, value, schema)
    if type(value) == "table" then
        if is_asset_ref(value) then
            return { name = key, value = value, type = FieldType.OTHER }
        end

        return { name = key, value = describe_table(value), type = FieldType.OTHER }
    end

    local row = {}
    local field_meta = schema ~= nil and schema.types ~= nil and schema.types[key] or nil

    if field_meta ~= nil then
        for meta_key, meta_value in pairs(field_meta) do
            row[meta_key] = meta_value
        end
    end

    row.name = key
    row.value = value
    row.type = (field_meta ~= nil and field_meta.type) or get_field_type_from_value(value)

    return row
end

-- An array is a value; a map is a section, which is what makes a prefab read as its components.
local function is_section(value)
    return type(value) == "table" and not is_asset_ref(value) and #value == 0
end

local function to_definition_fields(source, schema)
    local fields = {}
    for _, key in ipairs(sorted_string_keys(source)) do
        fields[#fields + 1] = to_definition_field(key, source[key], schema)
    end

    return fields
end

---@param registry string
---@param name string
---@return table|nil
function Editor.get_definition_sections(registry, name)
    local def = get_definition_table(registry, name)
    if type(def) ~= "table" then
        return nil
    end

    local sections = {}
    local root_fields = {}

    for _, key in ipairs(sorted_string_keys(def)) do
        local value = def[key]
        if is_section(value) then
            sections[#sections + 1] = {
                name = key,
                is_lua = false,
                fields = to_definition_fields(value, get_section_schema(registry, key)),
            }
        else
            root_fields[#root_fields + 1] = to_definition_field(key, value)
        end
    end

    if #root_fields > 0 then
        table.insert(sections, 1, { name = name, is_lua = false, fields = root_fields })
    end

    return sections
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

    for _, instance in ipairs(entity:get_lua_components()) do
        out[#out + 1] = {
            name = instance.class_name or "?",
            is_lua = true,
            fields = get_lua_component_fields(instance),
        }
    end

    return out
end
