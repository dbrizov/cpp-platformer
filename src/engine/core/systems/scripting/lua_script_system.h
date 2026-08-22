#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace sol {
    class state;
}

namespace hob {
    class Engine;
    class Console;
    struct LuaScriptSystemImpl;

    class LuaScriptSystem {
        Engine& m_engine;
        std::unique_ptr<LuaScriptSystemImpl> m_impl;

        std::filesystem::file_time_type m_last_script_write_time{};
        bool m_has_script_write_baseline = false;
        float m_script_watch_accumulator = 0.0f;

    public:
        LuaScriptSystem(Engine& engine, bool run_project_main_on_boot);
        ~LuaScriptSystem();

        LuaScriptSystem(const LuaScriptSystem&) = delete;
        LuaScriptSystem& operator=(const LuaScriptSystem&) = delete;

        LuaScriptSystem(LuaScriptSystem&&) = delete;
        LuaScriptSystem& operator=(LuaScriptSystem&&) = delete;

        void register_cvars(Console& console);

        sol::state& get_lua();

        bool hot_reload();
        void poll_hot_reload(float delta_time);

        bool run_file(const std::filesystem::path& base, const std::filesystem::path& relative_path);
        bool run_folder(const std::filesystem::path& base,
                        const std::filesystem::path& relative_path,
                        const std::vector<std::string>& excludes);

        bool run_engine_file(const std::filesystem::path& relative_path);
        bool run_engine_folder(const std::filesystem::path& relative_path,
                               const std::vector<std::string>& excludes = {});

        bool run_project_file(const std::filesystem::path& relative_path);
        bool run_project_folder(const std::filesystem::path& relative_path,
                                const std::vector<std::string>& excludes = {});

        bool run_bootstrap();
        bool run_project_main();

    private:
        void refresh_lua_component_class_caches();

        void register_bindings();

        void bind_schema();
        void bind_asset();
        void bind_math();
        void bind_entity();
        void bind_components();
        void bind_camera();
        void bind_renderer();
        void bind_timer();
        void bind_input();
        void bind_ui();
        void bind_physics();
        void bind_audio();
        void bind_entity_spawner();
        void bind_scripts();
        void bind_debug();
        void bind_logging();

        void dump_component_schemas();
        void dump_asset_factory_schemas();

        void dump_bindings_meta();
        void dump_asset_factory_schemas_meta();
        void dump_asset_names_meta();
        void dump_shader_params_meta();
        void dump_entity_registry_meta();
        void dump_component_registry_meta();
        void dump_scene_registry_meta();
    };
} // namespace hob
