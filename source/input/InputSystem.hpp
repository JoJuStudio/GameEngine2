#pragma once
#include <switch/services/hid.h>
#include <array>
#include <switch.h>

class InputSystem {
public:
    enum { MAX_PADS = 1 }; // expand if you want multi-Joy-Con

    /** Call once per frame. */
    void Pump();

    /** Button query helpers. */
    bool IsPressed(u64 key) const;
    bool WasJustPressed(u64 key) const;
    bool WasJustReleased(u64 key) const;

    /* Stick axes: –1.0 … 1.0  */
    float LeftX() const { return m_lx; }
    float LeftY() const { return m_ly; }
    float RightX() const { return m_rx; }
    float RightY() const { return m_ry; }

private:
    HidControllerID m_id { CONTROLLER_P1_AUTO };

    u64 m_down { 0 };
    u64 m_held { 0 };
    u64 m_up { 0 };

    float m_lx { 0 }, m_ly { 0 }, m_rx { 0 }, m_ry { 0 };
};
