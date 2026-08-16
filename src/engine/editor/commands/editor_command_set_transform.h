#pragma once

#include <string>

#include "editor_command.h"
#include "engine/entity/entity.h"
#include "engine/math/vector2.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    struct TransformState {
        Vector2 position;
        float rotation = 0.0f; // In radians
        Vector2 scale = Vector2(1.0f, 1.0f);

        static TransformState capture(const TransformComponent& transform);
    };

    class EditorCommandSetTransform : public EditorCommand {
        EntityId m_entity_id;
        TransformState m_old_state;
        TransformState m_new_state;

    public:
        EditorCommandSetTransform(std::string label,
                                  EntityId entity_id,
                                  const TransformState& old_state,
                                  const TransformState& new_state);

        void undo(Engine& engine) override;
        void redo(Engine& engine) override;

        static void apply(Engine& engine, EntityId entity_id, const TransformState& state);
    };
} // namespace hob::editor
