#include "editor.h"

#include <algorithm>
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
        : m_engine(engine)
        , m_imgui_ini_path(PathUtils::get_editor_imgui_ini_path().string()) {
        ImGui::GetIO().IniFilename = m_imgui_ini_path.c_str();

        m_reset_layout = !std::filesystem::exists(m_imgui_ini_path);

        log::engine.info("Editor::Initialise");
    }

    Editor::~Editor() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.IniFilename != nullptr) {
            ImGui::SaveIniSettingsToDisk(io.IniFilename);
            io.IniFilename = nullptr;
        }

        save_layout();
        release_scene_color_target();

        log::engine.info("Editor::Shutdown");
    }

    void Editor::set_state(State state) {
        if (state == m_state) {
            return;
        }

        const bool entering_play = (m_state == State::Edit);
        const bool leaving_play = (state == State::Edit);

        if (entering_play) {
            m_engine.open_game_window();
            m_engine.get_lua_script_system().run_project_main();
        }
        else if (leaving_play) {
            m_engine.get_entity_spawner().clear();
            m_engine.close_game_window();
        }

        m_state = state;
    }

    bool Editor::is_simulating() const {
        return m_simulate_this_frame;
    }

    bool Editor::wants_game_input() const {
        return m_state == State::Play && m_engine.get_game_window() && m_engine.get_game_window()->has_focus();
    }

    void Editor::tick(float delta_time) {
        m_simulate_this_frame = (m_state == State::Play) || (m_state == State::Paused && m_step_requested);
        m_step_requested = false;
    }

    void Editor::draw_gui() {
        draw_dockspace();
        draw_scene_view();

        for (const char* name : PANELS) {
            if (ImGui::Begin(name)) {
                ImGui::TextDisabled("%s (empty)", name);
            }
            ImGui::End();
        }
    }

    void Editor::render_passes() {
        if (m_scene_color_target == nullptr) {
            return;
        }

        const Vector2 scene_size(static_cast<float>(m_scene_color_target_width),
                                 static_cast<float>(m_scene_color_target_height));
        const Matrix4x4 view_proj = m_camera.build_view_projection(scene_size);

        m_engine.get_renderer().render_world_pass_to(m_scene_color_target, view_proj);
    }

    void Editor::ensure_scene_color_target(uint32_t width, uint32_t height) {
        width = std::max(width, 1u);
        height = std::max(height, 1u);

        if (m_scene_color_target && width == m_scene_color_target_width && height == m_scene_color_target_height) {
            return;
        }

        release_scene_color_target();

        m_scene_color_target = m_engine.get_renderer().create_color_target(width, height);
        if (m_scene_color_target == nullptr) {
            return;
        }

        m_scene_color_target_width = width;
        m_scene_color_target_height = height;
    }

    void Editor::release_scene_color_target() {
        if (m_scene_color_target == nullptr) {
            return;
        }

        SDL_ReleaseGPUTexture(m_engine.get_renderer().get_gpu_device(), m_scene_color_target);
        m_scene_color_target = nullptr;
        m_scene_color_target_width = 0;
        m_scene_color_target_height = 0;
    }

    void Editor::draw_dockspace() {
        if (ImGui::BeginMainMenuBar()) {
            draw_menu_bar();
            draw_toolbar();
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

    void Editor::draw_toolbar() {
        ImGui::Separator();

        switch (m_state) {
            case State::Edit: {
                if (ImGui::SmallButton("Play")) {
                    set_state(State::Play);
                }
                break;
            }
            case State::Play: {
                if (ImGui::SmallButton("Pause")) {
                    set_state(State::Paused);
                }
                if (ImGui::SmallButton("Stop")) {
                    set_state(State::Edit);
                }
                break;
            }
            case State::Paused: {
                if (ImGui::SmallButton("Resume")) {
                    set_state(State::Play);
                }
                if (ImGui::SmallButton("Step")) {
                    m_step_requested = true;
                }
                if (ImGui::SmallButton("Stop")) {
                    set_state(State::Edit);
                }
                break;
            }
        }

        const char* label = (m_state == State::Edit) ? "Edit" : (m_state == State::Play) ? "Play" : "Paused";
        ImGui::Separator();
        ImGui::TextDisabled("%s", label);
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
