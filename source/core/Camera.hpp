#pragma once
#include "Component.hpp"
#include <glm/glm.hpp>

class Camera : public Component {
public:
    // fov in degrees, aspect (w/h), near/far planes
    Camera(GameObject* owner,
        float fov = 60.f,
        float aspect = 1280.f / 720.f,
        float near = 0.1f,
        float far = 100.f);

    ComponentTypeID type() const override { return componentTypeID<Camera>(); }
    void update(float /*dt*/) override { } // no per-frame logic here

    const glm::mat4& viewMatrix() const;
    const glm::mat4& projectionMatrix() const;

    void setAspect(float aspect) { m_aspect = aspect; }

private:
    float m_fov, m_aspect, m_near, m_far;
    mutable glm::mat4 m_view;
    mutable glm::mat4 m_proj;
};
