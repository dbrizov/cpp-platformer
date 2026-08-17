#include "editor.h"

#include <algorithm>
#include <filesystem>

#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "editor_config.h"
#include "editor_gui_utils.h"
#include "editor_style.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/core/systems/window.h"

namespace hob::editor {
    namespace {
        constexpr const char* EDITOR_SCRIPTS_FOLDER = "scripts/editor";

        constexpr float LAYOUT_RIGHT_COLUMNS_RATIO = 0.46f;
        constexpr float LAYOUT_INSPECTOR_RATIO = 0.50f;
        constexpr float LAYOUT_ASSETS_RATIO = 0.30f;
    } // namespace

    Editor::Editor(Engine& engine)
        : m_engine(engine)
        , m_imgui_ini_path(get_editor_imgui_ini_file_path().string()) {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = m_imgui_ini_path.c_str();
        io.ConfigDragClickToInputText = true;

        m_engine.get_lua_script_system().run_engine_folder(EDITOR_SCRIPTS_FOLDER);

        apply_style();
        m_engine.get_imgui_system().set_clear_color(COLOR_CLEAR);
        m_engine.get_imgui_system().set_clear_swapchain(true); // Nothing but ImGui draws to the editor window.

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
        reset_inspector_edit_state();

        log::engine.info("Editor::Shutdown");
    }

    void Editor::set_state(State state) {
        if (state == m_state) {
            return;
        }

        const bool entering_play = (m_state == State::Edit);
        const bool leaving_play = (state == State::Edit);

        if (entering_play || leaving_play) {
            m_commands.clear();
            m_selection.clear();
            m_range_selection_anchor = INVALID_ENTITY_ID;
            m_pick_cycle_last_entity_id = INVALID_ENTITY_ID;
            reset_inspector_edit_state();
        }

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

    void Editor::tick(float delta_time) {
        const bool simulate = (m_state == State::Play) || (m_state == State::Paused && m_step_requested);
        m_step_requested = false;

        m_engine.set_simulation_enabled(simulate);
        m_engine.set_game_input_enabled(m_state == State::Play);

        prune_selection();
        handle_undo_redo_shortcuts();
    }

    void Editor::draw_gui() {
        if (ImGui::BeginMainMenuBar()) {
            draw_menu_bar();
            draw_toolbar();
            ImGui::EndMainMenuBar();
        }

        const ImGuiID dock_space_id = dock_space_over_viewport(ImGuiDockNodeFlags_PassthruCentralNode);
        if (m_reset_layout) {
            m_reset_layout = false;
            build_default_layout(dock_space_id);
        }

        draw_scene_view();
        draw_hierarchy();
        draw_inspector();
        draw_assets();
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

    void Editor::on_lua_hot_reloaded() {
        m_engine.get_lua_script_system().run_engine_folder(EDITOR_SCRIPTS_FOLDER);
    }

    bool Editor::on_quit_requested() {
        if (m_engine.get_game_window() == nullptr) {
            return false;
        }

        set_state(State::Edit);
        return true;
    }

    bool Editor::on_window_close_requested(SDL_WindowID window_id) {
        const Window* game_window = m_engine.get_game_window();
        if (game_window == nullptr || game_window->get_id() != window_id) {
            return false;
        }

        set_state(State::Edit);
        return true;
    }

    void Editor::handle_undo_redo_shortcuts() {
        const ImGuiIO& io = ImGui::GetIO();
        if (!io.KeyCtrl || io.WantTextInput) {
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
            m_commands.undo(m_engine);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
            m_commands.redo(m_engine);
        }
    }

    void Editor::build_default_layout(ImGuiID dock_space_id) {
        const ImGuiDockNodeFlags node_flags =
            static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_DockSpace) | ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockBuilderRemoveNode(dock_space_id);
        ImGui::DockBuilderAddNode(dock_space_id, node_flags);
        ImGui::DockBuilderSetNodeSize(dock_space_id, ImGui::GetMainViewport()->Size);

        ImGuiID center = dock_space_id;
        ImGuiID right =
            ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, LAYOUT_RIGHT_COLUMNS_RATIO, nullptr, &center);
        const ImGuiID inspector =
            ImGui::DockBuilderSplitNode(right, ImGuiDir_Right, LAYOUT_INSPECTOR_RATIO, nullptr, &right);
        const ImGuiID assets =
            ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, LAYOUT_ASSETS_RATIO, nullptr, &center);

        ImGui::DockBuilderDockWindow(PANEL_HIERARCHY, right);
        ImGui::DockBuilderDockWindow(PANEL_INSPECTOR, inspector);
        ImGui::DockBuilderDockWindow(PANEL_ASSETS, assets);
        ImGui::DockBuilderDockWindow(PANEL_SCENE, center);

        ImGui::DockBuilderFinish(dock_space_id);
    }

    void Editor::save_layout() {
        SDL_Window* window = m_engine.get_main_window().get_window();
        EditorConfig editor_config;
        SDL_GetWindowPosition(window, &editor_config.x, &editor_config.y);
        SDL_GetWindowSize(window, &editor_config.width, &editor_config.height);
        editor_config.maximized = (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) != 0;
        editor_config.save(get_editor_config_file_path());
    }
} // namespace hob::editor
