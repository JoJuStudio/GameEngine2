// source/core/Transform.hpp
#pragma once
#include "Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform : public Component {
public:
    explicit Transform(GameObject* owner)
        : Component(owner)
    {
    }

    ComponentTypeID type() const override { return componentTypeID<Transform>(); }

    glm::vec3 position { 0.f };
    glm::quat rotation {};
    glm::vec3 scale { 1.f };

    glm::mat4 worldMatrix() const
    {
        glm::mat4 m(1.f);
        m = glm::translate(m, position);
        m *= glm::mat4_cast(rotation);
        m = glm::scale(m, scale);
        return m;
    }
}; /* <-- keep this semicolon */
