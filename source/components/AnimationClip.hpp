#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

class Transform;

class AnimationClip {
public:
    struct Keyframe {
        float time = 0.0f;
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };

    void ApplyTo(Transform& transform, float currentTime) const; // <--- must be `const` here!
    void AddKeyframe(const Keyframe& keyframe) { m_keyframes.push_back(keyframe); }

    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

private:
    std::vector<Keyframe> m_keyframes;
    std::string m_name;
};
