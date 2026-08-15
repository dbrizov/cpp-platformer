#include <algorithm>
#include <cmath>
#include <optional>

#include <imgui.h>

#include "editor.h"
#include "editor_gui_utils.h"
#include "editor_style.h"
#include "engine/components/camera_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/core/systems/renderer/renderer.h"
#include "engine/math/aabb.h"
#include "engine/math/constants.h"
#include "engine/math/matrix2x3.h"

// Four coordinate spaces here:
// - screen px  : absolute ImGui coordinates, y-down. What ImDrawList consumes.
// - panel px   : pixels relative to the SceneView image's top-left, y-down. What EditorCamera converts to and from.
// - world m    : world meters, y-up.
// - local m    : meters in a sprite's unrotated axes, relative to its own world origin.
namespace hob::editor {
    namespace {
        constexpr float MIN_PANEL_SIZE_PX = 8.0f;

        constexpr float MIN_GRID_SPACING_PX = 24.0f;
        constexpr float GRID_SPACING_STEP_FACTOR = 5.0f;

        const ImU32 GRID_MINOR_COLOR = IM_COL32(255, 255, 255, 24);
        const ImU32 GRID_AXIS_COLOR = IM_COL32(255, 255, 255, 90);

        // Hit radius around a sprite-less entity's origin. Independent of the marker drawn for it,
        // which is a touch larger so the outline sits just outside what it selects.
        constexpr float PICK_RADIUS_PX = 12.0f;

        // How far the cursor may move and still count as "clicking the same spot", which is what
        // steps to the next candidate under the previous pick.
        constexpr float PICK_CYCLE_TOLERANCE_PX = 5.0f;

        // Half-extent, in meters, of the box F frames around a sprite-less entity.
        constexpr float FOCUS_FALLBACK_EXTENT = 0.5f;

        ImVec2 panel_to_screen(const Vector2& panel_pos, const Vector2& panel_to_screen_offset) {
            return ImVec2(panel_pos.x + panel_to_screen_offset.x, panel_pos.y + panel_to_screen_offset.y);
        }

        ImVec2 world_to_screen(const EditorCamera& camera,
                               const Vector2& world_pos,
                               const Vector2& panel_to_screen_offset,
                               const Vector2& panel_size) {
            return panel_to_screen(camera.world_to_panel(world_pos, panel_size), panel_to_screen_offset);
        }

        struct SpriteRect {
            Vector2 origin;
            float rotation = 0.0f;
            Vector2 min;
            Vector2 max;
        };

        bool compute_sprite_rect(const SpriteComponent& sprite, SpriteRect& out_rect) {
            if (sprite.get_texture() == nullptr) {
                return false;
            }

            const Matrix2x3& world_matrix = sprite.get_entity().get_transform()->get_world_matrix();
            const Vector2 world_size = sprite.get_world_size();
            const Vector2 pivot = sprite.get_pivot();

            out_rect.origin = world_matrix.origin;
            out_rect.rotation = world_matrix.get_rotation();
            out_rect.min = Vector2(-pivot.x * world_size.x, -pivot.y * world_size.y);
            out_rect.max = Vector2((1.0f - pivot.x) * world_size.x, (1.0f - pivot.y) * world_size.y);

            return true;
        }

        bool is_point_inside_sprite_rect(const SpriteRect& rect, const Vector2& world_pos) {
            const Vector2 world_delta = world_pos - rect.origin;
            const float cos = std::cos(-rect.rotation);
            const float sin = std::sin(-rect.rotation);
            const Vector2 local_pos(world_delta.x * cos - world_delta.y * sin,
                                    world_delta.x * sin + world_delta.y * cos);

            return local_pos.x >= rect.min.x && local_pos.x <= rect.max.x && local_pos.y >= rect.min.y &&
                   local_pos.y <= rect.max.y;
        }

        AABB compute_entity_world_bounds(const Entity& entity) {
            const SpriteComponent* sprite = entity.get_component<SpriteComponent>();

            SpriteRect rect;
            if (sprite != nullptr && compute_sprite_rect(*sprite, rect)) {
                const Vector2 origin = rect.origin;
                const float rotation = rect.rotation;
                const Vector2 min = rect.min;
                const Vector2 max = rect.max;

                const Vector2 bottom_left = Vector2::rotate_around(origin + Vector2(min.x, min.y), origin, rotation);
                const Vector2 bottom_right = Vector2::rotate_around(origin + Vector2(max.x, min.y), origin, rotation);
                const Vector2 top_right = Vector2::rotate_around(origin + Vector2(max.x, max.y), origin, rotation);
                const Vector2 top_left = Vector2::rotate_around(origin + Vector2(min.x, max.y), origin, rotation);

                const Vector2 world_corners[4] = {bottom_left, bottom_right, top_right, top_left};

                Vector2 world_min = world_corners[0];
                Vector2 world_max = world_corners[0];
                for (int i = 1; i < 4; ++i) {
                    world_min =
                        Vector2(std::min(world_min.x, world_corners[i].x), std::min(world_min.y, world_corners[i].y));
                    world_max =
                        Vector2(std::max(world_max.x, world_corners[i].x), std::max(world_max.y, world_corners[i].y));
                }

                return AABB::from_min_max(world_min, world_max);
            }

            return AABB(entity.get_transform()->get_position(), Vector2(FOCUS_FALLBACK_EXTENT, FOCUS_FALLBACK_EXTENT));
        }
    } // namespace

