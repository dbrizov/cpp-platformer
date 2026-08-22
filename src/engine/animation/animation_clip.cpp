#include "animation_clip.h"

#include <utility>

namespace hob {
    const std::vector<AnimationTrackRef>& AnimationClip::get_tracks() const {
        return m_tracks;
    }

    void AnimationClip::add_track(AnimationTrackRef track) {
        m_tracks.push_back(std::move(track));
    }

    float AnimationClip::get_duration() const {
        return m_duration;
    }

    void AnimationClip::set_duration(float duration) {
        m_duration = duration;
    }

    bool AnimationClip::get_looping() const {
        return m_looping;
    }

    void AnimationClip::set_looping(bool looping) {
        m_looping = looping;
    }
} // namespace hob
