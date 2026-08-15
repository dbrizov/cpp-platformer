#include "editor_config.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/logging.h"

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
} // namespace hob::editor
