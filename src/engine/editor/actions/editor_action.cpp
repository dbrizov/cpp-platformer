#include "editor_action.h"

#include <iterator>

#include <imgui_internal.h>

#include "engine/core/assert.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_gui_utils.h"

namespace hob::editor {
    namespace {
        constexpr EditorAction ACTIONS[] = {
            {
                .id = EditorActionId::Undo,
                .label = "Undo",
                .chord = ImGuiMod_Ctrl | ImGuiKey_Z,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_commands().can_undo();
                    },
                .format_label =
                    [](const Editor& editor) {
                        return make_command_label("Undo", editor.get_commands().get_undo_label());
                    },
                .run =
                    [](Editor& editor) {
                        editor.get_commands().undo(editor.get_engine());
                    },
            },
            {
                .id = EditorActionId::Redo,
                .label = "Redo",
                .chord = ImGuiMod_Ctrl | ImGuiKey_Y,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_commands().can_redo();
                    },
                .format_label =
                    [](const Editor& editor) {
                        return make_command_label("Redo", editor.get_commands().get_redo_label());
                    },
                .run =
                    [](Editor& editor) {
                        editor.get_commands().redo(editor.get_engine());
                    },
            },
            {
                .id = EditorActionId::Play,
                .label = "Play",
                .chord = ImGuiKey_F5,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_state() != EditorState::Play;
                    },
                .format_label =
                    [](const Editor& editor) {
                        return std::string(editor.get_state() == EditorState::Paused ? "Resume" : "Play");
                    },
                .run =
                    [](Editor& editor) {
                        editor.set_state(EditorState::Play);
                    },
            },
            {
                .id = EditorActionId::Pause,
                .label = "Pause",
                .chord = ImGuiKey_None,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_state() == EditorState::Play;
                    },
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.set_state(EditorState::Paused);
                    },
            },
            {
                .id = EditorActionId::Step,
                .label = "Step",
                .chord = ImGuiKey_None,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_state() == EditorState::Paused;
                    },
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.request_step();
                    },
            },
            {
                .id = EditorActionId::Stop,
                .label = "Stop",
                .chord = ImGuiMod_Shift | ImGuiKey_F5,
                .context = EditorActionContext::Global,
                .is_enabled =
                    [](const Editor& editor) {
                        return editor.get_state() != EditorState::Edit;
                    },
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.set_state(EditorState::Edit);
                    },
            },
            {
                .id = EditorActionId::FocusSelection,
                .label = "Focus Selection",
                .chord = ImGuiKey_F,
                .context = EditorActionContext::SceneView,
                .is_enabled =
                    [](const Editor& editor) {
                        return !editor.get_selection().empty();
                    },
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.get_scene_view().focus_on_selection(editor);
                    },
            },
            {
                .id = EditorActionId::ResetLayout,
                .label = "Reset Layout",
                .chord = ImGuiKey_None,
                .context = EditorActionContext::Global,
                .is_enabled = nullptr,
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.request_reset_layout();
                    },
            },
            {
                .id = EditorActionId::Quit,
                .label = "Quit",
                .chord = ImGuiKey_None,
                .context = EditorActionContext::Global,
                .is_enabled = nullptr,
                .format_label = nullptr,
                .run =
                    [](Editor& editor) {
                        editor.request_quit();
                    },
            },
        };

        static_assert(std::size(ACTIONS) == static_cast<size_t>(EditorActionId::Count));
    } // namespace

    const EditorAction& get_action(EditorActionId id) {
        const EditorAction& action = ACTIONS[static_cast<size_t>(id)];
        HOB_CHECK(action.id == id, "Editor action table is out of order at index {}", static_cast<int>(id));

        return action;
    }

    std::span<const EditorAction> get_actions() {
        return std::span<const EditorAction>(ACTIONS);
    }

    uint32_t context_bit(EditorActionContext context) {
        return 1u << static_cast<uint32_t>(context);
    }

    bool is_action_enabled(const Editor& editor, EditorActionId id) {
        const EditorAction& action = get_action(id);
        return action.is_enabled == nullptr || action.is_enabled(editor);
    }

    std::string get_action_label(const Editor& editor, EditorActionId id) {
        const EditorAction& action = get_action(id);
        return action.format_label != nullptr ? action.format_label(editor) : std::string(action.label);
    }

    std::string make_command_label(const char* verb, const std::string& command_label) {
        return command_label.empty() ? std::string(verb) : std::string(verb) + " " + command_label;
    }

    std::string format_shortcut(ImGuiKeyChord chord) {
        const ImGuiKey key = static_cast<ImGuiKey>(chord & ~ImGuiMod_Mask_);
        if (key == ImGuiKey_None) {
            return std::string();
        }

        std::string text;
        if (chord & ImGuiMod_Ctrl) {
            text += "Ctrl+";
        }
        if (chord & ImGuiMod_Shift) {
            text += "Shift+";
        }
        if (chord & ImGuiMod_Alt) {
            text += "Alt+";
        }
        if (chord & ImGuiMod_Super) {
            text += "Super+";
        }

        text += ImGui::GetKeyName(key);

        return text;
    }

    bool is_chord_pressed(ImGuiKeyChord chord) {
        const ImGuiKey key = static_cast<ImGuiKey>(chord & ~ImGuiMod_Mask_);
        if (key == ImGuiKey_None) {
            return false;
        }

        // Exact match, so Ctrl+Shift+Z never fires the Ctrl+Z action.
        return ImGui::GetIO().KeyMods == (chord & ImGuiMod_Mask_) && ImGui::IsKeyPressed(key, false);
    }

    bool action_menu_item(Editor& editor, EditorActionId id) {
        const EditorAction& action = get_action(id);
        const std::string label = get_action_label(editor, id);
        const std::string shortcut = format_shortcut(action.chord);
        const bool enabled = is_action_enabled(editor, id);

        if (!menu_item(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), enabled)) {
            return false;
        }

        editor.request_action(id);

        return true;
    }

    bool action_bar_button(Editor& editor, EditorActionId id) {
        const std::string label = get_action_label(editor, id);
        if (!bar_button(label.c_str()) || !is_action_enabled(editor, id)) {
            return false;
        }

        editor.request_action(id);

        return true;
    }

    float action_bar_button_width(const Editor& editor, EditorActionId id) {
        return bar_button_width(get_action_label(editor, id).c_str());
    }
} // namespace hob::editor
