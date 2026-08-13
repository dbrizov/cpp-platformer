#pragma once

#include <string>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "engine/math/vector2.h"

namespace hob {
    struct WindowConfig {
        std::string title = "Hob2D";
        int width = 1152;
        int height = 648;
        int x = SDL_WINDOWPOS_CENTERED;
        int y = SDL_WINDOWPOS_CENTERED;
        bool maximized = false;
        bool vsync = true;
    };

    class Window {
        SDL_GPUDevice* m_gpu_device = nullptr;
        SDL_Window* m_window = nullptr;

    public:
        Window(SDL_GPUDevice* gpu_device, const WindowConfig& config);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        SDL_Window* get_window() const;
        SDL_WindowID get_id() const;

        Vector2 get_size() const;
        void get_size_px(int& width, int& height) const;
        float get_pixel_density() const;

        bool has_focus() const;
    };
} // namespace hob
