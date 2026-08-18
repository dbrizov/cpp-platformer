#include "editor_gui_utils.h"

#include <algorithm>
#include <cctype>
#include <concepts>
#include <cstdarg>
#include <cstdio>
#include <format>

#include <imgui_internal.h>

#include "editor_style.h"
#include "engine/math/constants.h"
#include "engine/math/mathf.h"

namespace hob::editor {
    namespace {
        constexpr int32_t DRAW_CHANNEL_COUNT = 2;
        constexpr int32_t DRAW_CHANNEL_BACKGROUND = 0;
        constexpr int32_t DRAW_CHANNEL_FOREGROUND = 1;

        constexpr const char* EMPTY_LABEL = "##";
        constexpr const char* FLOAT_FORMAT = "%.3f";
        constexpr const char* INT_FORMAT = "%lld";
        constexpr uint32_t FIELD_STRING_CAPACITY = 256;
        constexpr const char* COLOR_PICKER_POPUP = "picker";
        constexpr const char* BITMASK_POPUP = "flags";
        constexpr const char* BITMASK_BUTTON_ID = "###flags";
        constexpr const char* BITMASK_SEPARATOR = " | ";
        constexpr const char* BITMASK_NONE = "None";
        constexpr ImGuiColorEditFlags COLOR_EDIT_FLAGS = ImGuiColorEditFlags_Float;

        // Words that read wrong title-cased. Matched whole, against a lowercase snake_case word.
        constexpr std::string_view LABEL_ACRONYMS[] = {"aabb"};

        template<std::totally_ordered T>
        ImGuiSliderFlags clamp_flags(T min, T max) {
            return min < max ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;
        }

        char to_upper(char c) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        void append_label_word(std::string& label, std::string_view word) {
            for (const std::string_view acronym : LABEL_ACRONYMS) {
                if (word == acronym) {
                    for (const char c : word) {
                        label += to_upper(c);
                    }

                    return;
                }
            }

            label += to_upper(word.front());
            label.append(word.substr(1));
        }

        ImRect inset_rect(const ImVec2& min, const ImVec2& max, const ImVec2& inset) {
            return ImRect(min.x + inset.x, min.y + inset.y, max.x - inset.x, max.y - inset.y);
        }

        // The row highlight spans the whole window, not just the indented item rect.
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

        bool field_components(const char* label,
                              const ImVec4* colors,
                              float* const* values,
                              int32_t count,
                              float drag_speed,
                              float min,
                              float max) {
            begin_field(label);
            ImGui::BeginGroup();
            ImGui::PushMultiItemsWidths(count, ImGui::CalcItemWidth());

            bool changed = false;
            for (int32_t i = 0; i < count; ++i) {
                ImGui::PushID(i);
                if (i > 0) {
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                }

                ImGui::SetNextItemColorMarker(ImGui::GetColorU32(colors[i]));
                changed |=
                    ImGui::DragFloat(EMPTY_LABEL, values[i], drag_speed, min, max, FLOAT_FORMAT, clamp_flags(min, max));

                ImGui::PopID();
                ImGui::PopItemWidth();
            }

            ImGui::EndGroup();
            end_field();

            return changed;
        }
    } // namespace

    std::string to_display_label(std::string_view name) {
        std::string label;
        label.reserve(name.size());

        for (size_t start = 0; start <= name.size();) {
            const size_t separator = name.find('_', start);
            const size_t end = (separator == std::string_view::npos) ? name.size() : separator;

            if (end > start) {
                if (!label.empty()) {
                    label += ' ';
                }

                append_label_word(label, name.substr(start, end - start));
            }

            if (separator == std::string_view::npos) {
                break;
            }

            start = separator + 1;
        }

        return label;
    }

