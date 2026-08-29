#include "editor_files.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "editor.h"
#include "editor_file_dialog.h"
#include "editor_lua.h"
#include "editor_modal.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
#include "engine/core/systems/scripting/lua_schema_keys.h"
#include "engine/core/systems/scripting/lua_script_system.h"
#include "engine/core/systems/window.h"

namespace hob::editor {
    namespace {
        constexpr const char* SCRIPTS_FOLDER = "scripts";
        constexpr const char* SCENES_FOLDER = "scenes";

        constexpr const char* SCENE_FILE_FILTER_NAME = "Scene";

        constexpr const char* SCENE_CREATE_ERROR_TITLE = "Cannot Create Scene";
        constexpr const char* NEW_SCENE_DIALOG_TITLE = "New Scene";
        constexpr const char* SAVE_SCENE_AS_DIALOG_TITLE = "Save Scene As";

        bool write_file(const std::filesystem::path& path, const std::string& text) {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            if (!out) {
                log::editor.error("Cannot open '{}' for writing", path.string());
                return false;
            }

            out.write(text.data(), static_cast<std::streamsize>(text.size()));
            out.close();

            if (!out) {
                log::editor.error("Failed while writing '{}'", path.string());
                return false;
            }

            return true;
        }

        std::filesystem::path get_project_scripts_root() {
            return PathUtils::get_project_root() / SCRIPTS_FOLDER;
        }

        bool is_under_project_scripts(const std::filesystem::path& path) {
            std::error_code ec;

            const std::filesystem::path scripts_root =
                std::filesystem::weakly_canonical(get_project_scripts_root(), ec);
            if (ec) {
                return false;
            }

            const std::filesystem::path resolved = std::filesystem::weakly_canonical(path, ec);
            if (ec) {
                return false;
            }

            const std::filesystem::path relative = resolved.lexically_relative(scripts_root);

            return !relative.empty() && *relative.begin() != "..";
        }

        // The dialog filter spells extensions without the leading dot.
        std::string get_scene_file_filter_pattern() {
            return std::string(std::string_view(file_extension::SCENE).substr(1));
        }

        std::filesystem::path get_default_scene_folder(Editor& editor) {
            const sol::object file =
                editor_call(editor.get_engine(), editor_func::GET_SCENE_FILE, editor.get_current_scene());
            if (file.is<std::string>()) {
                return std::filesystem::path(file.as<std::string>()).parent_path();
            }

            const std::filesystem::path scripts_root = get_project_scripts_root();
            const std::filesystem::path scenes_folder = scripts_root / SCENES_FOLDER;

            return std::filesystem::exists(scenes_folder) ? scenes_folder : scripts_root;
        }

        EditorFileDialogConfig make_scene_file_dialog_config(Editor& editor, const char* title) {
            return {
                .type = EditorFileDialogType::SaveFile,
                .title = title,
                .filters = {{.name = SCENE_FILE_FILTER_NAME, .pattern = get_scene_file_filter_pattern()}},
                .default_location = get_default_scene_folder(editor),
                .required_suffix = file_extension::SCENE,
                .parent_window = editor.get_engine().get_main_window().get_window(),
            };
        }

        void report_scene_create_error(Editor& editor, const std::filesystem::path& path, const std::string& reason) {
            log::editor.error("Cannot create a scene at '{}' because {}", path.string(), reason);

            editor.get_modal().open({
                .title = SCENE_CREATE_ERROR_TITLE,
                .message = std::format("Cannot create a scene at '{}'.", path.string()),
                .reason = reason,
                .buttons = {.confirm = "OK"},
            });
        }

        std::string get_scene_name_for_file(Engine& engine, const std::filesystem::path& path) {
            const sol::object name = editor_call(engine, editor_func::GET_SCENE_NAME_FOR_FILE, path.string());

            return name.is<std::string>() ? name.as<std::string>() : std::string();
        }

        // The reload is what runs the file just written, which is what makes M8a's recorder stamp its
        // path before the scene is opened from it.
        void publish_new_scene(Editor& editor, const std::string& scene_name) {
            LuaScriptSystem& lua_script_system = editor.get_engine().get_lua_script_system();
            lua_script_system.hot_reload();
            lua_script_system.rebaseline_script_watch();

            editor.request_open_scene(scene_name);
        }
    } // namespace

    std::optional<std::string> get_scene_save_error(const Editor& editor) {
        if (editor.get_current_scene().empty()) {
            return "no scene is open";
        }

        if (editor.get_state() != WorldState::Stopped) {
            return "the scene document is only editable while the world is stopped";
        }

        Engine& engine = editor.get_engine();
        if (!get_editor_func(engine, editor_func::GET_SCENE_SAVE_ERROR).valid()) {
            return std::format("{} is unavailable", editor_func::GET_SCENE_SAVE_ERROR);
        }

        const sol::object result = editor_call(engine, editor_func::GET_SCENE_SAVE_ERROR, editor.get_current_scene());
        if (result.is<std::string>()) {
            return result.as<std::string>();
        }

        return std::nullopt;
    }

    bool can_save_scene(const Editor& editor) {
        return !get_scene_save_error(editor).has_value();
    }

