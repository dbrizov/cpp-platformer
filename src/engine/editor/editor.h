#pragma once

#include <imgui.h>

namespace hob {
    class Engine;

    class Editor {
        Engine& m_engine;

        bool m_reset_layout = false;

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

        void draw_gui();

    private:
        void draw_dockspace();
        void draw_menu_bar();

        void build_default_layout(ImGuiID dockspace_id);
        void save_layout();
    };
} // namespace hob
