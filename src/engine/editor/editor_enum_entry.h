#pragma once

#include <string>

namespace hob::editor {
    struct EditorEnumEntry {
        std::string name;
        int64_t value = 0;
    };
} // namespace hob::editor
