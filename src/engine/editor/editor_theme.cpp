#include "editor_theme.h"

#include <imgui_internal.h>

namespace hob::editor_theme {
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

    void apply() {
        ImGuiStyle& style = ImGui::GetStyle();

        style.FontSizeBase = FONT_SIZE_PX;

        style.WindowPadding = WINDOW_PADDING;
        style.WindowTitleAlign = WINDOW_TITLE_ALIGN;
        style.WindowRounding = WINDOW_ROUNDING;
        style.WindowBorderSize = WINDOW_BORDER_SIZE;
        style.WindowMenuButtonPosition = ImGuiDir_None;

        style.ChildRounding = CHILD_ROUNDING;
        style.ChildBorderSize = CHILD_BORDER_SIZE;

        style.PopupRounding = POPUP_ROUNDING;
        style.PopupBorderSize = POPUP_BORDER_SIZE;

        style.FramePadding = FRAME_PADDING;
        style.FrameRounding = FRAME_ROUNDING;
        style.FrameBorderSize = FRAME_BORDER_SIZE;

        style.ItemSpacing = ITEM_SPACING;
        style.ItemInnerSpacing = ITEM_INNER_SPACING;
        style.SelectableTextAlign = SELECTABLE_TEXT_ALIGN;
        style.ButtonTextAlign = BUTTON_TEXT_ALIGN;
        style.CellPadding = CELL_PADDING;
        style.IndentSpacing = INDENT_SPACING;
        style.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesToNodes;

        style.ScrollbarSize = SCROLLBAR_SIZE;
        style.ScrollbarRounding = SCROLLBAR_ROUNDING;

        style.GrabMinSize = GRAB_MIN_SIZE;
        style.GrabRounding = GRAB_ROUNDING;

        style.TabRounding = TAB_ROUNDING;
        style.TabBorderSize = TAB_BORDER_SIZE;
        style.TabBarBorderSize = TAB_BAR_BORDER_SIZE;
        style.TabBarOverlineSize = TAB_BAR_OVERLINE_SIZE;

        style.DockingSeparatorSize = DOCKING_SEPARATOR_SIZE;
        style.SeparatorSize = SEPARATOR_SIZE;
        style.ImageRounding = IMAGE_ROUNDING;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text] = COLOR_TEXT;
        colors[ImGuiCol_TextDisabled] = COLOR_TEXT_DIM;
        colors[ImGuiCol_WindowBg] = COLOR_BG_BASE;
        colors[ImGuiCol_ChildBg] = COLOR_TRANSPARENT;
        colors[ImGuiCol_PopupBg] = COLOR_BG_POPUP;
        colors[ImGuiCol_Border] = COLOR_BORDER;
        colors[ImGuiCol_BorderShadow] = COLOR_TRANSPARENT;

        colors[ImGuiCol_FrameBg] = COLOR_BG_FRAME;
        colors[ImGuiCol_FrameBgHovered] = COLOR_BG_HOVER;
        colors[ImGuiCol_FrameBgActive] = COLOR_BG_BASE;

        colors[ImGuiCol_TitleBg] = COLOR_BG_DARK;
        colors[ImGuiCol_TitleBgActive] = COLOR_BG_DARK;
        colors[ImGuiCol_TitleBgCollapsed] = COLOR_BG_DARK;
        colors[ImGuiCol_MenuBarBg] = COLOR_BG_DARK;

        colors[ImGuiCol_ScrollbarBg] = COLOR_TRANSPARENT;
        colors[ImGuiCol_ScrollbarGrab] = COLOR_GRAB;
        colors[ImGuiCol_ScrollbarGrabHovered] = COLOR_GRAB_HOVER;
        colors[ImGuiCol_ScrollbarGrabActive] = COLOR_GRAB_HOVER;

        colors[ImGuiCol_CheckMark] = COLOR_ACCENT;
        colors[ImGuiCol_CheckboxSelectedBg] = COLOR_BG_FRAME;
        colors[ImGuiCol_SliderGrab] = COLOR_GRAB;
        colors[ImGuiCol_SliderGrabActive] = COLOR_ACCENT;

        colors[ImGuiCol_Button] = COLOR_BUTTON;
        colors[ImGuiCol_ButtonHovered] = COLOR_BUTTON_HOVER;
        colors[ImGuiCol_ButtonActive] = COLOR_BUTTON_ACTIVE;

