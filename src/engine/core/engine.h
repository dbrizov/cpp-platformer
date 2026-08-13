#pragma once

#include <memory>

#include "systems/audio/audio.h"
#include "systems/console.h"
#include "systems/entity_spawner.h"
#include "systems/imgui_system.h"
#include "systems/input.h"
#include "systems/physics.h"
#include "systems/renderer/renderer.h"
#include "systems/scripting/lua_script_system.h"
#include "systems/sdl_context.h"
#include "systems/timer.h"
#include "systems/ui/ui_system.h"
#include "systems/window.h"

namespace hob {
    struct EngineConfig;
    struct EditorConfig;
    class CameraComponent;
    class Editor;

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

        std::unique_ptr<Editor> m_editor;
        std::unique_ptr<Window> m_game_window;

        CameraComponent* m_active_camera = nullptr;
        bool m_warned_no_active_camera = false;

    public:
        Engine(const EngineConfig& config, const EditorConfig& editor_config);
        ~Engine();

        void run();

        SdlContext& get_sdl_context();
        const Window& get_main_window() const;
        Console& get_console();
        Renderer& get_renderer();
        Timer& get_timer();
        Input& get_input();
        UiSystem& get_ui_system();
        Physics& get_physics();
        Audio& get_audio();
        EntitySpawner& get_entity_spawner();
        LuaScriptSystem& get_lua_script_system();

        Editor* get_editor() const;
        const Window& get_game_window() const;

        CameraComponent* get_active_camera() const;
        void set_active_camera(CameraComponent* camera);
        void clear_active_camera(CameraComponent* camera);

    private:
        void draw_entities();
        void flush_debug_draws_to_renderer(float delta_time);

        static bool has_moving_physics_body(const Entity& entity);
    };
} // namespace hob