    ImGuiID dock_space_over_viewport(ImGuiDockNodeFlags flags) {
        const ImGuiStyle& style = ImGui::GetStyle();

        StyleVarStack vars;
        vars.push(ImGuiStyleVar_ItemInnerSpacing, ImVec2(DOCK_TAB_SPACING_X, style.ItemInnerSpacing.y));
        vars.push(ImGuiStyleVar_FramePadding, ImVec2(DOCK_TAB_PADDING_X, style.FramePadding.y));

        const ImGuiID dock_space_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), flags);
        vars.pop();

        return dock_space_id;
    }

    bool begin_dock(const char* name, ImGuiWindowFlags flags) {
        const ImGuiWindow* window = ImGui::FindWindowByName(name);
        const bool is_selected_tab = window && window->DockTabIsVisible;

        ImGui::PushStyleColor(ImGuiCol_TabHovered, is_selected_tab ? COLOR_BG_BASE : COLOR_BG_HOVER);
        const bool visible = ImGui::Begin(name, nullptr, flags);
        ImGui::PopStyleColor();

        return visible;
    }

    void end_dock() {
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
        vars.push(ImGuiStyleVar_ItemSpacing, ImVec2(MENU_BAR_ITEM_PADDING_X, style.ItemSpacing.y));
        vars.push(ImGuiStyleVar_WindowPadding, MENU_BAR_POPUP_PADDING);

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + MENU_BAR_ITEM_SPACING_X);
        const bool open = ImGui::BeginMenu(label);
        vars.pop();
        colors.pop();

        if (open || ImGui::IsItemHovered()) {
            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(draw_list,
                           ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()),
                           open ? COLOR_MENU_BAR_ITEM_ACTIVE : COLOR_MENU_BAR_ITEM_HOVER,
                           MENU_BAR_ITEM_ROUNDING);
        }

        splitter.Merge(draw_list);

        if (open) {
            ImGui::PushStyleColor(ImGuiCol_Separator, COLOR_MENU_BAR_SEPARATOR);
        }

        return open;
    }

    void end_menu() {
        ImGui::PopStyleColor();
        ImGui::EndMenu();
    }

    float bar_button_width(const char* label) {
        return ImGui::CalcTextSize(label).x + MENU_BAR_ITEM_PADDING_X * 2.0f + MENU_BAR_ITEM_SPACING_X;
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
        vars.push(ImGuiStyleVar_ItemSpacing, ImVec2(MENU_BAR_ITEM_PADDING_X * 2.0f, style.ItemSpacing.y));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + MENU_BAR_ITEM_SPACING_X);
        const bool pressed = ImGui::Selectable(label, false, ImGuiSelectableFlags_None, ImGui::CalcTextSize(label));
        vars.pop();
        colors.pop();

        const ImVec4& color = ImGui::IsItemActive()    ? COLOR_BUTTON_ACTIVE
                              : ImGui::IsItemHovered() ? COLOR_BUTTON_HOVER
                                                       : COLOR_BUTTON;

        splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
        draw_highlight(
            draw_list, ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), color, MENU_BAR_ITEM_ROUNDING);
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

        // ImGui draws the shortcut left-aligned in its own column, so hide it and redraw it flush to the right edge.
        colors.push(ImGuiCol_TextDisabled, COLOR_TRANSPARENT);

        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        const float shortcut_y = window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset;

        const bool selected = false;
        const bool pressed = ImGui::MenuItem(label, shortcut, selected, enabled);
        colors.pop();

        if (shortcut && shortcut[0]) {
            const ImGuiStyle& style = ImGui::GetStyle();
            ImVec4 color = style.Colors[ImGuiCol_TextDisabled];
            color.w *= enabled ? 1.0f : style.DisabledAlpha;

            const float shortcut_x = window->WorkRect.Max.x - ImGui::CalcTextSize(shortcut).x;
            draw_list->AddText(ImVec2(shortcut_x, shortcut_y), ImGui::GetColorU32(color), shortcut);
        }

        // A disabled row is inert, so it must not light up under the cursor either.
        if (enabled && ImGui::IsItemHovered()) {
            splitter.SetCurrentChannel(draw_list, DRAW_CHANNEL_BACKGROUND);
            draw_highlight(
                draw_list, row_rect(MENU_BAR_POPUP_ITEM_INSET), COLOR_MENU_BAR_ITEM_HOVER, MENU_BAR_ITEM_ROUNDING);
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
                           row_rect(HIERARCHY_ITEM_INSET),
                           selected ? COLOR_HIERARCHY_ITEM_SELECTED : COLOR_HIERARCHY_ITEM_HOVER,
                           HIERARCHY_ITEM_ROUNDING);
        }

        splitter.Merge(draw_list);

        return open;
    }

    bool component_header(const char* label) {
        StyleColorStack colors;
        colors.push(ImGuiCol_Header, COLOR_INSPECTOR_HEADER);
        colors.push(ImGuiCol_HeaderHovered, COLOR_INSPECTOR_HEADER_HOVER);
        colors.push(ImGuiCol_HeaderActive, COLOR_INSPECTOR_HEADER_HOVER);

        const bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
        colors.pop();

        return open;
    }

    void begin_field(const char* label) {
        ImGui::PushID(label);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        // The label may be indented (nested rows), so measure to its right edge, not its width.
        const float label_end_x = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x;
        const float column_x = std::max(INSPECTOR_LABEL_WIDTH, label_end_x + ITEM_SPACING.x);

        // SameLine adds the enclosing group's offset, which would push the rows nested inside a
        // composite field out of the column every other row sits in. Cancel it.
        ImGui::SameLine(column_x - ImGui::GetCurrentWindow()->DC.GroupOffset.x);
        ImGui::SetNextItemWidth(-EPSILON);
    }

    void end_field() {
        ImGui::PopID();
    }

    bool field_angle(const char* label, float& degrees, float drag_speed) {
        float normalized = math::normalize_angle(degrees);
        float* components[] = {&normalized};
        const bool changed =
            field_components(label, &COLOR_AXIS_Z, components, IM_COUNTOF(components), drag_speed, 0.0f, 0.0f);

        if (changed) {
            degrees = normalized;
        }

        return changed;
    }

    bool field_float(const char* label, float& value, float drag_speed, float min, float max) {
        begin_field(label);
        const bool changed =
            ImGui::DragFloat(EMPTY_LABEL, &value, drag_speed, min, max, FLOAT_FORMAT, clamp_flags(min, max));
        end_field();

        return changed;
    }

    bool field_int(const char* label, int64_t& value, float drag_speed, int64_t min, int64_t max) {
        begin_field(label);
        const bool changed = ImGui::DragScalar(
            EMPTY_LABEL, ImGuiDataType_S64, &value, drag_speed, &min, &max, INT_FORMAT, clamp_flags(min, max));
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

    void field_text(const char* label, const std::string& value) {
        char buffer[FIELD_STRING_CAPACITY];
        std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());

        begin_field(label);
        ImGui::InputText(EMPTY_LABEL, buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
        end_field();
    }

    bool field_vector2(const char* label, Vector2& value, float drag_speed) {
        const ImVec4 colors[] = {COLOR_AXIS_X, COLOR_AXIS_Y};
        float* components[] = {&value.x, &value.y};

        return field_components(label, colors, components, IM_COUNTOF(components), drag_speed, 0.0f, 0.0f);
    }

    bool field_color(const char* label, Color& value) {
        const ImVec4 colors[] = {COLOR_AXIS_X, COLOR_AXIS_Y, COLOR_AXIS_Z, COLOR_AXIS_W};
        float* components[] = {&value.r, &value.g, &value.b, &value.a};
        constexpr int32_t count = IM_COUNTOF(components);

        const ImGuiStyle& style = ImGui::GetStyle();
        const float swatch_width = ImGui::GetFrameHeight(); // ColorButton is square at its default size.

        begin_field(label);
        ImGui::BeginGroup();

        // The drags share the row with the swatch, so they split what is left of the field width.
        ImGui::PushMultiItemsWidths(count, ImGui::CalcItemWidth() - swatch_width - style.ItemInnerSpacing.x);

        bool changed = false;
        for (int32_t i = 0; i < count; ++i) {
            ImGui::PushID(i);
            if (i > 0) {
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
            }

            ImGui::SetNextItemColorMarker(ImGui::GetColorU32(colors[i]));
            changed |= ImGui::DragFloat(EMPTY_LABEL,
                                        components[i],
                                        INSPECTOR_DRAG_SPEED_COLOR,
                                        0.0f,
                                        1.0f,
                                        FLOAT_FORMAT,
                                        ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();
            ImGui::PopItemWidth();
        }

        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);

        float rgba[4] = {value.r, value.g, value.b, value.a};
        if (ImGui::ColorButton(EMPTY_LABEL, ImVec4(rgba[0], rgba[1], rgba[2], rgba[3]), COLOR_EDIT_FLAGS)) {
            ImGui::OpenPopup(COLOR_PICKER_POPUP);
        }

        // Scoped by begin_field's PushID, so every color field gets its own popup.
        if (ImGui::BeginPopup(COLOR_PICKER_POPUP)) {
            if (ImGui::ColorPicker4(EMPTY_LABEL, rgba, COLOR_EDIT_FLAGS)) {
                value = Color(rgba[0], rgba[1], rgba[2], rgba[3]);
                changed = true;
            }
            ImGui::EndPopup();
        }

        ImGui::EndGroup();
        end_field();

        return changed;
    }

    bool field_enum(const char* label, int64_t& value, const std::vector<EditorEnumEntry>& entries) {
        std::string name = std::to_string(value);
        for (const EditorEnumEntry& entry : entries) {
            if (entry.value == value) {
                name = entry.name;
                break;
            }
        }

        begin_field(label);

        bool changed = false;
        if (ImGui::BeginCombo(EMPTY_LABEL, name.c_str())) {
            for (const EditorEnumEntry& entry : entries) {
                if (ImGui::Selectable(entry.name.c_str(), entry.value == value) && entry.value != value) {
                    value = entry.value;
                    changed = true;
                }
            }

            ImGui::EndCombo();
        }

        end_field();

        return changed;
    }

    bool field_bitmask(const char* label, int64_t& value, const std::vector<EditorEnumEntry>& entries) {
        int64_t named_mask = 0;
        std::string named_summary;

        for (const EditorEnumEntry& entry : entries) {
            if (entry.value == 0) {
                continue;
            }

            named_mask |= entry.value;
            if ((value & entry.value) == entry.value) {
                if (!named_summary.empty()) {
                    named_summary += BITMASK_SEPARATOR;
                }

                named_summary += entry.name;
            }
        }

        const int64_t rest = value & ~named_mask;
        if (rest != 0) {
            if (!named_summary.empty()) {
                named_summary += BITMASK_SEPARATOR;
            }

            named_summary += std::format("0x{:x}", static_cast<uint64_t>(rest));
        }

        if (named_summary.empty()) {
            named_summary = BITMASK_NONE;
        }

        begin_field(label);

        if (ImGui::Button((named_summary + BITMASK_BUTTON_ID).c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
            ImGui::OpenPopup(BITMASK_POPUP);
        }

        bool changed = false;

        if (ImGui::BeginPopup(BITMASK_POPUP)) {
            for (const EditorEnumEntry& entry : entries) {
                if (entry.value == 0) {
                    continue;
                }

                bool set = (value & entry.value) == entry.value;
                if (ImGui::Checkbox(entry.name.c_str(), &set)) {
                    value ^= entry.value;
                    changed = true;
                }
            }

            ImGui::EndPopup();
        }

        end_field();

        return changed;
    }

    bool field_aabb(const char* label, AABB& value) {
        ImGui::PushID(label);
        ImGui::BeginGroup();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        ImGui::Indent(INSPECTOR_NESTED_INDENT);
        bool changed = field_vector2("Center", value.center);
        changed |= field_vector2("Extents", value.extents);
        ImGui::Unindent(INSPECTOR_NESTED_INDENT);

        ImGui::EndGroup();
        ImGui::PopID();

        return changed;
    }

    bool field_capsule(const char* label, Capsule& value) {
        ImGui::PushID(label);
        ImGui::BeginGroup();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        ImGui::Indent(INSPECTOR_NESTED_INDENT);
        bool changed = field_vector2("Center A", value.center_a);
        changed |= field_vector2("Center B", value.center_b);
        changed |= field_float("Radius", value.radius, INSPECTOR_DRAG_SPEED_FLOAT, 0.0f, MAX_FLOAT);
        ImGui::Unindent(INSPECTOR_NESTED_INDENT);

        ImGui::EndGroup();
        ImGui::PopID();

        return changed;
    }

    bool field_circle(const char* label, Circle& value) {
        ImGui::PushID(label);
        ImGui::BeginGroup();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        ImGui::Indent(INSPECTOR_NESTED_INDENT);
        bool changed = field_vector2("Center", value.center);
        changed |= field_float("Radius", value.radius, INSPECTOR_DRAG_SPEED_FLOAT, 0.0f, MAX_FLOAT);
        ImGui::Unindent(INSPECTOR_NESTED_INDENT);

        ImGui::EndGroup();
        ImGui::PopID();

        return changed;
    }
} // namespace hob::editor
