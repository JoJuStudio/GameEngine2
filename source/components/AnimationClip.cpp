// source/components/AnimationClip.cpp

#include "AnimationClip.hpp"
#include <algorithm>
#include <glm/gtx/quaternion.hpp>

void AnimationClip::AddKeyframe(const std::string& boneName, const Keyframe& keyframe)
{
    auto& track = m_tracks[boneName];
    track.push_back(keyframe);

    // Sort keyframes by time after insertion
    std::sort(track.begin(), track.end(),
        [](const Keyframe& a, const Keyframe& b) {
            return a.time < b.time;
        });
}

const std::unordered_map<std::string, std::vector<AnimationClip::Keyframe>>& AnimationClip::GetTracks() const
{
    return m_tracks;
}
