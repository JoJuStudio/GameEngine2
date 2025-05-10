// source/components/AnimationClip.cpp

#include "AnimationClip.hpp"
#include <glm/gtx/quaternion.hpp> // <-- contains toMat4()

void AnimationClip::AddKeyframe(const std::string& boneName, const Keyframe& keyframe)
{
    m_tracks[boneName].push_back(keyframe);
}

const std::unordered_map<std::string, std::vector<AnimationClip::Keyframe>>& AnimationClip::GetTracks() const
{
    return m_tracks;
}
