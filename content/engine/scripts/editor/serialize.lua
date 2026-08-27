-- Editor serialize: the definition-table-to-Lua-source side of the Editor.* contract.

---@class Editor
_G.Editor = _G.Editor or {}

local INDENT = "    "
local MIN_FLOAT_PRECISION = 7
local MAX_FLOAT_PRECISION = 17

local SHAPES = {
    [FieldType.VECTOR2] = { ctor = "Vector2", fields = { "x", "y" } },
    [FieldType.COLOR] = { ctor = "Color", fields = { "r", "g", "b", "a" } },
    [FieldType.AABB] = { ctor = "AABB", fields = { "center", "extents" } },
    [FieldType.CAPSULE] = { ctor = "Capsule", fields = { "center_a", "center_b", "radius" } },
    [FieldType.CIRCLE] = { ctor = "Circle", fields = { "center", "radius" } },
}

local POSE_ORDER = { "position", "rotation_deg", "scale" }

local shape_by_metatable = nil

local function ensure_shape_metatables()
    if shape_by_metatable ~= nil then
        return
    end

    shape_by_metatable = {
        [getmetatable(Vector2())] = SHAPES[FieldType.VECTOR2],
        [getmetatable(Color())] = SHAPES[FieldType.COLOR],
        [getmetatable(AABB())] = SHAPES[FieldType.AABB],
        [getmetatable(Capsule())] = SHAPES[FieldType.CAPSULE],
        [getmetatable(Circle())] = SHAPES[FieldType.CIRCLE],
    }
end

local function fail(path, message)
    error("Editor.serialize_scene: " .. path .. " " .. message, 0)
end

local function format_number(value, path)
    if math.type(value) == "integer" then
        return tostring(value)
    end

    if value ~= value then
        fail(path, "is NaN, which has no Lua literal")
    end

    if value == math.huge then
        return "math.huge"
    end

    if value == -math.huge then
        return "-math.huge"
    end

    if value == 0.0 then
        return "0.0"
    end

    local text
    for precision = MIN_FLOAT_PRECISION, MAX_FLOAT_PRECISION do
        text = string.format("%." .. precision .. "g", value)
        if tonumber(text) == value then
            break
        end
    end

    if not text:find("[.eE]") then
        text = text .. ".0"
    end

    return text
end

local function format_enum(value, enum_name)
    local entries = Editor.get_enum_entries(enum_name)
    if entries == nil then
        return nil
    end

    for _, entry in ipairs(entries) do
        if entry.value == value then
            return enum_name .. "." .. entry.name
        end
    end

    return nil
end

