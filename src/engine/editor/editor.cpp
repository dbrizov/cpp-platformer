#include "editor.h"

#include <filesystem>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "editor_config.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"

namespace hob {
    Editor::Editor(Engine& engine)
        : m_engine(engine) {
        const std::string imgui_ini_path = PathUtils::get_editor_imgui_ini_path().string();
        ImGui::GetIO().IniFilename = imgui_ini_path.c_str();

        m_reset_layout = !std::filesystem::exists(imgui_ini_path);

        log::engine.info("Editor::Initialise");
    }

    Editor::~Editor() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.IniFilename != nullptr) {
            ImGui::SaveIniSettingsToDisk(io.IniFilename);
            io.IniFilename = nullptr;
        }

        save_layout();

        log::engine.info("Editor::Shutdown");
    }

    void Editor::draw_gui() {
        draw_dockspace();

        for (const char* name : PANELS) {
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
        const ImGuiID dockspace_id =
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if (m_reset_layout) {
            m_reset_layout = false;
            build_default_layout(dockspace_id);
        }
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

    void Editor::build_default_layout(ImGuiID dockspace_id) {
#ifdef IMGUI_HAS_DOCK
        const ImGuiDockNodeFlags node_flags =
            static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_DockSpace) | ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, node_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID center = dockspace_id;
        ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.40f, nullptr, &center);
        const ImGuiID inspector = ImGui::DockBuilderSplitNode(right, ImGuiDir_Right, 0.50f, nullptr, &right);
        const ImGuiID assets = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);

        ImGui::DockBuilderDockWindow(PANEL_HIERARCHY, right);
        ImGui::DockBuilderDockWindow(PANEL_INSPECTOR, inspector);
        ImGui::DockBuilderDockWindow(PANEL_ASSETS, assets);
        ImGui::DockBuilderDockWindow(PANEL_SCENE, center);

        ImGui::DockBuilderFinish(dockspace_id);
#else
        (void)dockspace_id;
#endif
    }

    void Editor::save_layout() {
        SDL_Window* window = m_engine.get_main_window().get_window();
        EditorConfig editor_config;
        SDL_GetWindowPosition(window, &editor_config.x, &editor_config.y);
        SDL_GetWindowSize(window, &editor_config.width, &editor_config.height);
        editor_config.maximized = (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) != 0;
        editor_config.save(PathUtils::get_editor_config_path());
    }
} // namespace hob
