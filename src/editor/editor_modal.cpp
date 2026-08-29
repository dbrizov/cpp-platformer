#include "editor_modal.h"

#include <utility>

#include <imgui.h>
#include <imgui_internal.h>

#include "editor_gui_utils.h"
#include "editor_style.h"

namespace hob::editor {
    namespace {
        enum class EditorModalChoice : uint8_t {
            None,
            Confirm,
            Discard,
            Cancel,
        };

        bool begin_modal(const char* id) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSizeConstraints(ImVec2(MODAL_MIN_WIDTH, 0.0f), ImVec2(FLT_MAX, FLT_MAX));

            EditorStyleColorStack colors;
            colors.push(ImGuiCol_PopupBg, COLOR_MODAL_BG);
            colors.push(ImGuiCol_Border, COLOR_MODAL_BORDER);

            EditorStyleVarStack vars;
            vars.push(ImGuiStyleVar_WindowPadding, MODAL_PADDING);
            vars.push(ImGuiStyleVar_WindowBorderSize, MODAL_BORDER_SIZE);

            const bool open = ImGui::BeginPopupModal(
                id, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

            vars.pop();
            colors.pop();

            return open;
        }

        void end_modal() {
            ImGui::EndPopup();
        }

        void draw_message(const char* message, const std::optional<std::string>& reason) {
            ImGui::TextUnformatted(message);

            if (reason.has_value()) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, COLOR_MODAL_REASON);
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + MODAL_MIN_WIDTH);
                ImGui::TextUnformatted(reason->c_str());
                ImGui::PopTextWrapPos();
                ImGui::PopStyleColor();
            }

            ImGui::Dummy(ImVec2(0.0f, MODAL_MESSAGE_SPACING));
        }

        EditorModalChoice draw_button_row(const EditorModalButtons& buttons) {
            float width = MODAL_BUTTON_MIN_WIDTH;
            for (const std::string* label : {&buttons.confirm, &buttons.discard, &buttons.cancel}) {
                if (!label->empty()) {
                    width = ImMax(width, ImGui::CalcTextSize(label->c_str()).x + FRAME_PADDING.x * 2.0f);
                }
            }

            const ImVec2 button_size(width, 0.0f);
            EditorModalChoice choice = EditorModalChoice::None;
            bool is_first = true;

            const auto draw_button = [&](const std::string& label, EditorModalChoice value, bool is_enabled) {
                if (label.empty()) {
                    return;
                }

                if (!is_first) {
                    ImGui::SameLine();
                }
                is_first = false;

                ImGui::BeginDisabled(!is_enabled);
                if (ImGui::Button(label.c_str(), button_size)) {
                    choice = value;
                }
                ImGui::EndDisabled();
            };

            draw_button(buttons.confirm, EditorModalChoice::Confirm, buttons.is_confirm_enabled);
            draw_button(buttons.discard, EditorModalChoice::Discard, true);
            draw_button(buttons.cancel, EditorModalChoice::Cancel, true);

            if (!buttons.cancel.empty() && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                choice = EditorModalChoice::Cancel;
            }

            if (choice != EditorModalChoice::None) {
                ImGui::CloseCurrentPopup();
            }

            return choice;
        }

        const std::function<void()>& get_handler(const EditorModalConfig& config, EditorModalChoice choice) {
            static const std::function<void()> NONE;

            switch (choice) {
                case EditorModalChoice::Confirm:
                    return config.on_confirm;
                case EditorModalChoice::Discard:
                    return config.on_discard;
                case EditorModalChoice::Cancel:
                    return config.on_cancel;
                case EditorModalChoice::None:
                    break;
            }

            return NONE;
        }
    } // namespace

    void EditorModal::open(EditorModalConfig config) {
        m_config = std::move(config);
    }

    bool EditorModal::is_open() const {
        return m_config.has_value();
    }

    void EditorModal::draw() {
        if (!m_config.has_value()) {
            return;
        }

        if (!ImGui::IsPopupOpen(m_config->title.c_str())) {
            ImGui::OpenPopup(m_config->title.c_str());
        }

        if (!begin_modal(m_config->title.c_str())) {
            return;
        }

        draw_message(m_config->message.c_str(), m_config->reason);
        const EditorModalChoice choice = draw_button_row(m_config->buttons);

        end_modal();

        if (choice == EditorModalChoice::None) {
            return;
        }

        // Taken by value and cleared before running, so a handler is free to open the next modal.
        const EditorModalConfig config = std::move(*m_config);
        m_config.reset();

        if (const std::function<void()>& handler = get_handler(config, choice)) {
            handler();
        }
    }
} // namespace hob::editor
