#pragma once

#include <imgui.h>

namespace hob::editor {
    void apply_style();

    constexpr ImVec4 with_alpha(const ImVec4& color, float alpha) {
        return ImVec4(color.x, color.y, color.z, alpha);
    }

    // Palette
    constexpr ImVec4 COLOR_TRANSPARENT{0.0f, 0.0f, 0.0f, 0.0f};
    constexpr ImVec4 COLOR_CLEAR{0.078f, 0.078f, 0.078f, 1.0f};
    constexpr ImVec4 COLOR_ACCENT{0.337f, 0.620f, 1.000f, 1.0f};

    constexpr ImVec4 COLOR_BG_DARK{0.078f, 0.078f, 0.078f, 1.0f};
    constexpr ImVec4 COLOR_BG_BASE{0.161f, 0.161f, 0.161f, 1.0f};
    constexpr ImVec4 COLOR_BG_FRAME{0.106f, 0.106f, 0.106f, 1.0f};
    constexpr ImVec4 COLOR_BG_POPUP{0.106f, 0.106f, 0.106f, 1.0f};
    constexpr ImVec4 COLOR_BG_HOVER{0.129f, 0.129f, 0.129f, 1.0f};
    constexpr ImVec4 COLOR_BG_PRESSED{0.192f, 0.192f, 0.192f, 1.0f};
    constexpr ImVec4 COLOR_BG_ACTIVE{0.286f, 0.286f, 0.286f, 1.0f};
    constexpr ImVec4 COLOR_BORDER{0.078f, 0.078f, 0.078f, 1.0f};

    constexpr ImVec4 COLOR_TEXT{0.792f, 0.792f, 0.792f, 1.0f};
    constexpr ImVec4 COLOR_TEXT_DIM{0.584f, 0.584f, 0.584f, 1.0f};

    constexpr ImVec4 COLOR_BUTTON{0.259f, 0.259f, 0.259f, 1.0f};
    constexpr ImVec4 COLOR_BUTTON_HOVER{0.302f, 0.302f, 0.302f, 1.0f};
    constexpr ImVec4 COLOR_BUTTON_ACTIVE{0.349f, 0.349f, 0.349f, 1.0f};

    constexpr ImVec4 COLOR_GRAB{0.349f, 0.349f, 0.349f, 1.0f};
    constexpr ImVec4 COLOR_GRAB_HOVER{0.431f, 0.431f, 0.431f, 1.0f};

    constexpr ImVec4 COLOR_AXIS_X{0.898f, 0.286f, 0.345f, 1.0f};
    constexpr ImVec4 COLOR_AXIS_Y{0.510f, 0.749f, 0.239f, 1.0f};
    constexpr ImVec4 COLOR_AXIS_Z{0.318f, 0.565f, 0.906f, 1.0f};
    constexpr ImVec4 COLOR_AXIS_W{0.584f, 0.584f, 0.584f, 1.0f};

    // ImGuiStyle metrics
    constexpr float FONT_SIZE_PX = 18.0f;

    constexpr ImVec2 WINDOW_PADDING{6.0f, 6.0f};
    constexpr ImVec2 WINDOW_TITLE_ALIGN{0.0f, 0.5f};
    constexpr float WINDOW_ROUNDING = 0.0f;
    constexpr float WINDOW_BORDER_SIZE = 0.0f;
    constexpr ImGuiDir WINDOW_MENU_BUTTON_POSITION = ImGuiDir_None;

    constexpr float CHILD_ROUNDING = 3.0f;
    constexpr float CHILD_BORDER_SIZE = 0.0f;

    constexpr float POPUP_ROUNDING = 3.0f;
    constexpr float POPUP_BORDER_SIZE = 0.0f;

    constexpr ImVec2 FRAME_PADDING{10.0f, 6.0f};
    constexpr float FRAME_ROUNDING = 3.0f;
    constexpr float FRAME_BORDER_SIZE = 0.0f;
    constexpr float COLOR_MARKER_SIZE = 3.0f;

    constexpr ImVec2 ITEM_SPACING{7.0f, 5.0f};
    constexpr ImVec2 ITEM_INNER_SPACING{6.0f, 4.0f};
    constexpr ImVec2 SELECTABLE_TEXT_ALIGN{0.0f, 0.5f};
    constexpr ImVec2 BUTTON_TEXT_ALIGN{0.5f, 0.5f};
    constexpr ImVec2 CELL_PADDING{6.0f, 3.0f};
    constexpr float INDENT_SPACING = 18.0f;
    constexpr ImGuiTreeNodeFlags TREE_LINES_FLAGS = ImGuiTreeNodeFlags_DrawLinesToNodes;

    constexpr float SCROLLBAR_SIZE = 12.0f;
    constexpr float SCROLLBAR_ROUNDING = 6.0f;

    constexpr float GRAB_MIN_SIZE = 12.0f;
    constexpr float GRAB_ROUNDING = 3.0f;

    constexpr float TAB_ROUNDING = 4.0f;
    constexpr float TAB_BORDER_SIZE = 0.0f;
    constexpr float TAB_BAR_BORDER_SIZE = 0.0f;
    constexpr float TAB_BAR_OVERLINE_SIZE = 0.0f;

    constexpr float DOCKING_SEPARATOR_SIZE = 2.0f;
    constexpr float SEPARATOR_SIZE = 1.0f;
    constexpr float IMAGE_ROUNDING = 0.0f;

