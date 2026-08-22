#include "editor_command_set_field.h"

#include <string>
#include <utility>

#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"
#include "engine/core/systems/scripting/lua_schema_keys.h"
#include "engine/editor/editor_lua.h"

namespace hob::editor {
    namespace {
        void sync_transform_to_physics(Engine& engine, EntityId entity_id) {
            Entity* entity = engine.get_entity_spawner().get_entity(entity_id);
            if (entity == nullptr) {
                return;
            }

            RigidbodyComponent* rigidbody = entity->get_rigidbody();
            if (rigidbody == nullptr || rigidbody->get_body_type() == BodyType::Static) {
                return;
            }

            const TransformComponent* transform = entity->get_transform();
            rigidbody->set_position(transform->get_position());
            rigidbody->set_rotation(transform->get_rotation());
        }
    } // namespace

    EditorCommandSetField::EditorCommandSetField(std::string label,
                                                 EditorFieldTarget target,
                                                 sol::object old_value,
                                                 sol::object new_value)
        : EditorCommand(std::move(label))
        , m_target(std::move(target))
        , m_old_value(std::move(old_value))
        , m_new_value(std::move(new_value)) {}

    void EditorCommandSetField::undo(Engine& engine) {
        apply(engine, m_target, m_old_value);
    }

    void EditorCommandSetField::redo(Engine& engine) {
        apply(engine, m_target, m_new_value);
    }

    void EditorCommandSetField::apply(Engine& engine, const EditorFieldTarget& target, const sol::object& value) {
        if (target.is_lua) {
            editor_call(engine, "set_live_lua_field", target.entity_id, target.component_index, target.field, value);
            return;
        }

        editor_call(engine, "set_live_field", target.entity_id, target.component_key, target.field, value);

        if (target.component_key == transform_key::SECTION) {
            sync_transform_to_physics(engine, target.entity_id);
        }
    }
} // namespace hob::editor
