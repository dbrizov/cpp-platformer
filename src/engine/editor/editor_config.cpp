#include "editor_config.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
#include "engine/core/systems/window.h"

namespace hob::editor {
    namespace {
        constexpr int JSON_INDENT = 4;
    } // namespace

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

        if (!json.contains("window")) {
            return;
        }

        const auto& w = json["window"];
        x = w.value("x", x);
        y = w.value("y", y);
        width = w.value("width", width);
        height = w.value("height", height);
        maximized = w.value("maximized", maximized);

        if (width <= 0 || height <= 0) {
            width = 0;
            height = 0;
        }
    }

    void EditorConfig::save(const std::filesystem::path& json_path) const {
        nlohmann::json json;
        std::ifstream in(json_path);
        if (in.is_open()) {
            try {
                in >> json;
            }
            catch (const nlohmann::json::exception&) {
                json = nlohmann::json::object();
            }
        }

        json["window"] = {
            {"x", x},
            {"y", y},
            {"width", width},
            {"height", height},
            {"maximized", maximized},
        };

        std::ofstream out(json_path);
        if (!out.is_open()) {
            log::engine.error("Cannot write editor config file '{}'", json_path.string());
            return;
        }
        out << json.dump(JSON_INDENT) << '\n';
    }

    std::filesystem::path get_editor_config_file_path() {
        return PathUtils::get_project_config_root() / "editor_config.json";
    }

    std::filesystem::path get_editor_imgui_ini_file_path() {
        return PathUtils::get_project_config_root() / "editor_imgui.ini";
    }

    HostConfig make_editor_host_config(const GraphicsConfig& graphics_config, const EditorConfig& editor_config) {
        WindowConfig window_config;
        window_config.title = "Hob2D Editor";
        window_config.vsync = graphics_config.vsync_enabled;

        const bool have_saved_geometry = editor_config.width > 0 && editor_config.height > 0;
        if (have_saved_geometry) {
            window_config.width = editor_config.width;
            window_config.height = editor_config.height;
            window_config.x = editor_config.x;
            window_config.y = editor_config.y;
            window_config.maximized = editor_config.maximized;
        }
        else {
            window_config.maximized = true;
        }

        HostConfig host_config;
        host_config.main_window_override = window_config;
        host_config.main_window_hosts_game = false;
        host_config.run_project_main_on_boot = false;

        return host_config;
    }
} // namespace hob::editor
