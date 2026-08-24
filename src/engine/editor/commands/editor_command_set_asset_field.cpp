#include "editor_command_set_asset_field.h"

#include <utility>

#include "editor_command_set_field.h"
#include "engine/core/engine.h"
#include "engine/core/systems/scripting/lua_script_system.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_lua.h"

namespace hob::editor {
    namespace {
        sol::object resolve(Engine& engine, const std::string& factory_name, const EditorAssetValue& value) {
            if (!value.asset_name.empty()) {
                return editor_call(engine, editor_func::GET_ASSET_REF, factory_name, value.asset_name);
            }

            if (value.inline_asset.valid()) {
                return value.inline_asset;
            }

            return sol::make_object(engine.get_lua_script_system().get_lua(), sol::lua_nil);
        }
    } // namespace

    EditorCommandSetAssetField::EditorCommandSetAssetField(std::string label,
                                                           EditorFieldTarget target,
                                                           std::string factory_name,
                                                           EditorAssetValue old_value,
                                                           EditorAssetValue new_value)
        : EditorCommand(std::move(label))
        , m_target(std::move(target))
        , m_factory_name(std::move(factory_name))
        , m_old_value(std::move(old_value))
        , m_new_value(std::move(new_value)) {}

    void EditorCommandSetAssetField::undo(Editor& editor) {
        EditorCommandSetField::apply(editor, m_target, resolve(editor.get_engine(), m_factory_name, m_old_value));
    }

    void EditorCommandSetAssetField::redo(Editor& editor) {
        EditorCommandSetField::apply(editor, m_target, resolve(editor.get_engine(), m_factory_name, m_new_value));
    }
} // namespace hob::editor
