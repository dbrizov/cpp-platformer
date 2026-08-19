#pragma once

#include <memory>
#include <string>
#include <vector>

#include "animation_track.h"

namespace hob {
    struct AnimationClip {
        std::string name;
        std::vector<AnimationTrackRef> tracks;
        float duration = 0.0f; // seconds
        bool looping = true;
    };

    using AnimationClipRef = std::shared_ptr<AnimationClip>;
} // namespace hob
