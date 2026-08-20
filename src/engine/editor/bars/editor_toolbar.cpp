#include "editor_toolbar.h"

#include <algorithm>
#include <iterator>

#include <imgui.h>

#include "engine/editor/actions/editor_action.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_gui_utils.h"
#include "engine/editor/editor_style.h"

namespace hob::editor {
    namespace {
        struct EditorToolbarItem {
            EditorActionId id;
            EditorBarIcon icon;
        };

        constexpr EditorToolbarItem TOOLBAR_ITEMS[] = {
            {EditorActionId::Play, EditorBarIcon::Play},
            {EditorActionId::Pause, EditorBarIcon::Pause},
            {EditorActionId::Step, EditorBarIcon::Step},
            {EditorActionId::Stop, EditorBarIcon::Stop},
        };

        constexpr int32_t TOOLBAR_ITEM_COUNT = static_cast<int32_t>(std::size(TOOLBAR_ITEMS));
    } // namespace

    void EditorToolbar::draw(Editor& editor) {
        const float toolbar_width =
            TOOLBAR_BUTTON_WIDTH * TOOLBAR_ITEM_COUNT + TOOLBAR_BUTTON_SPACING_X * (TOOLBAR_ITEM_COUNT - 1);

        const float cursor_x = ImGui::GetCursorPosX();
        const float centered_x = (ImGui::GetWindowWidth() - toolbar_width) * 0.5f - TOOLBAR_BUTTON_SPACING_X;
        ImGui::SetCursorPosX(std::max(cursor_x, centered_x));

        for (const EditorToolbarItem& item : TOOLBAR_ITEMS) {
            action_bar_icon_button(editor, item.id, item.icon);
        }
    }
} // namespace hob::editor
