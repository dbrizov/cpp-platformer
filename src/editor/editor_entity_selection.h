#pragma once

#include <algorithm>
#include <vector>

#include "engine/entity/entity.h"

namespace hob::editor {
    using EditorInstanceId = int64_t;
    constexpr EditorInstanceId INVALID_EDITOR_INSTANCE_ID = -1;

    struct EditorSelectionClick {
        EntityId entity_id = INVALID_ENTITY_ID;
        bool additive = false; // Ctrl
        bool range = false; // Shift
    };

    struct EditorEntitySelection {
        std::vector<EntityId> ids;
        EntityId range_anchor = INVALID_ENTITY_ID;

        bool empty() const {
            return ids.empty();
        }

        bool contains(EntityId id) const {
            return std::ranges::find(ids, id) != ids.end();
        }

        EntityId primary() const {
            return ids.empty() ? INVALID_ENTITY_ID : ids.back();
        }

        void clear() {
            ids.clear();
            range_anchor = INVALID_ENTITY_ID;
        }

        void set(EntityId id) {
            ids.clear();
            ids.push_back(id);
        }

        void add(EntityId id) {
            remove(id);
            ids.push_back(id);
        }

        void remove(EntityId id) {
            std::erase(ids, id);
        }

        void toggle(EntityId id) {
            if (contains(id)) {
                remove(id);
            }
            else {
                add(id);
            }
        }

        // visible_order is the rows a range select may span, and is empty where ranges make no sense.
        void apply_click(const EditorSelectionClick& click, const std::vector<EntityId>& visible_order);
    };

    struct EditorSelectionInstanceIds {
        std::vector<EditorInstanceId> ids;
        EditorInstanceId range_anchor = INVALID_EDITOR_INSTANCE_ID;
    };
} // namespace hob::editor
