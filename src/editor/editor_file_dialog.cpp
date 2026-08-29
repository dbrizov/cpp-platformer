#include "editor_file_dialog.h"

#include <memory>
#include <mutex>
#include <utility>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_properties.h>

#include "engine/core/logging.h"

namespace hob::editor {
    namespace {
        SDL_FileDialogType to_sdl_type(EditorFileDialogType type) {
            switch (type) {
                case EditorFileDialogType::OpenFile:
                    return SDL_FILEDIALOG_OPENFILE;
                case EditorFileDialogType::OpenFolder:
                    return SDL_FILEDIALOG_OPENFOLDER;
                case EditorFileDialogType::SaveFile:
                    break;
            }

            return SDL_FILEDIALOG_SAVEFILE;
        }

        std::filesystem::path with_suffix(const std::filesystem::path& path, const std::string& suffix) {
            if (suffix.empty()) {
                return path;
            }

            const std::string text = path.string();
            if (text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0) {
                return path;
            }

            return std::filesystem::path(text + suffix);
        }
    } // namespace

    void EditorFileDialog::open(EditorFileDialogConfig config) {
        if (is_open()) {
            return;
        }

        // A fresh state per dialog, so a callback still in flight cannot report into the next one.
        m_state = std::make_shared<EditorFileDialogState>();
        m_state->config = std::move(config);

        m_state->filters.reserve(m_state->config.filters.size());
        for (const EditorFileDialogFilter& filter : m_state->config.filters) {
            m_state->filters.push_back({filter.name.c_str(), filter.pattern.c_str()});
        }

        const SDL_PropertiesID props = SDL_CreateProperties();
        if (props == 0) {
            log::editor.error("Cannot create the file dialog properties: {}", SDL_GetError());
            m_state.reset();
            return;
        }

        if (!m_state->filters.empty()) {
            SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, m_state->filters.data());
            SDL_SetNumberProperty(
                props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, static_cast<int64_t>(m_state->filters.size()));
        }

        if (m_state->config.parent_window != nullptr) {
            SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, m_state->config.parent_window);
        }

        // SDL splits the location at its last separator and pre-fills the name box with the rest, so a
        // folder needs a trailing one or it arrives as a suggested filename.
        const std::filesystem::path& default_location = m_state->config.default_location;
        const std::string location = std::filesystem::is_directory(default_location) ? (default_location / "").string()
                                                                                     : default_location.string();
        if (!location.empty()) {
            SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, location.c_str());
        }

        if (!m_state->config.title.empty()) {
            SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_TITLE_STRING, m_state->config.title.c_str());
        }

        // The state holds itself until the callback releases it, so a dialog still open when the
        // Editor dies reports into a live object rather than a freed one.
        m_state->keep_alive_until_callback = m_state;

        const auto on_dialog_finished = [](void* data, const char* const* filelist, int) {
            EditorFileDialogState& state = *static_cast<EditorFileDialogState*>(data);

            if (filelist == nullptr) {
                log::editor.error("The file dialog failed: {}", SDL_GetError());
            }

            // Released after the lock: dropping the last reference destroys the mutex being held.
            std::shared_ptr<EditorFileDialogState> owner;
            {
                const std::lock_guard<std::mutex> lock(state.mutex);
                state.has_result = true;
                if (filelist != nullptr && filelist[0] != nullptr) {
                    state.picked_path = std::filesystem::path(filelist[0]);
                }

                owner = std::move(state.keep_alive_until_callback);
            }
        };

        SDL_ShowFileDialogWithProperties(to_sdl_type(m_state->config.type), on_dialog_finished, m_state.get(), props);
        SDL_DestroyProperties(props);
    }

    bool EditorFileDialog::is_open() const {
        return m_state != nullptr;
    }

    void EditorFileDialog::poll() {
        if (!is_open()) {
            return;
        }

        std::filesystem::path picked_path;
        {
            const std::lock_guard<std::mutex> lock(m_state->mutex);
            if (!m_state->has_result) {
                return;
            }

            picked_path = std::move(m_state->picked_path);
        }

        // Detached before the handler runs, so the handler is free to open the next dialog.
        const std::shared_ptr<EditorFileDialogState> state = std::move(m_state);
        const EditorFileDialogConfig config = std::move(state->config);

        if (picked_path.empty()) {
            if (config.on_cancel) {
                config.on_cancel();
            }

            return;
        }

        if (config.on_pick) {
            config.on_pick(with_suffix(picked_path, config.required_suffix));
        }
    }
} // namespace hob::editor
