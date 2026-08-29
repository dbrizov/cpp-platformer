#pragma once

#include <optional>
#include <string>

namespace hob::editor {
    class Editor;

    std::optional<std::string> get_scene_save_error(const Editor& editor);
    bool can_save_scene(const Editor& editor);
    void save_scene(Editor& editor);
    void revert_scene(Editor& editor);
} // namespace hob::editor