    void Editor::draw_scene_view() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        const bool visible = begin_panel(PANEL_SCENE, ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleVar();

        if (visible) {
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > MIN_PANEL_SIZE_PX && avail.y > MIN_PANEL_SIZE_PX) {
                ensure_scene_color_target(static_cast<uint32_t>(avail.x), static_cast<uint32_t>(avail.y));

                if (m_scene_color_target != nullptr) {
                    const ImVec2 image_size(static_cast<float>(m_scene_color_target_width),
                                            static_cast<float>(m_scene_color_target_height));

                    ImGui::Image(m_scene_color_target, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

                    const ImVec2 image_min = ImGui::GetItemRectMin();
                    const Vector2 panel_to_screen_offset(image_min.x, image_min.y);
                    const Vector2 panel_size(image_size.x, image_size.y);

                    if (ImGui::IsItemHovered()) {
                        handle_scene_view_input(panel_to_screen_offset, panel_size);
                    }

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->PushClipRect(
                        image_min, ImVec2(image_min.x + image_size.x, image_min.y + image_size.y), true);
                    draw_grid(draw_list, panel_to_screen_offset, panel_size);
                    draw_camera_view_rect(draw_list, panel_to_screen_offset, panel_size);
                    draw_selection_overlay(draw_list, panel_to_screen_offset, panel_size);
                    draw_list->PopClipRect();
                }
            }
        }
        end_panel();
    }

