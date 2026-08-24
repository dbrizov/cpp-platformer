#pragma once

#include "editor_dock.h"

namespace hob::editor {
    class EditorDockOutput : public EditorDock {
    public:
        EditorDockOutput();

        void draw(Editor& editor) override;
    };
} // namespace hob::editor