local function format_bitmask(value, enum_name)
    local entries = Editor.get_enum_entries(enum_name)
    if entries == nil then
        return nil
    end

    local remaining = value
    local names = {}
    for _, entry in ipairs(entries) do
        if entry.value ~= 0 and (remaining & entry.value) == entry.value then
            names[#names + 1] = enum_name .. "." .. entry.name
            remaining = remaining & ~entry.value
        end
    end

    if remaining ~= 0 or #names == 0 then
        return nil
    end

    return table.concat(names, " | ")
end

local function format_named_number(value, field_meta, path)
    local enum_name = field_meta and field_meta.enum
    if enum_name ~= nil and math.type(value) == "integer" then
        local text = nil
        if field_meta.type == FieldType.ENUM then
            text = format_enum(value, enum_name)
        elseif field_meta.type == FieldType.BITMASK then
            text = format_bitmask(value, enum_name)
        end

        if text ~= nil then
            return text
        end
    end

    return format_number(value, path)
end

local function sorted_keys(source, skip)
    local keys = {}
    for key in pairs(source) do
        if type(key) == "string" and key:sub(1, 2) ~= "__" and (skip == nil or skip[key] == nil) then
            keys[#keys + 1] = key
        end
    end
    table.sort(keys)

    return keys
end

local function ordered_keys(source, order)
    local keys = {}
    local emitted = {}

    if order ~= nil then
        for _, key in ipairs(order) do
            if source[key] ~= nil then
                keys[#keys + 1] = key
                emitted[key] = true
            end
        end
    end

    for _, key in ipairs(sorted_keys(source, emitted)) do
        keys[#keys + 1] = key
    end

    return keys
end

local function wrap_fields(parts)
    if #parts == 0 then
        return nil
    end

    return "{ " .. table.concat(parts, ", ") .. " }"
end

local serialize_value

local function serialize_shape(shape, value, path)
    local args = {}
    for index, field in ipairs(shape.fields) do
        args[index] = serialize_value(value[field], nil, path .. "." .. field)
    end

    return shape.ctor .. "(" .. table.concat(args, ", ") .. ")"
end

local function serialize_plain_table(value, path)
    local parts = {}

    for index = 1, #value do
        parts[#parts + 1] = serialize_value(value[index], nil, path .. "[" .. index .. "]")
    end

    for _, key in ipairs(sorted_keys(value)) do
        parts[#parts + 1] = key .. " = " .. serialize_value(value[key], nil, path .. "." .. key)
    end

    return wrap_fields(parts) or "{}"
end

serialize_value = function(value, field_meta, path)
    if value == None then
        return "None"
    end

    local value_type = type(value)

    if value_type == "number" then
        return format_named_number(value, field_meta, path)
    end

    if value_type == "boolean" then
        return tostring(value)
    end

    if value_type == "string" then
        return string.format("%q", value)
    end

    if value_type == "table" then
        local mt = getmetatable(value)
        if mt == nil then
            return serialize_plain_table(value, path)
        end

        local registry = rawget(mt, "__registry")
        local asset_name = rawget(value, "__name")
        if registry == nil or asset_name == nil then
            fail(path, "holds a table whose metatable is not a declared asset registry")
        end

        return registry .. "." .. asset_name
    end

    if value_type == "userdata" then
        ensure_shape_metatables()

        local shape = SHAPES[field_meta and field_meta.type] or shape_by_metatable[getmetatable(value)]
        if shape == nil then
            fail(path, "holds '" .. tostring(value) ..
                "', which is neither a declared asset nor a value with a Lua literal")
        end

        return serialize_shape(shape, value, path)
    end

    fail(path, "holds a " .. value_type .. " value, which cannot be written to a scene file")
end

local function serialize_pose_overrides(pose, path)
    local parts = {}
    for _, field in ipairs(ordered_keys(pose, POSE_ORDER)) do
        parts[#parts + 1] = field .. " = " .. serialize_value(pose[field], nil, path .. "." .. field)
    end

    return wrap_fields(parts)
end

local function serialize_cpp_section(section, schema, path)
    if type(section) ~= "table" then
        fail(path, "is not a table")
    end

    local types = schema and schema.types

    local parts = {}
    for _, field in ipairs(ordered_keys(section, schema and schema.__order)) do
        local field_meta = types and types[field]
        parts[#parts + 1] = field .. " = " .. serialize_value(section[field], field_meta, path .. "." .. field)
    end

    return wrap_fields(parts)
end

local function serialize_cpp_overrides(cpp_overrides, path)
    local schemas = _G.__component_schemas

    local parts = {}
    for _, key in ipairs(ordered_keys(cpp_overrides, schemas.__order)) do
        local section = serialize_cpp_section(cpp_overrides[key], schemas[key], path .. "." .. key)
        if section ~= nil then
            parts[#parts + 1] = key .. " = " .. section
        end
    end

    return wrap_fields(parts)
end

local function serialize_lua_overrides(lua_overrides, path)
    local parts = {}
    for _, class_name in ipairs(sorted_keys(lua_overrides)) do
        local section_path = path .. "." .. class_name
        local overrides = lua_overrides[class_name]
        if type(overrides) ~= "table" then
            fail(section_path, "is not a table")
        end

        local fields = {}
        for _, field in ipairs(sorted_keys(overrides)) do
            fields[#fields + 1] = field .. " = " ..
                serialize_value(overrides[field], nil, section_path .. "." .. field)
        end

        local section = wrap_fields(fields)
        if section ~= nil then
            parts[#parts + 1] = class_name .. " = " .. section
        end
    end

    return wrap_fields(parts)
end

local function append_overrides(parts, inst, key, serialize_section, path)
    local overrides = inst[key]
    if type(overrides) ~= "table" then
        return
    end

    local section = serialize_section(overrides, path .. "." .. key)
    if section ~= nil then
        parts[#parts + 1] = key .. " = " .. section
    end
end

local function serialize_instance(inst, path)
    if type(inst.prefab) ~= "string" then
        fail(path, "does not name a prefab")
    end

    local parts = { "prefab = " .. DefRegistry.ENTITIES .. "." .. inst.prefab }

    append_overrides(parts, inst, SceneKey.POSE_OVERRIDES, serialize_pose_overrides, path)
    append_overrides(parts, inst, SceneKey.CPP_OVERRIDES, serialize_cpp_overrides, path)
    append_overrides(parts, inst, SceneKey.LUA_OVERRIDES, serialize_lua_overrides, path)

    return "{ " .. table.concat(parts, ", ") .. " }"
end

---@param name string
---@return string
function Editor.serialize_scene(name)
    local def = _G.__scene_registry[name]
    if def == nil then
        error("Editor.serialize_scene: scene '" .. tostring(name) .. "' is not registered", 0)
    end

    local lines = {}
    lines[#lines + 1] = "DefineScene." .. name .. " = {"
    lines[#lines + 1] = INDENT .. "entities = {"

    for index, inst in ipairs(def.entities) do
        lines[#lines + 1] = INDENT .. INDENT .. serialize_instance(inst, "entities[" .. index .. "]") .. ","
    end

    lines[#lines + 1] = INDENT .. "},"
    lines[#lines + 1] = "}"

    return table.concat(lines, "\n") .. "\n"
end
