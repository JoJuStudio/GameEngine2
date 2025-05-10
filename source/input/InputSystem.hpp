// source/input/InputSystem.hpp
#pragma once
#include <switch.h>

/* One-pad façade built on libnx high-level PAD API */
class InputSystem {
public:
    InputSystem(); // ctor sets up libnx

    void update(); // call once per frame

    /* --- button state bit-fields --- */
    u64 keysDown() const { return m_down; } // newly pressed this frame
    u64 keysHeld() const { return m_held; } // currently held
    u64 keysUp() const { return m_up; } // newly released

    /* --- stick axes (−1 … 1) --- */
    float leftX() const { return m_leftX; }
    float leftY() const { return m_leftY; }
    float rightX() const { return m_rightX; }
    float rightY() const { return m_rightY; }

private:
    PadState m_pad {};

    u64 m_down { 0 }, m_held { 0 }, m_up { 0 };
    float m_leftX { 0 }, m_leftY { 0 },
        m_rightX { 0 }, m_rightY { 0 };
};
