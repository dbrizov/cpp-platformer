#pragma once

#include <algorithm>
#include <string>

#include "vector2.h"

namespace hob {
    struct AABB {
        Vector2 center;
        Vector2 extents;

        constexpr AABB() = default;

        constexpr AABB(const Vector2& center_, const Vector2& extents_)
            : center(center_)
            , extents(extents_) {}

        std::string to_string() const;

        bool operator==(const AABB& right) const {
            return center == right.center && extents == right.extents;
        }

        bool operator!=(const AABB& right) const {
            return !operator==(right);
        }

        Vector2 min() const {
            return center - extents;
        }

        Vector2 max() const {
            return center + extents;
        }

        Vector2 size() const {
            return extents * 2.0f;
        }

        static AABB from_min_max(const Vector2& min, const Vector2& max) {
            return AABB((min + max) * 0.5f, (max - min) * 0.5f);
        }

        // The smallest box containing both.
        static AABB combine(const AABB& a, const AABB& b) {
            const Vector2 a_min = a.min();
            const Vector2 a_max = a.max();
            const Vector2 b_min = b.min();
            const Vector2 b_max = b.max();

            const Vector2 min = Vector2(std::min(a_min.x, b_min.x), std::min(a_min.y, b_min.y));
            const Vector2 max = Vector2(std::max(a_max.x, b_max.x), std::max(a_max.y, b_max.y));

            return from_min_max(min, max);
        }
    };
} // namespace hob
