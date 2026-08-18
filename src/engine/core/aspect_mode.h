#pragma once

#include <string_view>

#include "engine/math/vector2.h"

namespace hob {
    enum class AspectMode {
        keep_width,
        keep_height,
        expand,
        shrink,
    };

    bool aspect_mode_from_string(std::string_view str, AspectMode& out);

    Vector2 compute_logical_size(int32_t window_width,
                                 int32_t window_height,
                                 const Vector2& reference_size,
                                 AspectMode mode);
} // namespace hob
