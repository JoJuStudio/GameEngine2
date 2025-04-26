#pragma once
#include "Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform : public Component {
public:
    explicit Transform(GameObject* owner);

    glm::vec3 position {};
    glm::quat rotation {};
    glm::vec3 scale { 1.f, 1.f, 1.f };

    /** Returns world matrix, recursively combining parents. */
    glm::mat4 WorldMatrix() const;

private:
    mutable bool m_dirty { true };
    mutable glm::mat4 m_cached {};
};
