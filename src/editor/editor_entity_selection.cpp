#include "editor_entity_selection.h"

#include <utility>

namespace hob::editor {
    void EditorEntitySelection::apply_click(const EditorSelectionClick& click,
                                            const std::vector<EntityId>& visible_order) {
        if (click.additive) {
            toggle(click.entity_id);
            range_anchor = click.entity_id;
            return;
        }

        const auto anchor_it = std::ranges::find(visible_order, range_anchor);
        const auto clicked_it = std::ranges::find(visible_order, click.entity_id);

        if (click.range && anchor_it != visible_order.end() && clicked_it != visible_order.end()) {
            auto first = anchor_it;
            auto last = clicked_it;
            if (first > last) {
                std::swap(first, last);
            }

            ids.clear();
            for (auto it = first; it != last + 1; ++it) {
                add(*it);
            }

            // Keep the clicked row primary regardless of which way the range runs.
            add(click.entity_id);
            return;
        }

        set(click.entity_id);
        range_anchor = click.entity_id;
    }
} // namespace hob::editor
