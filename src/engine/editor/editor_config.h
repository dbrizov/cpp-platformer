#pragma once

#include <filesystem>

#include "engine/core/engine_config.h"

namespace hob::editor {
    struct EditorConfig {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool maximized = false;

        EditorConfig() = default;
        explicit EditorConfig(const std::filesystem::path& json_path);

        void save(const std::filesystem::path& json_path) const;
    };

    std::filesystem::path get_editor_config_file_path();
    std::filesystem::path get_editor_imgui_ini_file_path();

    HostConfig make_editor_host_config(const GraphicsConfig& graphics_config, const EditorConfig& editor_config);
} // namespace hob::editor
