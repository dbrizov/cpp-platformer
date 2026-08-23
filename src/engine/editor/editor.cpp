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
#include "editor_lua.h"
#include "editor_style.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
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

        const EditorConfig editor_config(get_editor_config_file_path());
        m_pending_scene_open = editor_config.last_open_scene;

        if (editor_config.game_window.has_size()) {
            WindowConfig game_window_config = m_engine.get_game_window_config();
            apply_editor_window_config(editor_config.game_window, game_window_config);
            m_engine.set_game_window_config(game_window_config);
        }

        m_engine.get_lua_script_system().run_engine_folder(EDITOR_SCRIPTS_FOLDER);

        apply_style();
        m_icons.load(m_engine.get_renderer());
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
            reset_edit_session();
        }

        if (entering_play) {
            clear_world();
            m_engine.open_game_window();
            load_scene();
        }
        else if (leaving_play) {
            clear_world();
            m_engine.close_game_window();
            load_scene();
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

    void Editor::request_open_scene(const std::string& name) {
        m_pending_scene_open = name;
        request_action(EditorActionId::OpenScene);
    }

    std::vector<std::string> Editor::get_scene_names() const {
        std::vector<std::string> names;

        const sol::object result = editor_call(m_engine, "get_scene_names");
        if (result.is<sol::table>()) {
            const sol::table table = result.as<sol::table>();
            names.reserve(table.size());
            for (int32_t i = 1; i <= table.size(); ++i) {
                names.push_back(table.get<std::string>(i));
            }
        }

        return names;
    }

    const std::string& Editor::get_current_scene() const {
        return m_current_scene;
    }

    bool Editor::is_scene_dirty() const {
        const sol::object result = editor_call(m_engine, "is_scene_dirty");
        return result.is<bool>() && result.as<bool>();
    }

    void Editor::open_pending_scene() {
        if (m_pending_scene_open.empty()) {
            return;
        }

        const std::string name = std::move(m_pending_scene_open);
        m_pending_scene_open.clear();

        const sol::object result = editor_call(m_engine, "open_scene", name);
        if (!result.is<bool>() || !result.as<bool>()) {
            clear_world();
            m_current_scene.clear();
            reset_edit_session();
            return;
        }

        m_current_scene = name;
        reset_edit_session();
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

    const EditorIcons& Editor::get_icons() const {
        return m_icons;
    }

    EditorMenuBar& Editor::get_menu_bar() {
        return m_menu_bar;
    }

    const EditorMenuBar& Editor::get_menu_bar() const {
        return m_menu_bar;
    }

    EditorToolbar& Editor::get_toolbar() {
        return m_toolbar;
    }

    const EditorToolbar& Editor::get_toolbar() const {
        return m_toolbar;
    }

    EditorDockSceneView& Editor::get_scene_view() {
        return m_scene_view;
    }

    const EditorDockSceneView& Editor::get_scene_view() const {
        return m_scene_view;
    }

    EditorDockHierarchy& Editor::get_hierarchy() {
        return m_hierarchy;
    }

    const EditorDockHierarchy& Editor::get_hierarchy() const {
        return m_hierarchy;
    }

    EditorDockInspector& Editor::get_inspector() {
        return m_inspector;
    }

    const EditorDockInspector& Editor::get_inspector() const {
        return m_inspector;
    }

    EditorDockAssets& Editor::get_assets() {
        return m_assets;
    }

    const EditorDockAssets& Editor::get_assets() const {
        return m_assets;
    }

    void Editor::init() {
        const std::vector<std::string> names = get_scene_names();
        if (names.empty()) {
            log::engine.info("Editor: the project defines no scenes; starting with an empty world");
            return;
        }

        if (std::find(names.begin(), names.end(), m_pending_scene_open) == names.end()) {
            m_pending_scene_open = names.front();
        }

        open_pending_scene();
    }

    void Editor::end_frame() {
        m_actions.flush(*this);
    }

    void Editor::tick(float delta_time) {
        update_input();
        update_window_title();

        const bool simulate = (m_state == EditorState::Play) || (m_state == EditorState::Paused && m_step_requested);
        m_step_requested = false;

        m_engine.set_simulation_enabled(simulate);
        m_engine.set_game_input_enabled(m_state == EditorState::Play);

        prune_selection();
    }

    void Editor::draw_gui() {
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
    }

    void Editor::render_passes() {
        m_scene_view.render_pass(*this);
    }

    void Editor::on_lua_hot_reloaded() {
        m_engine.get_lua_script_system().run_engine_folder(EDITOR_SCRIPTS_FOLDER);
        clear_asset_entry_cache();

        const sol::object rebound = editor_call(m_engine, "rebind_instance_defs");
        if (rebound.is<bool>() && !rebound.as<bool>()) {
            request_open_scene(m_current_scene);
        }
    }

    bool Editor::on_quit_requested() {
        const Window* game_window = m_engine.get_game_window();
        if (game_window == nullptr || !game_window->has_focus()) {
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

    void Editor::update_window_title() {
        const std::string project = PathUtils::get_project_root().filename().string();

        std::string title;
        if (!m_current_scene.empty()) {
            title = is_scene_dirty() ? "(*) " : "";
            title += m_current_scene;
            title += " - ";
        }

        title += project;
        title += " - ";
        title += EDITOR_WINDOW_TITLE;

        if (title != m_window_title) {
            m_window_title = title;
            m_engine.get_main_window().set_title(m_window_title);
        }
    }

    void Editor::update_input() {
        m_active_contexts = 0;

        const bool has_focus = m_engine.get_main_window().has_focus() || m_engine.get_play_window().has_focus();
        if (has_focus && !ImGui::GetIO().WantTextInput) {
            m_active_contexts |= context_bit(EditorActionContext::Global);

            // Last frame's, since draw() writes them after this runs and a dock rect only exists mid-draw.
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

    void Editor::reset_edit_session() {
        m_commands.clear();
        m_selection.clear();
        m_scene_view.reset_pick_cycle();
        m_scene_view.reset_gizmo();
        m_inspector.reset_edit_state();
    }

    void Editor::clear_world() {
        editor_call(m_engine, "clear_world");
    }

    void Editor::load_scene() {
        editor_call(m_engine, "load_scene");
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

        ImGui::DockBuilderDockWindow(m_hierarchy.get_name().c_str(), right);
        ImGui::DockBuilderDockWindow(m_inspector.get_name().c_str(), inspector);
        ImGui::DockBuilderDockWindow(m_assets.get_name().c_str(), assets);
        ImGui::DockBuilderDockWindow(m_scene_view.get_name().c_str(), center);

        ImGui::DockBuilderFinish(dock_space_id);
    }

    void Editor::save_layout() {
        EditorConfig editor_config;
        editor_config.main_window = create_editor_window_config_from_window(m_engine.get_main_window());
        editor_config.game_window = create_editor_window_config_from_window_config(m_engine.get_game_window_config());
        editor_config.last_open_scene = m_current_scene;
        editor_config.save(get_editor_config_file_path());
    }
} // namespace hob::editor
