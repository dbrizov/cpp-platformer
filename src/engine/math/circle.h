#pragma once

#include <string>

#include "mathf.h"
#include "vector2.h"

namespace hob {
    struct Circle {
        Vector2 center;
        float radius = 0.0f;

        constexpr Circle() = default;

        constexpr Circle(const Vector2& center_, float radius_)
            : center(center_)
            , radius(radius_) {}

        std::string to_string() const;

        bool operator==(const Circle& right) const {
            return center == right.center && math::approx_equal(radius, right.radius);
        }

        bool operator!=(const Circle& right) const {
            return !operator==(right);
        }
    };
} // namespace hob
