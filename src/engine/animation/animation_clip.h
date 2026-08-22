#pragma once

#include <memory>
#include <vector>

#include "animation_track.h"
#include "engine/core/asset.h"

namespace hob {
    class AnimationClip;
    using AnimationClipRef = std::shared_ptr<AnimationClip>;
    using AnimationClipWeakRef = std::weak_ptr<AnimationClip>;

    class AnimationClip : public Asset {
        std::vector<AnimationTrackRef> m_tracks;
        float m_duration = 0.0f; // seconds
        bool m_looping = true;

    public:
        const std::vector<AnimationTrackRef>& get_tracks() const;
        void add_track(AnimationTrackRef track);

        float get_duration() const;
        void set_duration(float duration);

        bool get_looping() const;
        void set_looping(bool looping);
    };
} // namespace hob
