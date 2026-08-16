#include "editor_gui_utils.h"

#include <concepts>
#include <cstdarg>
#include <cstdio>

#include <imgui_internal.h>

#include "editor_style.h"
#include "engine/math/constants.h"

namespace hob::editor {
    namespace {
        constexpr int DRAW_CHANNEL_COUNT = 2;
        constexpr int DRAW_CHANNEL_BACKGROUND = 0;
        constexpr int DRAW_CHANNEL_FOREGROUND = 1;

        constexpr const char* EMPTY_LABEL = "##";
        constexpr uint32_t FIELD_STRING_CAPACITY = 256;

        template<std::totally_ordered T>
        ImGuiSliderFlags clamp_flags(T min, T max) {
            return min < max ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;
        }

        ImRect inset_rect(const ImVec2& min, const ImVec2& max, const ImVec2& inset) {
            return ImRect(min.x + inset.x, min.y + inset.y, max.x - inset.x, max.y - inset.y);
        }

        // The row highlight spans the whole panel, not just the indented item rect.
        ImRect row_rect(const ImVec2& inset) {
            const float window_min_x = ImGui::GetWindowPos().x;
            const float window_max_x = window_min_x + ImGui::GetWindowWidth();

            return inset_rect(ImVec2(window_min_x, ImGui::GetItemRectMin().y),
                              ImVec2(window_max_x, ImGui::GetItemRectMax().y),
                              inset);
        }

