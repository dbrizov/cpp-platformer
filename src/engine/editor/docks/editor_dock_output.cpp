#include "editor_dock_output.h"

#include <imgui.h>

namespace hob::editor {
    EditorDockOutput::EditorDockOutput()
        : EditorDock("Output", EditorActionContext::Output) {}

    void EditorDockOutput::draw(Editor&) {
        if (begin()) {
            ImGui::TextDisabled("(empty)");
        }
        end();
    }
} // namespace hob::editor
