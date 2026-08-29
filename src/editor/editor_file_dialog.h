#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <SDL3/SDL_dialog.h>

namespace hob::editor {
    enum class EditorFileDialogType : uint8_t {
        OpenFile,
        SaveFile,
        OpenFolder,
    };

    struct EditorFileDialogFilter {
        std::string name;
        std::string pattern; // Extensions without the leading dot, separated by ';'.
    };

    struct EditorFileDialogConfig {
        EditorFileDialogType type = EditorFileDialogType::SaveFile;
        std::string title;
        std::vector<EditorFileDialogFilter> filters;
        std::filesystem::path default_location;
        std::string required_suffix; // Appended to the pick when missing. An empty one leaves it alone.
        SDL_Window* parent_window = nullptr;

        // A null handler dismisses the dialog without doing anything.
        std::function<void(const std::filesystem::path&)> on_pick;
        std::function<void()> on_cancel;
    };

    struct EditorFileDialogState {
        std::mutex mutex;
        bool has_result = false;
        std::filesystem::path picked_path;

        // Main-thread only, but the filter array must outlive the call: SDL keeps the pointer.
        EditorFileDialogConfig config;
        std::vector<SDL_DialogFileFilter> filters;

        std::shared_ptr<EditorFileDialogState> keep_alive_until_callback;
    };

    class EditorFileDialog {
        std::shared_ptr<EditorFileDialogState> m_state;

    public:
        void open(EditorFileDialogConfig config);
        bool is_open() const;
        void poll();
    };
} // namespace hob::editor
