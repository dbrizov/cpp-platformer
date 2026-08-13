#pragma once

#include <SDL3/SDL_gpu.h>

namespace hob {
    class SdlContext {
        SDL_GPUDevice* m_gpu_device = nullptr;

    public:
        SdlContext();
        ~SdlContext();

        SdlContext(const SdlContext&) = delete;
        SdlContext& operator=(const SdlContext&) = delete;

        SdlContext(SdlContext&&) = delete;
        SdlContext& operator=(SdlContext&&) = delete;

        SDL_GPUDevice* get_gpu_device() const;
    };
} // namespace hob
