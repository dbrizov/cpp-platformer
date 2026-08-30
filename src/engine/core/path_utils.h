#pragma once

#include <filesystem>
#include <span>

namespace hob {
    class PathUtils {
    public:
        // Engine content (framework Lua + builtin assets) that ships with the binary.
        static const std::filesystem::path& get_engine_root();
        static const std::filesystem::path& get_engine_scripts_root();
        static const std::filesystem::path& get_engine_assets_root();

        // Active game project (Lua + assets + config).
        static std::filesystem::path resolve_project_root(int32_t argc, char* argv[]);
        static void set_project_root(const std::filesystem::path& project_root);
        static const std::filesystem::path& get_project_root();
        static const std::filesystem::path& get_project_scripts_root();
        static const std::filesystem::path& get_project_assets_root();
        static const std::filesystem::path& get_project_config_root();

        // The folders the Lua definition scan runs and the hot-reload watcher polls.
        static std::span<const std::filesystem::path> get_project_definition_roots();

        // Individual file paths.
        static std::filesystem::path get_engine_config_file_path();
        static std::filesystem::path get_input_config_file_path();
        static std::filesystem::path get_log_file_path();

        // Resolve an asset by relative path: the project's assets win, engine assets are the fallback.
        static std::filesystem::path resolve_asset_path(const std::filesystem::path& relative_path);
    };
} // namespace hob
