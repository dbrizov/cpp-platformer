#include "editor_gui_utils.h"

#include <imgui_internal.h>

#include "editor_style.h"

namespace hob::editor {
    namespace {
        constexpr int DRAW_CHANNEL_COUNT = 2;
        constexpr int DRAW_CHANNEL_BACKGROUND = 0;
        constexpr int DRAW_CHANNEL_FOREGROUND = 1;

        ImRect inset_rect(const ImVec2& min, const ImVec2& max, const ImVec2& inset) {
            return ImRect(min.x + inset.x, min.y + inset.y, max.x - inset.x, max.y - inset.y);
        }

        void draw_highlight(ImDrawList* draw_list, const ImRect& rect, const ImVec4& color) {
            draw_list->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(color), MENU_ITEM_ROUNDING);
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
                           open ? COLOR_MENU_ACTIVE : COLOR_MENU_HOVER);
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
        draw_highlight(draw_list, ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), color);
        splitter.Merge(draw_list);

        return pressed;
    }

    bool menu_item(const char* label, const char* shortcut) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, DRAW_CHANNEL_COUNT);
        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_FOREGROUND);

        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderHovered, COLOR_TRANSPARENT);
        colors.push(ImGuiCol_HeaderActive, COLOR_TRANSPARENT);

        const bool pressed = ImGui::MenuItem(label, shortcut);
        colors.pop();

        if (ImGui::IsItemHovered()) {
            const float window_min_x = ImGui::GetWindowPos().x;
            const float window_max_x = window_min_x + ImGui::GetWindowWidth();
            const ImVec2 item_min = ImGui::GetItemRectMin();
            const ImVec2 item_max = ImGui::GetItemRectMax();

            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(
                draw_list,
                inset_rect(ImVec2(window_min_x, item_min.y), ImVec2(window_max_x, item_max.y), MENU_ITEM_INSET),
                COLOR_MENU_HOVER);
        }

        splitter.Merge(draw_list);

        return pressed;
    }
} // namespace hob::editor
