#pragma once

#include <string>
#include <string_view>

#include <imgui.h>

#include "editor_style.h"
#include "engine/math/color.h"
#include "engine/math/vector2.h"

namespace hob::editor {
    struct StyleColorStack {
        int count = 0;

        void push(ImGuiCol index, const ImVec4& color) {
            ImGui::PushStyleColor(index, color);
            count += 1;
        }

        void pop() {
            ImGui::PopStyleColor(count);
            count = 0;
        }
    };

    struct StyleVarStack {
        int count = 0;

        void push(ImGuiStyleVar index, const ImVec2& value) {
            ImGui::PushStyleVar(index, value);
            count += 1;
        }

        void pop() {
            ImGui::PopStyleVar(count);
            count = 0;
        }
    };

    inline ImVec2 to_imvec(const Vector2& v) {
        return ImVec2(v.x, v.y);
    }

    std::string to_display_label(std::string_view name);

    ImGuiID dock_space_over_viewport(ImGuiDockNodeFlags flags);

    bool begin_dock(const char* name, ImGuiWindowFlags flags = 0);
    void end_dock();

    bool begin_menu(const char* label);
    void end_menu();

    bool menu_item(const char* label, const char* shortcut = nullptr, bool enabled = true);

    bool bar_button(const char* label);
    float bar_button_width(const char* label);

    bool tree_item(const void* id, ImGuiTreeNodeFlags flags, bool selected, const char* fmt, ...) IM_FMTARGS(4);

    bool component_header(const char* label);

    void begin_field(const char* label);
    void end_field();

    bool field_angle(const char* label, float& degrees, float drag_speed = INSPECTOR_DRAG_SPEED_ROTATION_DEG);
    bool field_float(const char* label,
                     float& value,
                     float drag_speed = INSPECTOR_DRAG_SPEED_FLOAT,
                     float min = 0.0f,
                     float max = 0.0f);
    bool field_int(const char* label,
                   int64_t& value,
                   float drag_speed = INSPECTOR_DRAG_SPEED_INT,
                   int64_t min = 0,
                   int64_t max = 0);
    bool field_bool(const char* label, bool& value);
    bool field_string(const char* label, std::string& value);
    void field_text(const char* label, const std::string& value);
    bool field_vector2(const char* label, Vector2& value, float drag_speed = INSPECTOR_DRAG_SPEED_FLOAT);
    bool field_color(const char* label, Color& value);
} // namespace hob::editor
