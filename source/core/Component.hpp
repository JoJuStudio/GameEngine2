#pragma once
#include <cstdint>

class GameObject;

/** Base class for all attachable behaviours. */
class Component {
public:
    explicit Component(GameObject* owner)
        : m_owner(owner)
    {
    }
    virtual ~Component() = default;

    /** Per-frame logic. */
    virtual void Update(float dt) { }

protected:
    GameObject* Owner() const { return m_owner; }

private:
    GameObject* m_owner;
};
