#pragma once

#include <string>

#include <sol/sol.hpp>

#include "editor_command.h"
#include "engine/editor/editor_field_target.h"

namespace hob {
    class Engine;
} // namespace hob

namespace hob::editor {
    struct EditorAssetValue {
        std::string asset_name;
        sol::object inline_asset; // Set only when the registry cannot name the asset; keeps it alive.
    };

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

        void undo(Engine& engine) override;
        void redo(Engine& engine) override;
    };
} // namespace hob::editor
