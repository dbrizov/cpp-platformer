#include <SDL3/SDL_events.h>

#include "editor.h"
#include "editor_gui_utils.h"

namespace hob::editor {
    void Editor::draw_menu_bar() {
        if (begin_menu("File")) {
            if (menu_item("Quit")) {
                SDL_Event quit_event{};
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
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
