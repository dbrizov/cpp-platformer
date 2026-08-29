#pragma once

#include <array>
#include <string>
#include <vector>

#include <imgui.h>

#include "actions/editor_action_queue.h"
#include "bars/editor_menu_bar.h"
#include "bars/editor_toolbar.h"
#include "commands/editor_command_stack.h"
#include "docks/editor_dock_assets.h"
#include "docks/editor_dock_hierarchy.h"
#include "docks/editor_dock_inspector.h"
#include "docks/editor_dock_output.h"
#include "docks/editor_dock_scene_view.h"
#include "editor_config.h"
#include "editor_entity_selection.h"
#include "editor_gui_utils.h"
#include "editor_icons.h"
#include "engine/core/engine_hooks.h"
#include "engine/core/world_state.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    enum class EditorPendingConfirm : uint8_t {
        None,
        OpenScene,
        Quit,
    };

    class Editor : public EngineHooks {
        Engine& m_engine;

        std::string m_imgui_ini_path;
        bool m_reset_layout = false;

        std::string m_window_title;
        std::string m_current_scene;
        std::string m_pending_scene_open;

        EditorPendingConfirm m_pending_confirm = EditorPendingConfirm::None;
        bool m_quit_confirmed = false;

        EditorEntitySelection m_selection;

        EditorCommandStack m_commands;

        EditorActionQueue m_actions;
        uint32_t m_active_contexts = 0;

        EditorIcons m_icons;

        EditorMenuBar m_menu_bar;
        EditorToolbar m_toolbar;
        EditorDockSceneView m_scene_view;
        EditorDockHierarchy m_hierarchy;
        EditorDockInspector m_inspector;
        EditorDockAssets m_assets;
        EditorDockOutput m_output;

    public:
        explicit Editor(Engine& engine);
        ~Editor() override;

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;

        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        Engine& get_engine() const;

        WorldState get_state() const;
        void set_state(WorldState state);

        void request_step();
        void request_reset_layout();
        void request_quit();
        void request_action(EditorActionId id);
        void request_open_scene(const std::string& name);

        std::vector<std::string> get_scene_names() const;
        const std::string& get_current_scene() const;
        bool is_scene_dirty() const;
        void open_pending_scene();

        EditorEntitySelection& get_selection();
        const EditorEntitySelection& get_selection() const;

        EditorCommandStack& get_commands();
        const EditorCommandStack& get_commands() const;

        const EditorIcons& get_icons() const;

        EditorMenuBar& get_menu_bar();
        const EditorMenuBar& get_menu_bar() const;

        EditorToolbar& get_toolbar();
        const EditorToolbar& get_toolbar() const;

        EditorDockSceneView& get_scene_view();
        const EditorDockSceneView& get_scene_view() const;

        EditorDockHierarchy& get_hierarchy();
        const EditorDockHierarchy& get_hierarchy() const;

        EditorDockInspector& get_inspector();
        const EditorDockInspector& get_inspector() const;

        EditorDockAssets& get_assets();
        const EditorDockAssets& get_assets() const;

        EditorDockOutput& get_output();
        const EditorDockOutput& get_output() const;

#pragma region EngineHooks
        void init() override;
        void end_frame() override;
        void tick(float delta_time) override;
        void draw_gui() override;
        void render_passes() override;
        void on_lua_hot_reloaded() override;
        bool on_quit_requested() override;
        bool on_window_close_requested(SDL_WindowID window_id) override;
#pragma endregion

    private:
        static constexpr size_t DOCK_COUNT = 5;

        std::array<EditorDock*, DOCK_COUNT> get_docks();

        void update_input();
        void update_window_title();
        bool is_context_active(EditorActionContext context) const;

        void open_scene_without_prompt(const std::string& name);
        bool try_prompt_unsaved_changes();
        void draw_unsaved_changes_modal();
        void resolve_pending_confirm(EditorModalChoice choice);

        void prune_selection();
        EditorSelectionInstanceIds capture_selection_instance_ids() const;
        void restore_selection(const EditorSelectionInstanceIds& captured);

        void reset_edit_session();
        void clear_world();
        void load_scene();

        void build_default_layout(ImGuiID dock_space_id);
        void save_layout();
    };
} // namespace hob::editor
