#pragma once

#include <SDL3/SDL_events.h>

struct ImGuiContext;
struct ImVec4;

namespace hob {
    class Renderer;

    class ImGuiSystem {
        ImGuiContext* m_context = nullptr;
        const Renderer& m_renderer;
        bool m_is_editor_enabled;

        SDL_FColor m_clear_color{0.0f, 0.0f, 0.0f, 1.0f};

    public:
        explicit ImGuiSystem(const Renderer& renderer, bool is_editor_enabled);
        ~ImGuiSystem();

        ImGuiSystem(const ImGuiSystem&) = delete;
        ImGuiSystem& operator=(const ImGuiSystem&) = delete;

        ImGuiSystem(ImGuiSystem&&) = delete;
        ImGuiSystem& operator=(ImGuiSystem&&) = delete;

        void set_clear_color(const ImVec4& color);

        void process_event(const SDL_Event& event);

        void new_frame();
        void render_pass();
        void discard_frame();
    };
} // namespace hob
