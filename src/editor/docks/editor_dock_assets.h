#pragma once

#include <memory>

#include "editor_dock.h"

namespace hob::editor {
    struct EditorFileTree;

    class EditorDockAssets : public EditorDock {
        std::unique_ptr<EditorFileTree> m_tree;

    public:
        EditorDockAssets();
        ~EditorDockAssets() override;

        void draw(Editor& editor) override;

        void request_rebuild();
    };
} // namespace hob::editor
