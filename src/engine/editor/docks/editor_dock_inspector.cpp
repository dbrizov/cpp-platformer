#include "editor_dock_inspector.h"

#include <memory>
#include <string>
#include <utility>

#include <imgui.h>
#include <sol/sol.hpp>

#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/core/systems/scripting/lua_script_system.h"
#include "engine/editor/commands/editor_command_set_field.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_gui_utils.h"
#include "engine/editor/editor_lua.h"
#include "engine/entity/entity.h"
#include "engine/math/color.h"
#include "engine/math/constants.h"
#include "engine/math/vector2.h"

namespace hob::editor {
    struct EditorDockInspectorPendingEdit {
        EditorFieldTarget target;
        sol::object old_value;
        bool active = false;
    };

    namespace {
        template<typename T>
        T value_or(const sol::object& value, const T& fallback) {
            return value.is<T>() ? value.as<T>() : fallback;
        }

        void begin_pending_edit(EditorDockInspectorPendingEdit& pending,
                                const EditorFieldTarget& target,
                                const sol::object& old_value) {
            pending.target = target;
            pending.old_value = old_value;
            pending.active = true;
        }

        void clear_pending_edit(EditorDockInspectorPendingEdit& pending) {
            pending = EditorDockInspectorPendingEdit{};
        }

        void commit_field_edit(Editor& editor,
                               EditorDockInspectorPendingEdit& pending,
                               const EditorFieldTarget& target,
                               const std::string& command_label,
                               const sol::object& old_value,
                               const sol::object& new_value,
                               bool changed) {
            Engine& engine = editor.get_engine();

            if (changed) {
                if (!pending.active || !(pending.target == target)) {
                    begin_pending_edit(pending, target, old_value);
                }

                EditorCommandSetField::apply(engine, target, new_value);
            }

            if (!pending.active || !(pending.target == target)) {
                return;
            }

            // Drags, checkboxes and text input all report their release. A pick inside field_color's
            // popup does not -- its items live in another window, so the row's own group never sees the
            // deactivation -- hence the second condition: nothing anywhere is being dragged any more.
            if (ImGui::IsItemDeactivatedAfterEdit() || !ImGui::IsAnyItemActive()) {
                const sol::object final_value = changed ? new_value : old_value;
                editor.get_commands().push(engine,
                                           std::make_unique<EditorCommandSetField>(
                                               command_label, pending.target, pending.old_value, final_value));
                clear_pending_edit(pending);
            }
        }

        void draw_field(Editor& editor,
                        EditorDockInspectorPendingEdit& pending,
                        const EditorFieldTarget& component_target,
                        const std::string& component_label,
                        const sol::table& field) {
            const std::string name = field.get_or<std::string>("name", "?");
            const std::string label = to_display_label(name);
            const std::string kind = field.get_or<std::string>("kind", "");
            const sol::object value = field["value"];

            Engine& engine = editor.get_engine();
            sol::state& lua = engine.get_lua_script_system().get_lua();
            sol::object new_value;
            bool changed = false;

            if (kind == "int") {
                int64_t number = value_or<int64_t>(value, 0);
                if (field_int(label.c_str(), number)) {
                    new_value = sol::make_object(lua, number);
                    changed = true;
                }
            }
            else if (kind == "float") {
                float number = value_or<float>(value, 0.0f);
                if (field_float(label.c_str(), number)) {
                    new_value = sol::make_object(lua, number);
                    changed = true;
                }
            }
            else if (kind == "angle") {
                float degrees = value_or<float>(value, 0.0f) * RAD_TO_DEG;
                if (field_angle(label.c_str(), degrees)) {
                    new_value = sol::make_object(lua, degrees * DEG_TO_RAD);
                    changed = true;
                }
            }
            else if (kind == "bool") {
                bool flag = value_or<bool>(value, false);
                if (field_bool(label.c_str(), flag)) {
                    new_value = sol::make_object(lua, flag);
                    changed = true;
                }
            }
            else if (kind == "string") {
                std::string text = value_or<std::string>(value, "");
                if (field_string(label.c_str(), text)) {
                    new_value = sol::make_object(lua, text);
                    changed = true;
                }
            }
            else if (kind == "vector2") {
                Vector2 vector = value_or<Vector2>(value, Vector2());
                if (field_vector2(label.c_str(), vector)) {
                    new_value = sol::make_object(lua, vector);
                    changed = true;
                }
            }
            else if (kind == "color") {
                Color color = value_or<Color>(value, Color());
                if (field_color(label.c_str(), color)) {
                    new_value = sol::make_object(lua, color);
                    changed = true;
                }
            }
            else {
                field_text(label.c_str(), lua_object_to_display_string(engine, value));
                return;
            }

            EditorFieldTarget target = component_target;
            target.field = name;

            commit_field_edit(
                editor, pending, target, "Set " + component_label + " " + label, value, new_value, changed);
        }

