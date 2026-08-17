#pragma once

#include "editor_dock.h"

namespace hob::editor {
    class EditorDockAssets : public EditorDock {
    public:
        EditorDockAssets();

        void draw(Editor& editor) override;
    };
} // namespace hob::editor
