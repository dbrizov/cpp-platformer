#pragma once

#include <cmath>

#include "constants.h"

namespace hob::math {
    // Returns [0, 360).
    inline float normalize_angle_deg(float angle_deg) {
        angle_deg = std::fmod(angle_deg, 360.0f);
        if (angle_deg < 0) {
            angle_deg += 360.0f;
        }

        return angle_deg;
    }

    // Returns [-PI, PI].
    inline float wrap_angle_rad(float angle_rad) {
        return std::remainder(angle_rad, PI * 2.0f);
    }

    inline float lerp(float a, float b, float t) {
        return a * (1.0f - t) + b * t;
    }

    inline float lerp_angle(float a_deg, float b_deg, float t) {
        const float diff = std::remainder(b_deg - a_deg, 360.0f); // in range [-180, 180]
        const float b_wrapped = a_deg + diff;
        const float result = a_deg * (1.0f - t) + b_wrapped * t;
        const float normalized = normalize_angle_deg(result);

        return normalized;
    }
} // namespace hob::math
