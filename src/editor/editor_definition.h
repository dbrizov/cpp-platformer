#pragma once

#include <string>

namespace hob::editor {
    struct EditorDefinition {
        std::string registry;
        std::string name;
        std::string file;
        bool read_only = false;
    };
} // namespace hob::editor
