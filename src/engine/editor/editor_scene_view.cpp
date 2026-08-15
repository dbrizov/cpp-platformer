#include <cmath>

#include <imgui.h>

#include "editor.h"
#include "editor_gui_utils.h"

namespace hob::editor {
    namespace {
        constexpr float MIN_PANEL_SIZE_PX = 8.0f;

        constexpr float MIN_GRID_SPACING_PX = 24.0f;
        constexpr float GRID_SPACING_STEP_FACTOR = 5.0f;

        const ImU32 GRID_MINOR_COLOR = IM_COL32(255, 255, 255, 24);
        const ImU32 GRID_AXIS_COLOR = IM_COL32(255, 255, 255, 90);
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
                    const Vector2 panel_pos(image_min.x, image_min.y);
                    const Vector2 panel_size(image_size.x, image_size.y);

                    if (ImGui::IsItemHovered()) {
                        handle_scene_view_input(panel_pos, panel_size);
                    }

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->PushClipRect(
                        image_min, ImVec2(image_min.x + image_size.x, image_min.y + image_size.y), true);
                    draw_grid(draw_list, panel_pos, panel_size);
                    draw_list->PopClipRect();
                }
            }
        }
        end_panel();
    }

    void Editor::handle_scene_view_input(const Vector2& panel_pos, const Vector2& panel_size) {
        const ImGuiIO& io = ImGui::GetIO();
        const Vector2 mouse_panel(io.MousePos.x - panel_pos.x, io.MousePos.y - panel_pos.y);

        if (io.MouseWheel != 0.0f) {
            m_camera.zoom_at(mouse_panel, panel_size, io.MouseWheel);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f) ||
            ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
            m_camera.pan_by_panel_delta(Vector2(io.MouseDelta.x, io.MouseDelta.y));
        }
    }

    void Editor::draw_grid(ImDrawList* draw_list, const Vector2& panel_pos, const Vector2& panel_size) const {
        const float ppm = m_camera.pixels_per_meter;

        float step = 1.0f;
        while (step * ppm < MIN_GRID_SPACING_PX) {
            step *= GRID_SPACING_STEP_FACTOR;
        }

        // Y is inverted, so the world minimum is at the panel's bottom-left corner.
        const Vector2 world_min = m_camera.panel_to_world(Vector2(0.0f, panel_size.y), panel_size);
        const Vector2 world_max = m_camera.panel_to_world(Vector2(panel_size.x, 0.0f), panel_size);

        const float start_x = std::floor(world_min.x / step) * step;
        for (float x = start_x; x <= world_max.x; x += step) {
            const Vector2 p = m_camera.world_to_panel(Vector2(x, 0.0f), panel_size);
            const ImU32 color = (std::abs(x) < step * 0.5f) ? GRID_AXIS_COLOR : GRID_MINOR_COLOR;
            draw_list->AddLine(
                ImVec2(panel_pos.x + p.x, panel_pos.y), ImVec2(panel_pos.x + p.x, panel_pos.y + panel_size.y), color);
        }

        const float start_y = std::floor(world_min.y / step) * step;
        for (float y = start_y; y <= world_max.y; y += step) {
            const Vector2 p = m_camera.world_to_panel(Vector2(0.0f, y), panel_size);
            const ImU32 color = (std::abs(y) < step * 0.5f) ? GRID_AXIS_COLOR : GRID_MINOR_COLOR;
            draw_list->AddLine(
                ImVec2(panel_pos.x, panel_pos.y + p.y), ImVec2(panel_pos.x + panel_size.x, panel_pos.y + p.y), color);
        }
    }
} // namespace hob::editor
