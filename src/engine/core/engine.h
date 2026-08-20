#pragma once

#include <memory>

#include "systems/audio/audio.h"
#include "systems/console.h"
#include "systems/entity_spawner.h"
#include "systems/imgui_system.h"
#include "systems/input.h"
#include "systems/physics/physics.h"
#include "systems/renderer/renderer.h"
#include "systems/scripting/lua_script_system.h"
#include "systems/sdl_context.h"
#include "systems/timer.h"
#include "systems/ui/ui_system.h"
#include "systems/window.h"

namespace hob {
    struct EngineConfig;
    class CameraComponent;
    class EngineHooks;

    class Engine {
        // Order matters
        SdlContext m_sdl_context;
        Window m_main_window;
        Renderer m_renderer;
        Timer m_timer;
        Input m_input;
        UiSystem m_ui_system;
        ImGuiSystem m_imgui_system;
        Console m_console;
        Physics m_physics;
        Audio m_audio;
        EntitySpawner m_entity_spawner;
        LuaScriptSystem m_lua_script_system;

        std::unique_ptr<Window> m_game_window;
        WindowConfig m_game_window_config;

        EngineHooks* m_hooks = nullptr;

        bool m_is_simulation_enabled = true;
        bool m_is_game_input_enabled = true;

        CameraComponent* m_active_camera = nullptr;
        mutable bool m_warned_no_active_camera = false;

    public:
        explicit Engine(const EngineConfig& config);
        ~Engine();

        EngineHooks* get_hooks() const;
        void set_hooks(EngineHooks* hooks);

        void run();

        SdlContext& get_sdl_context();
        Renderer& get_renderer();
        Timer& get_timer();
        Input& get_input();
        UiSystem& get_ui_system();
        ImGuiSystem& get_imgui_system();
        Console& get_console();
        Physics& get_physics();
        Audio& get_audio();
        EntitySpawner& get_entity_spawner();
        LuaScriptSystem& get_lua_script_system();

        const Window& get_main_window() const;
        const Window& get_play_window() const;
        const Window* get_game_window() const;
        WindowConfig get_game_window_config() const;
        void set_game_window_config(const WindowConfig& config);
        void open_game_window();
        void close_game_window();

        bool is_simulation_enabled() const;
        void set_simulation_enabled(bool enabled);

        bool is_game_input_enabled() const;
        void set_game_input_enabled(bool enabled);

        CameraComponent* get_active_camera() const;
        void set_active_camera(CameraComponent* camera);
        void clear_active_camera(CameraComponent* camera);

    private:
        Matrix4x4 get_game_camera_view_projection() const;

        void draw_entities();
        void flush_debug_draws_to_renderer(float delta_time);

        static bool has_moving_physics_body(const Entity& entity);
    };
} // namespace hob
