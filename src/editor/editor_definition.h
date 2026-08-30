#pragma once

#include <string>

namespace hob::editor {
    struct EditorDefinitionRef {
        std::string registry;
        std::string name;

        bool is_valid() const {
            return !registry.empty() && !name.empty();
        }

        bool operator==(const EditorDefinitionRef&) const = default;
    };

    struct EditorDefinition {
        EditorDefinitionRef ref;
        std::string file;
        bool read_only = false;
    };
} // namespace hob::editor
