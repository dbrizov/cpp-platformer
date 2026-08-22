#pragma once

#include <limits>
#include <memory>

#include "engine/math/matrix2x3.h"
#include "engine/math/vector2.h"
#include "material.h"
#include "texture.h"

namespace hob {
    using SpriteDrawId = int64_t;
    constexpr SpriteDrawId INVALID_SPRITE_DRAW_ID = -1;

    using SpriteDrawIndex = uint32_t;
    constexpr SpriteDrawIndex INVALID_SPRITE_DRAW_INDEX = std::numeric_limits<SpriteDrawIndex>::max();

    struct SpriteDrawData {
        const Texture* texture = nullptr;
        const Material* material = nullptr;
        Matrix2x3 world_matrix = Matrix2x3::identity();
        Vector2 local_size; // meters (texture/ppm * sprite scale)
        Vector2 pivot = Vector2(0.5f, 0.5f); // pivot as a 0..1 fraction of the quad
        int32_t z_index = 0;

        const Shader* get_shader() const {
            return material != nullptr ? material->get_shader() : nullptr;
        }
    };
} // namespace hob
