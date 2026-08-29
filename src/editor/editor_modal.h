#pragma once

#include <functional>
#include <optional>
#include <string>

namespace hob::editor {
    struct EditorModalButtons {
        std::string confirm; // An empty label omits the button.
        std::string discard;
        std::string cancel;
        bool is_confirm_enabled = true;
    };

    struct EditorModalConfig {
        std::string title;
        std::string message;
        std::optional<std::string> reason;
        EditorModalButtons buttons;

        // A null handler closes the modal without doing anything.
        std::function<void()> on_confirm;
        std::function<void()> on_discard;
        std::function<void()> on_cancel;
    };

    class EditorModal {
        std::optional<EditorModalConfig> m_config;

    public:
        void open(EditorModalConfig config);
        bool is_open() const;
        void draw();
    };
} // namespace hob::editor
