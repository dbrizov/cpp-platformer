#include "editor.h"

#include <SDL3/SDL_events.h>
#include <imgui.h>

#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"

namespace hob {
    Editor::Editor(Engine& engine)
        : m_engine(engine) {
        m_imgui_ini_path = (PathUtils::get_project_root() / "editor_imgui.ini").string();
        ImGui::GetIO().IniFilename = m_imgui_ini_path.c_str();

        log::engine.info("Editor::Initialise");
    }

    Editor::~Editor() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.IniFilename != nullptr) {
            ImGui::SaveIniSettingsToDisk(io.IniFilename);
            io.IniFilename = nullptr;
        }

        log::engine.info("Editor::Shutdown");
    }

    void Editor::draw_gui() {
        draw_dockspace();

        for (const char* name : {"Scene", "Hierarchy", "Inspector", "Assets"}) {
            if (ImGui::Begin(name)) {
                ImGui::TextDisabled("%s (empty)", name);
            }
            ImGui::End();
        }
    }

    void Editor::draw_dockspace() {
        if (ImGui::BeginMainMenuBar()) {
            draw_menu_bar();
            ImGui::EndMainMenuBar();
        }

#ifdef IMGUI_HAS_DOCK
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
#endif
    }

    void Editor::draw_menu_bar() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Quit")) {
                SDL_Event quit_event{};
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
            }

            ImGui::EndMenu();
        }
    }
} // namespace hob
