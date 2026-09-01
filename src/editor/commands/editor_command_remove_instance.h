#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor/editor_instances.h"
#include "editor/editor_selection.h"
#include "editor_command.h"

namespace hob::editor {
    class EditorCommandRemoveInstance : public EditorCommand {
        sol::table m_instance;
        EditorInstanceId m_instance_id;

        // Recorded by redo rather than by the constructor: every removal shifts the instances after it,
        // so a composite's reverse-order undo only lands where it started when each command carries the
        // index that held at the moment it ran.
        int32_t m_index = APPEND_INSTANCE_INDEX;

    public:
        EditorCommandRemoveInstance(std::string label, sol::table instance, EditorInstanceId instance_id);

        void undo(Editor& editor) override;
        void redo(Editor& editor) override;
    };
} // namespace hob::editor
