#include "editor_menu_bar.h"

#include "engine/editor/actions/editor_action.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_gui_utils.h"

namespace hob::editor {
    void EditorMenuBar::draw(Editor& editor) {
        if (begin_menu("File")) {
            action_menu_item(editor, EditorActionId::Quit);
            end_menu();
        }

        if (begin_menu("Edit")) {
            action_menu_item(editor, EditorActionId::Undo);
            action_menu_item(editor, EditorActionId::Redo);
            end_menu();
        }

        if (begin_menu("Editor")) {
            action_menu_item(editor, EditorActionId::ResetLayout);
            end_menu();
        }
    }
} // namespace hob::editor
