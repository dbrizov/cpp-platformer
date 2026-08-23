#include "editor_config.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
#include "engine/core/systems/window.h"

namespace hob::editor {
    namespace {
        constexpr int32_t JSON_INDENT = 4;

        nlohmann::ordered_json editor_window_config_to_json(const EditorWindowConfig& config) {
            return {
                {"x", config.x},
                {"y", config.y},
                {"width", config.width},
                {"height", config.height},
                {"maximized", config.maximized},
            };
        }

        EditorWindowConfig json_to_editor_window_config(const nlohmann::json& json, const char* window_name) {
            EditorWindowConfig config;
            if (!json.contains(window_name)) {
                return config;
            }

            const auto& w = json[window_name];
            config.x = w.value("x", config.x);
            config.y = w.value("y", config.y);
            config.width = w.value("width", config.width);
            config.height = w.value("height", config.height);
            config.maximized = w.value("maximized", config.maximized);

            if (!config.has_size()) {
                config.width = 0;
                config.height = 0;
            }

            return config;
        }
    } // namespace

    bool EditorWindowConfig::has_size() const {
        return width > 0 && height > 0;
    }

    EditorConfig::EditorConfig(const std::filesystem::path& json_path) {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            return;
        }

        nlohmann::json json;
        try {
            file >> json;
        }
        catch (const nlohmann::json::exception& e) {
            log::engine.error("Cannot parse editor config file '{}': {}", json_path.string(), e.what());
            return;
        }

        main_window = json_to_editor_window_config(json, "main_window");
        game_window = json_to_editor_window_config(json, "game_window");

        last_open_scene = json.value("last_open_scene", last_open_scene);
    }

    void EditorConfig::save(const std::filesystem::path& json_path) const {
        nlohmann::ordered_json json;
        json["main_window"] = editor_window_config_to_json(main_window);
        json["game_window"] = editor_window_config_to_json(game_window);
        json["last_open_scene"] = last_open_scene;

        std::ofstream out(json_path);
        if (!out.is_open()) {
            log::engine.error("Cannot write editor config file '{}'", json_path.string());
            return;
        }
        out << json.dump(JSON_INDENT) << '\n';
    }

    EditorWindowConfig create_editor_window_config_from_window(const Window& window) {
        EditorWindowConfig config;
        window.get_position(config.x, config.y);
        window.get_size(config.width, config.height);
        config.maximized = window.is_maximized();
        return config;
    }

    EditorWindowConfig create_editor_window_config_from_window_config(const WindowConfig& window_config) {
        EditorWindowConfig editor_window_config;
        editor_window_config.x = window_config.x;
        editor_window_config.y = window_config.y;
        editor_window_config.width = window_config.width;
        editor_window_config.height = window_config.height;
        editor_window_config.maximized = window_config.maximized;
        return editor_window_config;
    }

    void apply_editor_window_config(const EditorWindowConfig& editor_window_config, WindowConfig& out_window_config) {
        out_window_config.x = editor_window_config.x;
        out_window_config.y = editor_window_config.y;
        out_window_config.width = editor_window_config.width;
        out_window_config.height = editor_window_config.height;
        out_window_config.maximized = editor_window_config.maximized;
    }

    std::filesystem::path get_editor_config_file_path() {
        return PathUtils::get_project_config_root() / "editor_config.json";
    }

    std::filesystem::path get_editor_imgui_ini_file_path() {
        return PathUtils::get_project_config_root() / "editor_imgui.ini";
    }

    HostConfig make_editor_host_config(const GraphicsConfig& graphics_config, const EditorConfig& editor_config) {
        WindowConfig main_window_config;
        if (editor_config.main_window.has_size()) {
            apply_editor_window_config(editor_config.main_window, main_window_config);
        }
        else {
            main_window_config.maximized = true;
        }

        main_window_config.title = "Hob2D Editor";
        main_window_config.vsync = graphics_config.vsync_enabled;

        HostConfig host_config;
        host_config.main_window_override = main_window_config;
        host_config.main_window_hosts_game = false;
        host_config.run_project_main_on_boot = false;

        return host_config;
    }
} // namespace hob::editor
