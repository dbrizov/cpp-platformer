#include "editor_style.h"

#include <imgui.h>

namespace hob::editor {
    void apply_style() {
        ImGuiStyle& style = ImGui::GetStyle();

        style.FontSizeBase = FONT_SIZE_PX;

        style.WindowPadding = WINDOW_PADDING;
        style.WindowTitleAlign = WINDOW_TITLE_ALIGN;
        style.WindowRounding = ROUNDING;
        style.WindowBorderSize = WINDOW_BORDER_SIZE;
        style.WindowMenuButtonPosition = WINDOW_MENU_BUTTON_POSITION;

        style.ChildRounding = ROUNDING;
        style.ChildBorderSize = CHILD_BORDER_SIZE;

        style.PopupRounding = ROUNDING;
        style.PopupBorderSize = POPUP_BORDER_SIZE;

        style.FramePadding = FRAME_PADDING;
        style.FrameRounding = ROUNDING;
        style.FrameBorderSize = FRAME_BORDER_SIZE;
        style.ColorMarkerSize = COLOR_MARKER_SIZE;

        style.ItemSpacing = ITEM_SPACING;
        style.ItemInnerSpacing = ITEM_INNER_SPACING;
        style.SelectableTextAlign = SELECTABLE_TEXT_ALIGN;
        style.ButtonTextAlign = BUTTON_TEXT_ALIGN;
        style.CellPadding = CELL_PADDING;
        style.IndentSpacing = INDENT_SPACING;
        style.TreeLinesFlags = TREE_LINES_FLAGS;

        style.ScrollbarSize = SCROLLBAR_SIZE;
        style.ScrollbarRounding = ROUNDING;

        style.GrabMinSize = GRAB_MIN_SIZE;
        style.GrabRounding = ROUNDING;

        style.TabRounding = ROUNDING;
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
        colors[ImGuiCol_Border] = COLOR_BORDER_DARK;
        colors[ImGuiCol_BorderShadow] = COLOR_TRANSPARENT;

        colors[ImGuiCol_FrameBg] = COLOR_BG_FRAME;
        colors[ImGuiCol_FrameBgHovered] = COLOR_ITEM_HOVER;
        colors[ImGuiCol_FrameBgActive] = COLOR_ITEM_ACTIVE;

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

        colors[ImGuiCol_Header] = COLOR_ITEM_ACTIVE;
        colors[ImGuiCol_HeaderHovered] = COLOR_ITEM_HOVER;
        colors[ImGuiCol_HeaderActive] = COLOR_ITEM_ACTIVE;

        colors[ImGuiCol_Separator] = COLOR_SEPARATOR;
        colors[ImGuiCol_SeparatorHovered] = COLOR_RESIZE_GRIP_HOVER;
        colors[ImGuiCol_SeparatorActive] = COLOR_RESIZE_GRIP_ACTIVE;

        colors[ImGuiCol_ResizeGrip] = COLOR_BORDER_LIGHT;
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
        colors[ImGuiCol_ModalWindowDimBg] = COLOR_TRANSPARENT;
    }
} // namespace hob::editor