    void Editor::handle_scene_view_input(const Vector2& panel_to_screen_offset, const Vector2& panel_size) {
        const ImGuiIO& io = ImGui::GetIO();
        const Vector2 mouse_panel_pos(io.MousePos.x - panel_to_screen_offset.x,
                                      io.MousePos.y - panel_to_screen_offset.y);

        if (io.MouseWheel != 0.0f) {
            m_camera.zoom_at(mouse_panel_pos, panel_size, io.MouseWheel);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
            m_camera.pan_by_panel_delta(Vector2(io.MouseDelta.x, io.MouseDelta.y));
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            handle_scene_view_pick(mouse_panel_pos, m_camera.panel_to_world(mouse_panel_pos, panel_size));
        }

        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
            focus_camera_on_selection(panel_size);
        }
    }

    void Editor::handle_scene_view_pick(const Vector2& mouse_panel_pos, const Vector2& mouse_world_pos) {
        const bool additive = ImGui::GetIO().KeyCtrl;

        std::vector<EntityId> candidates;
        gather_pick_candidates(mouse_world_pos, candidates);

        if (candidates.empty()) {
            if (!additive) {
                m_selection.clear();
                m_range_selection_anchor = INVALID_ENTITY_ID;
            }

            m_pick_cycle_last_entity_id = INVALID_ENTITY_ID;
            return;
        }

        // Clicking the same spot again steps to the next candidate underneath the previous one.
        size_t index = 0;
        const Vector2 cycle_panel_delta = mouse_panel_pos - m_pick_cycle_panel_position;
        if (cycle_panel_delta.length() <= PICK_CYCLE_TOLERANCE_PX) {
            const auto it = std::find(candidates.begin(), candidates.end(), m_pick_cycle_last_entity_id);
            if (it != candidates.end()) {
                index = (static_cast<size_t>(it - candidates.begin()) + 1) % candidates.size();
            }
        }

        const EntityId picked_entity_id = candidates[index];
        m_pick_cycle_panel_position = mouse_panel_pos;
        m_pick_cycle_last_entity_id = picked_entity_id;

        if (additive) {
            m_selection.toggle(picked_entity_id);
        }
        else {
            m_selection.set(picked_entity_id);
        }

        m_range_selection_anchor = picked_entity_id;
        m_scroll_hierarchy_to_primary = true;
    }

    void Editor::gather_pick_candidates(const Vector2& world_pos, std::vector<EntityId>& out_candidates) const {
        out_candidates.clear();

        const EntitySpawner& spawner = m_engine.get_entity_spawner();
        const std::vector<SpriteComponent*>& sprites = spawner.get_sprites();

        struct SpriteHit {
            EntityId entity_id;
            int z_index;
            int sprite_index;
        };

        std::vector<SpriteHit> sprite_hits;
        for (int i = 0; i < sprites.size(); ++i) {
            const SpriteComponent* sprite = sprites[i];

            SpriteRect rect;
            if (!compute_sprite_rect(*sprite, rect) || !is_point_inside_sprite_rect(rect, world_pos)) {
                continue;
            }

            sprite_hits.push_back(SpriteHit{sprite->get_entity().get_id(), sprite->get_z_index(), i});
        }

        if (!sprite_hits.empty()) {
            // Topmost first: higher z wins, and at equal z the later-registered sprite draws on top.
            std::sort(sprite_hits.begin(), sprite_hits.end(), [](const SpriteHit& a, const SpriteHit& b) {
                return (a.z_index != b.z_index) ? (a.z_index > b.z_index) : (a.sprite_index > b.sprite_index);
            });

            for (const SpriteHit& hit : sprite_hits) {
                out_candidates.push_back(hit.entity_id);
            }

            return;
        }

        // Sprite-less entities:
        // a fixed screen radius around the origin, so the grab area stays the same size on screen at any zoom.
        const float pick_radius_world = PICK_RADIUS_PX / m_camera.pixels_per_meter;

        struct OriginHit {
            EntityId entity_id;
            float world_distance;
        };

        std::vector<Entity*> entities;
        spawner.get_entities(entities);

        std::vector<OriginHit> origin_hits;
        for (const Entity* entity : entities) {
            const float world_distance = (entity->get_transform()->get_position() - world_pos).length();
            if (world_distance <= pick_radius_world) {
                origin_hits.push_back(OriginHit{entity->get_id(), world_distance});
            }
        }

        std::sort(origin_hits.begin(), origin_hits.end(), [](const OriginHit& a, const OriginHit& b) {
            return a.world_distance < b.world_distance;
        });

        for (const OriginHit& hit : origin_hits) {
            out_candidates.push_back(hit.entity_id);
        }
    }

    void Editor::focus_camera_on_selection(const Vector2& panel_size) {
        const EntitySpawner& spawner = m_engine.get_entity_spawner();

        std::optional<AABB> world_bounds;
        for (EntityId id : m_selection.ids) {
            const Entity* entity = spawner.get_entity(id);
            if (entity == nullptr) {
                continue;
            }

            const AABB entity_bounds = compute_entity_world_bounds(*entity);
            world_bounds = world_bounds.has_value() ? AABB::combine(*world_bounds, entity_bounds) : entity_bounds;
        }

        if (world_bounds.has_value()) {
            m_camera.focus_on(*world_bounds, panel_size);
        }
    }

    void Editor::prune_selection() {
        const EntitySpawner& spawner = m_engine.get_entity_spawner();

        std::erase_if(m_selection.ids, [&spawner](EntityId id) {
            return spawner.get_entity(id) == nullptr;
        });

        if (m_range_selection_anchor != INVALID_ENTITY_ID && spawner.get_entity(m_range_selection_anchor) == nullptr) {
            m_range_selection_anchor = INVALID_ENTITY_ID;
        }
    }

    void Editor::draw_grid(ImDrawList* draw_list,
                           const Vector2& panel_to_screen_offset,
                           const Vector2& panel_size) const {
        const float ppm = m_camera.pixels_per_meter;

        float world_step = 1.0f;
        while (world_step * ppm < MIN_GRID_SPACING_PX) {
            world_step *= GRID_SPACING_STEP_FACTOR;
        }

        // Y is inverted, so the world minimum is at the panel's bottom-left corner.
        const Vector2 world_min = m_camera.panel_to_world(Vector2(0.0f, panel_size.y), panel_size);
        const Vector2 world_max = m_camera.panel_to_world(Vector2(panel_size.x, 0.0f), panel_size);

        const float world_start_x = std::floor(world_min.x / world_step) * world_step;
        for (float world_x = world_start_x; world_x <= world_max.x; world_x += world_step) {
            const Vector2 panel_pos = m_camera.world_to_panel(Vector2(world_x, 0.0f), panel_size);
            const float screen_x = panel_to_screen_offset.x + panel_pos.x;
            const ImU32 color = (std::abs(world_x) < world_step * 0.5f) ? GRID_AXIS_COLOR : GRID_MINOR_COLOR;
            draw_list->AddLine(ImVec2(screen_x, panel_to_screen_offset.y),
                               ImVec2(screen_x, panel_to_screen_offset.y + panel_size.y),
                               color);
        }

        const float world_start_y = std::floor(world_min.y / world_step) * world_step;
        for (float world_y = world_start_y; world_y <= world_max.y; world_y += world_step) {
            const Vector2 panel_pos = m_camera.world_to_panel(Vector2(0.0f, world_y), panel_size);
            const float screen_y = panel_to_screen_offset.y + panel_pos.y;
            const ImU32 color = (std::abs(world_y) < world_step * 0.5f) ? GRID_AXIS_COLOR : GRID_MINOR_COLOR;
            draw_list->AddLine(ImVec2(panel_to_screen_offset.x, screen_y),
                               ImVec2(panel_to_screen_offset.x + panel_size.x, screen_y),
                               color);
        }
    }

    void Editor::draw_camera_view_rect(ImDrawList* draw_list,
                                       const Vector2& panel_to_screen_offset,
                                       const Vector2& panel_size) const {
        const CameraComponent* camera = m_engine.get_active_camera();
        if (camera == nullptr) {
            return;
        }

        const Renderer& renderer = m_engine.get_renderer();
        const Vector2 view_size_px =
            (m_engine.get_game_window() != nullptr) ? renderer.get_logical_size() : renderer.get_reference_size();

        const float camera_ppm = camera->get_pixels_per_meter();
        if (view_size_px.x <= EPSILON || view_size_px.y <= EPSILON || camera_ppm <= EPSILON) {
            return;
        }

        const AABB view_bounds(camera->get_entity().get_transform()->get_position(), view_size_px * 0.5f / camera_ppm);

        // World +Y is up but panel +Y is down, so the world top-left corner is (min.x, max.y).
        const Vector2 top_left_panel_pos =
            m_camera.world_to_panel(Vector2(view_bounds.min().x, view_bounds.max().y), panel_size);
        const Vector2 bottom_right_panel_pos =
            m_camera.world_to_panel(Vector2(view_bounds.max().x, view_bounds.min().y), panel_size);

        draw_list->AddRect(panel_to_screen(top_left_panel_pos, panel_to_screen_offset),
                           panel_to_screen(bottom_right_panel_pos, panel_to_screen_offset),
                           ImGui::GetColorU32(COLOR_CAMERA_VIEW_RECT),
                           0.0f,
                           ImDrawFlags_None,
                           CAMERA_VIEW_RECT_THICKNESS);
    }

    void Editor::draw_selection_overlay(ImDrawList* draw_list,
                                        const Vector2& panel_to_screen_offset,
                                        const Vector2& panel_size) const {
        const EntitySpawner& spawner = m_engine.get_entity_spawner();
        const EntityId primary_entity_id = m_selection.primary();

        // m_selection.ids is ordered with the primary last, so it naturally paints on top.
        for (EntityId id : m_selection.ids) {
            const Entity* entity = spawner.get_entity(id);
            if (entity == nullptr) {
                continue;
            }

            const ImU32 color =
                ImGui::GetColorU32((id == primary_entity_id) ? COLOR_SELECTION_PRIMARY : COLOR_SELECTION);

            const SpriteComponent* sprite = entity->get_component<SpriteComponent>();

            SpriteRect rect;
            if (sprite != nullptr && compute_sprite_rect(*sprite, rect)) {
                const Vector2 origin = rect.origin;
                const float rotation = rect.rotation;
                const Vector2 min = rect.min;
                const Vector2 max = rect.max;

                const Vector2 bottom_left = Vector2::rotate_around(origin + Vector2(min.x, min.y), origin, rotation);
                const Vector2 bottom_right = Vector2::rotate_around(origin + Vector2(max.x, min.y), origin, rotation);
                const Vector2 top_right = Vector2::rotate_around(origin + Vector2(max.x, max.y), origin, rotation);
                const Vector2 top_left = Vector2::rotate_around(origin + Vector2(min.x, max.y), origin, rotation);

                const ImVec2 screen_corners[4] = {
                    world_to_screen(m_camera, bottom_left, panel_to_screen_offset, panel_size),
                    world_to_screen(m_camera, bottom_right, panel_to_screen_offset, panel_size),
                    world_to_screen(m_camera, top_right, panel_to_screen_offset, panel_size),
                    world_to_screen(m_camera, top_left, panel_to_screen_offset, panel_size),
                };

                draw_list->AddPolyline(screen_corners, 4, color, ImDrawFlags_Closed, SELECTION_OUTLINE_THICKNESS);
            }
            else {
                const Vector2 world_pos = entity->get_transform()->get_position();

                draw_list->AddCircle(world_to_screen(m_camera, world_pos, panel_to_screen_offset, panel_size),
                                     SELECTION_MARKER_RADIUS_PX,
                                     color,
                                     0,
                                     SELECTION_OUTLINE_THICKNESS);
            }
        }
    }
} // namespace hob::editor
