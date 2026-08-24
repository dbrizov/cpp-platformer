#pragma once

#include <memory>

#include "editor_dock.h"

namespace hob::editor {
    // Holds a sol::object, so it is defined in the .cpp to keep <sol/sol.hpp> out of this header.
    struct EditorDockInspectorPendingEdit;

    class EditorDockInspector : public EditorDock {
        std::unique_ptr<EditorDockInspectorPendingEdit> m_pending;

    public:
        EditorDockInspector();
        ~EditorDockInspector() override;

        void draw(Editor& editor) override;

        void reset_edit_state();
    };
} // namespace hob::editor
