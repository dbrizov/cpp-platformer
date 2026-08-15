#pragma once

#include <filesystem>

namespace hob::editor {
    struct EditorConfig {
        bool enabled = false;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool maximized = false;

        EditorConfig() = default;
        explicit EditorConfig(const std::filesystem::path& json_path);

        void save(const std::filesystem::path& json_path) const;
    };
} // namespace hob::editor
