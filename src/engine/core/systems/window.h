#pragma once

#include <string>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "engine/math/vector2.h"

namespace hob {
    struct WindowConfig {
        std::string title = "Hob2D";
        std::string icon = "icons/hob_icon.ico";
        int32_t width = 1152;
        int32_t height = 648;
        int32_t x = SDL_WINDOWPOS_CENTERED;
        int32_t y = SDL_WINDOWPOS_CENTERED;
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
        void get_size(int32_t& width, int32_t& height) const;
        void get_size_px(int32_t& width, int32_t& height) const;
        void get_position(int32_t& x, int32_t& y) const;
        float get_pixel_density() const;

        bool has_focus() const;
        bool is_maximized() const;

    private:
        void set_icon(const std::string& relative_path);
    };
} // namespace hob
