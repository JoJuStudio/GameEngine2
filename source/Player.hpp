// source/Player.hpp
#pragma once

#include "core/Component.hpp"
#include "input/InputSystem.hpp"
#include <glm/glm.hpp>

class GameObject;

class Player : public Component {
public:
    Player(GameObject* owner, InputSystem* input,
        float moveSpeed = 5.f, float lookSpeed = 90.f);

    ComponentTypeID type() const override
    {
        return componentTypeID<Player>();
    }

    void update(float dt) override;

private:
    InputSystem* m_input;
    float m_moveSpeed;
    float m_lookSpeed;
    float m_yaw;
    float m_pitch;
};
