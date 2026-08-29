#pragma once

#include <string>

#include "mathf.h"

namespace hob {
    struct Color {
        float r;
        float g;
        float b;
        float a;

        constexpr Color()
            : Color(0.0f, 0.0f, 0.0f, 0.0f) {}

        constexpr Color(float r_, float g_, float b_, float a_ = 1.0f)
            : r(r_)
            , g(g_)
            , b(b_)
            , a(a_) {}

        std::string to_string() const;

        bool operator==(const Color& right) const {
            return math::approx_equal(r, right.r) && math::approx_equal(g, right.g) && math::approx_equal(b, right.b) &&
                   math::approx_equal(a, right.a);
        }

        bool operator!=(const Color& right) const {
            return !operator==(right);
        }

        // clang-format off
        static constexpr Color black() { return Color(0.0f, 0.0f, 0.0f); }
        static constexpr Color white() { return Color(1.0f, 1.0f, 1.0f); }
        static constexpr Color gray() { return Color(0.5f, 0.5f, 0.5f); }
        static constexpr Color red() { return Color(1.0f, 0.0f, 0.0f); }
        static constexpr Color green() { return Color(0.0f, 1.0f, 0.0f); }
        static constexpr Color blue() { return Color(0.0f, 0.0f, 1.0f); }
        static constexpr Color yellow() { return Color(1.0f, 1.0f, 0.0f); }
        static constexpr Color magenta() { return Color(1.0f, 0.0f, 1.0f); }
        static constexpr Color cyan() { return Color(0.0f, 1.0f, 1.0f); }
        static constexpr Color orange() { return Color(1.0f, 0.647f, 0.0f); }
        // clang-format on
    };
} // namespace hob
