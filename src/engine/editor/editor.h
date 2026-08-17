#pragma once

#include <string>

#include <imgui.h>

#include "commands/editor_command_stack.h"
#include "editor_action_queue.h"
#include "editor_assets.h"
#include "editor_entity_selection.h"
#include "editor_hierarchy.h"
#include "editor_inspector.h"
#include "editor_menu_bar.h"
#include "editor_scene_view.h"
#include "editor_toolbar.h"
#include "engine/core/engine_hooks.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    enum class EditorState {
        Edit,
        Play,
        Paused,
    };

    class Editor : public EngineHooks {
        Engine& m_engine;

        std::string m_imgui_ini_path;
        bool m_reset_layout = false;

        EditorState m_state = EditorState::Edit;
        bool m_step_requested = false;

        EditorEntitySelection m_selection;

        EditorCommandStack m_commands;

        EditorActionQueue m_actions;
        uint32_t m_active_contexts = 0;

        EditorMenuBar m_menu_bar;
        EditorToolbar m_toolbar;
        EditorSceneView m_scene_view;
        EditorHierarchy m_hierarchy;
        EditorInspector m_inspector;
        EditorAssets m_assets;

    public:
        explicit Editor(Engine& engine);
        ~Editor() override;

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;

        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        Engine& get_engine() const;

        EditorState get_state() const;
        void set_state(EditorState state);

        void request_step();
        void request_reset_layout();
        void request_quit();
        void request_action(EditorActionId id);

        EditorEntitySelection& get_selection();
        const EditorEntitySelection& get_selection() const;

        EditorCommandStack& get_commands();
        const EditorCommandStack& get_commands() const;

        EditorMenuBar& get_menu_bar();
        EditorToolbar& get_toolbar();
        EditorSceneView& get_scene_view();
        EditorHierarchy& get_hierarchy();
        EditorInspector& get_inspector();
        EditorAssets& get_assets();

#pragma region EngineHooks
        void tick(float delta_time) override;
        void draw_gui() override;
        void render_passes() override;
        void on_lua_hot_reloaded() override;
        bool on_quit_requested() override;
        bool on_window_close_requested(SDL_WindowID window_id) override;
#pragma endregion

    private:
        void update_input();
        bool is_context_active(EditorActionContext context) const;
        void prune_selection();

        void build_default_layout(ImGuiID dock_space_id);
        void save_layout();
    };
} // namespace hob::editor
