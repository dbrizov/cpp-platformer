#include "editor_toolbar.h"

#include <algorithm>

#include <imgui.h>

#include "editor.h"
#include "editor_action.h"
#include "editor_gui_utils.h"
#include "editor_style.h"

namespace hob::editor {
    namespace {
        constexpr int TOOLBAR_MAX_ITEMS = 3;
    } // namespace

    void EditorToolbar::draw(Editor& editor) {
        const EditorState state = editor.get_state();

        EditorActionId items[TOOLBAR_MAX_ITEMS]{};
        int item_count = 0;

        switch (state) {
            case EditorState::Edit: {
                items[item_count++] = EditorActionId::Play;
                break;
            }
            case EditorState::Play: {
                items[item_count++] = EditorActionId::Pause;
                items[item_count++] = EditorActionId::Stop;
                break;
            }
            case EditorState::Paused: {
                items[item_count++] = EditorActionId::Play;
                items[item_count++] = EditorActionId::Step;
                items[item_count++] = EditorActionId::Stop;
                break;
            }
        }

        const char* state_label = (state == EditorState::Edit)   ? "Edit"
                                  : (state == EditorState::Play) ? "Play"
                                                                 : "Paused";

        float toolbar_width = ImGui::CalcTextSize(state_label).x + MENU_BAR_ITEM_SPACING_X;
        for (int i = 0; i < item_count; ++i) {
            toolbar_width += action_bar_button_width(editor, items[i]);
        }

        const float cursor_x = ImGui::GetCursorPosX();
        const float right_edge_x = cursor_x + ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(std::max(cursor_x, right_edge_x - toolbar_width));

        for (int i = 0; i < item_count; ++i) {
            action_bar_button(editor, items[i]);
        }

        ImGui::TextDisabled("%s", state_label);
    }
} // namespace hob::editor