        void draw_component(Editor& editor,
                            EditorDockInspectorPendingEdit& pending,
                            EntityId entity_id,
                            int index,
                            const sol::table& component) {
            const std::string name = component.get_or<std::string>("name", "?");
            const std::string label = to_display_label(name);
            const bool is_lua = component.get_or("is_lua", false);
            const std::string header = is_lua ? label + " (Lua)" : label;

            EditorFieldTarget target;
            target.entity_id = entity_id;
            target.is_lua = is_lua;
            target.component_key = is_lua ? "" : name;
            target.component_index = is_lua ? component.get_or("index", 0) : 0;

            ImGui::PushID(index);

            if (component_header(header.c_str())) {
                const sol::object fields = component["fields"];
                if (fields.is<sol::table>()) {
                    const sol::table rows = fields.as<sol::table>();
                    for (int i = 1; i <= rows.size(); ++i) {
                        const sol::object row = rows[i];
                        if (row.is<sol::table>()) {
                            draw_field(editor, pending, target, label, row.as<sol::table>());
                        }
                    }
                }
            }

            ImGui::PopID();
        }

        void draw_components(Editor& editor, EditorDockInspectorPendingEdit& pending, EntityId entity_id) {
            if (pending.active && pending.target.entity_id != entity_id) {
                clear_pending_edit(pending);
            }

            Engine& engine = editor.get_engine();

            const sol::object components = editor_call(engine, "get_components", entity_id);
            if (!components.is<sol::table>()) {
                ImGui::TextDisabled("Editor.get_components is unavailable");
                return;
            }

            const sol::table sections = components.as<sol::table>();
            for (int i = 1; i <= sections.size(); ++i) {
                const sol::object section = sections[i];
                if (section.is<sol::table>()) {
                    draw_component(editor, pending, entity_id, i, section.as<sol::table>());
                }
            }
        }
    } // namespace

    EditorDockInspector::EditorDockInspector()
        : EditorDock(" Inspector ###Inspector", EditorActionContext::Inspector)
        , m_pending(std::make_unique<EditorDockInspectorPendingEdit>()) {}

    EditorDockInspector::~EditorDockInspector() = default;

    void EditorDockInspector::draw(Editor& editor) {
        if (begin()) {
            const EditorEntitySelection& selection = editor.get_selection();

            // Multi-selection inspects the primary; the rest still move together via the SceneView.
            Entity* entity = editor.get_engine().get_entity_spawner().get_entity(selection.primary());
            if (entity == nullptr) {
                ImGui::TextDisabled("Select an entity");
            }
            else {
                ImGui::Text("%s", entity->get_display_name().c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("#%lld", static_cast<long long>(entity->get_id()));

                if (!entity->get_prefab_name().empty()) {
                    ImGui::TextDisabled("Prefab: %s", entity->get_prefab_name().c_str());
                }

                if (selection.ids.size() > 1) {
                    ImGui::TextDisabled("(%zu selected)", selection.ids.size());
                }

                draw_components(editor, *m_pending, entity->get_id());
            }
        }
        end();
    }

    void EditorDockInspector::reset_edit_state() {
        clear_pending_edit(*m_pending);
    }
} // namespace hob::editor
