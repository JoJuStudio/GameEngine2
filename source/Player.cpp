// source/Player.cpp
#include "Player.hpp"
#include "core/GameObject.hpp"
#include "core/Transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <switch.h>

Player::Player(GameObject* owner, InputSystem* input,
    float moveSpeed, float lookSpeed)
    : Component(owner)
    , m_input(input)
    , m_moveSpeed(moveSpeed)
    , m_lookSpeed(lookSpeed)
    , m_yaw(0.f)
    , m_pitch(0.f)
{
}

void Player::update(float dt)
{
    auto& t = owner()->transform();

    // Raw stick input (forward/backward is now inverted by removing the Y inversion)
    float rawX = m_input->leftX();
    float rawY = m_input->leftY(); // Removed the Y inversion here

    // Build a yaw-only quaternion (ignore pitch)
    glm::quat qYaw = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0, 1, 0));

    // Local forward/right in XZ-plane
    glm::vec3 forwardXZ = qYaw * glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 rightXZ = qYaw * glm::vec3(1.f, 0.f, 0.f);

    // Horizontal move uses only XZ axes
    glm::vec3 horiz = forwardXZ * rawY + rightXZ * rawX;
    t.position += horiz * m_moveSpeed * dt;

    // Vertical (world-space) flying with A/B
    u64 held = m_input->keysHeld();
    if (held & HidNpadButton_A)
        t.position.y += m_moveSpeed * dt;
    if (held & HidNpadButton_B)
        t.position.y -= m_moveSpeed * dt;

    // Look: keep X inversion but remove Y inversion (so pushing up looks down)
    float rx = -m_input->rightX(); // Keep X inversion for standard look
    float ry = m_input->rightY(); // Removed Y inversion here

    m_yaw += rx * m_lookSpeed * dt;
    m_pitch += ry * m_lookSpeed * dt;
    m_pitch = glm::clamp(m_pitch, -89.f, 89.f);

    // Apply yaw then pitch (pitch around local right)
    glm::quat qPitch = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1, 0, 0));
    t.rotation = qYaw * qPitch;
}
