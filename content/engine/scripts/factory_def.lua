-- Generic factory-type registry.

-- Declared alias names per registry; read by C++ after bootstrap to emit factory_aliases_meta.generated.lua.
_G.__factory_alias_names = _G.__factory_alias_names or {}
_G.__factory_cache_clearers = {}

local function install_factory_registry(registry_name, schema)
    local defs = {}
    local built = {}

    _G.__factory_cache_clearers[#_G.__factory_cache_clearers + 1] = function()
        for name in pairs(built) do
            built[name] = nil
        end
    end

    local names = {}
    local seen = {}
    _G.__factory_alias_names[registry_name] = names

    local function build(name)
        local def = defs[name]
        if not def then
            Log.error(schema.lua_type .. " '" .. name .. "' is not defined")
            return nil
        end

        local cfg = {}
        for key, value in pairs(def) do
            cfg[key] = unwrap_def(value)
        end

        local ctor = _G[schema.lua_type]
        if ctor == nil then
            Log.error("Factory type '" .. schema.lua_type .. "' is not bound in Lua")
            return nil
        end

        local obj = ctor(cfg)
        if obj and obj.set_name then
            obj:set_name(name)
        end
        built[name] = obj
        return obj
    end

    local ref_mt = {
        __tostring = function(self)
            return schema.lua_type .. "(" .. self.__name .. ")"
        end,
        __unwrap = function(self)
            return built[self.__name] or build(self.__name)
        end,
    }

    _G[schema.define] = setmetatable({}, {
        __newindex = function(_, name, def)
            if type(def) == "string" then
                def = { path = def }
            elseif type(def) ~= "table" then
                Log.error(schema.define .. "." .. tostring(name) .. " must be assigned a table or a path string")
                return
            end

            defs[name] = def

            if not seen[name] then
                seen[name] = true
                names[#names + 1] = name
            end
        end,
    })

    _G[registry_name] = setmetatable({}, {
        __index = function(t, name)
            local wrapper = setmetatable({ __name = name }, ref_mt)
            rawset(t, name, wrapper)
            return wrapper
        end,
    })
end

function _G.__clear_factory_caches()
    for _, clear in ipairs(_G.__factory_cache_clearers) do
        clear()
    end
end

-- Eagerly build every declared shader so its GPU pipeline compiles at load, not on the gameplay hot path.
-- Only shaders are warmed: materials are a cheap CPU param buffer (no compile).
function _G.__warmup_shaders()
    local names = _G.__factory_alias_names["Shaders"]
    if names and Shaders then
        for _, name in ipairs(names) do
            unwrap_def(Shaders[name])
        end
    end
end

function _G.__install_factory_registries()
    local schemas = _G.__factory_schemas
    if schemas == nil then
        Log.error(
            "__install_factory_registries: __factory_schemas is missing (did factory_schemas.generated.lua run?)")
        return
    end

    for registry_name, schema in pairs(schemas) do
        install_factory_registry(registry_name, schema)
    end
end
