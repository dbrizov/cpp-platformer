#pragma once

#include <cmath>

#include "vector2.h"

namespace hob {
    struct Matrix2x3 {
        Vector2 basis_x;
        Vector2 basis_y;
        Vector2 origin;

        float determinant() const {
            return basis_x.x * basis_y.y - basis_y.x * basis_x.y;
        }

        bool is_reflected() const {
            return determinant() < 0.0f;
        }

        float get_rotation() const {
            return is_reflected() ? std::atan2(-basis_y.x, basis_y.y) : std::atan2(basis_x.y, basis_x.x);
        }

        Vector2 get_scale() const {
            const float sign_x = is_reflected() ? -1.0f : 1.0f;
            return Vector2(sign_x * basis_x.length(), basis_y.length());
        }

        Vector2 transform_point(const Vector2& p) const {
            return origin + basis_x * p.x + basis_y * p.y;
        }

        Matrix2x3 inverse() const {
            const float det = determinant();
            if (std::abs(det) <= EPSILON) {
                return Matrix2x3::identity();
            }

            const float inv_det = 1.0f / det;
            Matrix2x3 out;
            out.basis_x = Vector2(basis_y.y * inv_det, -basis_x.y * inv_det);
            out.basis_y = Vector2(-basis_y.x * inv_det, basis_x.x * inv_det);
            out.origin = -(out.basis_x * origin.x + out.basis_y * origin.y);
            return out;
        }

        static Matrix2x3 identity() {
            Matrix2x3 out;
            out.basis_x = Vector2::right();
            out.basis_y = Vector2::up();
            out.origin = Vector2::zero();
            return out;
        }

        static Matrix2x3 multiply(const Matrix2x3& a, const Matrix2x3& b) {
            Matrix2x3 out;
            out.basis_x = a.basis_x * b.basis_x.x + a.basis_y * b.basis_x.y;
            out.basis_y = a.basis_x * b.basis_y.x + a.basis_y * b.basis_y.y;
            out.origin = a.origin + a.basis_x * b.origin.x + a.basis_y * b.origin.y;
            return out;
        }

        static Matrix2x3 lerp(const Matrix2x3& a, const Matrix2x3& b, float t) {
            Matrix2x3 out;
            out.basis_x = a.basis_x * (1.0f - t) + b.basis_x * t;
            out.basis_y = a.basis_y * (1.0f - t) + b.basis_y * t;
            out.origin = a.origin * (1.0f - t) + b.origin * t;
            return out;
        }

        static Matrix2x3 make_rotate_around(const Vector2& pivot, float radians) {
            const float cos = std::cos(radians);
            const float sin = std::sin(radians);

            Matrix2x3 out;
            out.basis_x = Vector2(cos, sin);
            out.basis_y = Vector2(-sin, cos);
            out.origin = pivot - (out.basis_x * pivot.x + out.basis_y * pivot.y);
            return out;
        }
    };

    inline Matrix2x3 operator*(const Matrix2x3& a, const Matrix2x3& b) {
        return Matrix2x3::multiply(a, b);
    }
} // namespace hob
