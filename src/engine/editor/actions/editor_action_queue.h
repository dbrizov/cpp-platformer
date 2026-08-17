#pragma once

#include <vector>

#include "editor_action.h"

namespace hob::editor {
    class Editor;

    class EditorActionQueue {
        std::vector<EditorActionId> m_pending;

    public:
        void request(EditorActionId id);
        void flush(Editor& editor);
    };
} // namespace hob::editor
