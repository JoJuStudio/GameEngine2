// source/components/AnimationClip.hpp

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct BoneTransform {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

class AnimationClip {
public:
    struct Keyframe {
        float time;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    void AddKeyframe(const std::string& boneName, const Keyframe& keyframe);
    const std::unordered_map<std::string, std::vector<Keyframe>>& GetTracks() const;

    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

private:
    std::string m_name;
    std::unordered_map<std::string, std::vector<Keyframe>> m_tracks;
};
