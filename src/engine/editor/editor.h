#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <imgui.h>

#include "editor_camera.h"
#include "editor_entity_selection.h"
#include "engine/math/vector2.h"

struct ImDrawList;

namespace hob {
    class Engine;
    class TransformComponent;
} // namespace hob

namespace hob::editor {
    class Editor {
    public:
        enum class State {
            Edit,
            Play,
            Paused,
        };

    private:
        Engine& m_engine;

        std::string m_imgui_ini_path;
        bool m_reset_layout = false;

        State m_state = State::Edit;
        bool m_step_requested = false;
        bool m_simulate_this_frame = false;

        EditorCamera m_camera;

        EntitySelection m_selection;
        EntityId m_range_selection_anchor = INVALID_ENTITY_ID;
        bool m_scroll_hierarchy_to_primary = false;

        // Clicking the same spot repeatedly cycles through overlapping candidates.
        Vector2 m_pick_cycle_screen_position;
        EntityId m_pick_cycle_last_entity_id = INVALID_ENTITY_ID;

        SDL_GPUTexture* m_scene_color_target = nullptr;
        uint32_t m_scene_color_target_width = 0;
        uint32_t m_scene_color_target_height = 0;

    public:
        static constexpr const char* PANEL_SCENE = " Scene ###Scene";
        static constexpr const char* PANEL_HIERARCHY = " Hierarchy ###Hierarchy";
        static constexpr const char* PANEL_INSPECTOR = " Inspector ###Inspector";
        static constexpr const char* PANEL_ASSETS = " Assets ###Assets";

        explicit Editor(Engine& engine);
        ~Editor();

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;

        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        void set_state(State state);

        bool is_simulating() const;

        bool wants_game_input() const;

        void tick(float delta_time);
        void draw_gui();
        void render_passes();

    private:
        void draw_menu_bar();
        void draw_toolbar();

        void draw_scene_view();
        void draw_hierarchy();
        void draw_inspector();
        void draw_assets();

        void draw_hierarchy_entity(const TransformComponent* transform,
                                   std::vector<EntityId>& visible_order,
                                   EntityId& out_clicked_entity_id);
        void apply_hierarchy_click(EntityId entity_id, const std::vector<EntityId>& visible_order);

        void ensure_scene_color_target(uint32_t width, uint32_t height);
        void release_scene_color_target();
        void handle_scene_view_input(const SceneRect& scene_rect);
        void handle_scene_view_pick(const Vector2& mouse_screen_pos, const Vector2& mouse_world_pos);
        void gather_pick_candidates(const Vector2& world_pos, std::vector<EntityId>& out_candidates) const;
        void focus_camera_on_selection(const SceneRect& scene_rect);
        void prune_selection();

        void draw_grid(ImDrawList* draw_list, const SceneRect& scene_rect) const;
        void draw_camera_view_rect(ImDrawList* draw_list, const SceneRect& scene_rect) const;
        void draw_selection_overlay(ImDrawList* draw_list, const SceneRect& scene_rect) const;

        void build_default_layout(ImGuiID dock_space_id);
        void save_layout();
    };
} // namespace hob::editor
