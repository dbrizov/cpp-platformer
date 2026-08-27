#pragma once

namespace hob::editor {
    class Editor;

    bool can_save_scene(const Editor& editor);
    void save_scene(Editor& editor);
} // namespace hob::editor
