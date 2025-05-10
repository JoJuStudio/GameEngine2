// source/core/Camera.cpp
#include "Camera.hpp"
#include "GameObject.hpp"
#include "Transform.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(GameObject* owner,
    float fov,
    float aspect,
    float near,
    float far)
    : Component(owner)
    , m_fov(fov)
    , m_aspect(aspect)
    , m_near(near)
    , m_far(far)
{
}

const glm::mat4& Camera::viewMatrix() const
{
    // inverse of the GameObject’s world transform
    glm::mat4 world = owner()->transform().worldMatrix();
    m_view = glm::inverse(world);
    return m_view;
}

const glm::mat4& Camera::projectionMatrix() const
{
    m_proj = glm::perspective(
        glm::radians(m_fov),
        m_aspect,
        m_near,
        m_far);
    return m_proj;
}
