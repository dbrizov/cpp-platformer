#pragma once

#include <string>

namespace hob {
    class Engine;

    class Editor {
        Engine& m_engine;
        std::string m_imgui_ini_path;

    public:
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
    };
} // namespace hob
