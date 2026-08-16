#pragma once

#include <string>

#include <imgui.h>

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

    ImGuiID dock_space_over_viewport(ImGuiDockNodeFlags flags);

    bool begin_panel(const char* name, ImGuiWindowFlags flags = 0);
    void end_panel();

    bool begin_menu(const char* label);
    void end_menu();

    bool menu_item(const char* label, const char* shortcut = nullptr, bool enabled = true);

    bool bar_button(const char* label);
    float bar_button_width(const char* label);

    bool tree_item(const void* id, ImGuiTreeNodeFlags flags, bool selected, const char* fmt, ...) IM_FMTARGS(4);

    void begin_field(const char* label);
    void end_field();

    bool field_float(const char* label, float& value, float drag_speed, float min = 0.0f, float max = 0.0f);
    bool field_int(const char* label, int& value, float drag_speed, int min = 0, int max = 0);
    bool field_bool(const char* label, bool& value);
    bool field_string(const char* label, std::string& value);
    bool field_vector2(const char* label, Vector2& value, float drag_speed);
    bool field_color(const char* label, Color& value);
} // namespace hob::editor
