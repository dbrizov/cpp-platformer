#include "editor_action_queue.h"

#include <algorithm>
#include <vector>

#include "engine/editor/editor.h"

namespace hob::editor {
    void EditorActionQueue::request(EditorActionId id) {
        m_pending.push_back(id);
    }

    void EditorActionQueue::flush(Editor& editor) {
        if (m_pending.empty()) {
            return;
        }

        // Swapped out first: an action may queue another one, which then runs on the next frame.
        std::vector<EditorActionId> running;
        running.swap(m_pending);

        for (const EditorActionId id : running) {
            // Re-checked, since an earlier action in the same flush may have invalidated this one.
            if (!is_action_enabled(editor, id)) {
                continue;
            }

            get_action(id).run(editor);
        }
    }
} // namespace hob::editor
