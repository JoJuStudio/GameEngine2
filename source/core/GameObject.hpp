#pragma once
#include "Component.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Transform;

class GameObject {
public:
    explicit GameObject(std::string_view name = "GameObject");
    ~GameObject();

    GameObject& createChild(std::string_view name = "Child");
    GameObject* parent() const { return m_parent; }
    std::vector<std::unique_ptr<GameObject>>& children() { return m_children; }

    template <class T, class... Args>
    T& addComponent(Args&&... args);
    template <class T>
    T* getComponent() const;
    Transform& transform();

    void update(float dt);

private:
    std::string m_name;
    GameObject* m_parent { nullptr };
    std::vector<std::unique_ptr<Component>> m_components;
    std::vector<std::unique_ptr<GameObject>> m_children;
};

/* -------- template implementations (header-only) --------------------- */
template <class T, class... Args>
T& GameObject::addComponent(Args&&... args)
{
    auto& uptr = m_components.emplace_back(
        std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T&>(*uptr);
}

template <class T>
T* GameObject::getComponent() const
{
    ComponentTypeID id = componentTypeID<T>();
    for (auto& c : m_components)
        if (c->type() == id)
            return static_cast<T*>(c.get());
    return nullptr;
}
