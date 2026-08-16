#include <string>

#include <imgui.h>
#include <sol/sol.hpp>

#include "editor.h"
#include "editor_gui_utils.h"
#include "editor_lua.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/entity/entity.h"
#include "engine/math/color.h"
#include "engine/math/vector2.h"

namespace hob::editor {
    namespace {
        template<typename T>
        T value_or(const sol::object& value, const T& fallback) {
            return value.is<T>() ? value.as<T>() : fallback;
        }

        void draw_field(Engine& engine, const sol::table& field) {
            const std::string name = field.get_or<std::string>("name", "?");
            const std::string kind = field.get_or<std::string>("kind", "");
            const sol::object value = field["value"];

            ImGui::BeginDisabled();

            if (kind == "int") {
                int64_t number = value_or<int64_t>(value, 0);
                field_int(name.c_str(), number);
            }
            else if (kind == "float") {
                float number = value_or<float>(value, 0.0f);
                field_float(name.c_str(), number);
            }
            else if (kind == "angle") {
                float degrees = value_or<float>(value, 0.0f);
                field_angle(name.c_str(), degrees);
            }
            else if (kind == "bool") {
                bool flag = value_or<bool>(value, false);
                field_bool(name.c_str(), flag);
            }
            else if (kind == "string") {
                std::string text = value_or<std::string>(value, "");
                field_string(name.c_str(), text);
            }
            else if (kind == "vector2") {
                Vector2 vector = value_or<Vector2>(value, Vector2());
                field_vector2(name.c_str(), vector);
            }
            else if (kind == "color") {
                Color color = value_or<Color>(value, Color());
                field_color(name.c_str(), color);
            }
            else {
                field_text(name.c_str(), lua_object_to_display_string(engine, value));
            }

            ImGui::EndDisabled();
        }

        void draw_component(Engine& engine, int index, const sol::table& component) {
            const std::string name = component.get_or<std::string>("name", "?");
            const bool is_lua = component.get_or("is_lua", false);
            const std::string header = is_lua ? name + " (Lua)" : name;

            ImGui::SeparatorText(header.c_str());

            ImGui::PushID(index);

            const sol::object fields = component["fields"];
            if (fields.is<sol::table>()) {
                const sol::table rows = fields.as<sol::table>();
                for (int i = 1; i <= rows.size(); ++i) {
                    const sol::object row = rows[i];
                    if (row.is<sol::table>()) {
                        draw_field(engine, row.as<sol::table>());
                    }
                }
            }

            ImGui::PopID();
        }

        void draw_components(Engine& engine, EntityId entity_id) {
            const sol::object components = editor_call(engine, "get_components", entity_id);
            if (!components.is<sol::table>()) {
                ImGui::TextDisabled("Editor.get_components is unavailable");
                return;
            }

            const sol::table sections = components.as<sol::table>();
            for (int i = 1; i <= sections.size(); ++i) {
                const sol::object section = sections[i];
                if (section.is<sol::table>()) {
                    draw_component(engine, i, section.as<sol::table>());
                }
            }
        }
    } // namespace

    void Editor::draw_inspector() {
        if (begin_panel(PANEL_INSPECTOR)) {
            // Multi-selection inspects the primary; the rest still move together via the SceneView.
            Entity* entity = m_engine.get_entity_spawner().get_entity(m_selection.primary());
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

                if (m_selection.ids.size() > 1) {
                    ImGui::TextDisabled("(%zu selected)", m_selection.ids.size());
                }

                draw_components(m_engine, entity->get_id());
            }
        }
        end_panel();
    }
} // namespace hob::editor