    // ImGuiStyle colors
    constexpr ImVec4 COLOR_RESIZE_GRIP_HOVER = with_alpha(COLOR_ACCENT, 0.40f);
    constexpr ImVec4 COLOR_RESIZE_GRIP_ACTIVE = with_alpha(COLOR_ACCENT, 0.70f);
    constexpr ImVec4 COLOR_DOCKING_PREVIEW = with_alpha(COLOR_ACCENT, 0.45f);
    constexpr ImVec4 COLOR_TEXT_SELECTED_BG = with_alpha(COLOR_ACCENT, 0.35f);
    constexpr ImVec4 COLOR_DRAG_DROP_TARGET_BG = with_alpha(COLOR_ACCENT, 0.15f);
    constexpr ImVec4 COLOR_NAV_CURSOR = with_alpha(COLOR_ACCENT, 0.80f);
    constexpr ImVec4 COLOR_NAV_WINDOWING_HIGHLIGHT = with_alpha(COLOR_ACCENT, 0.70f);
    constexpr ImVec4 COLOR_TABLE_ROW_ALT{1.0f, 1.0f, 1.0f, 0.02f};
    constexpr ImVec4 COLOR_NAV_WINDOWING_DIM_BG{0.0f, 0.0f, 0.0f, 0.45f};
    constexpr ImVec4 COLOR_MODAL_DIM_BG{0.0f, 0.0f, 0.0f, 0.55f};

    // Dock
    constexpr float DOCK_TAB_PADDING_X = 0.0f;
    constexpr float DOCK_TAB_SPACING_X = 0.0f;

    // Menu Bar
    constexpr float MENU_BAR_ITEM_PADDING_X = 7.0f;
    constexpr float MENU_BAR_ITEM_SPACING_X = 4.0f;
    constexpr float MENU_BAR_ITEM_ROUNDING = 3.0f;
    constexpr ImVec2 MENU_BAR_POPUP_PADDING{16.0f, 6.0f};
    constexpr ImVec2 MENU_BAR_POPUP_ITEM_INSET{6.0f, 1.0f};

    constexpr ImVec4 COLOR_MENU_BAR_ITEM_HOVER{0.184f, 0.184f, 0.184f, 1.0f};
    constexpr ImVec4 COLOR_MENU_BAR_ITEM_ACTIVE{0.259f, 0.259f, 0.259f, 1.0f};
    constexpr ImVec4 COLOR_MENU_BAR_SEPARATOR{0.141f, 0.141f, 0.141f, 1.0f};

    // Hierarchy
    constexpr ImVec2 HIERARCHY_ITEM_SPACING{0.0f, 2.0f};
    constexpr ImVec2 HIERARCHY_ITEM_INSET{4.0f, 1.0f};
    constexpr float HIERARCHY_ITEM_ROUNDING = 3.0f;

    constexpr ImVec4 COLOR_HIERARCHY_ITEM_HOVER{0.212f, 0.212f, 0.212f, 1.0f};
    constexpr ImVec4 COLOR_HIERARCHY_ITEM_SELECTED{0.259f, 0.259f, 0.259f, 1.0f};

    // Inspector
    constexpr float INSPECTOR_LABEL_WIDTH = 180.0f;
    constexpr float INSPECTOR_DRAG_SPEED_FLOAT = 0.02f;
    constexpr float INSPECTOR_DRAG_SPEED_INT = 0.1f;
    constexpr float INSPECTOR_DRAG_SPEED_POSITION = 0.02f;
    constexpr float INSPECTOR_DRAG_SPEED_ROTATION_DEG = 0.5f;
    constexpr float INSPECTOR_DRAG_SPEED_SCALE = 0.01f;
    constexpr float INSPECTOR_DRAG_SPEED_COLOR = 0.005f;
    constexpr float INSPECTOR_NESTED_INDENT = 12.0f;

    constexpr const char* INSPECTOR_NONE_LABEL = "None";
    constexpr const char* INSPECTOR_EMPTY_LABEL = "##"; // ImGui expects this (don't modify)
    constexpr const char* INSPECTOR_INT_FORMAT = "%lld"; // This is driven by int64_t (don't modify)
    constexpr const char* INSPECTOR_FLOAT_FORMAT = "%.3f";
    constexpr ImGuiColorEditFlags INSPECTOR_COLOR_EDIT_FLAGS = ImGuiColorEditFlags_Float;

    constexpr ImVec4 COLOR_INSPECTOR_HEADER{0.220f, 0.220f, 0.220f, 1.0f};
    constexpr ImVec4 COLOR_INSPECTOR_HEADER_HOVER{0.259f, 0.259f, 0.259f, 1.0f};

    // Scene View
    constexpr float SCENE_VIEW_SELECTION_OUTLINE_THICKNESS = 2.0f;
    constexpr float SCENE_VIEW_SELECTION_MARKER_RADIUS_PX = 14.0f;
    constexpr float SCENE_VIEW_CAMERA_RECT_THICKNESS = 1.0f;

    constexpr ImVec4 COLOR_SCENE_VIEW_GRID_AXIS_X = with_alpha(COLOR_AXIS_X, 0.70f);
    constexpr ImVec4 COLOR_SCENE_VIEW_GRID_AXIS_Y = with_alpha(COLOR_AXIS_Y, 0.70f);
    constexpr ImVec4 COLOR_SCENE_VIEW_GRID_MINOR{1.0f, 1.0f, 1.0f, 0.094f};
    constexpr ImVec4 COLOR_SCENE_VIEW_GRID_MAJOR{1.0f, 1.0f, 1.0f, 0.20f};
    constexpr ImVec4 COLOR_SCENE_VIEW_SELECTION_PRIMARY{1.000f, 0.627f, 0.118f, 1.0f};
    constexpr ImVec4 COLOR_SCENE_VIEW_SELECTION = with_alpha(COLOR_SCENE_VIEW_SELECTION_PRIMARY, 0.55f);
    constexpr ImVec4 COLOR_SCENE_VIEW_CAMERA_RECT = with_alpha(COLOR_ACCENT, 0.70f);
} // namespace hob::editor
