#pragma once

#include <filesystem>
#include <string>

#include "engine/core/engine_config.h"

namespace hob {
    class Window;
} // namespace hob

namespace hob::editor {
    struct EditorWindowConfig {
        int32_t x = 0;
        int32_t y = 0;
        int32_t width = 0;
        int32_t height = 0;
        bool maximized = false;

        bool has_size() const;
    };

    struct EditorConfig {
        EditorWindowConfig main_window;
        EditorWindowConfig game_window;

        std::string last_open_scene;

        EditorConfig() = default;
        explicit EditorConfig(const std::filesystem::path& json_path);

        void save(const std::filesystem::path& json_path) const;
    };

    EditorWindowConfig window_to_editor_window_config(const Window& window);
    EditorWindowConfig window_config_to_editor_window_config(const WindowConfig& window_config);
    WindowConfig editor_window_config_to_window_config(const EditorWindowConfig& editor_window_config);

    std::filesystem::path get_editor_config_file_path();
    std::filesystem::path get_editor_imgui_ini_file_path();

    HostConfig make_editor_host_config(const GraphicsConfig& graphics_config, const EditorConfig& editor_config);
} // namespace hob::editor
