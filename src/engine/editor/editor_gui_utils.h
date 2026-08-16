#pragma once

#include <imgui.h>

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

    // Leaves the cursor ready for a full-width widget.
    // The widget that follows takes a hidden label ("##named_id"), since the visible one is here.
    void inspector_field_label(const char* label);
} // namespace hob::editor
