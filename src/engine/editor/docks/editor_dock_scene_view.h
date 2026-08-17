#pragma once

#include <vector>

#include <SDL3/SDL_gpu.h>

#include "editor_dock.h"
#include "engine/editor/editor_camera.h"
#include "engine/entity/entity.h"
#include "engine/math/vector2.h"

struct ImDrawList;

namespace hob::editor {
    class EditorDockSceneView : public EditorDock {
        EditorCamera m_camera;

        SDL_GPUTexture* m_color_target = nullptr;
        uint32_t m_color_target_width = 0;
        uint32_t m_color_target_height = 0;

        // Recorded while drawing, consumed by the next frame's input phase.
        SceneRect m_rect;
        bool m_rect_valid = false;

        // Clicking the same spot repeatedly cycles through overlapping candidates.
        Vector2 m_pick_cycle_screen_position;
        EntityId m_pick_cycle_last_entity_id = INVALID_ENTITY_ID;

    public:
        EditorDockSceneView();

        void update_input(Editor& editor);
        void draw(Editor& editor) override;
        void render_pass(Editor& editor);
        void release_color_target(Editor& editor);

        void focus_on_selection(const Editor& editor);
        void reset_pick_cycle();

    private:
        void ensure_color_target(Editor& editor, uint32_t width, uint32_t height);

        void handle_pick(Editor& editor, const Vector2& mouse_screen_pos, const Vector2& mouse_world_pos);
        void gather_pick_candidates(const Editor& editor,
                                    const Vector2& world_pos,
                                    std::vector<EntityId>& out_candidates) const;

        void draw_grid(ImDrawList* draw_list, const SceneRect& scene_rect) const;
        void draw_camera_view_rect(const Editor& editor, ImDrawList* draw_list, const SceneRect& scene_rect) const;
        void draw_selection_overlay(const Editor& editor, ImDrawList* draw_list, const SceneRect& scene_rect) const;
    };
} // namespace hob::editor
