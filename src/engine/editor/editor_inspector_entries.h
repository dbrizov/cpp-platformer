#pragma once

#include <string>

namespace hob::editor {
    struct EditorInspectorEntryEnum {
        std::string name;
        int64_t value = 0;
    };

    struct EditorInspectorEntryAsset {
        std::string display_name;
        std::string registry_alias;
    };
} // namespace hob::editor
