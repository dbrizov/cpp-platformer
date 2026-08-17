#include "editor.h"

#include <algorithm>
#include <filesystem>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "actions/editor_action.h"
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
        m_scene_view.release_color_target(*this);
        m_inspector.reset_edit_state();

        log::engine.info("Editor::Shutdown");
    }

    Engine& Editor::get_engine() const {
        return m_engine;
    }

    EditorState Editor::get_state() const {
        return m_state;
    }

    void Editor::set_state(EditorState state) {
        if (state == m_state) {
            return;
        }

        const bool entering_play = (m_state == EditorState::Edit);
        const bool leaving_play = (state == EditorState::Edit);

        if (entering_play || leaving_play) {
            m_commands.clear();
            m_selection.clear();
            m_scene_view.reset_pick_cycle();
            m_inspector.reset_edit_state();
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

    void Editor::request_step() {
        m_step_requested = true;
    }

    void Editor::request_reset_layout() {
        m_reset_layout = true;
    }

    void Editor::request_quit() {
        SDL_Event quit_event{};
        quit_event.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit_event);
    }

    void Editor::request_action(EditorActionId id) {
        m_actions.request(id);
    }

    EditorEntitySelection& Editor::get_selection() {
        return m_selection;
    }

    const EditorEntitySelection& Editor::get_selection() const {
        return m_selection;
    }

    EditorCommandStack& Editor::get_commands() {
        return m_commands;
    }

    const EditorCommandStack& Editor::get_commands() const {
        return m_commands;
    }

    EditorMenuBar& Editor::get_menu_bar() {
        return m_menu_bar;
    }

    EditorToolbar& Editor::get_toolbar() {
        return m_toolbar;
    }

    EditorDockSceneView& Editor::get_scene_view() {
        return m_scene_view;
    }

    EditorDockHierarchy& Editor::get_hierarchy() {
        return m_hierarchy;
    }

    EditorDockInspector& Editor::get_inspector() {
        return m_inspector;
    }

    EditorDockAssets& Editor::get_assets() {
        return m_assets;
    }

    void Editor::tick(float delta_time) {
        const bool simulate = (m_state == EditorState::Play) || (m_state == EditorState::Paused && m_step_requested);
        m_step_requested = false;

        m_engine.set_simulation_enabled(simulate);
        m_engine.set_game_input_enabled(m_state == EditorState::Play);

        prune_selection();
    }

    void Editor::draw_gui() {
        update_input();

        if (ImGui::BeginMainMenuBar()) {
            m_menu_bar.draw(*this);
            m_toolbar.draw(*this);
            ImGui::EndMainMenuBar();
        }

        const ImGuiID dock_space_id = dock_space_over_viewport(ImGuiDockNodeFlags_PassthruCentralNode);
        if (m_reset_layout) {
            m_reset_layout = false;
            build_default_layout(dock_space_id);
        }

        for (EditorDock* dock : get_docks()) {
            dock->draw(*this);
        }

        // Every Begin/End has closed, so an action is free to spawn, destroy or open a window.
        m_actions.flush(*this);
    }

    void Editor::render_passes() {
        m_scene_view.render_pass(*this);
    }

    void Editor::on_lua_hot_reloaded() {
        m_engine.get_lua_script_system().run_engine_folder(EDITOR_SCRIPTS_FOLDER);
    }

    bool Editor::on_quit_requested() {
        if (m_engine.get_game_window() == nullptr) {
            return false;
        }

        set_state(EditorState::Edit);
        return true;
    }

    bool Editor::on_window_close_requested(SDL_WindowID window_id) {
        const Window* game_window = m_engine.get_game_window();
        if (game_window == nullptr || game_window->get_id() != window_id) {
            return false;
        }

        set_state(EditorState::Edit);
        return true;
    }

    std::array<EditorDock*, Editor::DOCK_COUNT> Editor::get_docks() {
        return {&m_scene_view, &m_hierarchy, &m_inspector, &m_assets};
    }

    void Editor::update_input() {
        m_active_contexts = 0;

        // Panel records are from the previous frame, which is the only point a panel rect exists.
        if (m_engine.get_main_window().has_focus() && !ImGui::GetIO().WantTextInput) {
            m_active_contexts |= context_bit(EditorActionContext::Global);

            for (const EditorDock* dock : get_docks()) {
                if (dock->is_hovered() || dock->is_focused()) {
                    m_active_contexts |= context_bit(dock->get_context());
                }
            }

            for (const EditorAction& action : get_actions()) {
                if (is_context_active(action.context) && is_chord_pressed(action.chord) &&
                    is_action_enabled(*this, action.id)) {
                    m_actions.request(action.id);
                }
            }
        }

        m_scene_view.update_input(*this);
    }

    bool Editor::is_context_active(EditorActionContext context) const {
        return (m_active_contexts & context_bit(context)) != 0;
    }

    void Editor::prune_selection() {
        const EntitySpawner& spawner = m_engine.get_entity_spawner();

        std::erase_if(m_selection.ids, [&spawner](EntityId id) {
            return spawner.get_entity(id) == nullptr;
        });

        if (m_selection.range_anchor != INVALID_ENTITY_ID && spawner.get_entity(m_selection.range_anchor) == nullptr) {
            m_selection.range_anchor = INVALID_ENTITY_ID;
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

        ImGui::DockBuilderDockWindow(m_hierarchy.get_name(), right);
        ImGui::DockBuilderDockWindow(m_inspector.get_name(), inspector);
        ImGui::DockBuilderDockWindow(m_assets.get_name(), assets);
        ImGui::DockBuilderDockWindow(m_scene_view.get_name(), center);

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
