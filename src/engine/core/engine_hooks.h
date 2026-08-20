#pragma once

#include <SDL3/SDL_video.h>

namespace hob {
    class EngineHooks {
    public:
        virtual ~EngineHooks() = default;

        virtual void init() {}
        virtual void tick(float delta_time) {}
        virtual void draw_gui() {}
        virtual void render_passes() {}
        virtual void on_lua_hot_reloaded() {}

        virtual bool on_quit_requested() {
            return false;
        }

        virtual bool on_window_close_requested(SDL_WindowID window_id) {
            return false;
        }
    };
} // namespace hob
