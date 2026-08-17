#include "editor_dock_assets.h"

#include <imgui.h>

namespace hob::editor {
    EditorDockAssets::EditorDockAssets()
        : EditorDock(" Assets ###Assets", EditorActionContext::Assets) {}

    void EditorDockAssets::draw(Editor&) {
        if (begin()) {
            ImGui::TextDisabled("(empty)");
        }
        end();
    }
} // namespace hob::editor
