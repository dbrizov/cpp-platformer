#pragma once

#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "lua_type_names.h"

namespace hob {
    struct LuaAssetFactoryFieldInfo {
        std::string name;
    };

    struct LuaAssetFactorySchemaInfo {
        std::string define_name; // Lua-side define entry point, e.g. "DefineMaterial"
        std::string factory_name; // Lua-side factory table, e.g. "Materials"
        std::string lua_type; // Bound usertype name invoked as a factory, e.g. "Material"
        std::vector<LuaAssetFactoryFieldInfo> fields;
    };

    class LuaAssetFactorySchemaRegistry {
        std::vector<LuaAssetFactorySchemaInfo> m_schemas;

    public:
        void add_schema(LuaAssetFactorySchemaInfo info);

        const std::vector<LuaAssetFactorySchemaInfo>& get_schemas() const;

        bool write_to_file(const std::filesystem::path& full_path) const;
        bool write_meta_to_file(const std::filesystem::path& full_path) const;
    };

    // Records that a usertype `T` bound with a `factory_ctor` accepting a single config table
    // is authorable via a Lua-side `DefineX.Name = { ... }` factory. The Lua type name is
    // pulled from `LuaTypeName<T>`; the corresponding bind_usertype<T>(...).factory_ctor(...)
    // call must already exist (this function only records metadata).
    template<typename T>
    void bind_asset_factory_schema(LuaAssetFactorySchemaRegistry& schemas,
                                   const char* define_name,
                                   const char* factory_name,
                                   std::initializer_list<const char*> fields) {
        LuaAssetFactorySchemaInfo info;
        info.define_name = define_name;
        info.factory_name = factory_name;
        info.lua_type = LuaTypeName<T>::value;
        info.fields.reserve(fields.size());
        for (const auto& f : fields) {
            info.fields.emplace_back(f);
        }

        schemas.add_schema(std::move(info));
    }
} // namespace hob
