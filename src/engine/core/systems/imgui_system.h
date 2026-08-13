#pragma once

#include <SDL3/SDL_events.h>

struct ImGuiContext;

namespace hob {
    class Renderer;

    class ImGuiSystem {
        static constexpr SDL_FColor CLEAR_COLOR{0.06f, 0.06f, 0.08f, 1.0f};
        static constexpr float DEFAULT_FONT_SIZE_PX = 20.0f;

        ImGuiContext* m_context = nullptr;
        const Renderer& m_renderer;

    public:
        explicit ImGuiSystem(const Renderer& renderer);
        ~ImGuiSystem();

        ImGuiSystem(const ImGuiSystem&) = delete;
        ImGuiSystem& operator=(const ImGuiSystem&) = delete;

        ImGuiSystem(ImGuiSystem&&) = delete;
        ImGuiSystem& operator=(ImGuiSystem&&) = delete;

        void process_event(const SDL_Event& event);

        void new_frame();
        void render_pass();
        void discard_frame();
    };
} // namespace hob
