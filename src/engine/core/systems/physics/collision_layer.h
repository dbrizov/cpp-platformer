#pragma once

namespace hob {
    enum class CollisionLayer : uint64_t {
        None = 0,
        Default = 1 << 0,
    };
} // namespace hob
