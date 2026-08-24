#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor/editor_asset_value.h"
#include "editor/editor_field_target.h"
#include "editor_command.h"

namespace hob::editor {
    class EditorCommandSetAssetField : public EditorCommand {
        EditorFieldTarget m_target;
        std::string m_factory_name;
        EditorAssetValue m_old_value;
        EditorAssetValue m_new_value;

    public:
        EditorCommandSetAssetField(std::string label,
                                   EditorFieldTarget target,
                                   std::string factory_name,
                                   EditorAssetValue old_value,
                                   EditorAssetValue new_value);

        void undo(Editor& editor) override;
        void redo(Editor& editor) override;
    };
} // namespace hob::editor
