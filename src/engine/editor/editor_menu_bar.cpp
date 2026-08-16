#include <string>

#include <SDL3/SDL_events.h>

#include "editor.h"
#include "editor_gui_utils.h"

namespace hob::editor {
    namespace {
        std::string make_command_label(const char* verb, const std::string& command_label) {
            return command_label.empty() ? std::string(verb) : std::string(verb) + " " + command_label;
        }
    } // namespace

    void Editor::draw_menu_bar() {
        if (begin_menu("File")) {
            if (menu_item("Quit")) {
                SDL_Event quit_event{};
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
            }

            end_menu();
        }

        if (begin_menu("Edit")) {
            if (menu_item(
                    make_command_label("Undo", m_commands.get_undo_label()).c_str(), "Ctrl+Z", m_commands.can_undo())) {
                m_commands.undo(m_engine);
            }

            if (menu_item(
                    make_command_label("Redo", m_commands.get_redo_label()).c_str(), "Ctrl+Y", m_commands.can_redo())) {
                m_commands.redo(m_engine);
            }

            end_menu();
        }

        if (begin_menu("Editor")) {
            if (menu_item("Reset Layout")) {
                m_reset_layout = true;
            }

            end_menu();
        }
    }
} // namespace hob::editor
