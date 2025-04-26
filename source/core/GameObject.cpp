#include "GameObject.hpp"
#include "Transform.hpp"
#include <algorithm>

GameObject::GameObject(std::string_view name)
    : m_name(name)
{
    // Every GO gets a Transform
    AddComponent<Transform>(this);
}

GameObject::~GameObject() = default;

GameObject& GameObject::CreateChild(std::string_view name)
{
    auto& child = m_children.emplace_back(std::make_unique<GameObject>(name));
    child->m_parent = this;
    return *child;
}

void GameObject::Update(float dt)
{
    // Update components
    for (auto& c : m_components)
        c->Update(dt);

    // Update children
    for (auto& ch : m_children)
        ch->Update(dt);
}

// --- templates ---
template <class T, class... Args>
T& GameObject::AddComponent(Args&&... args)
{
    auto& ptr = m_components.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T&>(*ptr);
}

template <class T>
T* GameObject::GetComponent() const
{
    for (auto& c : m_components)
        if (auto cast = dynamic_cast<T*>(c.get()))
            return cast;
    return nullptr;
}

// Explicit instantiations to avoid link errors
template Transform& GameObject::AddComponent<Transform, GameObject*>(GameObject*&&);
template Transform* GameObject::GetComponent<Transform>() const;

Transform& GameObject::TransformComponent() { return *GetComponent<Transform>(); }
