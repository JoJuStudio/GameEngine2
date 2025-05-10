// source/input/InputSystem.cpp
#include "InputSystem.hpp"

InputSystem::InputSystem()
{
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&m_pad);
}

void InputSystem::update()
{
    padUpdate(&m_pad);

    m_down = padGetButtonsDown(&m_pad);
    m_up = padGetButtonsUp(&m_pad);
    m_held = padGetButtons(&m_pad);

    HidAnalogStickState l = padGetStickPos(&m_pad, 0);
    HidAnalogStickState r = padGetStickPos(&m_pad, 1);

    constexpr float inv = 1.0f / 32767.0f;
    m_leftX = l.x * inv;
    m_leftY = l.y * inv;
    m_rightX = r.x * inv;
    m_rightY = r.y * inv;
}
