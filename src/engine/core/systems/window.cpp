#include "window.h"

#include <SDL3/SDL.h>

#include "engine/core/assert.h"
#include "engine/core/logging.h"

namespace hob {
    Window::Window(SDL_GPUDevice* gpu_device, const WindowConfig& config)
        : m_gpu_device(gpu_device) {
        HOB_CHECK(m_gpu_device, "Window init failed: GPU device is null");

        SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (config.maximized) {
            window_flags |= SDL_WINDOW_MAXIMIZED;
        }

        m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, window_flags);
        HOB_CHECK(m_window, "Window SDL_CreateWindow failed: {}", SDL_GetError());

        if (config.x != SDL_WINDOWPOS_UNDEFINED && config.y != SDL_WINDOWPOS_UNDEFINED) {
            SDL_SetWindowPosition(m_window, config.x, config.y);
        }

        const bool window_claimed = SDL_ClaimWindowForGPUDevice(m_gpu_device, m_window);
        HOB_CHECK(window_claimed, "Window SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());

        SDL_GPUPresentMode present_mode = SDL_GPU_PRESENTMODE_VSYNC;
        if (!config.vsync && SDL_WindowSupportsGPUPresentMode(m_gpu_device, m_window, SDL_GPU_PRESENTMODE_MAILBOX)) {
            present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
        }

        SDL_SetGPUSwapchainParameters(m_gpu_device, m_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, present_mode);

        int32_t pixel_width = 0;
        int32_t pixel_height = 0;
        SDL_GetWindowSizeInPixels(m_window, &pixel_width, &pixel_height);
        log::sdl.info("Window created: '{}' ({}x{} pts, {}x{} px, density {})",
                      config.title,
                      config.width,
                      config.height,
                      pixel_width,
                      pixel_height,
                      get_pixel_density());
    }

    Window::~Window() {
        const std::string title = m_window ? SDL_GetWindowTitle(m_window) : "";

        if (m_gpu_device && m_window) {
            SDL_ReleaseWindowFromGPUDevice(m_gpu_device, m_window);
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        log::sdl.info("Window destroyed: '{}'", title);
    }

    SDL_Window* Window::get_window() const {
        return m_window;
    }

    SDL_WindowID Window::get_id() const {
        return SDL_GetWindowID(m_window);
    }

    Vector2 Window::get_size() const {
        int32_t width = 0;
        int32_t height = 0;
        SDL_GetWindowSize(m_window, &width, &height);
        return Vector2(static_cast<float>(width), static_cast<float>(height));
    }

    void Window::get_size_px(int32_t& width, int32_t& height) const {
        SDL_GetWindowSizeInPixels(m_window, &width, &height);
    }

    float Window::get_pixel_density() const {
        const float density = SDL_GetWindowPixelDensity(m_window);
        return (density > 0.0f) ? density : 1.0f;
    }

    bool Window::has_focus() const {
        return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
    }
} // namespace hob
