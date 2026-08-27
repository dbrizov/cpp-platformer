#include "editor_files.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include "editor.h"
#include "editor_lua.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/scripting/lua_script_system.h"

namespace hob::editor {
    namespace {
        std::string get_scene_save_error(const Editor& editor) {
            if (editor.get_current_scene().empty()) {
                return "no scene is open";
            }

            if (editor.get_state() != EditorState::Edit) {
                return "the scene document is only editable in Edit state";
            }

            Engine& engine = editor.get_engine();
            if (!get_editor_func(engine, editor_func::GET_SCENE_SAVE_ERROR).valid()) {
                return std::format("{} is unavailable", editor_func::GET_SCENE_SAVE_ERROR);
            }

            const sol::object result =
                editor_call(engine, editor_func::GET_SCENE_SAVE_ERROR, editor.get_current_scene());

            return result.is<std::string>() ? result.as<std::string>() : std::string();
        }

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
    } // namespace

    bool can_save_scene(const Editor& editor) {
        return get_scene_save_error(editor).empty();
    }

    void save_scene(Editor& editor) {
        const std::string reason = get_scene_save_error(editor);
        if (!reason.empty()) {
            log::editor.error("Cannot save the scene because {}", reason);
            return;
        }

        Engine& engine = editor.get_engine();
        const std::string scene_name = editor.get_current_scene();

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
} // namespace hob::editor
