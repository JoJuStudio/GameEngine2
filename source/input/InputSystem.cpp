#include "InputSystem.hpp"

void InputSystem::Pump()
{
    hidScanInput();
    m_down = hidKeysDown(m_id);
    m_up = hidKeysUp(m_id);
    m_held = hidKeysHeld(m_id);

    HidAnalogStickState l = hidAnalogStickRead(m_id, CONTROLLER_ANALOG_STICK_LEFT);
    HidAnalogStickState r = hidAnalogStickRead(m_id, CONTROLLER_ANALOG_STICK_RIGHT);

    constexpr float inv = 1.0f / 32767.0f;
    m_lx = l.x * inv;
    m_ly = l.y * inv;
    m_rx = r.x * inv;
    m_ry = r.y * inv;
}

bool InputSystem::IsPressed(u64 key) const { return m_held & key; }
bool InputSystem::WasJustPressed(u64 key) const { return m_down & key; }
bool InputSystem::WasJustReleased(u64 key) const { return m_up & key; }
