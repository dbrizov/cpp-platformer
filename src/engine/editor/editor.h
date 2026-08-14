#pragma once

#include <cstdint>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <imgui.h>

#include "editor_camera.h"
#include "engine/math/vector2.h"

struct ImDrawList;

namespace hob {
    class Engine;

    class Editor {
    public:
        enum class State {
            Edit,
            Play,
            Paused,
        };

    private:
        Engine& m_engine;

        std::string m_imgui_ini_path;
        bool m_reset_layout = false;

        State m_state = State::Edit;
        bool m_step_requested = false;
        bool m_simulate_this_frame = false;

        EditorCamera m_camera;

        SDL_GPUTexture* m_scene_color_target = nullptr;
        uint32_t m_scene_color_target_width = 0;
        uint32_t m_scene_color_target_height = 0;

    public:
        static constexpr const char* PANEL_SCENE = "Scene";
        static constexpr const char* PANEL_HIERARCHY = "Hierarchy";
        static constexpr const char* PANEL_INSPECTOR = "Inspector";
        static constexpr const char* PANEL_ASSETS = "Assets";
        static constexpr const char* PANELS[] = {PANEL_HIERARCHY, PANEL_INSPECTOR, PANEL_ASSETS};

        explicit Editor(Engine& engine);
        ~Editor();

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;

        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        void set_state(State state);

        bool is_simulating() const;

        bool wants_game_input() const;

        void tick(float delta_time);
        void draw_gui();
        void render_passes();

    private:
        void draw_dockspace();
        void draw_menu_bar();
        void draw_toolbar();
        void draw_scene_view();

        void ensure_scene_color_target(uint32_t width, uint32_t height);
        void release_scene_color_target();
        void handle_scene_view_input(const Vector2& panel_pos, const Vector2& panel_size);
        void draw_grid(ImDrawList* draw_list, const Vector2& panel_pos, const Vector2& panel_size) const;

        void build_default_layout(ImGuiID dockspace_id);
        void save_layout();
    };
} // namespace hob
