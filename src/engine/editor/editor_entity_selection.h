#pragma once

#include <algorithm>
#include <vector>

#include "engine/entity/entity.h"

namespace hob::editor {
    struct EditorEntitySelection {
        std::vector<EntityId> ids;

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
    };
} // namespace hob::editor
