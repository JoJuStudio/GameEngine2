#pragma once
#include <cstddef>

class GameObject;

/* ------------- lightweight type-ID system (no RTTI) ------------------ */
using ComponentTypeID = std::size_t;

inline ComponentTypeID newComponentTypeID() noexcept
{
    static ComponentTypeID last = 0;
    return last++;
}

template <class T>
inline ComponentTypeID componentTypeID() noexcept
{
    static ComponentTypeID id = newComponentTypeID();
    return id;
}

/* --------------------------- base class -------------------------------- */
class Component {
public:
    explicit Component(GameObject* owner)
        : m_owner(owner)
    {
    }
    virtual ~Component() = default;

    virtual ComponentTypeID type() const = 0;
    virtual void update(float /*dt*/) { } // optional per-frame hook

protected:
    GameObject* owner() const { return m_owner; }

private:
    GameObject* m_owner;
};
