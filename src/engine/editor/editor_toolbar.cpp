#include <algorithm>

#include <imgui.h>

#include "editor.h"
#include "editor_gui_utils.h"
#include "editor_style.h"

namespace hob::editor {
    namespace {
        constexpr int TOOLBAR_MAX_ITEMS = 3;

        enum class Action {
            Play,
            Pause,
            Step,
            Stop,
        };

        struct Item {
            const char* label;
            Action action;
        };
    } // namespace

    void Editor::draw_toolbar() {
        Item items[TOOLBAR_MAX_ITEMS]{};
        int item_count = 0;

        switch (m_state) {
            case State::Edit: {
                items[item_count++] = {"Play", Action::Play};
                break;
            }
            case State::Play: {
                items[item_count++] = {"Pause", Action::Pause};
                items[item_count++] = {"Stop", Action::Stop};
                break;
            }
            case State::Paused: {
                items[item_count++] = {"Resume", Action::Play};
                items[item_count++] = {"Step", Action::Step};
                items[item_count++] = {"Stop", Action::Stop};
                break;
            }
        }

        const char* state_label = (m_state == State::Edit) ? "Edit" : (m_state == State::Play) ? "Play" : "Paused";

        float toolbar_width = ImGui::CalcTextSize(state_label).x + BAR_ITEM_SPACING_X;
        for (int i = 0; i < item_count; ++i) {
            toolbar_width += bar_button_width(items[i].label);
        }

        const float cursor_x = ImGui::GetCursorPosX();
        const float right_edge_x = cursor_x + ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(std::max(cursor_x, right_edge_x - toolbar_width));

        for (int i = 0; i < item_count; ++i) {
            if (bar_button(items[i].label)) {
                switch (items[i].action) {
                    case Action::Play:
                        set_state(State::Play);
                        break;
                    case Action::Pause:
                        set_state(State::Paused);
                        break;
                    case Action::Step:
                        m_step_requested = true;
                        break;
                    case Action::Stop:
                        set_state(State::Edit);
                        break;
                }
            }
        }

        ImGui::TextDisabled("%s", state_label);
    }
} // namespace hob::editor