    void save_scene(Editor& editor) {
        const std::optional<std::string> reason = get_scene_save_error(editor);
        if (reason.has_value()) {
            log::editor.error("Cannot save the scene because {}", *reason);
            return;
        }

        Engine& engine = editor.get_engine();
        const std::string& scene_name = editor.get_current_scene();

        const sol::object file = editor_call(engine, editor_func::GET_SCENE_FILE, scene_name);
        if (!file.is<std::string>()) {
            log::editor.error("Scene '{}' has no recorded source file", scene_name);
            return;
        }

        const sol::object source = editor_call(engine, editor_func::SERIALIZE_SCENE, scene_name);
        if (!source.is<std::string>()) {
            return;
        }

        const std::filesystem::path path = file.as<std::string>();
        if (!write_file(path, source.as<std::string>())) {
            return;
        }

        editor_call(engine, editor_func::MARK_SCENE_SAVED);

        LuaScriptSystem& lua_script_system = engine.get_lua_script_system();
        lua_script_system.hot_reload();
        lua_script_system.rebaseline_script_watch();

        log::editor.info("Saved scene '{}' to '{}'", scene_name, path.string());
    }

    void revert_scene(Editor& editor) {
        Engine& engine = editor.get_engine();
        const std::string& scene_name = editor.get_current_scene();

        const sol::object file = editor_call(engine, editor_func::GET_SCENE_FILE, scene_name);
        if (!file.is<std::string>()) {
            log::editor.error("Cannot revert scene '{}' because it has no recorded source file", scene_name);
            return;
        }

        const std::filesystem::path path = file.as<std::string>();
        if (!engine.get_lua_script_system().run_file(path)) {
            return;
        }

        editor_call(engine, editor_func::MARK_SCENE_SAVED);

        log::editor.info("Reverted scene '{}' from '{}'", scene_name, path.string());
    }

    bool can_new_scene(const Editor& editor) {
        return editor.get_state() == WorldState::Stopped;
    }

    // Deliberately weaker than can_save_scene: a scene with no recorded file, or one sharing its file
    // with another definition, is exactly what Save As exists to move out of.
    bool can_save_scene_as(const Editor& editor) {
        return !editor.get_current_scene().empty() && editor.get_state() == WorldState::Stopped;
    }

    void show_new_scene_dialog(Editor& editor) {
        EditorFileDialogConfig config = make_scene_file_dialog_config(editor, NEW_SCENE_DIALOG_TITLE);
        config.on_pick = [&editor](const std::filesystem::path& path) {
            new_scene(editor, path);
        };

        editor.get_file_dialog().open(std::move(config));
    }

    void show_save_scene_as_dialog(Editor& editor) {
        EditorFileDialogConfig config = make_scene_file_dialog_config(editor, SAVE_SCENE_AS_DIALOG_TITLE);
        config.on_pick = [&editor](const std::filesystem::path& path) {
            save_scene_as(editor, path);
        };

        editor.get_file_dialog().open(std::move(config));
    }

    std::optional<std::string> get_scene_create_error(const Editor& editor, const std::filesystem::path& path) {
        if (!is_under_project_scripts(path)) {
            return std::format("a scene must live under '{}'", get_project_scripts_root().string());
        }

        Engine& engine = editor.get_engine();
        if (!get_editor_func(engine, editor_func::GET_SCENE_CREATE_ERROR).valid()) {
            return std::format("{} is unavailable", editor_func::GET_SCENE_CREATE_ERROR);
        }

        const sol::object result = editor_call(engine, editor_func::GET_SCENE_CREATE_ERROR, path.string());
        if (result.is<std::string>()) {
            return result.as<std::string>();
        }

        return std::nullopt;
    }

    void new_scene(Editor& editor, const std::filesystem::path& path) {
        const std::optional<std::string> reason = get_scene_create_error(editor, path);
        if (reason.has_value()) {
            report_scene_create_error(editor, path, *reason);
            return;
        }

        Engine& engine = editor.get_engine();
        const std::string scene_name = get_scene_name_for_file(engine, path);
        if (scene_name.empty()) {
            log::editor.error("'{}' does not name a scene", path.string());
            return;
        }

        const sol::object source = editor_call(engine, editor_func::SERIALIZE_NEW_SCENE, scene_name);
        if (!source.is<std::string>()) {
            return;
        }

        if (!write_file(path, source.as<std::string>())) {
            return;
        }

        log::editor.info("Created scene '{}' in '{}'", scene_name, path.string());

        publish_new_scene(editor, scene_name);
    }

    void save_scene_as(Editor& editor, const std::filesystem::path& path) {
        const std::optional<std::string> reason = get_scene_create_error(editor, path);
        if (reason.has_value()) {
            report_scene_create_error(editor, path, *reason);
            return;
        }

        Engine& engine = editor.get_engine();
        const std::string source_scene_name = editor.get_current_scene();
        const std::string scene_name = get_scene_name_for_file(engine, path);
        if (scene_name.empty()) {
            log::editor.error("'{}' does not name a scene", path.string());
            return;
        }

        const sol::object source = editor_call(engine, editor_func::SERIALIZE_SCENE, source_scene_name, scene_name);
        if (!source.is<std::string>()) {
            return;
        }

        if (!write_file(path, source.as<std::string>())) {
            return;
        }

        // Before the reload, so rebind_instance_defs takes the file values of the scene being left
        // rather than the in-memory overrides, which now belong to the new file.
        editor_call(engine, editor_func::MARK_SCENE_SAVED);

        log::editor.info("Saved scene '{}' as '{}' in '{}'", source_scene_name, scene_name, path.string());

        publish_new_scene(editor, scene_name);
    }
} // namespace hob::editor
