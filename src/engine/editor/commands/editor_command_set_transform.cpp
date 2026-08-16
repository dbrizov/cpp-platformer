#include "editor_command_set_transform.h"

#include <utility>

#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/entity_spawner.h"

namespace hob::editor {
    TransformState TransformState::capture(const TransformComponent& transform) {
        return TransformState{
            transform.get_local_position(),
            transform.get_local_rotation(),
            transform.get_local_scale(),
        };
    }

    EditorCommandSetTransform::EditorCommandSetTransform(std::string label,
                                                         EntityId entity_id,
                                                         const TransformState& old_state,
                                                         const TransformState& new_state)
        : EditorCommand(std::move(label))
        , m_entity_id(entity_id)
        , m_old_state(old_state)
        , m_new_state(new_state) {}

    void EditorCommandSetTransform::undo(Engine& engine) {
        apply(engine, m_entity_id, m_old_state);
    }

    void EditorCommandSetTransform::redo(Engine& engine) {
        apply(engine, m_entity_id, m_new_state);
    }

    void EditorCommandSetTransform::apply(Engine& engine, EntityId entity_id, const TransformState& state) {
        Entity* entity = engine.get_entity_spawner().get_entity(entity_id);
        if (entity == nullptr) {
            return;
        }

        TransformComponent* transform = entity->get_transform();
        transform->set_local_position(state.position);
        transform->set_local_rotation(state.rotation);
        transform->set_local_scale(state.scale);

        // Physics overrides the transforms of simulated rigidbodies, so apply the state in the physics world aswell
        RigidbodyComponent* rigidbody = entity->get_rigidbody();
        if (rigidbody != nullptr && rigidbody->get_body_type() != BodyType::Static) {
            rigidbody->set_position(transform->get_position());
            rigidbody->set_rotation(transform->get_rotation());
        }
    }
} // namespace hob::editor
