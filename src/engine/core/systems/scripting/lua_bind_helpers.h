#pragma once

#include <limits>
#include <type_traits>
#include <utility>

#include <sol/sol.hpp>

#include "engine/core/logging.h"
#include "engine/core/systems/renderer/texture.h"

namespace hob {
    class Renderer;

    TextureRef resolve_texture(Renderer& renderer, const sol::object& value);

    // Lua integers are always int64_t. Binding a narrower C++ type directly lets sol2
    // static_cast out-of-range values into silent garbage; this clamps and reports
    // instead. `context` names the binding, e.g. "Timer.set_fps".
    template<typename T>
    T lua_narrow(int64_t value, const char* context) {
        static_assert(std::is_integral_v<T>, "lua_narrow expects an integral target");
        static_assert(std::in_range<int64_t>(std::numeric_limits<T>::max()),
                      "lua_narrow cannot represent the full range of T as a Lua integer");

        constexpr int64_t min = static_cast<int64_t>(std::numeric_limits<T>::min());
        constexpr int64_t max = static_cast<int64_t>(std::numeric_limits<T>::max());

        if (value < min) {
            log::lua.error("{}: {} is below the minimum {}; clamping", context, value, min);
            return std::numeric_limits<T>::min();
        }

        if (value > max) {
            log::lua.error("{}: {} is above the maximum {}; clamping", context, value, max);
            return std::numeric_limits<T>::max();
        }

        return static_cast<T>(value);
    }
} // namespace hob
