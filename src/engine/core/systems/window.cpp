#include "window.h"

#include <iterator>
#include <memory>
#include <vector>

#include <SDL3/SDL.h>
#include <lunasvg.h>

#include "engine/core/assert.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"

namespace hob {
    namespace {
        constexpr int32_t ICON_SIZES[] = {256, 128, 64, 48, 32, 16};

        void apply_window_icon(SDL_Window* window, const std::string& relative_path) {
            if (relative_path.empty()) {
                return;
            }

            const std::filesystem::path path = PathUtils::resolve_asset_path(relative_path);

            std::unique_ptr<lunasvg::Document> document = lunasvg::Document::loadFromFile(path.string());
            if (!document) {
                log::sdl.error("Window icon failed to load: '{}'", path.string());
                return;
            }

            // SDL_CreateSurfaceFrom wraps the pixels instead of copying them, so every bitmap has to
            // stay alive until SDL_SetWindowIcon has taken its own copy.
            std::vector<lunasvg::Bitmap> bitmaps;
            std::vector<SDL_Surface*> surfaces;
            bitmaps.reserve(std::size(ICON_SIZES));
            surfaces.reserve(std::size(ICON_SIZES));

            for (const int32_t size : ICON_SIZES) {
                lunasvg::Bitmap bitmap = document->renderToBitmap(size, size);
                if (bitmap.isNull()) {
                    continue;
                }

                bitmap.convertToRGBA();
                bitmaps.push_back(std::move(bitmap));

                lunasvg::Bitmap& stored = bitmaps.back();
                SDL_Surface* surface = SDL_CreateSurfaceFrom(
                    size, size, SDL_PIXELFORMAT_RGBA32, stored.data(), static_cast<int32_t>(stored.stride()));

                if (surface) {
                    surfaces.push_back(surface);
                }
            }

            if (surfaces.empty()) {
                log::sdl.error("Window icon failed to rasterise: '{}'", path.string());
                return;
            }

            for (int32_t index = 1; index < surfaces.size(); ++index) {
                SDL_AddSurfaceAlternateImage(surfaces[0], surfaces[index]);
            }

            if (!SDL_SetWindowIcon(window, surfaces[0])) {
                log::sdl.error("Window icon SDL_SetWindowIcon failed: {}", SDL_GetError());
            }

            for (SDL_Surface* surface : surfaces) {
                SDL_DestroySurface(surface);
            }
        }
    } // namespace

    Window::Window(SDL_GPUDevice* gpu_device, const WindowConfig& config)
        : m_gpu_device(gpu_device) {
        HOB_CHECK(m_gpu_device, "Window init failed: GPU device is null");

        // Created hidden so the window can be placed before it is maximized - a maximized window
        // cannot be moved, so maximizing first would pin it to whichever display SDL picked.
        const SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;

        m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, window_flags);
        HOB_CHECK(m_window, "Window SDL_CreateWindow failed: {}", SDL_GetError());

        apply_window_icon(m_window, config.icon);

        if (config.x != SDL_WINDOWPOS_UNDEFINED && config.y != SDL_WINDOWPOS_UNDEFINED) {
            SDL_SetWindowPosition(m_window, config.x, config.y);
        }

        if (config.maximized) {
            SDL_MaximizeWindow(m_window);
        }

        SDL_ShowWindow(m_window);

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

    void Window::get_size(int32_t& width, int32_t& height) const {
        SDL_GetWindowSize(m_window, &width, &height);
    }

    void Window::get_size_px(int32_t& width, int32_t& height) const {
        SDL_GetWindowSizeInPixels(m_window, &width, &height);
    }

    void Window::get_position(int32_t& x, int32_t& y) const {
        SDL_GetWindowPosition(m_window, &x, &y);
    }

    float Window::get_pixel_density() const {
        const float density = SDL_GetWindowPixelDensity(m_window);
        return (density > 0.0f) ? density : 1.0f;
    }

    bool Window::has_focus() const {
        return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
    }

    bool Window::is_maximized() const {
        return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MAXIMIZED) != 0;
    }
} // namespace hob
