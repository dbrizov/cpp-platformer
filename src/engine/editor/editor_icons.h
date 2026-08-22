#pragma once

#include <imgui.h>

#include "engine/core/systems/renderer/texture.h"

namespace hob {
    class Renderer;
} // namespace hob

namespace hob::editor {
    enum class EditorBarIcon : uint8_t {
        Play,
        Pause,
        Step,
        Stop,
        TranslateRotate,
        Translate,
        Rotate,
        Scale,
        SpaceWorld,
        SpaceLocal,
        Count,
    };

    class EditorIcons {
        TextureRef m_atlas;

    public:
        void load(Renderer& renderer);

        bool is_loaded() const;

        SDL_GPUTexture* get_texture() const;
        ImVec2 get_uv_min(EditorBarIcon icon) const;
        ImVec2 get_uv_max(EditorBarIcon icon) const;
    };
} // namespace hob::editor
