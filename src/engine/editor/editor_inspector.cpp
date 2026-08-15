#include <imgui.h>

#include "editor.h"
#include "editor_gui_utils.h"

namespace hob::editor {
    void Editor::draw_inspector() {
        if (begin_panel(PANEL_INSPECTOR)) {
            ImGui::TextDisabled("(empty)");
        }
        end_panel();
    }
} // namespace hob::editor