        void draw_highlight(ImDrawList* draw_list, const ImRect& rect, const ImVec4& color, float rounding) {
            draw_list->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(color), rounding);
        }
    } // namespace

    ImGuiID dock_space_over_viewport(ImGuiDockNodeFlags flags) {
        const ImGuiStyle& style = ImGui::GetStyle();

        StyleVarStack vars;
        vars.push(ImGuiStyleVar_ItemInnerSpacing, ImVec2(TAB_SPACING_X, style.ItemInnerSpacing.y));
        vars.push(ImGuiStyleVar_FramePadding, ImVec2(TAB_PADDING_X, style.FramePadding.y));

        const ImGuiID dock_space_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), flags);
        vars.pop();

        return dock_space_id;
    }

    bool begin_panel(const char* name, ImGuiWindowFlags flags) {
        const ImGuiWindow* window = ImGui::FindWindowByName(name);
        const bool is_selected_tab = window && window->DockTabIsVisible;

        ImGui::PushStyleColor(ImGuiCol_TabHovered, is_selected_tab ? COLOR_BG_BASE : COLOR_BG_HOVER);
        const bool visible = ImGui::Begin(name, nullptr, flags);
        ImGui::PopStyleColor();

        return visible;
    }

    void end_panel() {
        ImGui::End();
    }

    bool begin_menu(const char* label) {
        const ImGuiStyle& style = ImGui::GetStyle();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, DRAW_CHANNEL_COUNT);
        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_FOREGROUND);

        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderHovered, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderActive, COLOR_TRANSPARENT);

        StyleVarStack vars;
        vars.push(ImGuiStyleVar_ItemSpacing, ImVec2(BAR_ITEM_PADDING_X, style.ItemSpacing.y));
        vars.push(ImGuiStyleVar_WindowPadding, MENU_POPUP_PADDING);

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + BAR_ITEM_SPACING_X);
        const bool open = ImGui::BeginMenu(label);
        vars.pop();
        colors.pop();

        if (open || ImGui::IsItemHovered()) {
            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(draw_list,
                           ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()),
                           open ? COLOR_MENU_ACTIVE : COLOR_MENU_HOVER,
                           MENU_ITEM_ROUNDING);
        }

        splitter.Merge(draw_list);

        if (open) {
            ImGui::PushStyleColor(ImGuiCol_Separator, COLOR_MENU_SEPARATOR);
        }

        return open;
    }

    void end_menu() {
        ImGui::PopStyleColor();
        ImGui::EndMenu();
    }

    float bar_button_width(const char* label) {
        return ImGui::CalcTextSize(label).x + BAR_ITEM_PADDING_X * 2.0f + BAR_ITEM_SPACING_X;
    }

    bool bar_button(const char* label) {
        const ImGuiStyle& style = ImGui::GetStyle();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, DRAW_CHANNEL_COUNT);
        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_FOREGROUND);

        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderHovered, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderActive, COLOR_TRANSPARENT);

        StyleVarStack vars;
        vars.push(ImGuiStyleVar_ItemSpacing, ImVec2(BAR_ITEM_PADDING_X * 2.0f, style.ItemSpacing.y));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + BAR_ITEM_SPACING_X);
        const bool pressed = ImGui::Selectable(label, false, ImGuiSelectableFlags_None, ImGui::CalcTextSize(label));
        vars.pop();
        colors.pop();

        const ImVec4& color = ImGui::IsItemActive()    ? COLOR_BUTTON_ACTIVE
                              : ImGui::IsItemHovered() ? COLOR_BUTTON_HOVER
                                                       : COLOR_BUTTON;

        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
        draw_highlight(draw_list, ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), color, MENU_ITEM_ROUNDING);
        splitter.Merge(draw_list);

        return pressed;
    }

    bool menu_item(const char* label, const char* shortcut, bool enabled) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, DRAW_CHANNEL_COUNT);
        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_FOREGROUND);

        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderHovered, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderActive, COLOR_TRANSPARENT);

        const bool selected = false;
        const bool pressed = ImGui::MenuItem(label, shortcut, selected, enabled);
        colors.pop();

        // A disabled row is inert, so it must not light up under the cursor either.
        if (enabled && ImGui::IsItemHovered()) {
            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(draw_list, row_rect(MENU_ITEM_INSET), COLOR_MENU_HOVER, MENU_ITEM_ROUNDING);
        }

        splitter.Merge(draw_list);

        return pressed;
    }

    bool tree_item(const void* id, ImGuiTreeNodeFlags flags, bool selected, const char* fmt, ...) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, DRAW_CHANNEL_COUNT);
        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_FOREGROUND);

        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderHovered, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderActive, COLOR_TRANSPARENT);

        if (selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        va_list args;
        va_start(args, fmt);
        const bool open = ImGui::TreeNodeExV(id, flags, fmt, args);
        va_end(args);

        colors.pop();

        if (selected || ImGui::IsItemHovered()) {
            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(draw_list,
                           row_rect(TREE_ITEM_INSET),
                           selected ? COLOR_TREE_ITEM_SELECTED : COLOR_TREE_ITEM_HOVER,
                           TREE_ITEM_ROUNDING);
        }

        splitter.Merge(draw_list);

        return open;
    }

    void begin_field(const char* label) {
        ImGui::PushID(label);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine(INSPECTOR_LABEL_WIDTH);
        ImGui::SetNextItemWidth(-EPSILON);
    }

    void end_field() {
        ImGui::PopID();
    }

    bool field_float(const char* label, float& value, float drag_speed, float min, float max) {
        begin_field(label);
        const bool changed = ImGui::DragFloat(EMPTY_LABEL, &value, drag_speed, min, max, "%.3f", clamp_flags(min, max));
        end_field();

        return changed;
    }

    bool field_int(const char* label, int& value, float drag_speed, int min, int max) {
        begin_field(label);
        const bool changed = ImGui::DragInt(EMPTY_LABEL, &value, drag_speed, min, max, "%d", clamp_flags(min, max));
        end_field();

        return changed;
    }

    bool field_bool(const char* label, bool& value) {
        begin_field(label);
        const bool changed = ImGui::Checkbox(EMPTY_LABEL, &value);
        end_field();

        return changed;
    }

    bool field_string(const char* label, std::string& value) {
        char buffer[FIELD_STRING_CAPACITY];
        std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());

        begin_field(label);
        const bool changed = ImGui::InputText(EMPTY_LABEL, buffer, sizeof(buffer));
        end_field();

        if (changed) {
            value = buffer;
        }

        return changed;
    }

    bool field_vector2(const char* label, Vector2& value, float drag_speed) {
        float xy[2] = {value.x, value.y};

        begin_field(label);
        const bool changed = ImGui::DragFloat2(EMPTY_LABEL, xy, drag_speed);
        end_field();

        if (changed) {
            value = Vector2(xy[0], xy[1]);
        }

        return changed;
    }

    bool field_color(const char* label, Color& value) {
        float rgba[4] = {value.r, value.g, value.b, value.a};

        begin_field(label);
        const bool changed = ImGui::ColorEdit4(EMPTY_LABEL, rgba);
        end_field();

        if (changed) {
            value = Color(rgba[0], rgba[1], rgba[2], rgba[3]);
        }

        return changed;
    }
} // namespace hob::editor
