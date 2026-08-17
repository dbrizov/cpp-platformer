-- Typed path-alias namespaces (DefineAsset, DefineTexture, DefineShader, ...).
--
-- Usage:
--   DefineTexture.PlayerTexture = "images/player/HJ_run01.png"
--   DefineShader.MyShader       = "shaders/my_shader"
--   DefineAsset.SomeFont        = "fonts/arial.ttf"

-- Declared alias names per registry; read by C++ after bootstrap to emit path_aliases_meta.generated.lua.
_G.__path_alias_names = _G.__path_alias_names or {}

local function install_path_registry(define_name, registry_name, type_label)
    local store = {}

    local names = {}
    local seen = {}
    _G.__path_alias_names[registry_name] = names

    local ref_mt = {
        __tostring = function(self)
            local path = store[self.__name]
            if not path then
                Log.error(type_label .. " '" .. self.__name .. "' is not defined")
                return ""
            end

            return path
        end,
        __unwrap = function(self)
            local path = store[self.__name]
            if not path then
                Log.error(type_label .. " '" .. self.__name .. "' is not defined")
                return ""
            end

            return path
        end,
    }

    _G[define_name] = setmetatable({}, {
        __newindex = function(_, name, def)
            if def == nil then
                Log.error(define_name .. "." .. tostring(name) .. " must not be nil")
                return
            end

            store[name] = def

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

function _G.__install_path_registries()
    local schemas = _G.__path_schemas
    if schemas == nil then
        Log.error("__install_path_registries: __path_schemas is missing (did path_schemas.generated.lua run?)")
        return
    end

    for _, s in ipairs(schemas) do
        install_path_registry(s.define, s.registry, s.type_label)
    end
end
