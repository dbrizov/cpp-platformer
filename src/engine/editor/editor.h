#pragma once

#include <imgui.h>

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

        bool m_reset_layout = false;

        State m_state = State::Edit;
        bool m_step_requested = false;
        bool m_simulate_this_frame = false;

    public:
        static constexpr const char* PANEL_SCENE = "Scene";
        static constexpr const char* PANEL_HIERARCHY = "Hierarchy";
        static constexpr const char* PANEL_INSPECTOR = "Inspector";
        static constexpr const char* PANEL_ASSETS = "Assets";
        static constexpr const char* PANELS[] = {PANEL_SCENE, PANEL_HIERARCHY, PANEL_INSPECTOR, PANEL_ASSETS};

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

    private:
        void draw_dockspace();
        void draw_menu_bar();
        void draw_toolbar();

        void build_default_layout(ImGuiID dockspace_id);
        void save_layout();
    };
} // namespace hob