        colors[ImGuiCol_Header] = COLOR_BG_ACTIVE;
        colors[ImGuiCol_HeaderHovered] = COLOR_BG_HOVER;
        colors[ImGuiCol_HeaderActive] = COLOR_BG_ACTIVE;

        colors[ImGuiCol_Separator] = COLOR_BG_DARK;
        colors[ImGuiCol_SeparatorHovered] = COLOR_GRAB;
        colors[ImGuiCol_SeparatorActive] = COLOR_ACCENT;

        colors[ImGuiCol_ResizeGrip] = COLOR_TRANSPARENT;
        colors[ImGuiCol_ResizeGripHovered] = COLOR_RESIZE_GRIP_HOVER;
        colors[ImGuiCol_ResizeGripActive] = COLOR_RESIZE_GRIP_ACTIVE;
        colors[ImGuiCol_InputTextCursor] = COLOR_TEXT;

        colors[ImGuiCol_Tab] = COLOR_BG_DARK;
        colors[ImGuiCol_TabHovered] = COLOR_BG_HOVER;
        colors[ImGuiCol_TabSelected] = COLOR_BG_BASE;
        colors[ImGuiCol_TabSelectedOverline] = COLOR_TRANSPARENT;
        colors[ImGuiCol_TabDimmed] = COLOR_BG_DARK;
        colors[ImGuiCol_TabDimmedSelected] = COLOR_BG_BASE;
        colors[ImGuiCol_TabDimmedSelectedOverline] = COLOR_TRANSPARENT;

        colors[ImGuiCol_DockingPreview] = COLOR_DOCKING_PREVIEW;
        colors[ImGuiCol_DockingEmptyBg] = COLOR_BG_DARK;

        colors[ImGuiCol_PlotLines] = COLOR_ACCENT;
        colors[ImGuiCol_PlotLinesHovered] = COLOR_ACCENT;
        colors[ImGuiCol_PlotHistogram] = COLOR_ACCENT;
        colors[ImGuiCol_PlotHistogramHovered] = COLOR_ACCENT;

        colors[ImGuiCol_TableHeaderBg] = COLOR_BG_FRAME;
        colors[ImGuiCol_TableBorderStrong] = COLOR_BG_DARK;
        colors[ImGuiCol_TableBorderLight] = COLOR_BG_FRAME;
        colors[ImGuiCol_TableRowBg] = COLOR_TRANSPARENT;
        colors[ImGuiCol_TableRowBgAlt] = COLOR_TABLE_ROW_ALT;

        colors[ImGuiCol_TextLink] = COLOR_ACCENT;
        colors[ImGuiCol_TextSelectedBg] = COLOR_TEXT_SELECTED_BG;
        colors[ImGuiCol_TreeLines] = COLOR_GRAB;
        colors[ImGuiCol_DragDropTarget] = COLOR_ACCENT;
        colors[ImGuiCol_DragDropTargetBg] = COLOR_DRAG_DROP_TARGET_BG;
        colors[ImGuiCol_UnsavedMarker] = COLOR_TEXT_DIM;

        colors[ImGuiCol_NavCursor] = COLOR_NAV_CURSOR;
        colors[ImGuiCol_NavWindowingHighlight] = COLOR_NAV_WINDOWING_HIGHLIGHT;
        colors[ImGuiCol_NavWindowingDimBg] = COLOR_NAV_WINDOWING_DIM_BG;
        colors[ImGuiCol_ModalWindowDimBg] = COLOR_MODAL_DIM_BG;
    }

    void push_no_padding() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    void pop_no_padding() {
        ImGui::PopStyleVar();
    }

    ImGuiID dockspace_over_viewport(ImGuiDockNodeFlags flags) {
        const ImGuiStyle& style = ImGui::GetStyle();

        StyleVarStack vars;
        vars.push(ImGuiStyleVar_ItemInnerSpacing, ImVec2(TAB_SPACING_X, style.ItemInnerSpacing.y));
        vars.push(ImGuiStyleVar_FramePadding, ImVec2(TAB_PADDING_X, style.FramePadding.y));

        const ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), flags);
        vars.pop();

        return dockspace_id;
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
} // namespace hob::editor_theme
